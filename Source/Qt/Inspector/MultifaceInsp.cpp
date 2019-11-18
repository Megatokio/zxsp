/*	Copyright  (c)	Günter Woigk 2015 - 2019
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

#include "MultifaceInsp.h"
#include <QMouseEvent>
#include <QLabel>
#include <QTimer>
#include "Machine.h"
#include "Multiface/Multiface.h"
#include "Templates/NVPtr.h"


MultifaceInsp::MultifaceInsp(QWidget* p, MachineController* mc, volatile IsaObject* o, cstr image, const QRect& btnbox)
:
	Inspector(p,mc,o,image),
	buttonbox(btnbox)
{
	QWidget* button = new QWidget(this);
	button->setGeometry(btnbox);
	button->setCursor(Qt::PointingHandCursor);
	button->setVisible(yes);

	label_nmi_pending = new QLabel("NMI pending",this);
	label_paged_in    = new QLabel("Paged in",this);

	label_nmi_pending->move(l_x,l_y+0*l_d);
	label_paged_in->move(l_x,l_y+1*l_d);

	timer->start(1000/60);		// --> updateWidgets()
}







/*	Mouse click handler:
	- press the red button
*/
void MultifaceInsp::mousePressEvent(QMouseEvent* e)
{
	if(e->button()!=Qt::LeftButton) { Inspector::mousePressEvent(e); return; }

	xlogline("MultifaceInspector: mouse down at %i,%i",e->x(),e->y());

	if(buttonbox.contains(e->pos())) pressRedButton();
}


void MultifaceInsp::pressRedButton()
{
	NVPtr<Multiface>(multiface())->triggerNmi();
}


void MultifaceInsp::updateWidgets()
{
	if(label_nmi_pending->isVisible() != multiface()->nmi_pending)
	{
		label_nmi_pending->setVisible(multiface()->nmi_pending);
	}

	if(label_paged_in->isVisible() != multiface()->paged_in)
	{
		label_paged_in->setVisible(multiface()->paged_in);
	}
}
