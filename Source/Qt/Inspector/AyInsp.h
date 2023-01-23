#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QLineEdit>
#include <QComboBox>
#include "Inspector.h"

class AyInsp : public Inspector
{
	QLineEdit	*clock, *pitch_a,*pitch_b,*pitch_c,*mixer,
				*vol_a,*vol_b,*vol_c,*pitch_n,*pitch_e,
				*shape_e, *port_a, *port_b;

	QComboBox	*stereo;

	struct
	{
		Frequency	clock;
		int			stereo;
		uint8		regs[16];
	} value;

public:
	AyInsp( QWidget*, MachineController*, volatile IsaObject* );

protected:
	void		updateWidgets() override;

private:
	QLineEdit*	new_led(cstr);
	void		set_register(uint,uint);
	void		handle_return_in_led(QLineEdit*);
};





























