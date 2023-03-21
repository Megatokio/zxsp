// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Interfaces/IFile.h"


/* Implementation of the IFile interface 
   for unbuffered files using Unix file descriptors.
*/
class File final : public IFile
{
	NO_COPY_MOVE(File);

	int fd = -1;

public:
	File(cstr path, Mode = READ);
	~File() noexcept override;

	size_t getFpos() const override;
	size_t setFpos(ssize_t, Whence = FromStart) override;
	size_t setFpos(size_t pos) { return setFpos(ssize_t(pos)); }
	size_t getFsize() const override;

	size_t avail(size_t required = UINT_MAX) const override;
	size_t free(size_t required = UINT_MAX) const override;

	size_t read(void*, size_t size, bool partial = false) override;
	size_t write(const void*, size_t size, bool partial = false) override;
	void   sync() override;
	void   close() override;

private:
	int _close();
};
