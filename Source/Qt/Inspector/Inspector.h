#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IsaObject.h"
#include "Templates/RCPtr.h"
#include "kio/peekpoke.h"
#include "zxsp_types.h"
#include <QLineEdit>
#include <QToolBar>

namespace gui
{

class Inspector : public QWidget
{
	Q_OBJECT

	friend class ToolWindow;

protected:
	MachineController*	controller;
	volatile IsaObject* object;
	volatile Machine*	machine;
	QPixmap				background;
	bool				is_visible;
	QTimer*				timer;
	QMenu*				contextmenu;
	QToolBar*			toolbar;

	volatile Item*				item() { return ItemPtr(object); }
	volatile Ula*				ula() { return UlaPtr(object); }
	volatile Keyboard*			kbd() { return KeyboardPtr(object); }
	volatile Z80*				cpu() { return Z80Ptr(object); }
	volatile Joy*				joy() { return JoyPtr(object); }
	volatile Ay*				ay() { return AyPtr(object); }
	volatile ZxIf2*				zxif2() { return ZxIf2Ptr(object); }
	volatile Zx3kRam*			zx3kram() { return Zx3kRamPtr(object); }
	volatile IcTester*			ic_tester() { return IcTesterPtr(object); }
	volatile Memotech64kRam*	memotech64kram() { return Memotech64kRamPtr(object); }
	volatile KempstonMouse*		mif() { return KempstonMousePtr(object); }
	volatile TapeRecorder*		tape_recorder() { return TapeRecorderPtr(object); }
	volatile SpectraVideo*		spectra() { return SpectraVideoPtr(object); }
	volatile MmuTc2068*			dock() { return MmuTc2068Ptr(object); }
	volatile DivIDE*			divide() { return DivIDEPtr(object); }
	volatile CurrahMicroSpeech* currah_uspeech() { return CurrahMicroSpeechPtr(object); }
	volatile Multiface*			multiface() { return MultifacePtr(object); }
	volatile Multiface1*		multiface1() { return Multiface1Ptr(object); }
	volatile Multiface128*		multiface128() { return Multiface128Ptr(object); }
	volatile Multiface3*		multiface3() { return Multiface3Ptr(object); }

public:
	// Inspector Factory:
	static Inspector* newInspector(QWidget*, MachineController*, volatile IsaObject*);
	Inspector(const Inspector&)			   = delete;
	Inspector& operator=(const Inspector&) = delete;
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

signals:
	void signalSizeConstraintsChanged(); // -> min, max, fix size, size incr, shrinktofit
	void updateCustomTitle();			 // customized title changed (--> getCustomTitle())
};

} // namespace gui
