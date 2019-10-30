/*	Copyright  (c)	Günter Woigk 2009 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
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






