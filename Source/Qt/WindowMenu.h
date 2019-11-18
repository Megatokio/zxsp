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

#ifndef WINDOWMENU_H
#define WINDOWMENU_H

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


#endif // WINDOWMENU_H






