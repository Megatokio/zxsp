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

#ifndef MMU128K_H
#define MMU128K_H

#include "Mmu.h"



class Mmu128k : public Mmu
{
protected:
	uint8	port_7ffd;

public:
	explicit Mmu128k(Machine*);

VIR void	setMmuLocked	(bool f)					{ setPort7ffd( f ? port_7ffd|0x20 : port_7ffd&~0x20 ); }
VIR uint    getPageC000		() volatile const noexcept	{ return port_7ffd&7; }
VIR uint    getPage8000		() volatile const noexcept	{ return 2; }
VIR uint    getPage4000		() volatile const noexcept	{ return 5; }
VIR uint    getPage0000		() volatile const noexcept	{ return (port_7ffd>>4) & 1; }	// except if ROMDIS
	uint    getVideopage	() volatile const noexcept	{ return port_7ffd&8 ? 7 : 5; }
	bool	port7ffdIsLocked() volatile const noexcept	{ return port_7ffd&0x20; }

protected:
	Mmu128k(Machine*, isa_id, cstr o_addr, cstr i_addr);

	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	//void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;

	// Mmu interface:
	void    romCS(bool active) override;	// from rear-item: daisy chain
	bool	hasPort7ffd() volatile const noexcept override	{ return yes; }
	uint8	getPort7ffd() volatile const override			{ return port_7ffd; }
	void	setPort7ffd(uint8) override;

private:
	void	page_rom_128k();
	void	page_ram_128k();
	void	page_mem_128k();
};


#endif








