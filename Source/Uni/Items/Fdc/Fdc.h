#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MassStorage.h"
class FloppyDiskDrive;


class Fdc : public MassStorage
{
protected:
	std::shared_ptr<FloppyDiskDrive> fdd[4];	// attached drives, mirrored drives may be inserted more than once
	FloppyDiskDrive*				 drive;		// selected drive
	bool							 motor_on;	// current state of the common motor-on output
	bool							 interrupt; // current state of the interrupt output; may be not connected

public:
	FloppyDiskDrive*				 getSelectedDrive() const volatile { return drive; }
	std::shared_ptr<FloppyDiskDrive> getDrive(uint n) const { return fdd[n & 3]; }
	bool							 isMotorOn() const volatile { return motor_on; }

	virtual void setMotor(Time, bool);
	virtual void initForSnapshot(int32 /*cc*/) {}
	virtual void attachDiskDrive(uint n, FloppyDiskDrive*);
	virtual void removeDiskDrive(uint n);

protected:
	Fdc(Machine*, isa_id, Internal internal, cstr o_addr, cstr i_addr);
	~Fdc() override = default;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time, int32 cc) override;
	// void	input			(Time, int32 cc, uint16 addr, uint8& byte, uint8& mask) override = 0;
	// void	output			(Time, int32 cc, uint16 addr, uint8 byte) override = 0;
	void audioBufferEnd(Time) override;
	// void	videoFrameEnd	(int32 cc);
	// uint8 handleRomPatch	(uint16,uint8) override;

	virtual void  raise_interrupt() { interrupt = on; }	 // to be reimplemented if fdc uses interrupts
	virtual void  clear_interrupt() { interrupt = off; } // to be reimplemented if fdc uses interrupts
	virtual void  send_byte_to_dma(uint8) {}			 // to be reimplemented if fdc uses dma
	virtual uint8 read_byte_from_dma() { return 0xFF; }	 // to be reimplemented if fdc uses dma
};
