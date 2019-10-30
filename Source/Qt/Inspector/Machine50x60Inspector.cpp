/*	Copyright  (c)	Günter Woigk 2012 - 2018
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


#define LOGLEVEL 1
#include "Machine50x60Inspector.h"
#include "Machine.h"
#include "IsaObject.h"
#include "Ula/UlaZx80.h"
#include <QSettings>
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "Templates/NVPtr.h"
#include "MachineController.h"
#include <QButtonGroup>

/*	Inspector for Machine
	includes 50/60 Hz switching
*/

Machine50x60Inspector::Machine50x60Inspector(QWidget* p, MachineController* mc, volatile Machine* m)
:
	MachineInspector(p,mc,m)
{
	assert(controller->action_setSpeed100_50 != NULL);
	assert(controller->action_setSpeed100_60 != NULL);

	QRadioButton* btn_50hz = new QRadioButton("50 Hz",this); btn_50hz->move(7,10);
	QRadioButton* btn_60hz = new QRadioButton("60 Hz",this); btn_60hz->move(7,30);

	btn_50hz->setAutoExclusive(off);	// because we want to be able to switch them both off
	btn_60hz->setAutoExclusive(off);	// if speed 200/400/800% is selected

	bool bonw = 1;	// black on white
	if(m->model == zx80) bonw=0;
	if(m->model == jupiter)	bonw=0;

	const QRgb fore = bonw ? 0 : 0xffffffff;
	const QRgb back = bonw ? 0xffffffff : 0;

	setColors(btn_50hz, fore, back);
	setColors(btn_60hz, fore, back);

	QAction* action_50hz = controller->action_setSpeed100_50;
	QAction* action_60hz = controller->action_setSpeed100_60;

	btn_50hz->setChecked(action_50hz->isChecked());
	btn_60hz->setChecked(action_60hz->isChecked());

	connect(btn_50hz, &QRadioButton::toggled, action_50hz, [=](bool f){if(f)action_50hz->setChecked(1);});
	connect(btn_60hz, &QRadioButton::toggled, action_60hz, [=](bool f){if(f)action_60hz->setChecked(1);});

	connect(action_50hz, &QAction::toggled, btn_50hz, &QAbstractButton::setChecked);
	connect(action_60hz, &QAction::toggled, btn_60hz, &QAbstractButton::setChecked);
}




