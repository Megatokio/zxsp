// Copyright (c) 2016 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "kio/kio.h"
#include "zxsp_globals.h"
#include <QBrush>
#include <QPen>
#include <QWidget>


// show notification above current active window:
// (same as in zxsp_globals.h)
extern void showMessage(MessageStyle, cstr text);

namespace gui
{

extern void showMessage(QWidget* parent, MessageStyle, cstr text);
extern void showMessage(QWidget* parent, uint ConfigDialog_style, cstr title, cstr text);
extern void showQueuedMessages();


class ConfigDialog : public QWidget
{
public:
	QWidget* controller;

	QBrush bg_brush;
	QPen   upper_rim_pen;
	QPen   lower_rim_pen;
	QPen   border_pen;

	uint style;
	int	 outer_padding; // outer padding width
	int	 border_width;	// border line width, may be 0
	int	 inner_padding; // inner padding width

	int w, h;	// inner dimensions
	int x0, y0; // total border width = offset for origin (0,0)

	// mouse state:
	int	   click_dx, click_dy; // after click, during move
	double click_t0;		   // after click
	QPoint click_p0;		   // after click

	static constexpr int NoBorder = 0 << 0,										//
		Border1 = 1 << 0, Border2 = 2 << 0, Border3 = 3 << 0,					//
		Border4 = 4 << 0, Border5 = 5 << 0, Border6 = 6 << 0, Border7 = 7 << 0, //
		Neutral = 0 << 3, Red = 1 << 3, Green = 2 << 3, Yellow = 3 << 3,		//
		Blue = 4 << 3, Magenta = 5 << 3, Cyan = 6 << 3, White = 7 << 3,			//
		Dark			= 0 << 6,												//
		Bright			= 1 << 6,												//
		IgnoreFocusOut	= 0 << 7,												//
		CloseOnFocusOut = 1 << 7,												//
		EatAllKeys		= 0 << 8,												//
		PropagateKeys	= 1 << 8,												//
		CloseOnClick	= 1 << 9,												//
		CloseOnEsc		= 1 << 10,												//
		DefaultStyle	= Dark + Neutral + Border2 + IgnoreFocusOut + EatAllKeys;

	// styles for popup messages:
	static constexpr uint InfoStyle	   = Blue + Border2 + CloseOnClick + CloseOnEsc;
	static constexpr uint WarningStyle = Yellow + Border2 + CloseOnClick + CloseOnEsc;
	static constexpr uint AlertStyle   = Red + Border2 + CloseOnClick + CloseOnEsc;

private:
	static QColor color_for_style(uint s, int h, int l, int n, int a = 0xff);
	static QBrush bg_brush_for_style(uint);
	static QPen	  upper_rim_pen_for_style(uint);
	static QPen	  lower_rim_pen_for_style(uint);
	static QPen	  border_pen_for_style(uint);

public:
	ConfigDialog(QWidget* mc, int w, int h, uint style = DefaultStyle);
	ConfigDialog(QWidget* mc, QWidget* the_widget, uint style = DefaultStyle);
	virtual ~ConfigDialog() override;

protected:
	void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void focusOutEvent(QFocusEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;
};

} // namespace gui
