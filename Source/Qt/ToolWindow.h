#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IsaObject.h"
#include "kio/kio.h"
#include "zxsp_types.h"
#include <QMainWindow>
#include <QMenu>
#include <QTimer>


class ToolWindow : public QMainWindow
{
	friend class MachineController;

	MachineController* const machine_controller;
	volatile IsaObject*		 item; // item or machine
	Inspector*				 inspector;
	QAction*				 show_action;
	QMenu					 contextmenu;		// not used
	QTimer					 adjust_size_timer; // check size
	QToolBar*				 toolbar;
	int						 toolbar_height;
	isa_id					 grp_id; // of item

	void init(volatile IsaObject* = nullptr, QAction* showaction = nullptr);
	void kill();
	void save_window_position();
	void restore_window_position();
	void set_window_title();
	void adjust_window_size();

protected:
	void resizeEvent(QResizeEvent*) override;
	void contextMenuEvent(QContextMenuEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;
	//	void		mousePressEvent(QMouseEvent*) override;
	//	void		hideEvent(QHideEvent*) override;
	//	void		mouseReleaseEvent(QMouseEvent*) override;
	//	void		mouseMoveEvent(QMouseEvent* event) override;
	//	bool		event(QEvent*) override;
	//	bool		eventFilter(QObject*, QEvent*) override;

private:
	ToolWindow(MachineController*, volatile IsaObject* item, QAction* showaction);

public:
	~ToolWindow();
	void fillContextMenu(QMenu*);
};
