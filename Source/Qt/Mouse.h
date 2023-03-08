#pragma once
// Copyright (c) 2007 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include <QApplication>
#include <QEvent>
#include <QWidget>
#include <Qt>


class Mouse
{
	QWidget* grabber = nullptr;

public:
	int dx = 0;
	int dy = 0;

public:
	Mouse() noexcept;
	~Mouse() noexcept;

	void	 grab(QWidget*);
	void	 ungrab();
	QWidget* getGrabber() { return grabber; }
	bool	 isGrabbed() { return grabber != nullptr; }

	bool getLeftButton() { return QApplication::mouseButtons() & Qt::LeftButton; }
	bool getMiddleButton() { return QApplication::mouseButtons() & Qt::MidButton; }
	bool getRightButton() { return QApplication::mouseButtons() & Qt::RightButton; }
	void updatePosition();
};


extern Mouse mouse;
