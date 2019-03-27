/*	Copyright  (c)	Günter Woigk 2016 - 2018
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

#include "kio/kio.h"
#include "Overlay.h"
#include "Machine.h"
#include "MachineController.h"
#include <QPainter>
#include "Screen/Screen.h"
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QFont>
#include "Joy/Joy.h"



Overlay::Overlay(Screen* scr, isa_id id, Position p)
:
	IsaObject(scr,id,isa_Overlay),
	screen(scr),
	position(p),
	x(0),y(0),w(0),h(0),
	zoom(scr->getZoom())
{}


void Overlay::setZoom(int z)
{
	if(z==zoom) return;
	w = w * z / zoom; h = h * z / zoom;
	xlogline("new size = %u x %u",w,h);
	zoom = z;
}



// ===================================================================
//			Overlay "Play"
// ===================================================================


OverlayPlay::OverlayPlay(Screen* s, Position pos)
:
	Overlay(s,isa_OverlayPlay,pos),
	background(catstr(appl_rsrc_path,"Overlays/play.png"))
{
	xlogIn("new OverlayPlay");
	assert(s);

	w = background.width()  * zoom / 4;
	h = background.height() * zoom / 4;
	xlogline("size = %u x %u",w,h);
}


void OverlayPlay::draw( QPainter& p)
{
	p.drawPixmap(x, y, w, h, background);
}



// ===================================================================
//			Overlay "Record"
// ===================================================================


OverlayRecord::OverlayRecord(Screen* s, Position pos)
:
	Overlay(s,isa_OverlayRecord,pos),
	background(catstr(appl_rsrc_path,"Overlays/record.png"))
{
	xlogIn("new OverlayRecord");
	assert(s);

	w = background.width()  * zoom / 4;
	h = background.height() * zoom / 4;
	xlogline("size = %u x %u",w,h);

//	TODO: tiny animation
//	connect(timer,&QTimer::timeout,this,&Inspector::updateWidgets,Qt::AutoConnection);
//	timer->start(1000/10);
}


void OverlayRecord::draw( QPainter& p)
{
	p.drawPixmap(x, y, w, h, background);
}



// ===================================================================
//			Overlay "Joystick"
// ===================================================================

QColor shadow_color(0x66000000);		// argb
QColor line_color(0xccffffff);
QColor hilite_color(0xccffcc00);
QColor text_color(0xccffffff);

#define SZ 3	// raster size

OverlayJoystick::OverlayJoystick(Screen* s, Joystick* joy, cstr idf, Position pos )
:
	Overlay(s, isa_OverlayJoystick, pos),
	joystick(joy),
	idf(idf),
	arrowL(3),
	arrowR(3),
	arrowU(3),
	arrowD(3)
{
	assert(idf);
	assert(joy != noJoystick);

	w = h = 8 * SZ;
	setZoom(zoom);
}



void OverlayJoystick::setZoom(int z)
{
	Overlay::setZoom(z);

	shadow_pen = QPen(shadow_color,5*z,Qt::SolidLine,Qt::FlatCap,Qt::MiterJoin);
	hilite_pen = QPen(hilite_color,3*z,Qt::SolidLine,Qt::FlatCap,Qt::MiterJoin);
	line_pen = QPen(line_color,1*z,Qt::SolidLine,Qt::FlatCap,Qt::MiterJoin);
	text_pen = QPen(text_color,1*z,Qt::SolidLine,Qt::FlatCap,Qt::MiterJoin);
	text_font = QFont("Arial",11*z);			// Geneva (weiter), Arial oder Gill Sans (enger)

	int sz = z*SZ;
	arrowL.putPoints(0,3, x+0*sz,y+4*sz, x+2*sz,y+3*sz, x+2*sz,y+5*sz);
	arrowR.putPoints(0,3, x+8*sz,y+4*sz, x+6*sz,y+3*sz, x+6*sz,y+5*sz);
	arrowU.putPoints(0,3, x+4*sz,y+0*sz, x+3*sz,y+2*sz, x+5*sz,y+2*sz);
	arrowD.putPoints(0,3, x+4*sz,y+8*sz, x+3*sz,y+6*sz, x+5*sz,y+6*sz);
	fire.setRect(x+3*sz,y+3*sz,2*sz,2*sz);
}



void OverlayJoystick::draw( QPainter& p)
{
	if(screen->windowState() & Qt::WindowActive && joystick->isConnected())
	{
		p.setPen(shadow_pen);
		p.drawPolygon(arrowL);
		p.drawPolygon(arrowR);
		p.drawPolygon(arrowU);
		p.drawPolygon(arrowD);
		p.drawEllipse(fire);

		uint state = joystick->getState(no);
		if(state)
		{
			p.setPen(hilite_pen);
			if(state&Joystick::button_left_mask)  p.drawPolygon(arrowL);
			if(state&Joystick::button_right_mask) p.drawPolygon(arrowR);
			if(state&Joystick::button_up_mask)	  p.drawPolygon(arrowU);
			if(state&Joystick::button_down_mask)  p.drawPolygon(arrowD);
			if(state&Joystick::button1_mask)	  p.drawEllipse(fire);
		}

		p.setPen(line_pen);
		p.drawPolygon(arrowL);
		p.drawPolygon(arrowR);
		p.drawPolygon(arrowU);
		p.drawPolygon(arrowD);
		p.drawEllipse(fire);

		p.setPen(text_pen);
		p.setFont(text_font);
		p.drawText(x,y+zoom*9,idf);
	}
}


































