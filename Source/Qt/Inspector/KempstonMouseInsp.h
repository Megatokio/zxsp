#pragma once
// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
class QLineEdit;
class QComboBox;
class QPushButton;


class KempstonMouseInsp : public Inspector
{
	// widgets:
	QLineEdit*	 display_x;
	QLineEdit*	 display_y;
	QLineEdit*	 display_buttons;
	QComboBox*	 combobox_scale;
	QPushButton* button_grab_mouse;

	// widgets state:
	uint8 old_x, old_y; // displayed x/y position
	int	  old_buttons;	// displayed button state
	bool  old_grabbed;

public:
	KempstonMouseInsp(QWidget*, MachineController*, volatile IsaObject*);

protected:
	void updateWidgets() override;
};
