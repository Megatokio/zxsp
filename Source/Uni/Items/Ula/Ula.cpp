/*	Copyright  (c)	Günter Woigk 1995 - 2018
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

#include "Ula.h"
#include "Machine.h"
#include "Dsp.h"
#include "Keyboard.h"



/*	creator for derived classes
	note: the ula is added like an item with no i/o
		  machine.Input() and Output() always call the ula
		  to add waitstates and floating bus bytes
	note: default Ula has no waitmap wg. B&W ULAs
	note: ula must be 1st item with i/o  (2nd after cpu)
*/
Ula::Ula(Machine* m, isa_id id, cstr o_addr, cstr i_addr)
:
	Crtc(m,id,isa_Ula,internal, o_addr, i_addr),
	ula_out_byte(0),
	beeper_volume(1.0),
	beeper_current_sample(0.0),
	beeper_last_sample_time(0.0)
{
	assert(_prev->isA(isa_Z80));
}

void Ula::powerOn(int32 cc)
{
	Crtc::powerOn(cc);

	ula_out_byte			= 0;
	beeper_current_sample	= 0.0;		// current beeper elongation
	beeper_last_sample_time	= 0.0;
	keymap.clear();
}

void Ula::audioBufferEnd( Time t )
{
	if( t > beeper_last_sample_time )
	{
		Dsp::outputSamples( beeper_current_sample, beeper_last_sample_time, t );
		beeper_last_sample_time = t;
	}
	beeper_last_sample_time -= t;
}

void Ula::setBeeperVolume( Sample new_vol )
{
	if( new_vol> 1.0 ) new_vol =  1.0;
	if( new_vol<-1.0 ) new_vol = -1.0;
	beeper_current_sample *= new_vol / beeper_volume;
	beeper_volume = new_vol;
}

void Ula::saveToFile( FD& fd ) const throws
{
	Crtc::saveToFile(fd);
	TODO();
}

void Ula::loadFromFile( FD& fd ) throws
{
	Crtc::loadFromFile(fd);
	TODO();
}

uint8 Ula::readKeyboard(uint16 addr)
{
	/*	Z80 input: merge in the keys:
		only bits 0-4 come from the keyboard  */

	uint8 byte = 0xff;
	if(keymap.keyPressed())
	{
		uint8* p = &keymap[0];
		uint8 minor = (~addr)>>8;
		while(minor) { if(minor&1) byte &= *p; minor>>=1; p++; }
	}
	return byte;
}

void Ula::setLinesBeforeScreen( int n )
{
	if(lines_before_screen == n) return;
	lines_before_screen = n;
	setupTiming();
}

void Ula::setLinesAfterScreen( int n )
{
	if(lines_after_screen == n) return;
	lines_after_screen = n;
	setupTiming();
}

void Ula::setCcPerLine( int n )
{
	if(cc_per_line == n) return;
	cc_per_line = n;
	setupTiming();
}

void Ula::setBytesPerLine( int n )
{
	setCcPerLine( n * cc_per_byte );
}


















