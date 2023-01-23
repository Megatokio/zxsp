// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


// Inves Spectrum+
// Unlicensed Spanish clone

int     const   inves_basemodel              = zxsp_i3;
cstr	const	inves_model_name			= "Inves Spectrum+ (Spanish)";
cstr	const	inves_rom_filename			= "plus_span.rom";
cstr	const	inves_kbd_filename			= "128k_kbd_span.jpg";
cstr	const	inves_image_filename		= "plus_span.jpg";

uint32	const	inves_cc_per_second			= 17734500/5;	// XTAL is 17.7345 = 5 * 3546900 Hz.	TODO: get info!
uint32	const	inves_cc_per_line			= 57*4;			// cc/frame = 70908 [tested: 2007-06-04 kio]
															// 70908 = (311 lines) * (57*4 cc_per_line)
uint	const	inves_lines_before_screen	= 63;			// TODO: verify		311 lines = 63 above + 192 screen + 56 below
uint	const	inves_lines_in_screen		= 24*8;			// TODO: verify		therse values are as for 128K models
uint	const	inves_lines_after_screen	= 56;			// TODO: verify

uint	const	inves_has_joystick_ports	= 1;
bool	const	inves_has_kempston_joystick_port	= yes;
bool	const	inves_has_sinclair_joystick_ports	= no;

uint32	const	inves_rom_size				= 16 * 1024;
uint32	const	inves_ram_size				= 48 * 1024;		// TODO: actually 64K, but 16K probably not accessible
uint32	const	inves_page_size				= 16 * 1024;

uint32	const	inves_tape_load_routine		= 0x0556;		// index in rom[]	TODO: verify
uint32	const	inves_tape_save_routine		= 0x04D0;		// index in rom[]	TODO: verify
uint32	const	inves_tape_load_ret_addr	= 0x053F;

// TODO: verify: Inves: probably no videoram contendion
//uint32 const	inves_contended_rampages	= 0x00000000;
//uint8	 const	inves_waitmap				= 0x00;

uint32	const	inves_videoram_start		= 0x0000;		// biased to ram[] start
uint32	const	inves_videoram_size			= 32*24*(8+1);

Sample	const	inves_earin_threshold		= 0.01;			// TODO: verify













