#pragma once
/*	Copyright  (c)	Günter Woigk 2016 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include "kio/kio.h"
#include <QMainWindow>
#include <QObject>
class MachineController;
#include "IsaObject.h"
#include <QPixmap>
#include "IsaObject.h"
class Screen;
#include <QFont>
#include <QPen>
#include <QPolygon>


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
	enum Position
	{
		Default = 0,

		Left   = 1,
		Center = 2,
		Right  = 3,

		Top    = 1*8,
		Middle = 2*8,
		Bottom = 3*8,

		TopLeft		= Top|Left,
		TopCenter	= Top|Center,
		TopRight	= Top|Right,

		MiddleLeft	= Left|Middle,
		MiddleRight	= Right|Middle,

		BottomLeft	= Bottom|Left,
		BottomCenter= Bottom|Center,
		BottomRight = Bottom|Right,

		BelowAll	= Bottom,			// at bottom, full width, below BottomCenter
	};

	Screen*		screen;
	Position	position;
	int			x,y,w,h;
	int			zoom;


protected:
				Overlay(Screen*, isa_id, Position);

public:
				~Overlay(){}

virtual	void		setZoom		(int);
virtual	void		draw		(QPainter&) = 0;
};




class OverlayPlay : public Overlay
{
public:
	QPixmap		background;

public:
	explicit	OverlayPlay	(Screen*, Position=TopLeft);
	void		draw		(QPainter&)	override;
};


class OverlayRecord : public Overlay
{
public:
	QPixmap		background;

public:
	explicit	OverlayRecord (Screen*, Position=TopLeft);
	void		draw		  (QPainter&)	override;
};


class OverlayJoystick : public Overlay
{
public:
	Joystick* joystick;
	cstr	idf;	// 1 or 2 char identifier

	QPen	shadow_pen;		// pen & font: subject to zoom scaling!
	QPen	hilite_pen;
	QPen	line_pen;
	QPen	text_pen;
	QFont	text_font;
	QPolygon arrowL,arrowR,arrowU,arrowD;
	QRect	fire;

public:
	explicit	OverlayJoystick	(Screen*, Joystick*, cstr idf, Position);
	void		draw			(QPainter&)	override;
	void		setZoom			(int) override;
};




































