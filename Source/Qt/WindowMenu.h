#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QMenu>

class QMainWindow;
class QAction;


/*
	This QMenu subclass implements a "Window" menu
	each MainWindow has it's own "Window" menu
	this->action is the menu item for the owner's window
*/


class WindowMenu : public QMenu
{
	QMainWindow* window;            // owner
	QAction*	 action;            // item which represents the owner window in this menu
	QAction*	 separator;         // a separator

	void addWindow(QAction*);
	void rmWindow(QAction*);


// ---- P U B L I C ------------

public:
	WindowMenu  (QMainWindow*);
	~WindowMenu ();

	void checkWindows();
	void setTitle();

};









