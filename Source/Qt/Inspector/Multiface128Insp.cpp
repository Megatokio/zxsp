/*	Copyright  (c)	Günter Woigk 2012 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

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

