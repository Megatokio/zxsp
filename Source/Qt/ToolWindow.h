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

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS sOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT sHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY sPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS sOFTWARE.
*/

#ifndef TOOLWINDOW_H
#define TOOLWINDOW_H

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


#endif
























