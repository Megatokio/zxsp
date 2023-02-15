#pragma once
// Copyright (c) 2007 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include <QApplication>
#include <QEvent>
#include <QWidget>
#include <Qt>


class Mouse : public QObject
{
	QWidget* grabber;
	QTimer*	 mouse_tracker_timer;

public:
	int dx;
	int dy;

public:
	Mouse();
	~Mouse();

	void	 grab(QWidget*);
	void	 ungrab();
	QWidget* getGrabber() { return grabber; }
	bool	 isGrabbed() { return grabber != nullptr; }

	bool getLeftButton() { return QApplication::mouseButtons() & Qt::LeftButton; }
	bool getMiddleButton() { return QApplication::mouseButtons() & Qt::MidButton; }
	bool getRightButton() { return QApplication::mouseButtons() & Qt::RightButton; }

private:
	void mouse_tracker(); // timer
};


extern Mouse mouse;
