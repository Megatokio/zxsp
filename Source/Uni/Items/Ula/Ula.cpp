// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Ula.h"
#include "Keyboard.h"
#include "Machine.h"


namespace zxsp
{

/*	constructor for derived classes
	note: the ula is added like an item with no i/o
		  machine.Input() and Output() always call the ula
		  to add waitstates and floating bus bytes
	note: default Ula has no waitmap wg. B&W ULAs
	note: ula must be 1st item with i/o  (2nd after cpu)
*/
Ula::Ula(Machine* m, isa_id id, cstr o_addr, cstr i_addr) :
	Crtc(m, id, isa_Ula, internal, o_addr, i_addr),
	ula_out_byte(0),
	beeper_volume(1.0),
	beeper_current_sample(0.0),
	beeper_last_sample_time(0.0),
	info(m->model_info),
	lines_in_screen(info->lines_in_screen),
	lines_before_screen(info->lines_before_screen),
	lines_after_screen(info->lines_after_screen),
	lines_per_frame(lines_before_screen + lines_in_screen + lines_after_screen),
	columns_in_screen(32 * 8),
	cc_per_line(info->cpu_cycles_per_line),
	cc_before_screen(cc_per_line * lines_before_screen),
	is60hz(info->frames_per_second > 55)
{
	video_ram = machine->ram.getData();
}

Ula::~Ula() {}

void Ula::powerOn(int32 cc)
{
	Crtc::powerOn(cc);

	border_color			= 0;
	ula_out_byte			= 0;
	beeper_current_sample	= 0.0f; // current beeper elongation
	beeper_last_sample_time = 0.0;
	keymap.clear();
}

void Ula::audioBufferEnd(Time t)
{
	if (t > beeper_last_sample_time)
	{
		machine->outputSamples(beeper_current_sample, beeper_last_sample_time, t);
		beeper_last_sample_time = t;
	}
	beeper_last_sample_time -= t;
}

void Ula::setBeeperVolume(Sample new_vol)
{
	if (new_vol > 1.0f) new_vol = 1.0f;
	if (new_vol < -1.0f) new_vol = -1.0f;
	beeper_current_sample *= new_vol / beeper_volume;
	beeper_volume = new_vol;
}

uint8 Ula::readKeyboard(uint16 addr)
{
	// Z80 input: merge in the keys:
	// only bits 0-4 come from the keyboard

	uint8 byte = 0xff;
	if (keymap.keyPressed())
	{
		uint8* p	 = &keymap[0];
		uint8  minor = (~addr) >> 8;
		while (minor)
		{
			if (minor & 1) byte &= *p;
			minor >>= 1;
			p++;
		}
	}
	return byte;
}

void Ula::setLinesBeforeScreen(int n)
{
	if (lines_before_screen == n) return;
	lines_before_screen = n;
	setupTiming();
}

void Ula::setLinesAfterScreen(int n)
{
	if (lines_after_screen == n) return;
	lines_after_screen = n;
	setupTiming();
}

void Ula::setCcPerLine(int n)
{
	if (cc_per_line == n) return;
	cc_per_line = n;
	setupTiming();
}

void Ula::setBytesPerLine(int n) { setCcPerLine(n * cc_per_byte); }


void Ula::set60Hz(bool f)
{
	is60hz = f;
	assert(f == (info->frames_per_second >= 55)); // must been set by caller

	lines_in_screen		= info->lines_in_screen;
	lines_before_screen = info->lines_before_screen;
	lines_after_screen	= info->lines_after_screen;
	lines_per_frame		= lines_before_screen + lines_in_screen + lines_after_screen;
	cc_per_line			= info->cpu_cycles_per_line;
	cc_before_screen	= cc_per_line * lines_before_screen;
}


} // namespace zxsp

/*
























*/
