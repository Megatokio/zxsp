// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Interfaces/IFile.h"


/* Implementation of the IFile interface 
   for buffered files.   
*/
class BufferedFile final : public IFile
{
	NO_COPY_MOVE(BufferedFile);

	static constexpr size_t buffer_size = 16 kB;
	static constexpr uint	min_buffers = 10;
	static constexpr uint	max_buffers = 1 MB / buffer_size;

	struct Buffer;
	struct Buffers;
	class SharedData;
	volatile SharedData* shared_data; // data shared with worker thread

public:
	BufferedFile(cstr path, Mode = READ, bool open_in_background = false);
	~BufferedFile() noexcept override;

	size_t getFpos() const override;
	size_t setFpos(ssize_t, Whence = FromStart) override;
	size_t setFpos(size_t pos) { return setFpos(ssize_t(pos)); }
	size_t getFsize() const override;

	size_t avail(size_t required = UINT_MAX) const override;
	size_t free(size_t required = UINT_MAX) const override;

	size_t read(void*, size_t count, bool partial = false) override;
	size_t write(const void*, size_t count, bool partial = false) override;
	void   sync() override;
	void   close() override;
};
