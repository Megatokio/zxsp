#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MultifaceInsp.h"
class QLineEdit;
class QCheckBox;
class QComboBox;
class QPushButton;


namespace gui
{

class Multiface1Insp : public MultifaceInsp
{
	QCheckBox*	 chkbox_joystick_enabled;
	QLineEdit*	 lineedit_display;
	uint8		 lineedit_state;
	QComboBox*	 joystick_selector;
	QPushButton* button_scan_usb;

public:
	Multiface1Insp(QWidget*, MachineController*, volatile IsaObject*);

protected:
	void updateWidgets() override;

private:
	void update_joystick_selector();
	void find_usb_joysticks();
	void joystick_selected();
	void enable_joystick(bool);
};

} // namespace gui
