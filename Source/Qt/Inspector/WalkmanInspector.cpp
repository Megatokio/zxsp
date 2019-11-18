/*	Copyright  (c)	Günter Woigk 2012 - 2019
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

#include <QLabel>
#include <QVBoxLayout>
#include <QFrame>
#include "WalkmanInspector.h"
#include "TapeRecorder.h"
#include "Audio/TapeFileDataBlock.h"
#include "MachineController.h"
#include "Qt/qt_util.h"
#include <QPainter>
#include <QApplication>
#include "Application.h"
#include <QPaintEvent>
#include "unix/files.h"
#include "Templates/RCPtr.h"
#include "Templates/NVPtr.h"


WalkmanInspector::WalkmanInspector( QWidget* parent, MachineController* mc, volatile IsaObject* item )
:
	TapeRecorderInsp(parent,mc,item,
		QPoint(58,113),						// major info
		QPoint(58,124),						// minor info
		QPoint(120,141),					// tape counter
		"Images/tape/walkman_hdgr.jpg",		// tr_image
		"Images/tape/walkman_lid.png",		// tr_window_background_image,
		QPoint(23,7),						// tr_window_pos,
		head_up,							// tr_head_position
		"Images/tape/axis_plus2.png",		// tr_axis_image,
		6,									// tr_axis_symmetries,
		95,193,91)							// axis_x1, _x2, _y
{
	xlogIn("new WalkmanInspector");

	const int btn_x = 25;	// position and size of first button
	const int btn_y = 172;
	const int btn_w = 34;
//	const int btn_h = 27;

	btn_eject = new MySimpleToggleButton(this, btn_x+0*btn_w, btn_y,
				":/Icons/button_eject.png", ":/Icons/button_eject_on.png", yes);
	btn_record= new MySimpleToggleButton(this, btn_x+1*btn_w, btn_y,
				":/Icons/button_record.png",":/Icons/button_record_on.png",no );
	btn_back  = new MySimpleToggleButton(this, btn_x+3*btn_w, btn_y,
				":/Icons/button_back.png",  ":/Icons/button_back_on.png",  yes);
	btn_play  = new MySimpleToggleButton(this, btn_x+4*btn_w, btn_y,
				":/Icons/button_pause.png", ":/Icons/button_run.png",      no );
	btn_fore  = new MySimpleToggleButton(this, btn_x+5*btn_w, btn_y,
				":/Icons/button_fore.png",  ":/Icons/button_fore_on.png",  yes);
	btn_prev  = new MySimpleToggleButton(this, btn_x+2*btn_w, btn_y,
				":/Icons/button_prev.png",  ":/Icons/button_prev_on.png",  yes);
	btn_next  = new MySimpleToggleButton(this, btn_x+6*btn_w, btn_y,
				":/Icons/button_next.png",  ":/Icons/button_next_on.png",  yes);

	#define NVTR NVPtr<TapeRecorder>(tape_recorder())
	connect(btn_eject, &MySimpleToggleButton::toggled, this, [=]{handleEjectButton();});
	connect(btn_record,&MySimpleToggleButton::toggled, this, [=]{NVTR->record();});
	connect(btn_back,  &MySimpleToggleButton::toggled, this, [=]{NVTR->pause(0)->rewind();});
	connect(btn_play,  &MySimpleToggleButton::toggled, this, [=]{NVTR->pause(0)->togglePlay();});
	connect(btn_fore,  &MySimpleToggleButton::toggled, this, [=]{NVTR->pause(0)->wind();});
	connect(btn_prev,  &MySimpleToggleButton::toggled, this, [=]{NVTR->pause(1)->rewind();});
	connect(btn_next,  &MySimpleToggleButton::toggled, this, [=]{NVTR->pause(1)->wind();});

	//timer->start(1000/60);		started by TapeRecorderInsp
}

void WalkmanInspector::updateWidgets()
{
	xlogIn("WalkmanInspector::updateWidgets");

	if(!is_visible) return;
	if(!object) return;

	Inspector::updateWidgets();		// bypass TapeRecorderInsp::updateWidgets()
	updateAnimation();

	volatile TapeRecorder* t = tape_recorder();
	btn_record->setDown(t->isRecordDown());
	btn_prev->setDown(t->isRewinding() && t->isPauseDown());
	btn_back->setDown(t->isRewinding() && !t->isPauseDown());
	btn_play->setDown(t->isPlayDown());
	btn_fore->setDown(t->isWinding() && !t->isPauseDown());
	btn_next->setDown(t->isWinding() && t->isPauseDown());

	// update window title:
	// da die Machine ein Tape direkt einlegen kann und ich da (und sonstwo noch) nicht immer nach einem
	// Taperecorder Inspector suchen will, wird das hier gepollt => single place.
	// updateCustomTitle() ist mit dem ToolWindow verbunden, das danach getCustomTitle() aufruft.
	cstr new_filepath = tape_recorder()->getFilepath();
	if(tape_filepath != new_filepath)
	{
		tape_filepath = new_filepath;
		emit updateCustomTitle();
	}
}




























