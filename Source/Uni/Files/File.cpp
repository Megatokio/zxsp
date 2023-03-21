// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "File.h"
#include "kio/exceptions.h"
#include <fcntl.h>


File::File(cstr path, Mode mode) : IFile(path, mode)
{
	assert(path != nullptr);

	if (path[0] == '~' && path[1] == '/') // test: add user's home directory path?
	{
		cstr home = getenv("HOME");
		if (home && home[0] == '/') // home dir known?
		{
			int f = home[strlen(home) - 1] == '/'; // ends with '/' ?
			path  = catstr(home, path + 1 + f);	   // prepend home dir to path
		}
	}

	int filemode;
	if (mode == 'r') filemode = O_RDONLY;
	else if (mode == 'm') filemode = O_RDWR | O_CREAT;
	else if (mode == 'n') filemode = O_WRONLY | O_CREAT | O_EXCL;
	else if (mode == 'a') filemode = O_WRONLY | O_CREAT | O_APPEND;
	else if (mode == 'w') filemode = O_WRONLY | O_CREAT | O_TRUNC;
	else filemode = mode;

	int fileperm = 0660;

	for (;;)
	{
		fd = open(path, filemode, fileperm);
		if (fd >= 0) return;
		if (errno == EINTR) continue;
		throw FileError(path, errno);
	}
}

File::~File() noexcept
{
	if (int err = _close()) logline("close %s: %s", filepath, strerror(err));
}

void File::close()
{
	int err	 = _close();
	fd		 = -1;
	readable = false;
	writable = false;
	if (err) throw FileError(filepath, err);
}

int File::_close()
{
	for (;;)
	{
		if (fd < 0 || ::close(fd) == 0) return noerror;
		else if (errno == EINTR) continue;			// interrupted
		else if (errno == EAGAIN) { usleep(1000); } // non-blocking dev only
		else return errno;
	}
}

size_t File::getFpos() const
{
	off_t fpos = lseek(fd, 0, SEEK_CUR);
	if (fpos != -1) return size_t(fpos);
	else throw FileError(filepath, errno);
}

size_t File::setFpos(ssize_t fpos, Whence whence)
{
	// set file position
	// return old file position

	int how; // if we are lucky then this all is a nop:
	if (whence == FromStart) how = SEEK_SET;
	else if (whence == Relative) how = SEEK_CUR;
	else if (whence == FromEnd) how = SEEK_END;
	else how = whence;

	fpos = lseek(fd, fpos, how);
	if (fpos != -1) return size_t(fpos);
	else throw FileError(filepath, errno);
}

size_t File::getFsize() const
{
	off_t fpos = lseek(fd, 0, SEEK_END);
	if (fpos != -1) fpos = lseek(fd, fpos, SEEK_SET);
	if (fpos != -1) return size_t(fpos);
	else throw FileError(filepath, errno);
}

size_t File::read(void* p, size_t size, bool partial)
{
	// read n bytes or throw
	// may suspend thread while waiting for slow devices
	// partial & eof: read at least one byte at eof or throw

	char* z = ptr(p);
	for (size_t cnt = 0;;)
	{
		ssize_t n = ::read(fd, z + cnt, size - cnt);
		if (n == ssize_t(size - cnt)) return size; // most common case
		else if (n > 0) { cnt += size_t(n); }	   // interrupted
		else if (n == 0)						   // eof
		{
			if (partial && cnt != 0) return cnt;
			else throw FileError(filepath, endoffile);
		}
		else if (errno == EINTR) continue;			// interrupted
		else if (errno == EAGAIN) { usleep(1000); } // non-blocking dev only
		else throw FileError(filepath, errno);
	}
}

size_t File::write(const void* p, size_t size, bool /*partial*/)
{
	// write n bytes or throw
	// may suspend thread while waiting for slow devices
	// returned value is always the requested value

	const char* q = cptr(p);
	for (size_t cnt = size;;)
	{
		ssize_t n = ::write(fd, q + cnt, size - cnt);
		if (n == ssize_t(size - cnt)) return size;	// most common case
		else if (n >= 0) { cnt += size_t(n); }		// interrupted
		else if (errno == EINTR) continue;			// interrupted
		else if (errno == EAGAIN) { usleep(1000); } // non-blocking dev only
		else throw FileError(filepath, errno);
	}
}

size_t File::avail(size_t /*required*/) const { return getFsize() - getFpos(); }

size_t File::free(size_t /*required*/) const { return UINT_MAX; }

void File::sync()
{
	int err = fdatasync(fd);
	if (err) throw FileError(filepath, errno);
}


/*
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
*/
