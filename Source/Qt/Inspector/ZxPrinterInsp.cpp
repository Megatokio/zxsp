// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZxPrinterInsp.h"

namespace zxsp
{

ZxPrinterInsp::ZxPrinterInsp(QWidget* w, MachineController* mc, volatile ZxPrinter* i) :
	Inspector(w, mc, i, "/Backgrounds/light-150-s.jpg")
{}

} // namespace zxsp
