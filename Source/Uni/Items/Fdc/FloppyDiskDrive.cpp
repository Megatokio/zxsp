// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "FloppyDiskDrive.h"
#include "Dsp.h"
#include "DspTime.h"
#include "Fdc.h"
#include "Files/FloppyDisk.h"
#include "Qt/qt_util.h"
#include "RecentFilesMenu.h"
#include "Templates/Array.h"
#include "globals.h"
#include <math.h>


inline uint16 random(uint n) //	16 bit random number in range [0 ... [n
{
	return (uint32(n) * uint16(random())) >> 16;
}

/*	Default Constructor
	creates a "no disk drive" which reacts to the control signals
	of the FDC as if no disk drive is attached.
	these signals are active low on the FDD bus and pulled high by resistors.
	so they should report "off"
	(note: the inputs of the FDC are active high and are connected via inverters)
	this should apply to:
			INDEX,
			FAULT / TRACK0,
			WPROT / 2SIDED,
			READY
*/
FloppyDiskDrive::FloppyDiskDrive() :
	type(NoDrive),
	sound_insert(nullptr),
	sound_eject(nullptr),
	sound_running(nullptr),
	sound_step(nullptr),
	sound_insert_size(0),
	sound_eject_size(0),
	sound_running_size(0),
	sound_step_size(0),
	sound_insert_index(0),
	sound_eject_index(0),
	sound_running_index(0),
	sound_step_index {0, 0, 0, 0},
	num_heads(1),
	num_tracks(1),
	rpm(300),
	step_delay(0),
	motor_on_delay(0),
	motor_off_delay(0),
	bytes_per_track(1000),
	bytes_per_second(bytes_per_track * rpm / 60),
	bytes_per_index(bytes_per_track),
	disk(nullptr),
	side_B_up(0),
	motor_on(no),
	bytepos(0),
	speed(0),
	time(0),
	track(0),
	stepping(0),
	step_end_time(0),
	is_ready(no),
	is_error(no),
	is_2sided(no),
	is_atindex(no),
	is_wprot(no),
	is_track0(no)
{}

/*	Constructor
	creates a 3", 3.5" or 5.25" Disk Drive
*/
FloppyDiskDrive::FloppyDiskDrive(FddType type, uint heads, uint tracks, Time step_delay, uint bytes_per_track) :
	type(type),
	sound_insert(nullptr),
	sound_eject(nullptr),
	sound_running(nullptr),
	sound_step(nullptr),
	sound_insert_size(0),
	sound_eject_size(0),
	sound_running_size(0),
	sound_step_size(0),
	sound_insert_index(0),
	sound_eject_index(0),
	sound_running_index(0),
	sound_step_index {0, 0, 0, 0},
	num_heads(heads),
	num_tracks(tracks),
	rpm(300),
	step_delay(step_delay),
	motor_on_delay(0.5),
	motor_off_delay(0.4),
	bytes_per_track(bytes_per_track),
	bytes_per_second(bytes_per_track * rpm / 60),
	bytes_per_index(bytes_per_track / 50),
	disk(nullptr),
	side_B_up(0),
	motor_on(no),
	bytepos(random(bytes_per_track)),
	speed(0),
	time(0),
	track(random(num_tracks)),
	stepping(0),
	step_end_time(0),
	is_ready(no),
	is_error(no),
	is_2sided(heads > 1),
	is_atindex(no),
	is_wprot(no),
	is_track0(no)
{
	FD	   fd(catstr(appl_rsrc_path, "/Audio/plus3/running.raw"), 'r');
	uint32 cnt = fd.file_size() >> 1;
	int16* zbu = new int16[cnt];
	fd.read_bytes(zbu, cnt << 1);
	fd.close_file(0);
	sound_running = new Sample[cnt];
	for (uint i = 0; i < cnt; i++) sound_running[i] = ldexpf((int16)peek2Z(zbu + i), -14);
	delete[] zbu;
	zbu					= 0;
	sound_running_index = sound_running_size = cnt;

	fd.open_file_r(catstr(appl_rsrc_path, "/Audio/plus3/insert.raw"));
	cnt = fd.file_size() >> 1;
	zbu = new int16[cnt];
	fd.read_bytes(zbu, cnt << 1);
	fd.close_file(0);
	sound_insert = new Sample[cnt];
	for (uint i = 0; i < cnt; i++) sound_insert[i] = ldexpf((int16)peek2Z(zbu + i), -16);
	delete[] zbu;
	zbu				   = 0;
	sound_insert_index = sound_insert_size = cnt;

	fd.open_file_r(catstr(appl_rsrc_path, "/Audio/plus3/eject.raw"));
	cnt = fd.file_size() >> 1;
	zbu = new int16[cnt];
	fd.read_bytes(zbu, cnt << 1);
	fd.close_file(0);
	sound_eject = new Sample[cnt];
	for (uint i = 0; i < cnt; i++) sound_eject[i] = ldexpf((int16)peek2Z(zbu + i), -16);
	delete[] zbu;
	zbu				  = 0;
	sound_eject_index = sound_eject_size = cnt;

	fd.open_file_r(catstr(appl_rsrc_path, "/Audio/plus3/step.raw"));
	cnt = fd.file_size() >> 1;
	zbu = new int16[cnt];
	fd.read_bytes(zbu, cnt << 1);
	fd.close_file(0);
	sound_step = new Sample[cnt];
	for (uint i = 0; i < cnt; i++) sound_step[i] = ldexpf((int16)peek2Z(zbu + i), -17);
	delete[] zbu;
	zbu = 0;
	for (uint i = 0; i < NELEM(sound_step_index); i++) sound_step_index[i] = cnt;
	sound_step_size = cnt;
}

