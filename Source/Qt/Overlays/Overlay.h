#pragma once
// Copyright (c) 2016 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IsaObject.h"
#include "gui_types.h"
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
	enum Location { RzxGroup = 1, TopLeft = 2, BottomCenter = 3 };

	Location location;

	int x {0}; // nominal, before zoom
	int y {0};
	int w {0};
	int h {0};


protected:
	Overlay(isa_id, Location);

public:
	~Overlay() override;

	virtual void draw(QPainter&) = 0;
};


class RzxOverlay : public Overlay
{
public:
	QPixmap background;
	bool	recording = false;

public:
	RzxOverlay();
	void draw(QPainter&) override;
	void setRecording(bool);
};


class JoystickOverlay : public Overlay
{
public:
	const Joystick* joystick;
	cstr			idf; // 1 or 2 char identifier

	QPen	 shadow_pen; // pen & font: subject to zoom scaling!
	QPen	 hilite_pen;
	QPen	 line_pen;
	QPen	 text_pen;
	QFont	 text_font;
	QPolygon arrowL, arrowR, arrowU, arrowD;
	QRect	 fire;

public:
	explicit JoystickOverlay(const Joystick*, cstr idf);
	void draw(QPainter&) override;
};

} // namespace gui
