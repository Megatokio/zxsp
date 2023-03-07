// Copyright (c) 2016 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ConfigDialog.h"
#include "MachineController.h"
#include "Mouse.h"
#include "kio/kio.h"
#include "qt_util.h"
#include <QApplication>
#include <QBrush>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QScreen>
#include <QTimer>
#include <QWidget>

namespace gui
{

// helper
//	 h = bright color component for colors
//	 l = dark   color component for colors
//	 n = neutral color component
//	 a = alpha

QColor ConfigDialog::color_for_style(uint s, int h, int l, int n, int a)
{
	return s & (7 << 3) ? QColor(s & 8 ? h : l, s & 16 ? h : l, s & 32 ? h : l, a) // colors
						  :
						  QColor(n, n, n, a); // neutral
}

QBrush ConfigDialog::bg_brush_for_style(uint style)
{
	return style & Bright ? //               lite  dark  grey  alpha
			   QBrush(color_for_style(style, 0xff, 0x99, 0xcc, 0xee)) :
			   QBrush(color_for_style(style, 0x66, 0x00, 0x33, 0xee));
}

QPen ConfigDialog::upper_rim_pen_for_style(uint style)
{
	return style & Bright ? //             lite  dark  grey
			   QPen(color_for_style(style, 0xff, 0xcc, 0xff), 1, Qt::SolidLine, Qt::FlatCap) :
			   QPen(color_for_style(style, 0x99, 0x33, 0x66), 1, Qt::SolidLine, Qt::FlatCap);
}

QPen ConfigDialog::lower_rim_pen_for_style(uint style)
{
	return style & Bright ? //             lite  dark  grey
			   QPen(color_for_style(style, 0xcc, 0x66, 0x99), 1, Qt::SolidLine, Qt::FlatCap) :
			   QPen(color_for_style(style, 0x33, 0x00, 0x00), 1, Qt::SolidLine, Qt::FlatCap);
}

QPen ConfigDialog::border_pen_for_style(uint style)
{
	if ((style & 7) == 0) return QPen(); // border width == 0

	return style & Bright ? //             lite  dark  grey   width
			   QPen(color_for_style(style, 0xcc, 0x33, 0x33), style & 7, Qt::SolidLine, Qt::FlatCap) :
			   QPen(color_for_style(style, 0xcc, 0x33, 0xcc), style & 7, Qt::SolidLine, Qt::FlatCap);
}


ConfigDialog::~ConfigDialog() {}

ConfigDialog::ConfigDialog(QWidget* mc, int w, int h, uint style) :
	QWidget(mc),
	controller(mc),
	bg_brush(bg_brush_for_style(style)),
	upper_rim_pen(upper_rim_pen_for_style(style)),
	lower_rim_pen(lower_rim_pen_for_style(style)),
	border_pen(border_pen_for_style(style)),
	style(style),
	outer_padding(10),
	border_width(style & 7),
	inner_padding(10),
	w(w),
	h(h)
{
	setWindowFlags(
		windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
		Qt::Tool); // <-- nicht ändern wg. auto-close on FocusOut. Alles andere geht schief!
	setAttribute(Qt::WA_TranslucentBackground, true);

	x0 = inner_padding + border_width + outer_padding;
	y0 = x0;

	this->resize(w + 2 * x0, h + 2 * y0);

	if (mc)
	{
		QRect		 obox = mc->rect();
		QPoint		 oo	  = mc->mapToGlobal(obox.topLeft());
		const QRect& ibox = this->rect();
		move(oo.x() + (obox.width() - ibox.width()) / 2, oo.y() + (obox.height() - ibox.height()) / 3);
	}
	else
	{
		QSize		 sz	  = QApplication::primaryScreen()->size();
		QPoint		 oo	  = QPoint(0, 0);
		const QRect& ibox = this->rect();
		move(oo.x() + (sz.width() - ibox.width()) / 2, oo.y() + (sz.height() - ibox.height()) / 3);
	}

	// setFocusPolicy(Qt::ClickFocus);
	setFocus();
}

ConfigDialog::ConfigDialog(QWidget* mc, QWidget* the_widget, uint style) :
	ConfigDialog(mc, the_widget->width(), the_widget->height(), style)
{
	the_widget->setParent(this);
	the_widget->move(x0, y0);
}

void ConfigDialog::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	xlogline("ConfigDialog::paintEvent; x0=%u, y0=%u", x0, y0);

	// translucent background:
	{
		QPainterPath path;
		path.addRoundedRect(this->rect(), x0, y0);
		p.setPen(Qt::NoPen);
		p.setBrush(bg_brush);
		p.drawPath(path);
	}

	// prepare for lines:
	p.setBrush(Qt::NoBrush);
	p.setRenderHint(QPainter::Antialiasing); // -> Linie wird immer mittig gezeichnet

	// left/upper rim of background:
	p.setPen(upper_rim_pen);
	p.drawLine(0, y0, 0, y0 + h);					   // left
	p.drawLine(x0, 0, x0 + w, 0);					   // top
	p.drawArc(0, 0, x0 * 2, y0 * 2, 16 * 90, 16 * 90); // top-left
	p.drawArc(w, 0, x0 * 2, y0 * 2, 16 * 0, 16 * 90);  // top-right

	// right/lower rim of background:
	p.setPen(lower_rim_pen);
	p.drawLine(x0, y0 * 2 + h, x0 + w, y0 * 2 + h);		 // bottom
	p.drawLine(x0 * 2 + w, y0, x0 * 2 + w, y0 + h);		 // right
	p.drawArc(0, h, x0 * 2, y0 * 2, -16 * 90, -16 * 90); // bottom-left
	p.drawArc(w, h, x0 * 2, y0 * 2, -16 * 90, 16 * 90);	 // bottom-right

