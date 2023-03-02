// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#define LOGLEVEL 1
#include "Machine50x60Inspector.h"
#include "Machine.h"
#include "MachineController.h"
#include "Qt/qt_util.h"
#include "Templates/NVPtr.h"
#include <QButtonGroup>
#include <QSettings>


namespace gui
{

/*	Inspector for Machine
	includes 50/60 Hz switching
*/

Machine50x60Inspector::Machine50x60Inspector(QWidget* p, MachineController* mc, volatile Machine* m) :
	MachineInspector(p, mc, m)
{
	assert(mc->action_setSpeed100_50 != nullptr);
	assert(mc->action_setSpeed100_60 != nullptr);

	QRadioButton* btn_50hz = new QRadioButton("50 Hz", this);
	btn_50hz->move(7, 10);
	QRadioButton* btn_60hz = new QRadioButton("60 Hz", this);
	btn_60hz->move(7, 30);

	btn_50hz->setAutoExclusive(off); // because we want to be able to switch them both off
	btn_60hz->setAutoExclusive(off); // if speed 200/400/800% is selected

	bool bonw = 1; // black on white
	if (m->model == zx80) bonw = 0;
	if (m->model == jupiter) bonw = 0;

	const QRgb fore = bonw ? 0 : 0xffffffff;
	const QRgb back = bonw ? 0xffffffff : 0;

	setColors(btn_50hz, fore, back);
	setColors(btn_60hz, fore, back);

	QAction* action_50hz = mc->action_setSpeed100_50;
	QAction* action_60hz = mc->action_setSpeed100_60;

	btn_50hz->setChecked(action_50hz->isChecked());
	btn_60hz->setChecked(action_60hz->isChecked());

	connect(btn_50hz, &QRadioButton::toggled, action_50hz, [=](bool f) {
		if (f) action_50hz->setChecked(1);
	});
	connect(btn_60hz, &QRadioButton::toggled, action_60hz, [=](bool f) {
		if (f) action_60hz->setChecked(1);
	});

	connect(action_50hz, &QAction::toggled, btn_50hz, &QAbstractButton::setChecked);
	connect(action_60hz, &QAction::toggled, btn_60hz, &QAbstractButton::setChecked);
}

} // namespace gui
