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

#include "MySimpleToggleButton.h"
#include <QPainter>
#include <QWidget>
#include <QPixmap>
#include <QImage>


MySimpleToggleButton::MySimpleToggleButton(
				QWidget* parent, int x, int y,
				cstr filepath_up, cstr filepath_down, bool sticky)
:
	QWidget(parent),
	image_up(QImage(filepath_up)),
	image_down(QImage(filepath_down)),
	state(up),
	sticky(sticky)
{
	setFixedSize(image_up.size());
	move(x,y);
	setCursor(Qt::PointingHandCursor);
}


MySimpleToggleButton::MySimpleToggleButton(
				QWidget* parent, int x, int y,
				cstr basepath, cstr ppath_up, cstr ppath_down, bool sticky)
:
	QWidget(parent),
	image_up(QImage(catstr(basepath,ppath_up))),
	image_down(QImage(catstr(basepath,ppath_down))),
	state(up),
	sticky(sticky)
{
	setFixedSize(image_up.size());
	move(x,y);
	setCursor(Qt::PointingHandCursor);
}


void MySimpleToggleButton::setDown(bool f)
{
	if(state!=f)
	{
		state = f;
		update();
	}
}

void MySimpleToggleButton::click()
{
	if(state==down && sticky) return;
	state = !state;
	update();
	emit toggled(state);
}


//virtual
void MySimpleToggleButton::paintEvent(QPaintEvent*)
{
	xlogIn("MySimpleToggleButton:paintEvent");

	QPainter p(this);
	p.drawImage(rect(),state==down?image_down:image_up);
}


//virtual
void MySimpleToggleButton::mousePressEvent(QMouseEvent*)
{
//	QWidget::mousePressEvent(e);
}


//virtual
void MySimpleToggleButton::mouseReleaseEvent(QMouseEvent*)
{
	click();
//	if(state==down && sticky) return;
//	state = !state;
//	update();
//	emit toggled(state);
}
