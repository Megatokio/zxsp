// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	ZX Spectrum plus2:
	zxplus2, zxplus2_span, zxplus2_french
*/


const int  zxplus2_basemodel	  = zx128;
const cstr zxplus2_model_name	  = "ZX Spectrum +2";
const cstr zxplus2_rom_filename	  = "plus2.rom";
const cstr zxplus2_kbd_filename	  = "plus2_kbd.png"; // grey plastic keys
const cstr zxplus2_image_filename = "plus2.jpg";

const cstr zxplus2_model_name_span	   = "Sinclair ZX Spectrum +2 (Spanish)";
const cstr zxplus2_rom_filename_span   = "plus2_span.rom";
const cstr zxplus2_kbd_filename_span   = "plus2_kbd.png"; // TODO: spanish +2 keyboard
const cstr zxplus2_image_filename_span = "plus2.jpg";	  // TODO: spanish +2 image

const cstr zxplus2_model_name_frz	  = "Sinclair ZX Spectrum +2 (French)";
const cstr zxplus2_rom_filename_frz	  = "plus2_frz.rom";
const cstr zxplus2_kbd_filename_frz	  = "plus2_kbd.png"; // TODO: french +2 keyboard
const cstr zxplus2_image_filename_frz = "plus2.jpg";	 // TODO: french +2 image

const bool zxplus2_has_port_7ffd			   = yes;
const bool zxplus2_has_port_1ffd			   = no;
const bool zxplus2_has_ay_soundchip			   = yes;
const bool zxplus2_has_tape_drive			   = yes;
const bool zxplus2_has_module_port			   = no;
const bool zxplus2_has_floppy_drive			   = no;
const bool zxplus2_has_printer_port			   = no;
const uint zxplus2_has_serial_ports			   = 2;
const uint zxplus2_has_joystick_ports		   = 2;
const bool zxplus2_has_kempston_joystick_port  = no;
const bool zxplus2_has_sinclair_joystick_ports = yes;

const uint32 zxplus2_cc_per_second = 3546900;
const uint32 zxplus2_cc_per_line   = 228; // 128 T screen, 24 T right border, 52 T hor. retrace, 24 T left border, o.Ä.
const uint	 zxplus2_lines_before_screen = 63;
const uint	 zxplus2_lines_in_screen	 = 24 * 8;
const uint	 zxplus2_lines_after_screen	 = 56;

const uint32 zxplus2_rom_size  = 32 * 1024;
const uint32 zxplus2_ram_size  = 128 * 1024;
const uint32 zxplus2_page_size = 16 * 1024;

const uint32 zxplus2_tape_load_routine	= 0x4000 + 0x0556; // index in rom[] of tape load routine
const uint32 zxplus2_tape_save_routine	= 0x4000 + 0x04D0; // index in rom[] of tape save routine
const uint32 zxplus2_tape_load_ret_addr = 0x4000 + 0x053F; // index in rom[] of common exit

const uint32 zxplus2_contended_rampages = 0x55000000; // ram#1,3,5,7 is contended		(128/+2)
const uint8	 zxplus2_waitmap =
	0xFC; // %11111100		kio tested in 48k mode on cold ZX128. timing on warm machine unstable
const uint32 zxplus2_videoram_start_0 = 16 kB * 5; // biased to ram[] start
const uint32 zxplus2_videoram_start_1 = 16 kB * 7; // biased to ram[] start
const uint32 zxplus2_videoram_size	  = 32 * 24 * (8 + 1);

const Sample zxplus2_earin_threshold_mic_lo = 0.05; // PCB 6U:	earout=0 & micout=0 => non-loading setting
const Sample zxplus2_earin_threshold_mic_hi = 0.01; //			earout=0 & micout=1 => loadFromTape setting
