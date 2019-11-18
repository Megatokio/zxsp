#pragma once
/*	Copyright  (c)	Günter Woigk 2002 - 2019
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





























