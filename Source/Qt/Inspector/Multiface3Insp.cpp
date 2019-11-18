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

#include "Multiface3Insp.h"
#include <QLabel>
#include "Multiface/Multiface3.h"


Multiface3Insp::Multiface3Insp(QWidget*w, MachineController* mc, volatile IsaObject *i )
:
	MultifaceInsp(w,mc,i,"Images/multiface3.jpg",QRect(234,22,30,30))	// red button		x y w h
{
	label_visible = new QLabel("Visible",this);
	label_ramonly = new QLabel("Ram at $0000",this);


	label_visible->move(l_x,l_y+0*l_d);
	label_nmi_pending->move(l_x,l_y+1*l_d);
	label_paged_in->move(l_x,l_y+2*l_d);
	label_ramonly->move(l_x,l_y+2*l_d);		// same position as paged_in
}


void Multiface3Insp::updateWidgets()
{
	MultifaceInsp::updateWidgets();

	if(label_visible->isVisible() != multiface3()->mf_enabled)
	{
		label_visible->setVisible(multiface3()->mf_enabled);
	}

	if(label_ramonly->isVisible() != multiface3()->all_ram)
	{
		label_ramonly->setVisible(multiface3()->all_ram);
	}
}
