// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Crtc.h"
#include "Item.h"
#include "Machine.h"
#include "MachineController.h"


Crtc::Crtc(Machine* m, isa_id id, isa_id grp, Internal i, cstr o_addr, cstr i_addr) :
	Item(m, id, grp, i, o_addr, i_addr),
	info(m->model_info),
	screen(nullptr), //(machine->controller->getScreen()),
	video_ram(machine->ram.getData()),
	lines_in_screen(info->lines_in_screen),
	lines_before_screen(info->lines_before_screen),
	lines_after_screen(info->lines_after_screen),
	lines_per_frame(lines_before_screen + lines_in_screen + lines_after_screen),
	cc_per_line(info->cpu_cycles_per_line),
	border_color(0),
	is60hz(info->frames_per_second > 55)
{}


void Crtc::powerOn(int32 cc)
{
	assert(screen != nullptr || this != machine->crtc);
	Item::powerOn(cc);
	border_color = 0;
}

void Crtc::reset(Time t, int32 cc)
{
	Item::reset(t, cc);
	// border_color = 0;		nope!
}


void Crtc::attachToScreen(IScreen* newscreen)
{
	screen = newscreen;
	if (screen) markVideoRam();
}
