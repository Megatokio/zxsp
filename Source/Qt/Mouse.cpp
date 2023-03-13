// Copyright (c) 2007 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Mouse.h"
#ifdef _MACOSX
  #include <ApplicationServices/ApplicationServices.h>
#endif
#include <QtGui>

Mouse mouse;

static bool _is_grabbed = false;

static void _ungrab() noexcept
{
	if (!_is_grabbed) return;
	_is_grabbed = false;

#ifdef _MACOSX
	CGAssociateMouseAndMouseCursorPosition(yes);
	CGDisplayShowCursor(kCGDirectMainDisplay);
#else
	// TODO
#endif
}

static void _grab() noexcept
{
	if (_is_grabbed) return;
	_is_grabbed = true;

#ifdef _MACOSX
	CGAssociateMouseAndMouseCursorPosition(no);
	CGDisplayHideCursor(kCGDirectMainDisplay);
#else
	// TODO
#endif
}

static void _get_last_mouse_delta(int* dx, int* dy) noexcept
{
	// mouse movement since last mouseMoved event

#ifdef _MACOSX
	CGGetLastMouseDelta(dx, dy);
#else
	// TODO
	*dx = 0;
	*dy = 0;
#endif
}

Mouse::Mouse() noexcept
{
	xlogline("new Mouse");
	assert(this == &mouse);
	atexit(&_ungrab);
}

Mouse::~Mouse() noexcept
{
	xlogline("~Mouse");
	_ungrab();
}

void Mouse::grab(QWidget* w)
{
	if (grabber) ungrab();
	grabber = w;
	w->setFocus();
	w->grabMouse();
	w->activateWindow();
	_grab();
}

void Mouse::ungrab()
{
	if (!grabber) return;
	grabber->releaseMouse();
	grabber = nullptr;
	_ungrab();
}

void Mouse::updatePosition()
{
	int32_t dx, dy;
	_get_last_mouse_delta(&dx, &dy);
	this->dx += dx;
	this->dy -= dy;
}
