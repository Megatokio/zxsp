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

#include <QWidget>
#include "MachineController.h"
#include <QMenu>
#include <QAction>
#include <QTimer>

class Lenslok : public QWidget
{
	MachineController*	controller;

	QPixmap background_a;
	QPixmap background_b;
	QPixmap*background;
	QMenu*	contextmenu;
	QAction* actions[10];
	QTimer*	timer;
	uint	game_id;
	bool	ignore_focusout;

// state:
	int		click_dx,click_dy;	// after click, during move
	double	click_t0;			// after click
	QPoint	click_p0;			// after click

public:
	Lenslok(MachineController*, cstr name1, cstr name2);
	~Lenslok();

protected:
	void	paintEvent(QPaintEvent*) override;
	void	mousePressEvent(QMouseEvent*) override;
	void	mouseMoveEvent(QMouseEvent*) override;
	void	mouseReleaseEvent(QMouseEvent*) override;
	bool	event(QEvent*) override;
	void	focusOutEvent(QFocusEvent*) override;
	void	keyPressEvent(QKeyEvent*) override;
	void	keyReleaseEvent(QKeyEvent*) override;
	void	contextMenuEvent(QContextMenuEvent*) override;
	//void	enterEvent(QEvent*) override;
	//void	leaveEvent(QEvent*) override;
	void	moveEvent(QMoveEvent*) override;

private:
	void	draw_prism(QPainter&, QRectF, const QRectF&);
	void	select_game();
};


