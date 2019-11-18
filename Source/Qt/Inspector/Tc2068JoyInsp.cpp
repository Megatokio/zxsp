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

#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>
#include "Tc2068JoyInsp.h"
#include "Joy/Tc2068Joy.h"
#include "Ay/AySubclasses.h"


Tc2068JoyInsp::Tc2068JoyInsp(QWidget* w, MachineController* mc, volatile IsaObject *j, cstr img_path)
:
	JoyInsp(w,mc,j,img_path)
{
	assert(j->isA(isa_Tc2068Joy));

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10,10,10,5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0,100);
	g->setColumnStretch(0,50);
	g->setColumnStretch(1,50);

	g->addWidget( joystick_selectors[1],1,0 );
	g->addWidget( joystick_selectors[0],1,1 );
	g->addWidget( lineedit_display[1],2,0 );
	g->addWidget( lineedit_display[0],2,1 );

	g->addWidget( button_scan_usb, 3,0, Qt::AlignHCenter|Qt::AlignVCenter );
	g->addWidget( button_set_keys, 3,1, Qt::AlignHCenter|Qt::AlignVCenter );
}


void Tc2068JoyInsp::updateWidgets()
{
	xlogIn("Tc2068JoyInsp::updateWidgets");
	if(!object) return;

	for(int i=0;i<num_ports;i++)
	{
		uint8 newstate = AyForTc2068::ayByteForJoystickByte(joy()->getStateForInspector(i));
		if(newstate==lineedit_state[i]) continue;
		lineedit_display[i]->setText(binstr(newstate,"%F000RLDU","%--------"));
		lineedit_state[i] = newstate;
	}
}


