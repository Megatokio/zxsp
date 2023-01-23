// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	ZX Spectrum+ 128K:
	zx128, zx128_span
*/


int     const   zx128_basemodel             = zx128;
cstr	const	zx128_model_name			= "Sinclair ZX Spectrum+ 128K";
cstr	const	zx128_rom_filename			= "128k.rom";
cstr	const	zx128_kbd_filename			= "128k_kbd.jpg";		// black plastic keys
cstr	const	zx128_image_filename		= "128k.jpg";

cstr	const	zx128_model_name_span		= "Sinclair ZX Spectrum+ 128K (Spanish)";
cstr	const	zx128_rom_filename_span		= "128k_span.rom";
cstr	const	zx128_kbd_filename_span		= "128k_kbd_span.jpg";	// black plastic keys
cstr	const	zx128_image_filename_span	= "128k_span.jpg";

bool	const	zx128_has_port_7ffd			= yes;
bool	const	zx128_has_port_1ffd			= no;
bool	const	zx128_has_ay_soundchip		= yes;
bool	const	zx128_has_tape_drive		= no;
bool	const	zx128_has_module_port		= no;
bool	const	zx128_has_floppy_drive		= no;
bool	const	zx128_has_printer_port		= no;
uint	const	zx128_has_serial_ports		= 2;
uint	const	zx128_has_joystick_ports	= 0;
bool	const	zx128_has_kempston_joystick_port	= no;
bool	const	zx128_has_sinclair_joystick_ports	= no;

uint32	const	zx128_cc_per_second			= 3546900;
uint32	const	zx128_cc_per_line			= 228;				// 128 T screen, 24 T right border, 52 T hor. retrace, 24 T left border, o.Ä.
uint	const	zx128_lines_before_screen	= 63;
uint	const	zx128_lines_in_screen		= 24*8;
uint	const	zx128_lines_after_screen	= 56;

uint32	const	zx128_rom_size				= 32 * 1024;
uint32	const	zx128_ram_size				= 128 * 1024;
uint32	const	zx128_page_size				= 16 * 1024;

uint32	const	zx128_tape_load_routine		= 0x4000 + 0x0556;	// index in rom[] of tape load routine
uint32	const	zx128_tape_save_routine		= 0x4000 + 0x04D0;	// index in rom[] of tape save routine
uint32	const	zx128_tape_load_ret_addr	= 0x4000 + 0x053F;	// index in rom[] of common exit

uint32	const	zx128_contended_rampages	= 0x55000000;		// ram#1,3,5,7 is contended		(128/+2)
uint8	const	zx128_waitmap				= 0xFC;				// %11111100		kio tested: in 48k mode on cold ZX128. timing on warm machine unstable
uint32	const	zx128_videoram_start_0		= 16 * 1024 * 5;		// biased to ram[] start
uint32	const	zx128_videoram_start_1		= 16 * 1024 * 7;		// biased to ram[] start
uint32	const	zx128_videoram_size			= 32*24*(8+1);

Sample	const	zx128_earin_threshold_mic_lo = 0.05;			// PCB 6U:	earout=0 & micout=0 => non-loading setting
Sample	const	zx128_earin_threshold_mic_hi = 0.01;			//			earout=0 & micout=1 => loadFromTape setting



















