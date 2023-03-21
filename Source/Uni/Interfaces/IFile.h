// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "kio/kio.h"


/*	Interface for disk files.
	there should be an implementation for normal files and buffered files.
	throw FileError on any error.
	
	File: reading and writing goes straight to the underlying system.
		the file may be based on Unix file handles or FILE.
		
	BufferedFile:
		reading and writing is buffered and done on a background thread. 
		opening and closing may be done buffered or in foreground.
		reading is non-blocking as soon as data is prefetched at the current fpos.
		writing in the middle of a file may be blocking as for reading.
		writing at eof is normally non-blocking.
		open file and setFpos() trigger prefetch at the new fpos.
*/

class IFile
{
	NO_COPY_MOVE(IFile);

public:
	enum Mode : char { READ = 'r', WRITE = 'w', MODIFY = 'm', NEW = 'n', APPEND = 'a' };
	enum Whence { FromStart, Relative, FromEnd };

	const cstr filepath;
	const Mode mode;
	bool	   readable;
	bool	   writable;

public:
	/* if the file is not already closed then flush buffers to disk and close the file.
	   errors during this process are silently ignored and just logged.
	*/
	virtual ~IFile() noexcept { delete[] filepath; }

	/* get the current file read/write position.
	*/
	virtual size_t getFpos() const = 0;

	/* set the current file read/write position.
	*/
	virtual size_t setFpos(ssize_t, Whence = FromStart) = 0;

	size_t setFpos(size_t pos) { return setFpos(ssize_t(pos)); } // convenience

	/* get file size. 
	   may block even for buffered files if called immediately after opening the file.	
	*/
	virtual size_t getFsize() const = 0;

	/* get number of bytes ready to read.
	   for normal files this is the number of bytes until file end.
	   for buffered files this is the amount of bytes prefetched at the current fpos.
	   the maximum amount possibly available is limited by the total buffers allocated.
	*/
	virtual size_t avail(size_t required = UINT_MAX) const = 0;

	/* get number of bytes ready to write.
	   for normal files this is unlimited, disk space is not considered.
	   for buffered files this is the amount of bytes prefetched at the current fpos 
		 if the write position is in the middle of a file.
	   otherwise the maximum amount possibly free is limited by the total buffers allocated.
	*/
	virtual size_t free(size_t required = UINT_MAX) const = 0;

	/* read the requested number of bytes at the current fpos and advance fpos accordingly.
	   end of file:
		 partial=false: don't advance fpos and throw.
		 partial=true:  read at least one byte (except if count=0), else throw.
	   buffered file:
		 partial=false: block until all data read.
		 partial=true:  read as many bytes as available without blocking, possibly none.
	*/
	virtual size_t read(void*, size_t size, bool partial = false) = 0;

	/* write the requested number of bytes at the current fpos and advance fpos accordingly.
	   normal file:
	     partial flag ignored.
	   buffered file:
		 partial=false: block until all data written.
		 partial=true:  write as many bytes as possible without blocking, possibly none.
	*/
	virtual size_t write(const void*, size_t size, bool partial = false) = 0;

	/* write modified buffers to disk.
	   buffered file: buffers are written in background.
	*/
	virtual void sync() = 0;

	/* flush buffers to disk and close file.
	   throws on error. 
	   buffered file: closes the file in foreground even for buffered files. 
	   buffered file: if close() is not called then the destructor will flush and close the file in background.
	*/
	virtual void close() = 0;

protected:
	/* derived class constructor: open the file using the requested mode.
	   normal file:   throw if the file cannot be opened.
	   buffered file: if the file is opened in background then the error is thrown at the next call.
	*/
	IFile(cstr fpath, Mode m) noexcept : filepath(newcopy(fpath)), mode(m), readable(r(m)), writable(w(m)) {}

private:
	static constexpr bool r(Mode m) noexcept { return m == 'r' || m == 'm'; }
	static constexpr bool w(Mode m) noexcept { return m != 'r'; }
};


/*
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
*/
