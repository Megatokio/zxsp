// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Machine.h"


namespace zxsp
{

class MachineZxsp : public Machine
{
protected:
	MachineZxsp(IMachineController*, IScreen*, Model, isa_id id);

	bool handleSaveTapePatch() override;
	bool handleLoadTapePatch() override;

public:
	MachineZxsp(IMachineController*, IScreen*, Model);

	void loadScr(FD& fd) override;
	void saveScr(FD& fd) override;
	void loadSna(FD& fd) override;
	void saveSna(FD& fd) override;
	// void	loadTap         (FD& fd) override;
};

} // namespace zxsp
