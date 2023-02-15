// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	ZX Spectrum+ 128K:
	zx128, zx128_span
*/


const int  zx128_basemodel		= zx128;
const cstr zx128_model_name		= "Sinclair ZX Spectrum+ 128K";
const cstr zx128_rom_filename	= "128k.rom";
const cstr zx128_kbd_filename	= "128k_kbd.jpg"; // black plastic keys
const cstr zx128_image_filename = "128k.jpg";

const cstr zx128_model_name_span	 = "Sinclair ZX Spectrum+ 128K (Spanish)";
const cstr zx128_rom_filename_span	 = "128k_span.rom";
const cstr zx128_kbd_filename_span	 = "128k_kbd_span.jpg"; // black plastic keys
const cstr zx128_image_filename_span = "128k_span.jpg";

const bool zx128_has_port_7ffd				 = yes;
const bool zx128_has_port_1ffd				 = no;
const bool zx128_has_ay_soundchip			 = yes;
const bool zx128_has_tape_drive				 = no;
const bool zx128_has_module_port			 = no;
const bool zx128_has_floppy_drive			 = no;
const bool zx128_has_printer_port			 = no;
const uint zx128_has_serial_ports			 = 2;
const uint zx128_has_joystick_ports			 = 0;
const bool zx128_has_kempston_joystick_port	 = no;
const bool zx128_has_sinclair_joystick_ports = no;

const uint32 zx128_cc_per_second = 3546900;
const uint32 zx128_cc_per_line	 = 228; // 128 T screen, 24 T right border, 52 T hor. retrace, 24 T left border, o.Ä.
const uint	 zx128_lines_before_screen = 63;
const uint	 zx128_lines_in_screen	   = 24 * 8;
const uint	 zx128_lines_after_screen  = 56;

const uint32 zx128_rom_size	 = 32 * 1024;
const uint32 zx128_ram_size	 = 128 * 1024;
const uint32 zx128_page_size = 16 * 1024;

const uint32 zx128_tape_load_routine  = 0x4000 + 0x0556; // index in rom[] of tape load routine
const uint32 zx128_tape_save_routine  = 0x4000 + 0x04D0; // index in rom[] of tape save routine
const uint32 zx128_tape_load_ret_addr = 0x4000 + 0x053F; // index in rom[] of common exit

const uint32 zx128_contended_rampages = 0x55000000; // ram#1,3,5,7 is contended		(128/+2)
const uint8	 zx128_waitmap =
	0xFC; // %11111100		kio tested: in 48k mode on cold ZX128. timing on warm machine unstable
const uint32 zx128_videoram_start_0 = 16 * 1024 * 5; // biased to ram[] start
const uint32 zx128_videoram_start_1 = 16 * 1024 * 7; // biased to ram[] start
const uint32 zx128_videoram_size	= 32 * 24 * (8 + 1);

const Sample zx128_earin_threshold_mic_lo = 0.05; // PCB 6U:	earout=0 & micout=0 => non-loading setting
const Sample zx128_earin_threshold_mic_hi = 0.01; //			earout=0 & micout=1 => loadFromTape setting
