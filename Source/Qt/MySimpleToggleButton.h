#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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


























