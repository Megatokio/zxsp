/*	Copyright  (c)	Günter Woigk 2007 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
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




