std::shared_ptr<FloppyDiskDrive> FloppyDiskDrive::noFloppyDiskDrive()
{
	// create and return a dummy drive
	// returns the same drive on every request

	static auto no_fdd = std::shared_ptr<FloppyDiskDrive>(new FloppyDiskDrive());
	return no_fdd;
}

std::shared_ptr<FloppyDiskDrive>
FloppyDiskDrive::newFloppyDiskDrive(FddType fddtype, uint heads, uint tracks, Time step_delay, uint bytes_per_track)
{
	return std::shared_ptr<FloppyDiskDrive>(new FloppyDiskDrive(fddtype, heads, tracks, step_delay, bytes_per_track));
}

FloppyDiskDrive::~FloppyDiskDrive()
{
	delete disk;
	delete[] sound_eject;
	delete[] sound_insert;
	delete[] sound_running;
	delete[] sound_step;
}

void FloppyDiskDrive::setMotor(Time t, bool f)
{
	if (type == NoDrive) return;

	update(t);
	motor_on = f;
	if (f)
	{
		if (sound_running_index == sound_running_size) sound_running_index = 0;
	}
	else { sound_running_index = sound_running_size; }
	update_signals();
}

void FloppyDiskDrive::update(Time t)
{
	// update track and byte position inside track
	// update motor speed

	if (type == NoDrive) return;
	if (t <= time) return;

	// stepper motor control:

	if (stepping) // -1/0/+1
	{
		if (t >= step_end_time)
		{
			track += stepping;
			if (track >= num_tracks) track -= stepping; // limit in both directions
			stepping = 0;
			update_signals();
		}
	}

	// spindle motor control:

	if (motor_on)
	{
		if (speed < bytes_per_second) // speed up
		{
			Time dt = motor_on_delay * (bytes_per_second - speed) / bytes_per_second;

			if (dt < t - time) // reach 100% speed
			{
				bytepos += dt * (speed + bytes_per_second) / 2;
				time += dt;
				speed = bytes_per_second;
				update_signals();
			}
			else // don't reach 100% speed
			{
				dt				= t - time;
				uint speed_at_t = speed + bytes_per_second * dt / motor_on_delay;
				bytepos += dt * (speed + speed_at_t) / 2;
				time  = t;
				speed = speed_at_t;
			}
		}

		bytepos += speed * (t - time);
		while (bytepos >= bytes_per_track) { bytepos -= bytes_per_track; }
	}
	else
	{
		if (speed > 0) // speed down
		{
			Time dt = motor_off_delay * speed / bytes_per_second;

			if (dt < t - time) // reach 0% speed
			{
				bytepos += dt * speed / 2;
			}
			else // don't reach 0% speed
			{
				dt				= t - time;
				uint speed_at_t = speed - bytes_per_second * dt / motor_off_delay;
				bytepos += dt * (speed + speed_at_t) / 2;
				while (bytepos >= bytes_per_track) bytepos -= bytes_per_track;
			}
		}
	}

	is_atindex = !disk || bytepos < bytes_per_index;
	time	   = t;
}

