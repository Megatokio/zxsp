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





























