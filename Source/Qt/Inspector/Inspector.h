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

#include <QLineEdit>
#include "zxsp_types.h"
#include "IsaObject.h"
#include <QToolBar>
#include "Templates/RCPtr.h"
#include "kio/peekpoke.h"
#define VIR virtual

class Inspector : public QWidget
{
	Q_OBJECT

	friend class ToolWindow;

protected:
	MachineController* controller;
	volatile IsaObject*  object;
	volatile Machine*	machine;
	QPixmap		background;
	bool		is_visible;
	QTimer*		timer;
	QMenu*		contextmenu;
	QToolBar*	toolbar;

	volatile Item*				item()				{ return ItemPtr(object); }
	volatile Ula*				ula()				{ return UlaPtr(object); }
	volatile Keyboard*			kbd()				{ return KeyboardPtr(object); }
	volatile Z80*				cpu()				{ return Z80Ptr(object); }
	volatile Joy*				joy()				{ return JoyPtr(object); }
	volatile Ay*				ay()				{ return AyPtr(object); }
	volatile ZxIf2*				zxif2()				{ return ZxIf2Ptr(object); }
	volatile Zx3kRam*			zx3kram()			{ return Zx3kRamPtr(object); }
	volatile IcTester*			ic_tester()			{ return IcTesterPtr(object); }
	volatile Memotech64kRam*	memotech64kram()	{ return Memotech64kRamPtr(object); }
	volatile KempstonMouse*		mif()				{ return KempstonMousePtr(object); }
	volatile TapeRecorder*		tape_recorder()		{ return TapeRecorderPtr(object); }
	volatile SpectraVideo*		spectra()			{ return SpectraVideoPtr(object); }
	volatile MmuTc2068*			dock()				{ return MmuTc2068Ptr(object); }
	volatile DivIDE*			divide()			{ return DivIDEPtr(object); }
	volatile CurrahMicroSpeech* currah_uspeech()	{ return CurrahMicroSpeechPtr(object); }
	volatile Multiface*			multiface()			{ return MultifacePtr(object); }
	volatile Multiface1*		multiface1()		{ return Multiface1Ptr(object); }
	volatile Multiface128*		multiface128()		{ return Multiface128Ptr(object); }
	volatile Multiface3*		multiface3()		{ return Multiface3Ptr(object); }

public:
	// Inspector Factory:
	static Inspector* newInspector(QWidget*, MachineController*, volatile IsaObject*);
	Inspector(const Inspector&) = delete;
	Inspector& operator=(const Inspector&) = delete;
	~Inspector() override;

protected:
	Inspector(QWidget*, MachineController*, volatile IsaObject*, cstr bgr = "/Backgrounds/light-grey-75.jpg");

	void	paintEvent(QPaintEvent*) override;
	void	mousePressEvent(QMouseEvent*) override;
	bool	event(QEvent*) override;
	void	contextMenuEvent(QContextMenuEvent*) override;
	void	showEvent(QShowEvent*) override		{ is_visible = true; }
	void	hideEvent(QHideEvent*) override		{ is_visible = false; }

VIR	void	fillContextMenu(QMenu*)		{}
VIR	void	saveSettings()				{}		// called in Inspector dtor
VIR	void	adjustSize(QSize&)			{}		// from ToolWindow
VIR	void	adjustMaxSizeDuringResize()	{}		// from ToolWindow
VIR	cstr	getCustomTitle()			{ return NULL; } // override if inspector wishes a customized title
VIR	void	updateWidgets()				{}		// called by timer. Timer must be started by subclass ctor.

	static QLineEdit* newLineEdit(cstr text, int min_width=80);

signals:
	void	signalSizeConstraintsChanged();		// -> min, max, fix size, size incr, shrinktofit
	void	updateCustomTitle();				// customized title changed (--> getCustomTitle())
};





