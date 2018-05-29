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



int     const   zx81_basemodel              = zx81;
cstr	const	zx81_model_name_uk			= "Sinclair ZX81";
cstr	const	zx81_model_name_us			= "Timex 1000";
cstr	const	zx81_rom_filename			= "zx81.rom";
cstr	const	zx81_kbd_filename			= "zx81_kbd.png";		// TODO: variants: "New Line" vs. "Return"
cstr	const	zx81_image_filename			= "zx81.jpg";			// TODO: variants

uint32	const	zx81_cc_per_second			= 3250000;
uint32	const	zx81_cc_per_line			= 207;					// TODO: verify
uint	const	zx81_lines_before_screen_50	= 56;					// TODO: verify; programmable, 50 Hz
uint	const	zx81_lines_in_screen		= 8*24;					// TODO: verify; programmable
uint	const	zx81_lines_after_screen_50	= 312-56-8*24;			// TODO: verify; programmable, 50 Hz

uint32	const	zx81_rom_size				= 8 * 1024;
uint32	const	zx81_ram_size_uk			= 1 * 1024;
uint32	const	zx81_ram_size_us			= 2 * 1024;
uint32	const	zx81_page_size				= 1 * 1024;

#if 0	// version 1 'standard' rom
uint32	const	zx81_tape_load_routine		= 0x0340;		?		// index in rom[] of start of tape load routine
uint32	const	zx81_tape_save_routine		= 0x02FC;		?		// index in rom[] of start of tape save routine
uint32	const	zx81_tape_load_ret_addr		= 0x0206;		?		// SLOW_FAST
															?		//	• ret addr = 0x031C = SAVE
															?		//	• ret addr = 0x0383 = LOAD
#else	// version 2 'improved' rom
uint32	const	zx81_tape_load_routine		= 0x0340 + 7;			// NEXT_PROG
uint32	const	zx81_tape_save_routine		= 0x02F6 + 6;			// SAVE + 6 ((after ex hl,de))
uint32	const	zx81_tape_load_ret_addr		= 0x0207 + 3;			// SLOW_FAST + 3 ((we use next instr; $0207 is also tape load entry on ZX80))
																	//	• ret addr = 0x031C? = SAVE
																	//	• ret addr = 0x0383? = LOAD
#endif



