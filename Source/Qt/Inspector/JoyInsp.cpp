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

#include <QtGui>
#include <QComboBox>
#include <QPushButton>
#include "JoyInsp.h"
#include "Joy/Joy.h"
#include "Joystick.h"
#include "Dialogs/ConfigDialog.h"
#include "Dialogs/ConfigureKeyboardJoystickDialog.h"
#include "Templates/NVPtr.h"


JoyInsp::JoyInsp( QWidget* w, MachineController* mc, volatile IsaObject*j, cstr imgpath )
: Inspector(w,mc,j,imgpath)
{
	assert(object->isA(isa_Joy));

	num_ports = joy()->getNumPorts();
	xlogIn("new JoyInsp for %s (%i ports)", object->name, num_ports);

	joystick_selectors[1] = joystick_selectors[2] = NULL;
	lineedit_display[1] = lineedit_display[2] = NULL;

	for( int i=0; i<num_ports; i++ )
	{
		lineedit_display[i] = new QLineEdit(this);
		lineedit_display[i]->setText("%--------"); lineedit_state[i]=0;
		lineedit_display[i]->setAlignment(Qt::AlignHCenter);
		lineedit_display[i]->setReadOnly(yes);
		lineedit_display[i]->setMinimumWidth(100);
		joystick_selectors[i] = new QComboBox(this);
		joystick_selectors[i]->setFocusPolicy(Qt::NoFocus);
		joystick_selectors[i]->setMinimumWidth(80);
		connect(joystick_selectors[i], static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
				this, &JoyInsp::joystick_selected);
	}

	update_joystick_selectors();

	button_scan_usb = new QPushButton("Scan USB",this);
	button_scan_usb->setMinimumWidth(100);
	connect(button_scan_usb,&QPushButton::clicked,this,&JoyInsp::find_usb_joysticks);

	button_set_keys = new QPushButton("Set Keys",this);
	button_set_keys->setMinimumWidth(100);
	connect(button_set_keys,&QPushButton::clicked,this,&JoyInsp::set_keyboard_joystick_keys);

	timer->start(1000/10);
}


void JoyInsp::find_usb_joysticks()
{
	xlogIn("JoyInsp::scanUSB");
	findUsbJoysticks();
	update_joystick_selectors();
}


void JoyInsp::set_keyboard_joystick_keys()
{
	xlogIn("JoyInsp::setKeys");

	ConfigDialog* d = new ConfigureKeyboardJoystickDialog(controller);
	d->show();
}


void JoyInsp::joystick_selected()
{
	xlogIn("JoyInsp::joySelected");

	for(int i=0; i<num_ports; i++)
	{
		int j = joystick_selectors[i]->currentIndex();
		NVPtr<Joy>(joy())->insertJoystick(i, joystick_selectors[i]->itemData(j).toInt());
	}
}


void JoyInsp::updateWidgets()		// Kempston
{
	xlogIn("JoyInsp::updateWidgets");

	if(!object) return;

	for(int i=0; i<num_ports; i++)
	{
		uint8 newstate = joy()->getStateForInspector(i);
		if(newstate==lineedit_state[i]) continue;

		char s[] = "%111FUDLR";
		for(int j=0;j<8;j++){ if(((~newstate)<<j)&0x80) s[j+1]='-'; }
		lineedit_display[i]->setText(s);
		lineedit_state[i] = newstate;
	}
}


void JoyInsp::update_joystick_selectors()
{
	bool is_connected[max_joy]; int i;
	bool is_in_list[max_joy] = {0,0,0,0,0};

	// find real-world joysticks:
	for(i=0; i<max_joy; i++) { is_connected[i] = joysticks[i]->isConnected(); }

	// joysticks currently in selector list:
	for(i=0; i<joystick_selectors[0]->count(); i++)
		is_in_list[joystick_selectors[0]->itemData(i).toInt()] = true;

	// compare:
	for(i=0; i<max_joy; i++) { if(is_connected[i] != is_in_list[i]) break; }
	if(i==max_joy) return;	// no change

	// first call or a joystick has been plugged / unplugged:

	static cstr jname[5] = { "USB Joystick 1", "USB Joystick 2", "USB Joystick 3", "Keyboard", "no Joystick" };

	// if selectors send events then slotSelectJoystick() will mess up the settings:
	for(int s=0; s<num_ports; s++) { joystick_selectors[s]->blockSignals(1); }

	// for all ports of this interface:
	for(int s=0; s<num_ports; s++)
	{
		// empty selector list:
		while(joystick_selectors[s]->count()) { joystick_selectors[s]->removeItem(0); }

		// add existing real-world joysticks to selector list
		// and select the currently selected one, default = no_joy:
		int selected_id = joy()->getJoystickID(s);	// id of the real-world joystick
		int selected_idx = -1;				// index in list
		for(i=0; i<max_joy; i++)
		{
			if(!is_connected[i]) continue;
			if(i == selected_id) selected_idx = joystick_selectors[s]->count();
			joystick_selectors[s]->addItem(jname[i],i);
		}
		joystick_selectors[s]->setCurrentIndex(selected_idx>=0 ? selected_idx : joystick_selectors[s]->count()-1);
	}

	for(int s=0; s<num_ports; s++) { joystick_selectors[s]->blockSignals(0); }
}





















