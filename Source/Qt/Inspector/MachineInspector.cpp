// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineInspector.h"
#include "Machine.h"


namespace gui
{

MachineInspector::MachineInspector(QWidget* p, MachineController* mc, volatile Machine* m) :
	Inspector(p, mc, m, catstr("Images/", m->model_info->image_filename))
{}

} // namespace gui
