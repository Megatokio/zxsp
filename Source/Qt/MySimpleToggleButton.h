#pragma once
/*	Copyright  (c)	Günter Woigk 2013 - 2019
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

#include <functional>
#include <QPixmap>
#include <QImage>
#include <QWidget>
#include "kio/kio.h"


class MySimpleToggleButton : public QWidget
{
	Q_OBJECT

	QImage	image_up;
	QImage	image_down;
	bool	state;					enum { up=0, down=1 };
	bool	sticky;					// sticky: radio button behaviour, else the button can be toggle down and up again

public:
	MySimpleToggleButton(QWidget* parent, int x, int y, cstr filepath_up, cstr filepath_down, bool sticky = no);
	MySimpleToggleButton(QWidget* parent, int x, int y, cstr basepath, cstr ppath_up, cstr ppath_down, bool sticky = no);

	bool	isDown()				const	{ return state==down; }
	void	setDown(bool);			// no signal
	void	click();				// emits signal if successfully toggled (except if sticky & already down)

protected:
	void	paintEvent(QPaintEvent*) override;
	void	mousePressEvent(QMouseEvent*) override;
	void	mouseReleaseEvent(QMouseEvent*) override;
	//void	mouseMoveEvent(QMouseEvent*) override;
	//void	showEvent(QShowEvent*) override;
	//void	hideEvent (QHideEvent*) override;

signals:
	void	toggled(bool);
};


























