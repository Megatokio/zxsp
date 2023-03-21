// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "BufferedFile.h"
#include "File.h"
#include "Templates/BitArray.h"
#include "Templates/NVPtr.h"
#include <condition_variable>
#include <mutex>
#include <thread>


/*	open file:
		application or worker thread. 
		set state = Opened when done.
	
	read, write buffers <-> file:
		worker thread.
		
	sync, close, dispose file:
		worker thread.
		
	store buffer into buffers:
		worker thread.
		sometimes (writing whole blocks) application thread too.
		
	remove buffer from buffers:
		worker thread.

	a buffer is locked (won't be removed by worker), 
		if it is one of the min_buffers starting at fpos.
		
	when adding or removing buffers the mutex must be locked.
	the application must only access the min_buffers starting at fpos.
*/


enum What { Open = 1, Read = 2, Sync = 4, Close = 8, Dispose = 16 };
enum State { Initial, Opened, Closed };

template<typename T>
T min(T a, T b, T c)
{
	return min(a, min(b, c));
}


struct BufferedFile::Buffer
{
	bool dirty = false;
	char padding[3];
	int	 access; // last access TODO
	char data[buffer_size];
};

struct BufferedFile::Buffers : public Array<BufferedFile::Buffer*>
{
	~Buffers()
	{
		while (count()) delete pop();
	}
};

class BufferedFile::SharedData
{
	NO_COPY_MOVE(SharedData);

	mutable std::mutex		mutex;
	std::condition_variable cond;
	int						access_sn = 0; //TODO
	int16					what	  = 0;
	int16					state	  = 0;
	Buffers					buffers;

	std::unique_ptr<File>			file;
	std::unique_ptr<std::exception> file_error;

public:
	size_t fsize = 0;
	size_t fpos	 = 0;

	inline void lock() const volatile noexcept { NV(mutex).lock(); }
	inline void unlock() const volatile noexcept { NV(mutex).unlock(); }

	SharedData() noexcept = default;

	~SharedData() noexcept
	{
		try
		{
			sync();
		}
		catch (std::exception& e)
		{
			logline("BufferedFile::sync(): %s", e.what());
		}
	}

	inline void check_error() const volatile throws
	{
		if (unlikely(NV(file_error))) throw *nvptr(this)->file_error;
	}

	inline void notify(What w) volatile noexcept
	{
		nvptr(this)->what |= w;
		NV(this)->cond.notify_one();
	}

	static void work(volatile SharedData* sd, cstr fpath, Mode mode) noexcept
	{
		for (int what = What::Open; ~what & What::Dispose; what = sd->get_work())
		{
			try
			{
				if (what & What::Open) nvptr(sd)->open(fpath, mode);
				if (what & What::Close) sd->close();
				if (what & What::Sync) sd->sync();
				sd->read_buffers();
			}
			catch (FileError& e)
			{
				nvptr(sd)->file_error.reset(new FileError(std::move(e)));
			}
			catch (std::exception& e)
			{
				nvptr(sd)->file_error.reset(new std::exception(std::move(e)));
			}
		}

		delete sd;
	}

	void open(cstr fpath, Mode mode)
	{
		// application or worker thread

		if (state >= Opened) return;
		file.reset(new File(fpath, mode));
		fsize	   = file->getFsize();
		size_t cnt = (fsize + buffer_size - 1) / buffer_size;
		buffers.grow(uint(cnt), uint(cnt) + file->writable * 100);
		state = Opened;
	}

	void setFpos(size_t fpos) volatile
	{
		// set fpos and trigger prefetch at fpos

		size_t idx = fpos / buffer_size;
		size_t old = this->fpos / buffer_size;
		this->fpos = fpos;

		if (idx == old) return;					// prefetch range didn't change
		if (idx >= NV(buffers).count()) return; // new block is at eof
		if (what != 0) return;					// already triggered

		notify(What::Read);
	}

	size_t avail(size_t required) volatile
	{
		uint i = uint(fpos / buffer_size);
		uint e = min(uint((fpos + required + buffer_size - 1) / buffer_size), NV(buffers).count(), i + min_buffers);

		while (i < e && NV(buffers)[i]) { i++; }
		return min(fsize, i * buffer_size) - fpos;
	}

	size_t free(size_t required) volatile
	{
		uint i = uint(fpos / buffer_size);
		uint e = min(uint((fpos + required + buffer_size - 1) / buffer_size), NV(buffers).count(), i + min_buffers);

		while (i < e && NV(buffers)[i]) { i++; }
		return i == e ? (max_buffers * buffer_size) : (i * buffer_size - fpos);
	}

	Buffer* waitForBuffer(uint idx) noexcept
	{
		what |= What::Read;
		unlock();
		cond.notify_one();
		while (buffers[idx] == nullptr) usleep(20);
		lock();
		return buffers[idx];
	}

	size_t read(char* bu, size_t size, bool blocking)
	{
		assert(size <= fsize - fpos);

		for (size_t count = 0, offset = fpos % buffer_size; count < size; offset = 0)
		{
			uint	idx	   = uint(fpos / buffer_size);
			Buffer* buffer = buffers[idx];
			if (!buffer)
			{
				if (!blocking) return count;
				buffer = waitForBuffer(idx);
			}

			size_t cnt = min(size - count, buffer_size - offset);
			memcpy(bu + count, &buffer->data[offset], cnt);
			count += cnt;
			fpos += cnt;
		}
		return size;
	}

