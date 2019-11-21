/*	Copyright  (c)	Günter Woigk 2007 - 2019
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
#include <ApplicationServices/ApplicationServices.h>
#include "Mouse.h"
#include "globals.h"


// global instance
//
Mouse mouse;


// security: remove grabber at App exit, e.g. on abort(…)
//
static void _ungrab()
{
	xlogline("Mouse:atexit:ungrab");
	mouse.ungrab();
}


Mouse::Mouse()
:
	grabber(NULL),
	mouse_tracker_timer(new QTimer(this)),
	dx(0),
	dy(0)
{
	xlogIn("new Mouse");
	assert(this==&mouse);

	connect(mouse_tracker_timer, &QTimer::timeout, this, &Mouse::mouse_tracker);

	atexit(&_ungrab);
}


Mouse::~Mouse()
{
	xlogIn("~Mouse");
	ungrab();
}

void Mouse::grab( QWidget* w )
{
	if(grabber) ungrab();
	grabber = w;

	connect(w, &QWidget::destroyed, this, &Mouse::ungrab);

	mouse_tracker_timer->start(1000/100);

	w->setFocus();
	w->grabMouse();
	w->activateWindow();
	CGAssociateMouseAndMouseCursorPosition(no);
	CGDisplayHideCursor(kCGDirectMainDisplay);
}

void Mouse::ungrab()
{
	if(!grabber) return;
	mouse_tracker_timer->stop();
	grabber->releaseMouse();
	CGAssociateMouseAndMouseCursorPosition(yes);
	CGDisplayShowCursor(kCGDirectMainDisplay);
	disconnect(grabber, &QWidget::destroyed, this, &Mouse::ungrab);
	grabber = NULL;
}

void Mouse::mouse_tracker()
{
	if(QApplication::keyboardModifiers()&Qt::CTRL) { ungrab(); return; }

	int32_t dx, dy;
	CGGetLastMouseDelta( &dx, &dy );			// mouse movement since last mouseMoved event
	this->dx += dx;
	this->dy -= dy;
}




















