// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MySimpleToggleButton.h"
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QWidget>


MySimpleToggleButton::MySimpleToggleButton(
	QWidget* parent, int x, int y, cstr filepath_up, cstr filepath_down, bool sticky) :
	QWidget(parent),
	image_up(QImage(filepath_up)), image_down(QImage(filepath_down)), state(up), sticky(sticky)
{
	setFixedSize(image_up.size());
	move(x, y);
	setCursor(Qt::PointingHandCursor);
}


MySimpleToggleButton::MySimpleToggleButton(
	QWidget* parent, int x, int y, cstr basepath, cstr ppath_up, cstr ppath_down, bool sticky) :
	QWidget(parent),
	image_up(QImage(catstr(basepath, ppath_up))), image_down(QImage(catstr(basepath, ppath_down))), state(up),
	sticky(sticky)
{
	setFixedSize(image_up.size());
	move(x, y);
	setCursor(Qt::PointingHandCursor);
}


void MySimpleToggleButton::setDown(bool f)
{
	if (state != f)
	{
		state = f;
		update();
	}
}

void MySimpleToggleButton::click()
{
	if (state == down && sticky) return;
	state = !state;
	update();
	emit toggled(state);
}


// virtual
void MySimpleToggleButton::paintEvent(QPaintEvent*)
{
	xlogIn("MySimpleToggleButton:paintEvent");

	QPainter p(this);
	p.drawImage(rect(), state == down ? image_down : image_up);
}


// virtual
void MySimpleToggleButton::mousePressEvent(QMouseEvent*)
{
	//	QWidget::mousePressEvent(e);
}


// virtual
void MySimpleToggleButton::mouseReleaseEvent(QMouseEvent*)
{
	click();
	//	if(state==down && sticky) return;
	//	state = !state;
	//	update();
	//	emit toggled(state);
}
