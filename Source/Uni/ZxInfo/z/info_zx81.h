// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


const int  zx81_basemodel	   = zx81;
const cstr zx81_model_name_uk  = "Sinclair ZX81";
const cstr zx81_model_name_us  = "Timex 1000";
const cstr zx81_rom_filename   = "zx81.rom";
const cstr zx81_kbd_filename   = "zx81_kbd.png"; // TODO: variants: "New Line" vs. "Return"
const cstr zx81_image_filename = "zx81.jpg";	 // TODO: variants

const uint32 zx81_cc_per_second			 = 3250000;
const uint32 zx81_cc_per_line			 = 207;				  // TODO: verify
const uint	 zx81_lines_before_screen_50 = 56;				  // TODO: verify; programmable, 50 Hz
const uint	 zx81_lines_in_screen		 = 8 * 24;			  // TODO: verify; programmable
const uint	 zx81_lines_after_screen_50	 = 312 - 56 - 8 * 24; // TODO: verify; programmable, 50 Hz

const uint32 zx81_rom_size	  = 8 * 1024;
const uint32 zx81_ram_size_uk = 1 * 1024;
const uint32 zx81_ram_size_us = 2 * 1024;
const uint32 zx81_page_size	  = 1 * 1024;

#if 0 // version 1 'standard' rom
uint32	const	zx81_tape_load_routine		= 0x0340;		?		// index in rom[] of start of tape load routine
uint32	const	zx81_tape_save_routine		= 0x02FC;		?		// index in rom[] of start of tape save routine
uint32	const	zx81_tape_load_ret_addr		= 0x0206;		?		// SLOW_FAST
															?		//	• ret addr = 0x031C = SAVE
															?		//	• ret addr = 0x0383 = LOAD
#else // version 2 'improved' rom
const uint32 zx81_tape_load_routine = 0x0340 + 7; // NEXT_PROG
const uint32 zx81_tape_save_routine = 0x02F6 + 6; // SAVE + 6 ((after ex hl,de))
const uint32 zx81_tape_load_ret_addr =
	0x0207 + 3; // SLOW_FAST + 3 ((we use next instr; $0207 is also tape load entry on ZX80))
				//	• ret addr = 0x031C? = SAVE
				//	• ret addr = 0x0383? = LOAD
#endif
