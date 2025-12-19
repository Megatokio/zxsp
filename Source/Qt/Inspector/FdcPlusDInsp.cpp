// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*
	MGT Plus D
	Disk interface with snapshot button. 
	Interface 1 compliant. 
	Up to 780k per disk.
*/

#include "FdcPlusDInsp.h"

namespace zxsp
{

FdcPlusDInsp::FdcPlusDInsp(QWidget* w, MachineController* mc, volatile FdcPlusD* i) : Inspector(w, mc, i) {}

} // namespace zxsp
