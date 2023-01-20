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


#include <QLabel>
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>
#include "Tk85JoyInsp.h"

Tk85JoyInsp::Tk85JoyInsp(QWidget*p, MachineController* mc, volatile IsaObject *j)
:
	JoyInsp(p,mc,j,"/Images/tk85_joy.jpg")
{
	assert(object->isA(isa_Tk85Joy));

	QLabel* label= new QLabel("Buttons:");

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10,10,10,5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0,100);
	g->setColumnStretch(0,25);
	g->setColumnStretch(1,25);
	g->setColumnStretch(2,50);

	g->addWidget( joystick_selectors[0],1,0, 1,2 );
	g->addWidget( button_set_keys, 1,2, Qt::AlignHCenter|Qt::AlignVCenter );
	g->addWidget( label,2,0);
	g->addWidget( lineedit_display[0],2,1 );
	g->addWidget( button_scan_usb, 2,2, Qt::AlignHCenter|Qt::AlignVCenter );
}
