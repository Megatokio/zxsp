// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface3Insp.h"
#include "Multiface/Multiface3.h"
#include <QLabel>


namespace gui
{

Multiface3Insp::Multiface3Insp(QWidget* w, MachineController* mc, volatile Multiface3* i) :
	MultifaceInsp(w, mc, i, "Images/multiface3.jpg", QRect(234, 22, 30, 30)) // red button		x y w h
{
	label_visible = new QLabel("Visible", this);
	label_ramonly = new QLabel("Ram at $0000", this);

	label_visible->move(l_x, l_y + 0 * l_d);
	label_nmi_pending->move(l_x, l_y + 1 * l_d);
	label_paged_in->move(l_x, l_y + 2 * l_d);
	label_ramonly->move(l_x, l_y + 2 * l_d); // same position as paged_in
}


void Multiface3Insp::updateWidgets()
{
	// timer

	xxlogIn("Multiface3Inspector::updateWidgets");
	//assert(validReference(mf3));

	MultifaceInsp::updateWidgets();

	if (label_visible->isVisible() != mf3->isEnabled()) label_visible->setVisible(mf3->isEnabled());
	if (label_ramonly->isVisible() != mf3->isAllRam()) label_ramonly->setVisible(mf3->isAllRam());
}

} // namespace gui
