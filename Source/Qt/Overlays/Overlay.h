#pragma once
// Copyright (c) 2016 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IsaObject.h"
#include <QFont>
#include <QPen>
#include <QPixmap>
#include <QPolygon>
class IScreen;

namespace gui
{

/*
M(		isa_OverlayPlay,		isa_Overlay,		"Overlay \"Play\"" ),
M(		isa_OverlayRecord,		isa_Overlay,		"Overlay \"Record\"" ),
M(		isa_OverlayJoystick,	isa_Overlay,		"Joystick Overlay" ),
M(		isa_OverlayTimeline,	isa_Overlay,		"Timeline Overlay" ),
M(		isa_OverlaySingleStep,	isa_Overlay,		"SingleStep Overlay" ),
*/


class Overlay : public IsaObject
{
public:
	enum Position {
		Default = 0,

		Left   = 1,
		Center = 2,
		Right  = 3,

		Top	   = 1 * 8,
		Middle = 2 * 8,
		Bottom = 3 * 8,

		TopLeft	  = Top | Left,
		TopCenter = Top | Center,
		TopRight  = Top | Right,

		MiddleLeft	= Left | Middle,
		MiddleRight = Right | Middle,

		BottomLeft	 = Bottom | Left,
		BottomCenter = Bottom | Center,
		BottomRight	 = Bottom | Right,

		BelowAll = Bottom, // at bottom, full width, below BottomCenter
	};

	IScreen* screen;
	Position position;
	int		 x, y, w, h;
	int		 zoom;


protected:
	Overlay(IScreen*, isa_id, Position);

public:
	~Overlay() override {}

	virtual void setZoom(int);
	virtual void draw(QPainter&) = 0;
};


class OverlayPlay : public Overlay
{
public:
	QPixmap background;

public:
	explicit OverlayPlay(IScreen*, Position = TopLeft);
	void draw(QPainter&) override;
};


class OverlayRecord : public Overlay
{
public:
	QPixmap background;

public:
	explicit OverlayRecord(IScreen*, Position = TopLeft);
	void draw(QPainter&) override;
};


class OverlayJoystick : public Overlay
{
public:
	Joystick* joystick;
	cstr	  idf; // 1 or 2 char identifier

	QPen	 shadow_pen; // pen & font: subject to zoom scaling!
	QPen	 hilite_pen;
	QPen	 line_pen;
	QPen	 text_pen;
	QFont	 text_font;
	QPolygon arrowL, arrowR, arrowU, arrowD;
	QRect	 fire;

public:
	explicit OverlayJoystick(IScreen*, Joystick*, cstr idf, Position);
	void draw(QPainter&) override;
	void setZoom(int) override;
};

} // namespace gui
