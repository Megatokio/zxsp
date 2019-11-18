#pragma once
/*	Copyright  (c)	Günter Woigk 2007 - 2019
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

#include <Qt>
#include <QWidget>
#include <QEvent>
#include "kio/kio.h"
#include <QApplication>


class Mouse : public QObject
{
	QWidget*	grabber;
	QTimer*		mouse_tracker_timer;

public:
	int			dx;
	int			dy;

public:
	Mouse();
	~Mouse();

	void		grab(QWidget*);
	void		ungrab();
	QWidget*	getGrabber()		{ return grabber; }
	bool		isGrabbed()         { return grabber!=NULL; }

	bool		getLeftButton()     { return QApplication::mouseButtons() & Qt::LeftButton; }
	bool		getMiddleButton()	{ return QApplication::mouseButtons() & Qt::MidButton; }
	bool		getRightButton()	{ return QApplication::mouseButtons() & Qt::RightButton; }

private:
	void		mouse_tracker();	// timer
};


extern Mouse mouse;















