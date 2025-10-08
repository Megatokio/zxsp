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

void RzxOverlay::draw(QPainter& p, int)
{
	p.drawPixmap(x, y, w, h, background); //TODO: blink recording lamp
}

void RzxOverlay::setRecording(bool f)
{
	if (recording == f) return;
	background = QPixmap(usingstr("%sOverlays/%s.png", appl_rsrc_path, f ? "record" : "play"));
	recording  = f;
}


// ===================================================================
//			Overlay "Joystick"
// ===================================================================

static QPolygon arrowL, arrowR, arrowU, arrowD;
static QRect	fire;

void JoystickOverlay::setZoom(int zoom)
{
	if (this->zoom != zoom)
	{
		this->zoom = zoom;
		pixmap	   = QPixmap(30 * zoom, 30 * zoom);
	}

	QColor background_color {0, 0, 0, 0x20}; // rgba
	QColor line_color(222, 222, 222, 0x80);
	QColor text_color(222, 222, 222, 0x80);

	pixmap.fill(Qt::GlobalColor::transparent);
	QPainter p(&pixmap);
	p.scale(zoom, zoom);
	p.setPen(Qt::NoPen);
	p.setBrush(background_color);
	p.drawRoundRect(0, 0, 30, 30, 4 * zoom, 4 * zoom);

	QPen line_pen = QPen(line_color, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	p.setPen(line_pen);
	p.setBrush(Qt::NoBrush);

	p.drawEllipse(fire);
	p.drawPolyline(arrowU);
	p.drawPolyline(arrowD);
	p.drawPolyline(arrowL);
	p.drawPolyline(arrowR);

	p.setPen(text_color);
	QFont text_font = QFont("Arial", 8); // Geneva (weiter), Arial oder Gill Sans (enger)
	p.setFont(text_font);
	p.drawText(idf[1] ? 20 : 22, 28, idf);
}


JoystickOverlay::JoystickOverlay() : //
	Overlay {isa_JoystickOverlay, TopLeft},
	pixmap {},
	state {0x00},
	idf {0},
	zoom {0}
{
	xlogIn("new JoystickOverlay");

	w = 30;
	h = 30;

	if (fire.isNull())
	{
		fire.setRect(11, 11, 8, 8);
		arrowU.setPoints(3, 12, +8, 15, +2, 18, +8);
		arrowD.setPoints(3, 12, 22, 15, 28, 18, 22);
		arrowL.setPoints(3, +8, 18, +2, 15, +8, 12);
		arrowR.setPoints(3, 22, 18, 28, 15, 22, 12);
	}
}

void JoystickOverlay::setState(uint8 newstate) { state = newstate & 0x1f; }

void JoystickOverlay::setIdf(cstr s)
{
	assert(s);
	if (eq(idf, s)) return;

	idf[0] = s[0];
	idf[1] = s[1];
	idf[2] = 0;
	if (zoom) setZoom(zoom);
}

void JoystickOverlay::draw(QPainter& p, int zoom)
{
	if (zoom != this->zoom) setZoom(zoom);
	p.drawPixmap(x, y, w, h, pixmap);

	if (state == 0x00) return;

	QColor hilite_color(255, 255, 255);
	QPen   hilite_pen = QPen(hilite_color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	p.setPen(hilite_pen);
	p.setBrush(Qt::NoBrush);

	if (state & JoystickButtons::button_fire1_mask) { p.drawEllipse(fire); }
	if (state & JoystickButtons::button_up_mask) { p.drawPolyline(arrowU); }
	if (state & JoystickButtons::button_down_mask) { p.drawPolyline(arrowD); }
	if (state & JoystickButtons::button_left_mask) { p.drawPolyline(arrowL); }
	if (state & JoystickButtons::button_right_mask) { p.drawPolyline(arrowR); }
}


} // namespace gui

/*



























*/
