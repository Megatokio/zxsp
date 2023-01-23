#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QAction>
#include <QLineEdit>
#include <QCheckBox>
#include "Inspector.h"
#include "Qt/MyLineEdit.h"


class Z80Insp : public Inspector
{
	union{
		struct{
		MyLineEdit	*pc,*sp,*bc,*de,*hl,*ix,*iy,*bc2,*de2,*hl2,
					*clock,*cc,*a,*f,*a2,*f2,*flags,*im,*i,*r;
		};
		MyLineEdit	*led[20];
	};

	QCheckBox		*ie,*irpt,*nmi;

	struct{			// currently displayed values
		uint16		pc,sp,bc,de,hl,ix,iy,bc2,de2,hl2;
		int32		cc;
		uint8		a,f,a2,f2,im,i,r,fstr;
		Frequency	clock;
		} value;

public:
	Z80Insp( QWidget*, MachineController*, volatile IsaObject* );

protected:
	void	updateWidgets() override;

private:
	MyLineEdit* new_led(cstr);
	void	return_pressed_in_lineedit(MyLineEdit* led);
	void	set_interrupt_enable(bool);
	void	set_nmi(bool);
	void	set_interrupt(bool);
};





























