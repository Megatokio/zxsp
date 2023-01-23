#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Machine.h"
#include "ZxInfo/ZxInfo.h"
#include "ZxInfo/info.h"
#include "Ula/UlaZxsp.h"
#include "Ula/MmuZxsp.h"


class MachineZxsp : public Machine
{
protected:
	MachineZxsp( MachineController*, Model, isa_id id );

	bool	handleSaveTapePatch() override;
	bool	handleLoadTapePatch() override;

public:
	MachineZxsp( MachineController*, Model );

	void    loadScr         (FD& fd) throws override;
	void    saveScr         (FD& fd) throws override;
	void    loadSna         (FD& fd) throws override;
	void    saveSna         (FD& fd) throws override;
	//void	loadTap         (FD& fd) throws override;
};


