// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#ifndef DISKDRIVE_H
#define DISKDRIVE_H

#include "Files/FloppyDisk.h"
#include "kio/kio.h"
#include "zxsp_types.h"


enum FddType { NoDrive = 1, Drive525 = 2, Drive35 = 4, Drive3 = 8 };


class FloppyDiskDrive
{
public:
	FddType type;

	// sounds:
	Sample* sound_insert;
	Sample* sound_eject;
	Sample* sound_running;
	Sample* sound_step;
	uint	sound_insert_size;
	uint	sound_eject_size;
	uint	sound_running_size;
	int		sound_step_size;
	uint	sound_insert_index;
	uint	sound_eject_index;
	uint	sound_running_index;
	int		sound_step_index[4];

	// Laufwerksparameter:
	uint num_heads;		   // +3: 1
	uint num_tracks;	   // +3: 40
	uint rpm;			   // 300
	Time step_delay;	   // Zeit für Step
	Time motor_on_delay;   // Zeit für 0->300rpm
	Time motor_off_delay;  // Zeit für 300rpm->0
	uint bytes_per_track;  // +3: MFM: 6250
	uint bytes_per_second; // rot. speed = bytes_per_track * 5 bei 300rpm
	uint bytes_per_index;  // dur. of index pulse: bytes_per_track / 50

	// Eingelegte Diskette:
	FloppyDisk* disk;
	uint		side_B_up;

	// Antriebsmotor & Winkelposition:
	bool  motor_on;
	float bytepos; // inside bytes_per_track <=> rot. pos.
	float speed;   // 0 .. spinning_up/down .. nominal_speed
	Time  time;	   // zugehöriger Zeitstempel

	// Steppermotor & Track:
	uint track;			// current physical track
	uint stepping;		// 0 o. ±1
	Time step_end_time; // if stepping

	// 6 lines drive -> fdc:
	bool is_ready;
	bool is_error;
	bool is_2sided;
	bool is_atindex;
	bool is_wprot;
	bool is_track0;

public:
	FloppyDiskDrive();
	FloppyDiskDrive(FddType, uint heads, uint tracks, Time step_delay = 0.006, uint bytes_per_track = 6250);
	~FloppyDiskDrive();

	// misc. querries:
	uint  bytesPerTrack() { return bytes_per_track; }
	bool  diskLoaded() { return disk != nullptr; }
	float floatBytePosition() { return bytepos; }
	uint  bytesPerSecond() { return bytes_per_second; }
	uint  bytePosition() { return uint(bytepos); }
	uint  bytePosition(Time t)
	{
		update(t);
		return uint(bytepos);
	}
	Time		timeSinceIndex() { return bytepos / bytes_per_second; }
	Time		timePerByte() { return 1.0 / bytes_per_second; }
	Time		timePerTrack() { return 60.0 / rpm; }
	FloppyDisk* getDisk() { return disk; }

	// read / write:
	uint8 readByte(uint head, uint track, uint bytepos);
	void  writeByte(uint head, uint track, uint bytepos, uint8 byte);
	uint8 readByte(uint head, uint bytepos);
	void  writeByte(uint head, uint bytepos, uint8 byte);

	// load / eject disk:
	void insertDisk(FloppyDisk*, bool side_B = no);
	void insertDisk(cstr filepath, bool side_B = no);
	void ejectDisk();
	void flipDisk() { side_B_up ^= 1; }

	// proceed time:
	void audioBufferEnd(Time);
	void update(Time);

	// 4 lines fdc -> drive:
	void step(Time, int dir); // dir = ±1
	void setMotor(Time, bool);
	void resetError() {} // todo

private:
	void update_signals();
};


inline uint8 FloppyDiskDrive::readByte(uint head, uint track, uint bytepos)
{
	return disk ? disk->readByte(head ^ side_B_up, track, bytepos) : 0xff;
}
inline void FloppyDiskDrive::writeByte(uint head, uint track, uint bytepos, uint8 byte)
{
	if (disk) disk->writeByte(head ^ side_B_up, track, bytepos, byte);
}
inline uint8 FloppyDiskDrive::readByte(uint head, uint bytepos)
{
	return disk ? disk->readByte(head ^ side_B_up, track, bytepos) : 0xff;
}
inline void FloppyDiskDrive::writeByte(uint head, uint bytepos, uint8 byte)
{
	if (disk) disk->writeByte(head ^ side_B_up, track, bytepos, byte);
}


#endif
