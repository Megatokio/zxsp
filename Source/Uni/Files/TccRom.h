/*	Copyright  (c)	Günter Woigk 2013 - 2018
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

#ifndef TIMEXDOCKCARTRIDGE_H
#define TIMEXDOCKCARTRIDGE_H

#include "kio/kio.h"
#include "Memory.h"
#include "Z80/Z80.h"
class Machine;


enum TccRomId			// must match hashes[]
{
	TccEmpty       = 0,	// hash = 0
	TccExRomRam32k = 0,	// hash = 0
	TccDockRam64k  = 0,	// hash = 0

// these are the known official ones:
	TccAndroids,TccBudgeter,TccCasino1,TccCrazyBugs,TccFlightSimulator,
	TccPinball,TccStatesAndCapitals,TccPenetrator,

// non-releases:
	TccBackgammon,TccChess,TccGyruss,TccHoraceAndTheSpiders,
	TccHungryHorace,TccLocoMotion,TccMontezumasRevenge,TccPlanetoids,
	TccPopeye,TccQBert,TccReturnOfTheJediDeathStarBattle,
	TccShadowOfTheUnicorn,TccSpaceRaiders,TccStarWarsTheArcadeGame,
	TccSwordfight,

// utilities etc.:
	TccEToolkit,TccMTerm,TccRwp32,TccSuperHotZDisassemblerV251,
	TccTaswordII,TccTimeWord,TccVuCalc,TccVuFile,TccZebraOS64,
	TccTc2048Emu,TccJupiterAceEmu,TccZxseEmu,TccZxspEmu,

	TccUnknown
};


class TccRom
{
	friend class MmuTc2068;

private:
	Machine* machine;
	cstr	fpath;
	TccRomId id;

	MemoryPtr	exrom[8];
	MemoryPtr	dock[8];
	uint8*	rom;

	uint8	exrom_r, exrom_w, exrom_d;	// bit mask: EXROM bank readable? / writeable? / data supplied?
	uint8	dock_r,  dock_w,  dock_d;	// bit mask: DOCK  bank readable? / writeable? / data supplied?
	uint8	home_r,  home_w,  home_d;	// bit mask: HOME  bank readable? / writeable? / data supplied?

public:
	TccRom(Machine*, cstr path);
	~TccRom();

	bool	isZxspEmu()					{ return id == TccZxspEmu; }
	TccRomId getTccId()					{ return id; }
	cstr	getFilepath()				{ return fpath; }
	void	saveAs(cstr fpath);

private:
	TccRom(const TccRom&) = delete;
	TccRom& operator= (const TccRom&) = delete;

	void	save_as(cstr fpath) throws;
};


#endif















