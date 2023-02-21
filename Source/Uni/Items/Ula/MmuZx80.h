#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Mmu.h"


class MmuZx80 : public Mmu
{
protected:
	MmuZx80(Machine* m, isa_id _id) : Mmu(m, _id, nullptr, nullptr) {}

public:
	MmuZx80(Machine* m) : Mmu(m, isa_MmuZx80, nullptr, nullptr) {}

protected:
	~MmuZx80() override = default;

	// Item interface:
	virtual void powerOn(/*t=0*/ int32 cc) override;
	// virtual void	reset			(Time t, int32 cc);
	// virtual void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask);
	// virtual void	output			(Time t, int32 cc, uint16 addr, uint8 byte);
	// virtual void	audioBufferEnd	(Time t);
	// virtual void	videoFrameEnd	(int32 cc);

	// MMU Interface:
	void mapMem() override;

	// virtual bool	hasPort7ffd		()	volatile const noexcept       { return no; }
	// virtual bool	hasPort1ffd		()	volatile const noexcept       { return no; }
	// virtual bool	hasPortF4		()	volatile const noexcept       { return no; }


	// ramCS: currently not used. Ram extensions simply add to machine.ram.
	// virtual void	ramCS			(bool);

	// romCS: this signal was not present on the ZX80
	// virtual void	romCS			(bool);
};
