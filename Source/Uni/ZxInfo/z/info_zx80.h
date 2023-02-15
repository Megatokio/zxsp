// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


const int  zx80_basemodel	   = zx80;
const cstr zx80_model_name	   = "Sinclair ZX80";
const cstr zx80_rom_filename   = "zx80.rom";
const cstr zx80_kbd_filename   = "zx80_kbd.png";
const cstr zx80_image_filename = "zx80.jpg";

const uint32 zx80_cc_per_second			 = 3250000;
const uint32 zx80_cc_per_line			 = 207;				  // programmable
const uint	 zx80_lines_before_screen_50 = 56;				  // programmable, 50 Hz
const uint	 zx80_lines_in_screen		 = 8 * 24;			  // programmable
const uint	 zx80_lines_after_screen_50	 = 312 - 56 - 8 * 24; // programmable, 50 Hz		TODO: verify

const uint32 zx80_rom_size	= 4 * 1024;
const uint32 zx80_ram_size	= 1 * 1024;
const uint32 zx80_page_size = 1 * 1024;

const uint32 zx80_tape_load_routine	 = 0x0207;	   // LOAD-1
const uint32 zx80_tape_save_routine	 = 0x01B6 + 1; // SAVE + 1		((after pop ret_addr))
const uint32 zx80_tape_load_ret_addr = 0x0202;	   // used in LOAD and SAVE command!
												   //	• ret addr = 0x01F6 = SAVE
												   //	• ret addr = 0x024B = LOAD