	size_t write(const char* bu, size_t size, bool blocking)
	{
		buffers.grow(uint((fpos + size + buffer_size - 1) / buffer_size));

		for (size_t count = 0, offset = fpos % buffer_size; count < size; offset = 0)
		{
			size_t	cnt	   = min(size - count, buffer_size - offset);
			uint	idx	   = uint(fpos / buffer_size);
			Buffer* buffer = buffers[idx];
			if (!buffer)
			{
				if (cnt == buffer_size || fpos == fsize) buffers[idx] = buffer = new Buffer;
				else if (!blocking) return count;
				else buffer = waitForBuffer(idx);
			}

			memcpy(&buffer->data[offset], bu + count, cnt);
			count += cnt;
			fpos += cnt;
			if (fpos > fsize) fsize = fpos;
			buffer->dirty = true;
		}
		return size;
	}

	bool  isProtected(uint idx) const volatile noexcept { return idx - fpos / buffer_size < min_buffers; }
	int16 getState() const volatile noexcept { return state; }

private:
	void notify(What); // must not be used when locked
	int	 get_work();   // must not be used when locked
	void close();	   // must not be used when locked
	//void sync();			// must not be used when locked
	void read_buffer(uint); // must not be used when locked
	void read_buffers();	// must not be used when locked
	void check_error();		// must not be used when locked

	int get_work() volatile noexcept
	{
		std::unique_lock<std::mutex> lock(NV(mutex));

		if (what == 0) NV(cond).wait(lock);
		int w = what;
		what  = 0;
		return w;
	}

	void close() volatile throws
	{
		if (state != Opened) return;
		sync();
		NV(file)->close();
		state = Closed;
	}

	void sync() volatile throws
	{
		if (state != Opened) return;
		if (!NV(file)->writable) return;

		// only the worker thread is allowed to add/remove buffers
		// => we can safely access buffers[]

		for (uint i = 0; i < NV(buffers).count(); i++)
		{
			Buffer* bu = NV(buffers)[i];
			if (bu && bu->dirty)
			{
				size_t fpos = i * buffer_size;
				NV(file)->setFpos(fpos);
				bu->dirty = false;
				NV(file)->write(bu->data, min(buffer_size, fsize - fpos));
			}
		}
		NV(file)->sync();
	}

	void purge_some_buffers()
	{
		// TODO
	}

	void read_buffer(uint i) volatile throws
	{
		assert(i < NV(buffers).count());
		assert(NV(buffers)[i] == nullptr);

		std::unique_ptr<Buffer> bu {new Buffer};
		NV(file)->setFpos(i * buffer_size);
		NV(file)->read(bu->data, buffer_size, true);
		NV(buffers)[i] = bu.release();
	}

	void read_buffers() volatile throws
	{
		if (state != Opened) return;
	a:
		uint a = uint(fpos / buffer_size);
		uint e = min(a + min_buffers, uint((fsize + buffer_size - 1) / buffer_size));

		assert(e <= NV(buffers).count());

		for (uint i = a; i < e; i++)
		{
			if ((what & Close) == 0) return;

			if (NV(buffers)[i] == nullptr)
			{
				std::unique_ptr<Buffer> bu {new Buffer};
				NV(file)->setFpos(i * buffer_size);
				NV(file)->read(bu->data, buffer_size, true);
				NV(buffers)[i] = bu.release();
			}

			if (a != uint(fpos / buffer_size)) goto a;
		}
	}
};


////////////////////////////////// BufferedFile //////////////////////////////////////


BufferedFile::BufferedFile(cstr path, Mode mode, bool open_in_background) : //
	IFile(path, mode),
	shared_data(nullptr)
{
	if (mode == Mode::APPEND) throw FileError(path, EINVAL); // can't handle append mode
	std::unique_ptr<SharedData> sd(new SharedData);
	if (!open_in_background) sd->open(path, mode); // may throw
	shared_data = sd.release();
	std::thread(SharedData::work, shared_data, path, mode).detach();
}

BufferedFile::~BufferedFile() noexcept
{
	// close & dispose in background:
	if (shared_data) shared_data->notify(What::Dispose);
}

void BufferedFile::close()
{
	shared_data->notify(What::Close);

	for (;; usleep(20))
	{
		shared_data->check_error();
		if (shared_data->getState() != State::Opened) return;
	}
}

size_t BufferedFile::getFsize() const
{
	for (;; usleep(10))
	{
		shared_data->check_error();
		if (shared_data->getState() != State::Initial) return shared_data->fsize;
	}
}

size_t BufferedFile::getFpos() const
{
	shared_data->check_error();
	return shared_data->fpos;
}

size_t BufferedFile::setFpos(ssize_t newpos, Whence whence)
{
	shared_data->check_error();
	size_t oldpos = shared_data->fpos;
	if (whence == Relative) { newpos += oldpos; }
	if (whence == FromEnd) { newpos += getFsize(); }
	if (size_t(newpos) > shared_data->fsize) throw FileError(filepath, EINVAL);
	shared_data->setFpos(size_t(newpos));
	return oldpos;
}

void BufferedFile::sync()
{
	shared_data->check_error();
	if (writable) shared_data->notify(What::Sync);
}

size_t BufferedFile::avail(size_t required) const
{
	shared_data->check_error();
	return shared_data->avail(required);
}

size_t BufferedFile::free(size_t required) const
{
	shared_data->check_error();
	return shared_data->free(required);
}

size_t BufferedFile::read(void* bu, size_t size, bool partial)
{
	shared_data->check_error();
	size_t max_avail = shared_data->fsize - shared_data->fpos;
	if (size <= max_avail) return nvptr(shared_data)->read(ptr(bu), size, !partial);
	if (max_avail && partial) return nvptr(shared_data)->read(ptr(bu), max_avail, !partial);
	throw FileError(filepath, endoffile);
}

size_t BufferedFile::write(const void* bu, size_t size, bool partial)
{
	shared_data->check_error();
	if (writable) return nvptr(shared_data)->write(cptr(bu), size, !partial);
	throw FileError(filepath, EBADF);
}


/*
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
*/
