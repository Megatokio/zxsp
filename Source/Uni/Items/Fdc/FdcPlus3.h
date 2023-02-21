#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc765.h"


class FdcPlus3 final : public Fdc765
{
public:
	explicit FdcPlus3(Machine*);

protected:
	~FdcPlus3() override = default;

	// Item interface:
	// void	init			(/*t=0*/ int32 cc) override;
	// void	reset			(Time, int32 cc) override;
	void input(Time, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time) override;
	// void	videoFrameEnd	(int32 cc) override;

	void attachDiskDrive(uint n, FloppyDiskDrive*) override;
	void removeDiskDrive(uint n) override;

private:
	bool is_fault() override { return no; }
	bool is_2sided() override { return drive->is_wprot; }
};
