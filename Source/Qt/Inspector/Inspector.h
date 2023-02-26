#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IsaObject.h"
#include "Templates/RCPtr.h"
#include "cpp/cppthreads.h"
#include "kio/peekpoke.h"
#include "zxsp_types.h"
#include <QLineEdit>
#include <QToolBar>


namespace gui
{

class Inspector : public QWidget
{
	Q_OBJECT
	NO_COPY_MOVE(Inspector);

	friend class ToolWindow;

protected:
	MachineController* const  controller;
	volatile Machine* const	  machine;
	volatile IsaObject* const object;
	QPixmap					  background;
	bool					  is_visible  = false;
	QTimer*					  timer		  = nullptr;
	QMenu*					  contextmenu = nullptr;
	QToolBar*				  toolbar	  = nullptr;

public:
	// Factory:
	static Inspector* newInspector(QWidget*, MachineController*, volatile IsaObject*);

	Inspector(QWidget*, MachineController*); // empty Inspector
	~Inspector() override;

protected:
	Inspector(QWidget*, MachineController*, volatile IsaObject*, cstr bgr = "/Backgrounds/light-grey-75.jpg");

	void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	bool event(QEvent*) override;
	void contextMenuEvent(QContextMenuEvent*) override;
	void showEvent(QShowEvent*) override { is_visible = true; }
	void hideEvent(QHideEvent*) override { is_visible = false; }

	virtual void fillContextMenu(QMenu*) {}
	virtual void saveSettings() {}					  // called in Inspector dtor
	virtual void adjustSize(QSize&) {}				  // from ToolWindow
	virtual void adjustMaxSizeDuringResize() {}		  // from ToolWindow
	virtual cstr getCustomTitle() { return nullptr; } // override if inspector wishes a customized title
	virtual void updateWidgets() {}					  // called by timer. Timer must be started by subclass ctor.

	static QLineEdit* newLineEdit(cstr text, int min_width = 80);

	bool validReference(volatile Item* item);

signals:
	void signalSizeConstraintsChanged(); // -> min, max, fix size, size incr, shrinktofit
	void updateCustomTitle();			 // customized title changed (--> getCustomTitle())
};

} // namespace gui
