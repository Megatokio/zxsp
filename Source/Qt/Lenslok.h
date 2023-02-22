#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineController.h"
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QWidget>


namespace gui
{

class Lenslok : public QWidget
{
	MachineController* controller;

	QPixmap	 background_a;
	QPixmap	 background_b;
	QPixmap* background;
	QMenu*	 contextmenu;
	QAction* actions[10];
	QTimer*	 timer;
	uint	 game_id;
	bool	 ignore_focusout;

	// state:
	int	   click_dx, click_dy; // after click, during move
	double click_t0;		   // after click
	QPoint click_p0;		   // after click

public:
	Lenslok(MachineController*, cstr name1, cstr name2);
	~Lenslok() override;

protected:
	void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	bool event(QEvent*) override;
	void focusOutEvent(QFocusEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;
	void contextMenuEvent(QContextMenuEvent*) override;
	// void	enterEvent(QEvent*) override;
	// void	leaveEvent(QEvent*) override;
	void moveEvent(QMoveEvent*) override;

private:
	void draw_prism(QPainter&, QRectF, const QRectF&);
	void select_game();
};

} // namespace gui
