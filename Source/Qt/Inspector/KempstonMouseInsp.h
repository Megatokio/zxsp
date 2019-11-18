#pragma once
/*	Copyright  (c)	Günter Woigk 2006 - 2019
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
class QLineEdit;
class QComboBox;
class QPushButton;


class KempstonMouseInsp : public Inspector
{
// widgets:
	QLineEdit*		display_x;
	QLineEdit*		display_y;
	QLineEdit*		display_buttons;
	QComboBox*		combobox_scale;
	QPushButton*	button_grab_mouse;

// widgets state:
	uint8			old_x, old_y;		// displayed x/y position
	int				old_buttons;		// displayed button state
	bool			old_grabbed;

public:
	KempstonMouseInsp( QWidget*, MachineController*, volatile IsaObject* );

protected:
	void			updateWidgets() override;
};



