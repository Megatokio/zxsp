// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#ifndef INFO_ZXPLUS2A_H
#define INFO_ZXPLUS2A_H


/*	ZX Spectrum +2A:
	zxplus2a, zxplus2a_span
*/


const int  zxplus2a_basemodel	   = zxplus2a;
const cstr zxplus2a_model_name	   = "ZX Spectrum +2A";
const cstr zxplus2a_rom_filename   = "plus3_v41.rom";
const cstr zxplus2a_kbd_filename   = "plus3_kbd.png"; // black plastic keys
const cstr zxplus2a_image_filename = "plus2b.jpg";

const cstr zxplus2a_model_name_span		= "ZX Spectrum +2A (Spanish)";
const cstr zxplus2a_rom_filename_span	= "plus3_span.rom";
const cstr zxplus2a_kbd_filename_span	= "plus2_kbd.png"; // black plastic keys	TODO: spanish keyboard
const cstr zxplus2a_image_filename_span = "plus2a_span.jpg";

const bool zxplus2a_has_floppy_drive = no;
const bool zxplus2a_has_tape_drive	 = yes;

// the following is common to all +2A / +3 models:

const bool zxplus2a_has_port_7ffd				= yes;
const bool zxplus2a_has_port_1ffd				= yes;
const bool zxplus2a_has_ay_soundchip			= yes;
const bool zxplus2a_has_module_port				= no;
const bool zxplus2a_has_printer_port			= yes;
const uint zxplus2a_has_serial_ports			= 2;
const uint zxplus2a_has_joystick_ports			= 2;
const bool zxplus2a_has_kempston_joystick_port	= no;
const bool zxplus2a_has_sinclair_joystick_ports = yes;

const uint32 zxplus2a_cc_per_second = 3546900;
const uint32 zxplus2a_cc_per_line = 228; // 128 T screen, 24 T right border, 52 T hor. retrace, 24 T left border, o.Ä.
const uint	 zxplus2a_lines_before_screen = 63; // identical to 32K rom machines
const uint	 zxplus2a_lines_in_screen	  = 24 * 8;
const uint	 zxplus2a_lines_after_screen  = 56;

const uint32 zxplus2a_rom_size	= 64 * 1024;
const uint32 zxplus2a_ram_size	= 128 * 1024;
const uint32 zxplus2a_page_size = 16 * 1024;

const uint32 zxplus2a_tape_load_routine	 = 0xC000 + 0x0556; // index in rom[] of tape load routine
const uint32 zxplus2a_tape_save_routine	 = 0xC000 + 0x04D0; // index in rom[] of tape save routine
const uint32 zxplus2a_tape_load_ret_addr = 0xC000 + 0x053F; // index in rom[] of common exit

const uint32 zxplus2a_contended_rampages = 0x0F000000;	  // ram#4,5,6,7 is contended		(+2A/+3)
const uint8	 zxplus2a_waitmap			 = 0xFE;		  // %11111110
const uint32 zxplus2a_videoram_start_0	 = 5 * 16 * 1024; // biased to ram[] start
const uint32 zxplus2a_videoram_start_1	 = 7 * 16 * 1024; // biased to ram[] start
const uint32 zxplus2a_videoram_size		 = 32 * 24 * (8 + 1);

const Sample zxplus2a_earin_threshold_mic_lo = 0.05; // TODO: verify: earout=0 & micout=0 => non-loading setting
const Sample zxplus2a_earin_threshold_mic_hi = 0.01; // TODO: verify: earout=0 & micout=1 => loadFromTape setting


#endif // INFO_ZXPLUS2A_H
