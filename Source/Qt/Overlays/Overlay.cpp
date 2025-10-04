// Copyright (c) 2016 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Overlay.h"
#include "Joy/Joy.h"
#include "Machine.h"
#include "MachineController.h"
#include "Screen/Screen.h"
#include "UsbJoystick.h"
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPen>

namespace gui
{

Overlay::Overlay(isa_id id, Location p) : IsaObject(id, isa_Overlay), location(p) {}

Overlay::~Overlay() {}


// ===================================================================
//			Rzx playback / recording Overlay
// ===================================================================

RzxOverlay::RzxOverlay() :
	Overlay(isa_RzxOverlay, RzxGroup),
	background(usingstr("%sOverlays/play.png", appl_rsrc_path))
{
	xlogIn("new RzxOverlay");

	w = background.width() / 4;
	h = background.height() / 4;
	xlogline("size = %u x %u", w, h);
}

void RzxOverlay::draw(QPainter& p)
{
	p.drawPixmap(x, y, w, h, background); //TODO: blink recording lamp
}

void RzxOverlay::setRecording(bool f)
{
	if (recording == f) return;
	background = QPixmap(usingstr("%sOverlays/%s.png", appl_rsrc_path, recording ? "record" : "play"));
	recording  = f;
}


// ===================================================================
//			Overlay "Joystick"
// ===================================================================

static QColor shadow_color(0x66000000); // argb
static QColor line_color(0xccffffff);
static QColor hilite_color(0xccffcc00);
static QColor text_color(0xccffffff);

#define SZ 3 // raster size

JoystickOverlay::JoystickOverlay(const Joystick* joy, cstr idf) :
	Overlay(isa_JoystickOverlay, TopLeft),
	joystick(joy),
	idf(idf),
	arrowL(3),
	arrowR(3),
	arrowU(3),
	arrowD(3)
{
	w = h = 8 * SZ;
}


/*
void JoystickOverlay::setZoom(int z)
{
	shadow_pen = QPen(shadow_color, 5 * z, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	hilite_pen = QPen(hilite_color, 3 * z, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	line_pen   = QPen(line_color, 1 * z, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	text_pen   = QPen(text_color, 1 * z, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	text_font  = QFont("Arial", 11 * z); // Geneva (weiter), Arial oder Gill Sans (enger)

	int sz = z * SZ;
	arrowL.putPoints(0, 3, x + 0 * sz, y + 4 * sz, x + 2 * sz, y + 3 * sz, x + 2 * sz, y + 5 * sz);
	arrowR.putPoints(0, 3, x + 8 * sz, y + 4 * sz, x + 6 * sz, y + 3 * sz, x + 6 * sz, y + 5 * sz);
	arrowU.putPoints(0, 3, x + 4 * sz, y + 0 * sz, x + 3 * sz, y + 2 * sz, x + 5 * sz, y + 2 * sz);
	arrowD.putPoints(0, 3, x + 4 * sz, y + 8 * sz, x + 3 * sz, y + 6 * sz, x + 5 * sz, y + 6 * sz);
	fire.setRect(x + 3 * sz, y + 3 * sz, 2 * sz, 2 * sz);
}
*/

void JoystickOverlay::draw(__unused QPainter& p)
{
#if 0
	if (screen->isActive() && joystick->isConnected())
	{
		p.setPen(shadow_pen);
		p.drawPolygon(arrowL);
		p.drawPolygon(arrowR);
		p.drawPolygon(arrowU);
		p.drawPolygon(arrowD);
		p.drawEllipse(fire);

		uint state = joystick->getState(no);
		if (state)
		{
			p.setPen(hilite_pen);
			if (state & JoystickButtons::button_left_mask) p.drawPolygon(arrowL);
			if (state & JoystickButtons::button_right_mask) p.drawPolygon(arrowR);
			if (state & JoystickButtons::button_up_mask) p.drawPolygon(arrowU);
			if (state & JoystickButtons::button_down_mask) p.drawPolygon(arrowD);
			if (state & JoystickButtons::button_fire1_mask) p.drawEllipse(fire);
		}

		p.setPen(line_pen);
		p.drawPolygon(arrowL);
		p.drawPolygon(arrowR);
		p.drawPolygon(arrowU);
		p.drawPolygon(arrowD);
		p.drawEllipse(fire);

		p.setPen(text_pen);
		p.setFont(text_font);
		p.drawText(x, y + zoom * 9, idf);
	}
#endif
}

} // namespace gui