	// border line:
	if (border_width)
	{
		int d = int(outer_padding + border_width / 2.0f + 0.5f);
		p.setPen(border_pen);
		QRect r = this->rect().adjusted(+d, +d, -d, -d);

		QPainterPath path;
		path.addRoundedRect(r, x0 - d, y0 - d);
		p.drawPath(path);
	}
}

void ConfigDialog::mousePressEvent(QMouseEvent* e)
{
	// Qt event: left or right mouse button pressed down
	// this may start a window dragging or a single click to flip the window between the two states.

	if (e->button() != Qt::LeftButton)
	{
		QWidget::mousePressEvent(e);
		return;
	}

	xlogline("ConfigDialog mousePressEvent");

	click_p0 = e->globalPos();
	click_dx = e->globalX() - x();
	click_dy = e->globalY() - y();
	click_t0 = now();
}

void ConfigDialog::mouseMoveEvent(QMouseEvent* e)
{
	// Qt event: mouse moved
	// the ConfigDialog has no titlebar, so we have to handle window dragging here.
	// if this occurs while the mouse button is down, we move the window accordingly.

	xlogline("ConfigDialog mouseMoveEvent");

	if (e->buttons() & Qt::LeftButton) { this->move(e->globalX() - click_dx, e->globalY() - click_dy); }
}

void ConfigDialog::mouseReleaseEvent(QMouseEvent* e)
{
	// Qt event: mouse button up
	// if the delay between mouse down and mouse up is very short and the mouse has not moved since then,
	// then this is a 'click' only.

	xlogline("ConfigDialog mouseReleaseEvent");

	if ((style & CloseOnClick) && (e->globalPos() == click_p0 && now() <= click_t0 + 0.5)) // this is a click only
		this->deleteLater();
}

void ConfigDialog::focusOutEvent(QFocusEvent*)
{
	// Qt callback: keyboard focus lost
	// the user has clicked outside the ConfigDialog window
	// this may be the signal for the ConfigDialog to disappear

	if (style & CloseOnFocusOut) this->deleteLater();
}


/*	Qt callback: key pressed
	since we grab keyboard focus we have to promote the received key events to the machine
*/
void ConfigDialog::keyPressEvent(QKeyEvent* e)
{
	xlogIn("ConfigDialog:keyPressEvent");

	if ((style & CloseOnEsc) && e->key() == Qt::Key_Escape) this->deleteLater();
	if (style & PropagateKeys && controller != nullptr) QApplication::sendEvent(controller, e);
}


/*	Qt callback: key released
	since we grab keyboard focus we have to promote the received key events to the machine
*/
void ConfigDialog::keyReleaseEvent(QKeyEvent* e)
{
	xlogIn("ConfigDialog:keyReleaseEvent");
	if (style & PropagateKeys && controller != nullptr)
		//		controller->keyReleaseEvent(e);
		QApplication::sendEvent(controller, e);
}


// ============================================================
//			Display Message on Main Thread:
// ============================================================


void showDialog(QWidget* parent, cstr title, cstr text, uint style)
{
	logline("%s", text);

	QTimer::singleShot(0, [=]() {
		mouse.ungrab();
		QWidget* w = new QWidget(nullptr);
		w->setFixedSize(300, 150);
		QFont	bigfont = QFont("Lucida Grande", 18);
		QLabel* ti		= new QLabel(title, w);
		setColors(ti, 0xffffffff);
		ti->setFont(bigfont);
		QLabel* te = new QLabel(text, w);
		setColors(te, 0xffffffff);
		te->setWordWrap(true);
		ti->move(0, 0);
		ti->setFixedWidth(300);
		te->move(0, 28);
		te->setFixedWidth(300);

		QWidget* p = parent ? parent : QApplication::activeWindow();
		(new ConfigDialog(p, w, style))->show();
	});
}

void showInfoDialog(QWidget* parent, cstr title, cstr text)
{
	showDialog(
		parent, title, text,
		ConfigDialog::Blue + ConfigDialog::Border2 + ConfigDialog::CloseOnClick + ConfigDialog::CloseOnEsc);
}

void showWarningDialog(QWidget* parent, cstr title, cstr text)
{
	showDialog(
		parent, title, text,
		ConfigDialog::Yellow + ConfigDialog::Border2 + ConfigDialog::CloseOnClick + ConfigDialog::CloseOnEsc);
}

void showAlertDialog(QWidget* parent, cstr title, cstr text)
{
	showDialog(
		parent, title, text,
		ConfigDialog::Red + ConfigDialog::Border2 + ConfigDialog::CloseOnClick + ConfigDialog::CloseOnEsc);
}

void showInfo(cstr msg, ...)
{
	va_list va;
	va_start(va, msg);
	str text = usingstr(msg, va);
	va_end(va);

	showInfoDialog(nullptr, "Information:", text);
}

void showWarning(cstr msg, ...)
{
	va_list va;
	va_start(va, msg);
	str text = usingstr(msg, va);
	va_end(va);

	showWarningDialog(nullptr, "Problem:", text);
}

void showAlert(cstr msg, ...)
{
	va_list va;
	va_start(va, msg);
	str text = usingstr(msg, va);
	va_end(va);

	showAlertDialog(nullptr, "Alert:", text);
}

} // namespace gui
