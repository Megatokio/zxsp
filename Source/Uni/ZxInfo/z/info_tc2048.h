// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	Timex Computer TC2048, Portugal
 */


const int  tc2048_basemodel		 = tc2048;
const cstr tc2048_model_name	 = "Timex TC 2048";
const cstr tc2048_rom_filename	 = "tc2048.rom";
const cstr tc2048_kbd_filename	 = "tc2048_kbd.png";
const cstr tc2048_image_filename = "tc2048.jpg";

const bool tc2048_has_port_7ffd = no;
const bool tc2048_has_port_1ffd = no;
const bool tc2048_has_port_F4	= yes;
const bool tc2048_has_port_FF	= yes;

const bool tc2048_has_ay_soundchip			  = no;
const bool tc2048_has_tape_drive			  = no;
const bool tc2048_has_module_port			  = no;
const bool tc2048_has_floppy_drive			  = no;
const bool tc2048_has_printer_port			  = no;
const uint tc2048_has_serial_ports			  = no;
const uint tc2048_has_joystick_ports		  = 1; // Kempston AFAIK
const bool tc2048_has_kempston_joystick_port  = yes;
const bool tc2048_has_sinclair_joystick_ports = no;

const uint32 tc2048_cc_per_second		= 3528000;
const uint32 tc2048_cc_per_line			= 224; // TODO: verify!
const uint	 tc2048_lines_before_screen = 64;  // TODO: verify!
const uint	 tc2048_lines_in_screen		= 24 * 8;
const uint	 tc2048_lines_after_screen	= 56; // TODO: verify!

const uint32 tc2048_rom_size  = 16 * 1024;
const uint32 tc2048_ram_size  = 48 * 1024;
const uint32 tc2048_page_size = 8 * 1024;

const uint32 tc2048_tape_load_routine  = 0x0556; // index in rom[] of tape load routine		TODO: verify!
const uint32 tc2048_tape_save_routine  = 0x04D0; // index in rom[] of tape save routine		TODO: verify!
const uint32 tc2048_tape_load_ret_addr = 0x053F; // index in rom[] of common exit			TODO: verify!

const uint32 tc2048_contended_rampages =
	0xC0000000;								   // ram#0 ($4000-$7fff) contended	TODO: verify!		(caveat: 8k pages!)
const uint8	 tc2048_waitmap			 = 0x7E;   // %01111110						TODO: verify!
const uint32 tc2048_videoram_start_1 = 0x0000; // biased to ram[] start
const uint32 tc2048_videoram_start_2 = 0x2000; // biased to ram[] start
const uint32 tc2048_videoram_size_1	 = 32 * 24 * (8 + 1);
const uint32 tc2048_videoram_size_2	 = 32 * 24 * (8 + 1);

const Sample tc2048_earin_threshold_mic_lo = 0.01; // earout=0 & micout=0 => non-loading setting		// TODO: verify!
const Sample tc2048_earin_threshold_mic_hi = 0.01; // earout=0 & micout=1 => loadFromTape setting		// TODO: verify!
