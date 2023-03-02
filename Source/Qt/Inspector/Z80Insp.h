#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include "Qt/MyLineEdit.h"
#include <QAction>
#include <QCheckBox>
#include <QLineEdit>


namespace gui
{

class Z80Insp : public Inspector
{
	volatile Z80* const cpu;

	union
	{
		struct
		{
			MyLineEdit *pc, *sp, *bc, *de, *hl, *ix, *iy, *bc2, *de2, *hl2, *clock, *cc, *a, *f, *a2, *f2, *flags, *im,
				*i, *r;
		};
		MyLineEdit* led[20];
	};

	QCheckBox *ie, *irpt, *nmi;

	struct
	{ // currently displayed values
		uint16	  pc, sp, bc, de, hl, ix, iy, bc2, de2, hl2;
		int32	  cc;
		uint8	  a, f, a2, f2, im, i, r, fstr;
		Frequency clock;
	} value;

public:
	Z80Insp(QWidget*, MachineController*, volatile Z80*);

protected:
	void updateWidgets() override;

private:
	MyLineEdit* new_led(cstr);
	void		slotReturnPressedInLineEdit(MyLineEdit* led);
	void		slotSetInterruptEnable(bool);
	void		slotSetNmi(bool);
	void		slotSetInterrupt(bool);
};

} // namespace gui
