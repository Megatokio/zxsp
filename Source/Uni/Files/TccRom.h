#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Memory.h"
#include "Z80/Z80.h"
#include "kio/kio.h"
class Machine;


enum TccRomId // must match hashes[]
{
	TccEmpty	   = 0, // hash = 0
	TccExRomRam32k = 0, // hash = 0
	TccDockRam64k  = 0, // hash = 0

	// these are the known official ones:
	TccAndroids,
	TccBudgeter,
	TccCasino1,
	TccCrazyBugs,
	TccFlightSimulator,
	TccPinball,
	TccStatesAndCapitals,
	TccPenetrator,

	// non-releases:
	TccBackgammon,
	TccChess,
	TccGyruss,
	TccHoraceAndTheSpiders,
	TccHungryHorace,
	TccLocoMotion,
	TccMontezumasRevenge,
	TccPlanetoids,
	TccPopeye,
	TccQBert,
	TccReturnOfTheJediDeathStarBattle,
	TccShadowOfTheUnicorn,
	TccSpaceRaiders,
	TccStarWarsTheArcadeGame,
	TccSwordfight,

	// utilities etc.:
	TccEToolkit,
	TccMTerm,
	TccRwp32,
	TccSuperHotZDisassemblerV251,
	TccTaswordII,
	TccTimeWord,
	TccVuCalc,
	TccVuFile,
	TccZebraOS64,
	TccTc2048Emu,
	TccJupiterAceEmu,
	TccZxseEmu,
	TccZxspEmu,

	TccUnknown
};


class TccRom
{
	friend class MmuTc2068;

private:
	Machine* machine;
	cstr	 fpath;
	TccRomId id;

	MemoryPtr exrom[8];
	MemoryPtr dock[8];
	uint8*	  rom;

	uint8 exrom_r, exrom_w, exrom_d; // bit mask: EXROM bank readable? / writeable? / data supplied?
	uint8 dock_r, dock_w, dock_d;	 // bit mask: DOCK  bank readable? / writeable? / data supplied?
	uint8 home_r, home_w, home_d;	 // bit mask: HOME  bank readable? / writeable? / data supplied?

public:
	TccRom(Machine*, cstr path);
	~TccRom();

	bool	 isZxspEmu() { return id == TccZxspEmu; }
	TccRomId getTccId() { return id; }
	cstr	 getFilepath() { return fpath; }
	void	 saveAs(cstr fpath);

private:
	TccRom(const TccRom&)			 = delete;
	TccRom& operator=(const TccRom&) = delete;

	void save_as(cstr fpath);
};