void FloppyDiskDrive::audioBufferEnd(Time dt)
{
	// shift time base

	if (time < dt) update(dt);
	time -= dt;
	step_end_time -= dt;

	if (sound_running_index < sound_running_size)
	{
		for (uint i = 0; i < uint(DSP_SAMPLES_PER_BUFFER); i++)
		{
			os::audio_out_buffer[i] += sound_running[sound_running_index++];
			if (sound_running_index == sound_running_size) sound_running_index = 0;
		}
	}

	if (sound_insert_index < sound_insert_size)
	{
		uint n = min(uint(DSP_SAMPLES_PER_BUFFER), sound_insert_size - sound_insert_index);
		for (uint i = 0; i < n; i++) { os::audio_out_buffer[i] += sound_insert[sound_insert_index++]; }
	}

	if (sound_eject_index < sound_eject_size)
	{
		uint n = min(uint(DSP_SAMPLES_PER_BUFFER), sound_eject_size - sound_eject_index);
		for (uint i = 0; i < n; i++) { os::audio_out_buffer[i] += sound_eject[sound_eject_index++]; }
	}

	for (uint ii = 0; ii < NELEM(sound_step_index); ii++)
	{
		int& ssi = sound_step_index[ii];
		if (ssi < sound_step_size)
		{
			int i = 0;
			if (ssi < 0)
			{
				i	= -ssi;
				ssi = 0;
			}
			int e = min(DSP_SAMPLES_PER_BUFFER, i + sound_step_size - ssi);
			while (i < e) { os::audio_out_buffer[i++] += sound_step[ssi++]; }
		}
	}
}

void FloppyDiskDrive::step(Time t, int dir) // dir = ±1
{
	if (type == NoDrive) return;

	assert(dir == 1 || dir == -1);

	//	update(t);
	stepping	  = dir;
	step_end_time = t + step_delay * 0.9f;

	assert(t < seconds_per_dsp_buffer_max());

	for (uint i = 0; i < NELEM(sound_step_index); i++) // search free stepper sound slot
	{
		if (sound_step_index[i] == sound_step_size)
		{
			sound_step_index[i] = -t * DSP_SAMPLES_PER_BUFFER / seconds_per_dsp_buffer(); // start sound
			break;
		}
	}
}

void FloppyDiskDrive::insertDisk(FloppyDisk* d, bool side_B)
{
	if (type == NoDrive) return;
	if (disk) ejectDisk();
	disk	  = d;
	side_B_up = side_B;

	if (disk->sides[0].count() > num_tracks || disk->sides[1].count() > num_tracks)
		showWarning("This disc has more tracks than can be accessed by this drive");

	speed = 0;
	update_signals();
	sound_insert_index = 0;				   // start sound
	sound_eject_index  = sound_eject_size; // stop it (if running)

	addRecentFile(gui::RecentPlus3Disks, d->filepath); // TODO: andere Disk drives
	addRecentFile(gui::RecentFiles, d->filepath);
}

void FloppyDiskDrive::insertDisk(cstr filepath, bool side_B)
{
	if (type == NoDrive) return;
	if (disk) ejectDisk();
	disk	  = new FloppyDisk(filepath);
	side_B_up = side_B;

	if (disk->sides[0].count() > num_tracks || disk->sides[1].count() > num_tracks)
		showWarning("This disc has more tracks than can be accessed by this drive");

	speed = 0;
	update_signals();
	sound_insert_index = 0;				   // start sound
	sound_eject_index  = sound_eject_size; // stop it (if running)

	addRecentFile(gui::RecentPlus3Disks, filepath); // TODO: andere Disk drives
	addRecentFile(gui::RecentFiles, filepath);
}

void FloppyDiskDrive::ejectDisk()
{
	if (type == NoDrive) return;

	FloppyDisk* old_disk = disk;
	disk				 = nullptr;
	delete old_disk;

	update_signals();
	sound_eject_index  = 0;					// start sound
	sound_insert_index = sound_insert_size; // stop it (if running)
}

void FloppyDiskDrive::update_signals()
{
	if (type == NoDrive) return;

	is_ready = disk != nullptr && motor_on && speed == bytes_per_second;
	//	is_error   = no;
	//	is_2sided  = num_heads > 1;
	is_atindex = !disk || bytepos < bytes_per_index;
	is_wprot   = !disk || disk->writeprotected;
	is_track0  = track == 0;

	if (type == Drive525 && !is_ready) is_wprot = no; // 5.25" drives don't report read-only when they're not ready
	if (type == Drive35 && !motor_on) is_track0 = no; // 3.5" don't report track 0 if motor is off
}


/*





























*/
