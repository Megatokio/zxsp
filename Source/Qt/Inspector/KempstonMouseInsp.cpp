/*	Copyright  (c)	Günter Woigk 2006 - 2019
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
#include "KempstonMouseInsp.h"
#include "KempstonMouse.h"
#include "Mouse.h"
#include "Machine.h"
#include "MachineController.h"
#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include "Templates/NVPtr.h"


KempstonMouseInsp::KempstonMouseInsp(QWidget* w, MachineController* mc, volatile IsaObject *i )
:
	Inspector(w,mc,i,"/Images/kempston_mouse_if.jpg"),
	display_x(newLineEdit("0",32)),
	display_y(newLineEdit("0",32)),
	display_buttons(newLineEdit("%------11",100)),
	combobox_scale(new QComboBox()),
	button_grab_mouse(new QPushButton("Grab mouse",this)),
	old_x(0),
	old_y(0),
	old_buttons(0xff),
	old_grabbed(no)
{
	xlogIn("new KempstonMouseInsp");
	assert(object->isA(isa_KempstonMouse));

	display_buttons->setMaximumWidth(100);

	combobox_scale->setFocusPolicy(Qt::NoFocus);
	combobox_scale->addItems(QStringList()<<"1:1"<<"1:2"<<"1:3"<<"1:4");
	combobox_scale->setMinimumWidth(80);
	connect(combobox_scale, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		[=](int index)
		{
			NVPtr<KempstonMouse>(mif())->setScale(index+1);
		});

	button_grab_mouse->setMinimumWidth(100);
	connect(button_grab_mouse, &QPushButton::clicked, [=]
		{
			mouse.grab(this);
			timer->start(1000/20);
		});

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(20,10,20,5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0,100);
//	g->setColumnStretch(0,0);	// X=
//	g->setColumnStretch(1,0);	// [X]
//	g->setColumnStretch(2,0);	// Y=
//	g->setColumnStretch(3,0);	// [Y]
	g->setColumnStretch(4,100);	// spacer
//	g->setColumnStretch(5,0);	// (btn)

	g->addWidget(new QLabel("X ="),1,0);			g->addWidget(display_x,1,1);
	g->addWidget(new QLabel("Y ="),1,2);			g->addWidget(display_y,1,3);

	g->addWidget(new QLabel("Buttons:"),2,0,1,4,Qt::AlignLeft);
	g->addWidget(display_buttons,2,0,1,4,Qt::AlignRight);

	g->addWidget(new QLabel(""),1,4);	// Spalte muss belegt werden, sonst wird sie vom Layout ignoriert

	g->addWidget(combobox_scale,1,5);
	g->addWidget(button_grab_mouse,2,5);

	clearFocus();
	updateWidgets();	// once
}


void KempstonMouseInsp::updateWidgets()
{
	xxlogIn("KempstonMouseInsp:updateWidgets");

	if(!object) return;

	uint newx,newy;
	int newbuttons;

	{
		NVPtr<KempstonMouse> mif(this->mif());
		newx = mif->getXPos();
		newy = mif->getYPos();
		newbuttons = mif->getButtons();
	}

	if(old_x!=newx) { old_x=newx; display_x->setText(tostr(newx)); }
	if(old_y!=newy) { old_y=newy; display_y->setText(tostr(newy)); }

	if(old_buttons != newbuttons)
	{
		old_buttons = newbuttons;
		display_buttons->setText(binstr(newbuttons,"%------LR","%------11"));
	}

	bool newgrabbed = mouse.isGrabbed();
	if(old_grabbed != newgrabbed)
	{
		old_grabbed = newgrabbed;
		button_grab_mouse->setText(newgrabbed ? "CMD to exit" : "Grab mouse");
		if(!newgrabbed) timer->stop();
	}

	if(combobox_scale->currentIndex() != mif()->getScale()-1)
	{
		combobox_scale->setCurrentIndex(mif()->getScale()-1);
	}
}






























