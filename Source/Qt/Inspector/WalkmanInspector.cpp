// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "WalkmanInspector.h"
#include "Application.h"
#include "Audio/TapeFileDataBlock.h"
#include "MachineController.h"
#include "Qt/qt_util.h"
#include "TapeRecorder.h"
#include "Templates/NVPtr.h"
#include "Templates/RCPtr.h"
#include "unix/files.h"
#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QVBoxLayout>


namespace gui
{

WalkmanInspector::WalkmanInspector(QWidget* parent, MachineController* mc, volatile Walkman* item) :
	TapeRecorderInsp(
		parent, mc, item, QPoint(58, 113), // major info
		QPoint(58, 124),				   // minor info
		QPoint(120, 141),				   // tape counter
		"Images/tape/walkman_hdgr.jpg",	   // tr_image
		"Images/tape/walkman_lid.png",	   // tr_window_background_image,
		QPoint(23, 7),					   // tr_window_pos,
		head_up,						   // tr_head_position
		"Images/tape/axis_plus2.png",	   // tr_axis_image,
		6,								   // tr_axis_symmetries,
		95, 193,
		91) // axis_x1, _x2, _y
{
	xlogIn("new WalkmanInspector");

	const int btn_x = 25; // position and size of first button
	const int btn_y = 172;
	const int btn_w = 34;
	//	const int btn_h = 27;

	btn_eject = new MySimpleToggleButton(
		this, btn_x + 0 * btn_w, btn_y, ":/Icons/button_eject.png", ":/Icons/button_eject_on.png", yes);
	btn_record = new MySimpleToggleButton(
		this, btn_x + 1 * btn_w, btn_y, ":/Icons/button_record.png", ":/Icons/button_record_on.png", no);
	btn_back = new MySimpleToggleButton(
		this, btn_x + 3 * btn_w, btn_y, ":/Icons/button_back.png", ":/Icons/button_back_on.png", yes);
	btn_play = new MySimpleToggleButton(
		this, btn_x + 4 * btn_w, btn_y, ":/Icons/button_pause.png", ":/Icons/button_run.png", no);
	btn_fore = new MySimpleToggleButton(
		this, btn_x + 5 * btn_w, btn_y, ":/Icons/button_fore.png", ":/Icons/button_fore_on.png", yes);
	btn_prev = new MySimpleToggleButton(
		this, btn_x + 2 * btn_w, btn_y, ":/Icons/button_prev.png", ":/Icons/button_prev_on.png", yes);
	btn_next = new MySimpleToggleButton(
		this, btn_x + 6 * btn_w, btn_y, ":/Icons/button_next.png", ":/Icons/button_next_on.png", yes);

#define NVTR nvptr(&dynamic_cast<volatile TapeRecorder&>(*object))
	connect(btn_eject, &MySimpleToggleButton::toggled, this, [=] { handleEjectButton(); });
	connect(btn_record, &MySimpleToggleButton::toggled, this, [=] { NVTR->record(); });
	connect(btn_back, &MySimpleToggleButton::toggled, this, [=] { NVTR->pause(0)->rewind(); });
	connect(btn_play, &MySimpleToggleButton::toggled, this, [=] { NVTR->pause(0)->togglePlay(); });
	connect(btn_fore, &MySimpleToggleButton::toggled, this, [=] { NVTR->pause(0)->wind(); });
	connect(btn_prev, &MySimpleToggleButton::toggled, this, [=] { NVTR->pause(1)->rewind(); });
	connect(btn_next, &MySimpleToggleButton::toggled, this, [=] { NVTR->pause(1)->wind(); });

	// timer->start(1000/60);		started by TapeRecorderInsp
}

void WalkmanInspector::updateWidgets()
{
	xlogIn("WalkmanInspector::updateWidgets");

	if (!machine || !object) return;
	auto* taperecorder = dynamic_cast<volatile TapeRecorder*>(object);
	if (!taperecorder) return;

	// note: don't not call super class TapeRecorderInsp::updateWidgets()

	updateAnimation();

	btn_record->setDown(taperecorder->isRecordDown());
	btn_prev->setDown(taperecorder->isRewinding() && taperecorder->isPauseDown());
	btn_back->setDown(taperecorder->isRewinding() && !taperecorder->isPauseDown());
	btn_play->setDown(taperecorder->isPlayDown());
	btn_fore->setDown(taperecorder->isWinding() && !taperecorder->isPauseDown());
	btn_next->setDown(taperecorder->isWinding() && taperecorder->isPauseDown());

	// update window title:
	// da die Machine ein Tape direkt einlegen kann und ich da (und sonstwo noch) nicht immer nach einem
	// Taperecorder Inspector suchen will, wird das hier gepollt => single place.
	// updateCustomTitle() ist mit dem ToolWindow verbunden, das danach getCustomTitle() aufruft.
	cstr new_filepath = taperecorder->getFilepath();
	if (tape_filepath != new_filepath)
	{
		tape_filepath = new_filepath;
		emit updateCustomTitle();
	}
}

} // namespace gui
