/*	Copyright  (c)	Günter Woigk 2009 - 2018
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

#ifndef MMUTC2068_H
#define MMUTC2068_H

#include "MmuTc2048.h"
#include "Files/TccRom.h"


class MmuTc2068 : public MmuTc2048
{
	TccRom*	cartridge;
	bool	exrom_selected;

	// bank controller registers:
	uint8	hold;
	uint8	bna;
	uint8	port_FD;		// bank ctrl. cmd port

public:
	explicit MmuTc2068(Machine*, isa_id=isa_MmuTc2068);
	virtual ~MmuTc2068();

	void	selectBusExpansionUnit(bool);

	TccRom*	getCartridge	()					{ return cartridge; }
	void	ejectCartridge	();
	void	insertCartridge	(cstr filepath);
	void	saveCartridgeAs	(cstr fpath)		{ if(cartridge) cartridge->saveAs(fpath); }
	bool	isLoaded		() volatile const	{ return cartridge!=NULL; }

	bool	isZxspEmu()   volatile const{ assert(isMainThread()); return cartridge?cartridge->isZxspEmu():no; }
	TccRomId getTccId() volatile const{ assert(isMainThread()); return cartridge?cartridge->getTccId():TccUnknown; }
	cstr	getFilepath() volatile const{ assert(isMainThread()); return cartridge?cartridge->getFilepath():NULL; }

protected:
// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	//void	saveToFile		(FD&) const throws override;
	//void	loadFromFile	(FD&) throws override;

// Mmu interface:
	//bool	hasPortF4() volatile const noexcept	override { return yes; }
	//uint8	getPortF4() override						{ return port_F4; }
	void	setPortF4(uint8 n) override					{ set_port_f4(n,0xff); }
	void	romCS(bool disable) override;				// from rear-item: daisy chain

// MmuTc2048 interface:
	void	selectEXROM(bool) override;

private:
	void	set_port_f4(uint8,uint8);
};


#endif


















