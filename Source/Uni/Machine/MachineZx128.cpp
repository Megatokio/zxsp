// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineZx128.h"
#include "Ay/AySubclasses.h"
#include "Item.h"
#include "Keyboard.h"
#include "TapeRecorder.h"
#include "Ula/Mmu128k.h"
#include "Ula/Ula128k.h"
#include "Z80/Z80.h"
#include "ZxInfo.h"


namespace zxsp
{

MachineZx128::MachineZx128(IMachineController* m, IScreen* screen, Model model, isa_id id) : //
	MachineZxsp(m, screen, model, id)
{}

MachineZx128::MachineZx128(IMachineController* m, IScreen* screen, Model model) : //
	MachineZxsp(m, screen, model, zx_info[model].id)
{
	assert(model == zx128 || model == zx128_span);
	assert(model_info->id == isa_MachineZx128);

	addItem(new Z80(this));		// must be 1st item
	addItem(new Ula128k(this)); // should be 2nd item
	addItem(new Mmu128k(this));
	addItem(new KeyboardZxPlus(this));
	addItem(new AyForZx128(this));
	addItem(new Walkman(this));
}

} // namespace zxsp
