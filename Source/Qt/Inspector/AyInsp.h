// Copyright (c) 2002 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Inspector.h"
#include <QComboBox>
#include <QLineEdit>
class QGridLayout;

namespace zxsp
{

class AyInsp : public Inspector
{
	volatile Ay* const ay;

	QLineEdit *clock, *pitch_a, *pitch_b, *pitch_c, *mixer, *vol_a, *vol_b, *vol_c, *pitch_n, *pitch_e, *shape_e,
		*port_a, *port_b;

	QComboBox* stereo;

	struct
	{
		Frequency clock;
		int		  stereo;
		uint8	  regs[16];
	} value;

protected:
	QGridLayout* layout;

public:
	AyInsp(QWidget*, MachineController*, volatile Ay*, cstr background = nullptr);

protected:
	void updateWidgets() override;

private:
	QLineEdit* new_led(cstr);
	void	   set_register(volatile Ay* ay, uint, uint8);
	void	   handle_return_in_led(QLineEdit*);
};

} // namespace zxsp
