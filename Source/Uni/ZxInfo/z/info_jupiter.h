// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


const int  jupiter_basemodel	  = jupiter;
const cstr jupiter_model_name_uk  = "Jupiter Ace";
const cstr jupiter_model_name_us  = "Jupiter Ace 4000";
const cstr jupiter_rom_filename	  = "jupiter.rom";
const cstr jupiter_kbd_filename	  = "jupiter_kbd.png";
const cstr jupiter_image_filename = "jupiter.jpg";

const uint32 jupiter_cc_per_second			= 3250000;
const uint32 jupiter_cc_per_line			= 52 * 4;
const uint	 jupiter_lines_before_screen_50 = 8 * 7;
const uint	 jupiter_lines_in_screen		= 8 * 24;
const uint	 jupiter_lines_after_screen_50	= 8 * 8;

const uint32 jupiter_rom_size  = 8 * 1024;
const uint32 jupiter_ram_size  = 3 * 1024;
const uint32 jupiter_page_size = 1 * 1024;

const uint32 jupiter_tape_load_routine	= 0x0000; // TODO: index in rom[] of start of tape load routine
const uint32 jupiter_tape_save_routine	= 0x0000; // TODO: index in rom[] of start of tape save routine
const uint32 jupiter_tape_load_ret_addr = 0x0000; // TODO: used in LOAD and SAVE command!

const uint32 jupiter_videoram_start = 0;		// video buffer
const uint32 jupiter_videoram_size	= 24 * 32;	//
const uint32 jupiter_charram_start	= 1 * 1024; // character ram
const uint32 jupiter_charram_size	= 128 * 8;	//

// waitmap:		TODO: unknown.
// access to charram is contended depending on used address, else probably snow in video display
const uint32 jupiter_contended_rampages = 0x00; // TODO:	bits/page:  msb=first page to lsb=last page
const uint8	 jupiter_waitmap			= 0x00; // TODO:	bits/cc:    waitmap

const Sample jupiter_earin_threshold = 0.01; // TODO: unknown
