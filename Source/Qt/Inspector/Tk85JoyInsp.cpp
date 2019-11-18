/*	Copyright  (c)	Günter Woigk 2012 - 2018
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


#include <QLabel>
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>
#include "Tk85JoyInsp.h"

Tk85JoyInsp::Tk85JoyInsp(QWidget*p, MachineController* mc, volatile IsaObject *j)
:
	JoyInsp(p,mc,j,"/Images/tk85_joy.jpg")
{
	assert(object->isA(isa_Tk85Joy));

	QLabel* label= new QLabel("Buttons:");

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10,10,10,5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0,100);
	g->setColumnStretch(0,25);
	g->setColumnStretch(1,25);
	g->setColumnStretch(2,50);

	g->addWidget( joystick_selectors[0],1,0, 1,2 );
	g->addWidget( button_set_keys, 1,2, Qt::AlignHCenter|Qt::AlignVCenter );
	g->addWidget( label,2,0);
	g->addWidget( lineedit_display[0],2,1 );
	g->addWidget( button_scan_usb, 2,2, Qt::AlignHCenter|Qt::AlignVCenter );
}
