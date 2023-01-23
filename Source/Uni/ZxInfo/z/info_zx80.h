// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause



int     const   zx80_basemodel              = zx80;
cstr	const	zx80_model_name				= "Sinclair ZX80";
cstr	const	zx80_rom_filename			= "zx80.rom";
cstr	const	zx80_kbd_filename			= "zx80_kbd.png";
cstr	const	zx80_image_filename			= "zx80.jpg";

uint32	const	zx80_cc_per_second			= 3250000;
uint32	const	zx80_cc_per_line			= 207;					// programmable
uint	const	zx80_lines_before_screen_50	= 56;					// programmable, 50 Hz
uint	const	zx80_lines_in_screen		= 8*24;					// programmable
uint	const	zx80_lines_after_screen_50	= 312-56-8*24;			// programmable, 50 Hz		TODO: verify

uint32	const	zx80_rom_size				= 4 * 1024;
uint32	const	zx80_ram_size				= 1 * 1024;
uint32	const	zx80_page_size				= 1 * 1024;

uint32	const	zx80_tape_load_routine		= 0x0207;				// LOAD-1
uint32	const	zx80_tape_save_routine		= 0x01B6 + 1;			// SAVE + 1		((after pop ret_addr))
uint32	const	zx80_tape_load_ret_addr		= 0x0202;				// used in LOAD and SAVE command!
																	//	• ret addr = 0x01F6 = SAVE
																	//	• ret addr = 0x024B = LOAD



