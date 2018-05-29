/*	Copyright  (c)	Günter Woigk 2002 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef Z80INSP_H
#define Z80INSP_H

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


#endif


























