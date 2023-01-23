#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"


class JoyInsp : public Inspector
{
protected:
	int				num_ports;
	class QLineEdit*		lineedit_display[3];	uint8 lineedit_state[3];
	class QComboBox*		joystick_selectors[3];
	class QPushButton*	button_scan_usb;
	class QPushButton*	button_set_keys;

public:
	JoyInsp( QWidget*, MachineController*, volatile IsaObject*, cstr img_path );

protected:
	void	updateWidgets() override;

private:
	void	update_joystick_selectors();
	void	find_usb_joysticks();
	void	set_keyboard_joystick_keys();
	void	joystick_selected();
};





























