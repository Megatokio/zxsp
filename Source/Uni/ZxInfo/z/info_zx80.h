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



int     const   zx80_basemodel              = zx80;
cstr	const	zx80_model_name				= "Sinclair ZX80";
cstr	const	zx80_rom_filename			= "zx80.rom";
cstr	const	zx80_kbd_filename			= "zx80_kbd.png";
cstr	const	zx80_image_filename			= "zx80.jpg";

uint32	const	zx80_cc_per_second			= 3250000;
uint32	const	zx80_cc_per_line			= 207;					// programmable
uint	const	zx80_lines_before_screen_50	= 56;					// programmable, 50 Hz
uint	const	zx80_lines_in_screen		= 8*24;					// programmable
uint	const	zx80_lines_after_screen_50	= 312-56-8*24;			// programmable, 50 Hz		TODO: verify

uint32	const	zx80_rom_size				= 4 * 1024;
uint32	const	zx80_ram_size				= 1 * 1024;
uint32	const	zx80_page_size				= 1 * 1024;

uint32	const	zx80_tape_load_routine		= 0x0207;				// LOAD-1
uint32	const	zx80_tape_save_routine		= 0x01B6 + 1;			// SAVE + 1		((after pop ret_addr))
uint32	const	zx80_tape_load_ret_addr		= 0x0202;				// used in LOAD and SAVE command!
																	//	• ret addr = 0x01F6 = SAVE
																	//	• ret addr = 0x024B = LOAD



