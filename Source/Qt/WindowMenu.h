// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include <QMenu>

class QMainWindow;
class QAction;

namespace zxsp
{

/*
	This QMenu subclass implements a "Window" menu
	each MainWindow has it's own "Window" menu
	this->action is the menu item for the owner's window
*/


class WindowMenu : public QMenu
{
	QMainWindow* window;	// owner
	QAction*	 action;	// item which represents the owner window in this menu
	QAction*	 separator; // a separator

	void addWindow(QAction*);
	void rmWindow(QAction*);


	// ---- P U B L I C ------------

public:
	WindowMenu(QMainWindow*);
	~WindowMenu() override;

	void checkWindows();
	void setTitle();
};

} // namespace zxsp
