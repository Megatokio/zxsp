/*	Copyright  (c)	Günter Woigk 2009 - 2019
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

#include <QtGui>
#include <QGridLayout>
#include <QPushButton>
#include "SinclairJoyInsp.h"
#include "Joy/SinclairJoy.h"
#include "Joystick.h"


/*		vstretch	reset
		joysel0		joysel1
		display0	display1
		scanusb		setkeys
*/




SinclairJoyInsp::SinclairJoyInsp(QWidget* w, MachineController* mc, volatile IsaObject *j, cstr img_path )
:
	JoyInsp(w,mc,j,img_path)
{
	assert(object->isA(isa_SinclairJoy));

	if(j->isA(isa_ZxIf2)) return;       // ZxIf2 macht sein eigenes Layout

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10,10,10,5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0,100);
	g->setColumnStretch(0,50);
	g->setColumnStretch(1,50);

	g->addWidget( joystick_selectors[0],1,0 );
	g->addWidget( joystick_selectors[1],1,1 );
	g->addWidget( lineedit_display[0],2,0 );
	g->addWidget( lineedit_display[1],2,1 );

	g->addWidget( button_scan_usb, 3,0, Qt::AlignHCenter|Qt::AlignVCenter );
	g->addWidget( button_set_keys, 3,1, Qt::AlignHCenter|Qt::AlignVCenter );
}



void SinclairJoyInsp::updateWidgets()			// ZX Spectrum +2
{												//	Sinclair 1: %---FUDRL active low oK
	xlogIn("SinclairJoyInsp::updateWidgets");	//	Sinclair 2: %---LRDUF active low oK

	if(!object) return;

	uint8 newstate = joy()->getStateForInspector(0);   // Sinclair 1
	if(newstate!=lineedit_state[0])
	{
		lineedit_state[0] = newstate = SinclairJoy::calcS1ForJoy(newstate);
		lineedit_display[0]->setText(binstr(newstate,"%000FUDRL","%--------"));
	}

	newstate = joy()->getStateForInspector(1);         // Sinclair 2
	if(newstate!=lineedit_state[1])
	{
		lineedit_state[1] = newstate = SinclairJoy::calcS2ForJoy(newstate);
		lineedit_display[1]->setText(binstr(newstate,"%000LRDUF","%--------"));
	}
}















