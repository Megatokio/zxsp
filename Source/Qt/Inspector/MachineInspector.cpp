// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineInspector.h"
#include "Machine.h"


namespace zxsp
{

MachineInspector::MachineInspector(QWidget* p, MachineController* mc, volatile Machine* m) :
	Inspector(p, mc, m, catstr("Images/", m->model_info->image_filename))
{}

} // namespace zxsp
