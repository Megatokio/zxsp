// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc.h"
#include "FloppyDiskDrive.h"

static std::shared_ptr<FloppyDiskDrive> no_fdd = std::make_shared<FloppyDiskDrive>();


Fdc::Fdc(Machine* m, isa_id id, Internal internal, cstr o_addr, cstr i_addr) :
	MassStorage(m, id, internal, o_addr, i_addr),
	fdd {no_fdd, no_fdd, no_fdd, no_fdd},
	drive(fdd[0].get()),
	motor_on(no),
	interrupt(no)
{
	assert(no_fdd);
}

void Fdc::powerOn(/*t=0*/ int32 cc)
{
	MassStorage::powerOn(cc);
	setMotor(0, off);
	interrupt = off;
}

void Fdc::reset(Time t, int32 cc)
{
	MassStorage::reset(t, cc);
	setMotor(t, off);
	interrupt = off;
}

void Fdc::attachDiskDrive(uint i, FloppyDiskDrive* dd)
{
	Fdc::removeDiskDrive(i);
	fdd[i].reset(dd);
	if (motor_on) dd->setMotor(0.0, on);
}

void Fdc::removeDiskDrive(uint i)
{
	// remove drive.
	// does not remove mirrored drives.

	FloppyDiskDrive* dd = fdd[i].get();
	if (dd == no_fdd.get()) return;
	fdd[i] = no_fdd;

	for (uint i = 0; i < NELEM(fdd); i++)
	{
		if (fdd[i].get() == dd) return; // drive still exists at mirrored position
	}

	if (drive == dd) drive = no_fdd.get();
}

void Fdc::setMotor(Time t, bool f)
{
	if (motor_on != f)
	{
		xlogline(f ? "FDD: motor on" : "FDD: motor off");
		for (uint i = 0; i < NELEM(fdd); i++) fdd[i]->setMotor(t, f);
		motor_on = f;
	}
}


void Fdc::audioBufferEnd(Time t)
{
	for (uint j, i = 0; i < NELEM(fdd); i++)
	{
		FloppyDiskDrive* dd = fdd[i].get();
		for (j = 0; fdd[j].get() != dd; j++) {}
		if (i == j) dd->audioBufferEnd(t);
	}
}
