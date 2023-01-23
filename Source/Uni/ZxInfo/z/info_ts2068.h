// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	Timex Sinclair TS2086, US
*/


int     const   ts2068_basemodel            = ts2068;
cstr	const	ts2068_model_name			= "Timex TS 2068";
cstr	const	ts2068_rom_filename			= "ts2068.rom";
cstr	const	ts2068_kbd_filename			= "ts2068_kbd.png";
cstr	const	ts2068_image_filename		= "ts2068.jpg";

bool	const	ts2068_has_port_7ffd		= no;
bool	const	ts2068_has_port_1ffd		= no;
bool	const	ts2068_has_port_F4			= yes;
bool	const	ts2068_has_port_FF			= yes;

bool	const	ts2068_has_ay_soundchip		= yes;				// but different port
bool	const	ts2068_has_tape_drive		= no;
bool	const	ts2068_has_module_port		= yes;
bool	const	ts2068_has_floppy_drive		= no;
bool	const	ts2068_has_printer_port		= no;
uint	const	ts2068_has_serial_ports		= no;
uint	const	ts2068_has_joystick_ports	= 2;				// connected to AY chip
bool	const	ts2068_has_kempston_joystick_port	= no;
bool	const	ts2068_has_sinclair_joystick_ports	= no;

uint32	const	ts2068_ay_cycles_per_second	= 1764750;          // WoS
uint32	const	ts2068_cc_per_second		= 3528000;          // WoS
uint32	const	ts2068_cc_per_line			= 224;				//			TODO: verify!
uint	const	ts2068_lines_before_screen	= 34;				// 60 Hz	TODO: verify!
uint	const	ts2068_lines_in_screen		= 24*8;				//
uint	const	ts2068_lines_after_screen	= 34;				//			TODO: verify!

uint32	const	ts2068_rom_size				= 24 * 1024;
uint32	const	ts2068_ram_size				= 48 * 1024;
uint32	const	ts2068_page_size			= 8 * 1024;

uint32	const	ts2068_tape_load_routine	= 0x0556;			// index in rom[] of tape load routine		TODO: verify!
uint32	const	ts2068_tape_save_routine	= 0x04D0;			// index in rom[] of tape save routine		TODO: verify!
uint32	const	ts2068_tape_load_ret_addr	= 0x053F;			// index in rom[] of common exit			TODO: verify!

uint32	const	ts2068_contended_rampages	= 0xC0000000;		// ram#0 ($4000-$7fff) contended	TODO: verify!		(caveat: 8k pages!)
uint8	const	ts2068_waitmap				= 0x7E;				// %01111110						TODO: verify!
uint32	const	ts2068_videoram_start_1		= 0x0000;			// biased to ram[] start
uint32	const	ts2068_videoram_start_2		= 0x2000;			// biased to ram[] start
uint32	const	ts2068_videoram_size_1		= 32*24*(8+1);
uint32	const	ts2068_videoram_size_2		= 32*24*(8+1);

Sample	const	ts2068_earin_threshold_mic_lo = 0.01;			// earout=0 & micout=0 => non-loading setting		// TODO: verify!
Sample	const	ts2068_earin_threshold_mic_hi = 0.01;			// earout=0 & micout=1 => loadFromTape setting		// TODO: verify!























