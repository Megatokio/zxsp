// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface128Insp.h"
#include <QLabel>
#include "Multiface/Multiface128.h"


Multiface128Insp::Multiface128Insp(QWidget*w, MachineController* mc, volatile IsaObject *i )
:
	MultifaceInsp(w,mc,i,"Images/multiface128.jpg",QRect(220,20,30,30))	// red button		x y w h
{
	label_visibility = new QLabel("Visible",this);

	label_visibility->move(l_x,l_y+0*l_d);
	label_nmi_pending->move(l_x,l_y+1*l_d);
	label_paged_in->move(l_x,l_y+2*l_d);
}


void Multiface128Insp::updateWidgets()
{
	MultifaceInsp::updateWidgets();

	if(label_visibility->isVisible() != multiface128()->mf_enabled)
	{
		label_visibility->setVisible(multiface128()->mf_enabled);
	}
}

