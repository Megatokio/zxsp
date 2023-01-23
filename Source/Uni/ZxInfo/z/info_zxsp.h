// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	ZX Spectrum 16/48K:

	issue 1		dflt: grey keys, 16K
	issue 2		dflt: blue keys, 48K
	issue 3		different ear_in level
	issue 4		different ear_in level
	plus		black plastic keyboard;		dflt: issue 4
	span		spanish (not Inves!);		dflt: issue 4 (TODO)
*/


int     const   zxsp_basemodel              = zxsp_i3;
cstr	const	zxsp_model_name_i1			= "ZX Spectrum Issue 1";
cstr	const	zxsp_model_name_i2			= "ZX Spectrum Issue 2";
cstr	const	zxsp_model_name_i3			= "ZX Spectrum Issue 3";
cstr	const	zxsp_model_name_plus		= "ZX Spectrum+";
cstr	const	zxsp_model_name_span		= "ZX Spectrum (Spanish)";

cstr	const	zxsp_rom_filename			= "48k.rom";
cstr	const	zxsp_rom_filename_span		= "plus_span.rom";		// TODO: preliminary - this is the Inves rom!

cstr	const	zxsp_kbd_filename_i1		= "16k_kbd.png";		// grey keys
cstr	const	zxsp_kbd_filename_i23		= "48k_kbd.png";
cstr	const	zxsp_kbd_filename_plus		= "128k_kbd.jpg";		// black plastic keys
cstr	const	zxsp_kbd_filename_span		= "128k_kbd_span.jpg";	// TODO: preliminary - same keyboard as Inves?

cstr	const	zxsp_image_filename_i1		= "16k.jpg";		// grey keys
cstr	const	zxsp_image_filename_i23		= "48k.jpg";
cstr	const	zxsp_image_filename_plus	= "plus.jpg";
cstr	const	zxsp_image_filename_span	= "plus.jpg";		// TODO: spanish image

uint32	const	zxsp_cc_per_second			= 3500000;
uint32	const	zxsp_cc_per_line			= 224;				//  128 T screen, 24 T right border, 48 T hor. retrace, 24 T left border.
uint	const	zxsp_lines_before_screen	= 64;
uint	const	zxsp_lines_in_screen		= 24*8;
uint	const	zxsp_lines_after_screen		= 56;

uint32	const	zxsp_rom_size				= 16 * 1024;
uint32	const	zxsp_ram_size_16k			= 16 * 1024;			// default for Issue 1
uint32	const	zxsp_ram_size_48k			= 48 * 1024;			// default for Issue 2, spätestens ab Issue 3B immer
uint32	const	zxsp_page_size				= 16 * 1024;

uint32	const	zxsp_tape_load_ret_addr		= 0x053F;
uint32	const	zxsp_tape_load_routine		= 0x0556;			// index in rom[] of start of tape load routine
uint32	const	zxsp_tape_save_routine		= 0x04D0;			// index in rom[] of start of tape save routine

uint32	const	zxsp_contended_rampages		= 0x80000000;		// ram#0 ($4000-$7fff) contended
uint8	const	zxsp_waitmap				= 0x7E;				// %01111110		kio tested: ZX48K waitmap start:  warm: 14336+1;  cold: 14336+2
uint32	const	zxsp_videoram_start			= 0x0000;			// biased to ram start
uint32	const	zxsp_videoram_size			= 32*24*(8+1);

Sample	const	zxsp_earin_threshold_mic_lo		= 0.05;			// earout=0 & micout=0 => non-loading setting
Sample	const	zxsp_earin_threshold_mic_hi_i12	= -0.01;		// earout=0 & micout=1 => loadFromTape setting
Sample	const	zxsp_earin_threshold_mic_hi_i3	= 0.01;			// earout=0 & micout=1 => loadFromTape setting
Sample	const	zxsp_earin_threshold_mic_hi_i4	= 0.00;			// Issue 4A: earout=0&micout=1 => loadFromTape setting => rnd noise when no signal










