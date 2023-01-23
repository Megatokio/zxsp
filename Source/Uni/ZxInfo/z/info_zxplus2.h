// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	ZX Spectrum plus2:
	zxplus2, zxplus2_span, zxplus2_french
*/


int     const   zxplus2_basemodel           = zx128;
cstr	const	zxplus2_model_name			= "ZX Spectrum +2";
cstr	const	zxplus2_rom_filename		= "plus2.rom";
cstr	const	zxplus2_kbd_filename		= "plus2_kbd.png";		// grey plastic keys
cstr	const	zxplus2_image_filename		= "plus2.jpg";

cstr	const	zxplus2_model_name_span		= "Sinclair ZX Spectrum +2 (Spanish)";
cstr	const	zxplus2_rom_filename_span	= "plus2_span.rom";
cstr	const	zxplus2_kbd_filename_span	= "plus2_kbd.png";		// TODO: spanish +2 keyboard
cstr	const	zxplus2_image_filename_span	= "plus2.jpg";			// TODO: spanish +2 image

cstr	const	zxplus2_model_name_frz		= "Sinclair ZX Spectrum +2 (French)";
cstr	const	zxplus2_rom_filename_frz	= "plus2_frz.rom";
cstr	const	zxplus2_kbd_filename_frz	= "plus2_kbd.png";		// TODO: french +2 keyboard
cstr	const	zxplus2_image_filename_frz	= "plus2.jpg";			// TODO: french +2 image

bool	const	zxplus2_has_port_7ffd		= yes;
bool	const	zxplus2_has_port_1ffd		= no;
bool	const	zxplus2_has_ay_soundchip	= yes;
bool	const	zxplus2_has_tape_drive		= yes;
bool	const	zxplus2_has_module_port		= no;
bool	const	zxplus2_has_floppy_drive	= no;
bool	const	zxplus2_has_printer_port	= no;
uint	const	zxplus2_has_serial_ports	= 2;
uint	const	zxplus2_has_joystick_ports	= 2;
bool	const	zxplus2_has_kempston_joystick_port	= no;
bool	const	zxplus2_has_sinclair_joystick_ports	= yes;

uint32	const	zxplus2_cc_per_second		= 3546900;
uint32	const	zxplus2_cc_per_line			= 228;				// 128 T screen, 24 T right border, 52 T hor. retrace, 24 T left border, o.Ä.
uint	const	zxplus2_lines_before_screen	= 63;
uint	const	zxplus2_lines_in_screen		= 24*8;
uint	const	zxplus2_lines_after_screen	= 56;

uint32	const	zxplus2_rom_size			= 32 * 1024;
uint32	const	zxplus2_ram_size			= 128 * 1024;
uint32	const	zxplus2_page_size			= 16 * 1024;

uint32	const	zxplus2_tape_load_routine	= 0x4000 + 0x0556;	// index in rom[] of tape load routine
uint32	const	zxplus2_tape_save_routine	= 0x4000 + 0x04D0;	// index in rom[] of tape save routine
uint32	const	zxplus2_tape_load_ret_addr	= 0x4000 + 0x053F;	// index in rom[] of common exit

uint32	const	zxplus2_contended_rampages	= 0x55000000;		// ram#1,3,5,7 is contended		(128/+2)
uint8	const	zxplus2_waitmap				= 0xFC;				// %11111100		kio tested in 48k mode on cold ZX128. timing on warm machine unstable
uint32	const	zxplus2_videoram_start_0	= 16 kB * 5;		// biased to ram[] start
uint32	const	zxplus2_videoram_start_1	= 16 kB * 7;		// biased to ram[] start
uint32	const	zxplus2_videoram_size		= 32*24*(8+1);

Sample	const	zxplus2_earin_threshold_mic_lo = 0.05;			// PCB 6U:	earout=0 & micout=0 => non-loading setting
Sample	const	zxplus2_earin_threshold_mic_hi = 0.01;			//			earout=0 & micout=1 => loadFromTape setting




















