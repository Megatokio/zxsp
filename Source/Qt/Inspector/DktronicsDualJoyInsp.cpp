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

#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>
#include "DktronicsDualJoyInsp.h"
#include "Item.h"

DktronicsDualJoyInsp::DktronicsDualJoyInsp(QWidget* w, MachineController* mc, volatile IsaObject* item )
:
	JoyInsp(w,mc,item,"/Images/dktronics_dual_js_if.jpg")
{
	assert(item->isA(isa_DktronicsDualJoy));

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
