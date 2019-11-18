#pragma once
/*	Copyright  (c)	Günter Woigk 2009 - 2019
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

#include <QMainWindow>
#include <QMenu>
#include <QTimer>
#include "kio/kio.h"
#include "zxsp_types.h"
#include "IsaObject.h"


class ToolWindow : public QMainWindow
{
	friend class MachineController;

	MachineController* const machine_controller;
	volatile IsaObject* item;		// item or machine
	Inspector*	inspector;
	QAction*	show_action;
	QMenu		contextmenu;		// not used
	QTimer		adjust_size_timer;	// check size
	QToolBar*	toolbar;
	int			toolbar_height;
	isa_id		grp_id;				// of item

	void		init(volatile IsaObject* = NULL, QAction* showaction = NULL);
	void		kill();
	void		save_window_position();
	void		restore_window_position();
	void		set_window_title();
	void		adjust_window_size();

protected:
	void		resizeEvent(QResizeEvent*) override;
	void		contextMenuEvent(QContextMenuEvent*) override;
	void		keyPressEvent(QKeyEvent*) override;
	void		keyReleaseEvent(QKeyEvent*) override;
//	void		mousePressEvent(QMouseEvent*) override;
//	void		hideEvent(QHideEvent*) override;
//	void		mouseReleaseEvent(QMouseEvent*) override;
//	void		mouseMoveEvent(QMouseEvent* event) override;
//	bool		event(QEvent*) override;
//	bool		eventFilter(QObject*, QEvent*) override;

private:		ToolWindow(MachineController*, volatile IsaObject* item, QAction* showaction);
public:			~ToolWindow();
	void		fillContextMenu(QMenu*);
};



























