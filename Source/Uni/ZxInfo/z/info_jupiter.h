/*	Copyright  (c)	Günter Woigk 1994 - 2012
  					mailto:kio@little-bat.de

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 	Permission to use, copy, modify, distribute, and sell this software and
 	its documentation for any purpose is hereby granted without fee, provided
 	that the above copyright notice appear in all copies and that both that
 	copyright notice and this permission notice appear in supporting
 	documentation, and that the name of the copyright holder not be used
 	in advertising or publicity pertaining to distribution of the software
 	without specific, written prior permission.  The copyright holder makes no
 	representations about the suitability of this software for any purpose.
 	It is provided "as is" without express or implied warranty.

 	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 	PERFORMANCE OF THIS SOFTWARE.
*/


int     const   jupiter_basemodel               = jupiter;
cstr	const	jupiter_model_name_uk			= "Jupiter Ace";
cstr	const	jupiter_model_name_us			= "Jupiter Ace 4000";
cstr	const	jupiter_rom_filename			= "jupiter.rom";
cstr	const	jupiter_kbd_filename			= "jupiter_kbd.png";
cstr	const	jupiter_image_filename			= "jupiter.jpg";

uint32	const	jupiter_cc_per_second			= 3250000;
uint32	const	jupiter_cc_per_line				= 52*4;
uint	const	jupiter_lines_before_screen_50	= 8*7;
uint	const	jupiter_lines_in_screen			= 8*24;
uint	const	jupiter_lines_after_screen_50	= 8*8;

uint32	const	jupiter_rom_size				= 8 * 1024;
uint32	const	jupiter_ram_size				= 3 * 1024;
uint32	const	jupiter_page_size				= 1 * 1024;

uint32	const	jupiter_tape_load_routine		= 0x0000;		// TODO: index in rom[] of start of tape load routine
uint32	const	jupiter_tape_save_routine		= 0x0000;		// TODO: index in rom[] of start of tape save routine
uint32	const	jupiter_tape_load_ret_addr		= 0x0000;		// TODO: used in LOAD and SAVE command!

uint32	const	jupiter_videoram_start			= 0;			// video buffer
uint32	const	jupiter_videoram_size			= 24*32;		//
uint32	const	jupiter_charram_start			= 1 * 1024;		// character ram
uint32	const	jupiter_charram_size			= 128*8;		//

// waitmap:		TODO: unknown.
// access to charram is contended depending on used address, else probably snow in video display
uint32	const	jupiter_contended_rampages		= 0x00;			// TODO:	bits/page:  msb=first page to lsb=last page
uint8	const	jupiter_waitmap					= 0x00;			// TODO:	bits/cc:    waitmap

Sample	const	jupiter_earin_threshold			= 0.01;			// TODO: unknown







