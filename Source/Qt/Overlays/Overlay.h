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

	virtual void draw(QPainter&, int scale) = 0;
};


class RzxOverlay : public Overlay
{
public:
	QPixmap background;
	bool	recording = false;

public:
	RzxOverlay();
	void draw(QPainter&, int scale) override;
	void setRecording(bool);
};


class JoystickOverlay : public Overlay
{
public:
	QPixmap pixmap;
	uint8	state;	// Kempston: %000FUDLR
	char	idf[3]; // 1 or 2 char identifier
	int		zoom;	// current zoom of pixmap

public:
	JoystickOverlay();
	void draw(QPainter&, int scale) override;
	void setIdf(cstr);
	void setState(uint8); // Kempston: %000FUDLR

private:
	void setZoom(int zoom);
};

} // namespace gui
