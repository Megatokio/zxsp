// Copyright (c) 2007 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Mouse.h"
#include "globals.h"
#ifdef _MACOSX
  #include <ApplicationServices/ApplicationServices.h>
#endif
#include <QtGui>


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


Mouse::Mouse() : grabber(nullptr), mouse_tracker_timer(new QTimer(this)), dx(0), dy(0)
{
	xlogIn("new Mouse");
	assert(this == &mouse);

	connect(mouse_tracker_timer, &QTimer::timeout, this, &Mouse::mouse_tracker);

	atexit(&_ungrab);
}


Mouse::~Mouse()
{
	xlogIn("~Mouse");
	ungrab();
}

void Mouse::grab(QWidget* w)
{
	if (grabber) ungrab();
	grabber = w;

	connect(w, &QWidget::destroyed, this, &Mouse::ungrab);

	mouse_tracker_timer->start(1000 / 100);

	w->setFocus();
	w->grabMouse();
	w->activateWindow();
#ifdef _MACOSX
	CGAssociateMouseAndMouseCursorPosition(no);
	CGDisplayHideCursor(kCGDirectMainDisplay);
#else
	// TODO
#endif
}

void Mouse::ungrab()
{
	if (!grabber) return;
	mouse_tracker_timer->stop();
	grabber->releaseMouse();
#ifdef _MACOSX
	CGAssociateMouseAndMouseCursorPosition(yes);
	CGDisplayShowCursor(kCGDirectMainDisplay);
#else
	// TODO
#endif
	disconnect(grabber, &QWidget::destroyed, this, &Mouse::ungrab);
	grabber = nullptr;
}

void Mouse::mouse_tracker()
{
	if (QApplication::keyboardModifiers() & Qt::CTRL)
	{
		ungrab();
		return;
	}

#ifdef _MACOSX
	int32_t dx, dy;
	CGGetLastMouseDelta(&dx, &dy); // mouse movement since last mouseMoved event
	this->dx += dx;
	this->dy -= dy;
#else
	// TODO
#endif
}
