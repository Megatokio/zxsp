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
#include <QAction>
#include <QComboBox>
#include <QVariant>
#include <QSettings>
#include <QList>
#include <QRgb>
#include "Qt/qt_util.h"
#include "Qt/Settings.h"
#include "Items/Item.h"
#include "Memotech64kRamInsp.h"
#include "Items/Ram/Memotech64kRam.h"
#include "Machine/Machine.h"
#include "Templates/NVPtr.h"


static uint dipsw[5] = {8,4,2,6,1};  // {0b1000,0b0100,0b0010,0b0110,0b0001};


Memotech64kRamInsp::Memotech64kRamInsp(QWidget* p, MachineController* mc, volatile IsaObject* item)
:
    Inspector(p,mc,item,"/Images/memopak64k.jpg")
{
	assert(object->isA(isa_Memotech64kRam));

	jumper = new QComboBox(this);
	jumper->move(10,6);
	jumper->setFocusPolicy(Qt::NoFocus);

	jumper->insertItems( 0, QStringList()
		<< "1---:  all 64k Ram"  << "-1--:  8-12k: Ram"
		<< "--1-:  12-16k: Ram"  << "-11-:  8-16k: Ram"
		<< "---1:  8-16k: no Ram" );

	switch(memotech64kram()->getDipSwitches())//settings.value(key_memotech64k_dip_switches,6/*0b0110*/).toInt())
	{
	case /*0b1000*/ 8:    jumper->setCurrentIndex(0); break;
	case /*0b0100*/ 4:    jumper->setCurrentIndex(1); break;
	case /*0b0010*/ 2:    jumper->setCurrentIndex(2); break;
	case /*0b0110*/ 6:    jumper->setCurrentIndex(3); break;
	case /*0b0001*/ 1:    jumper->setCurrentIndex(4); break;
	}

	connect(jumper, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
			[=](int i){NVPtr<Memotech64kRam>(memotech64kram())->setDipSwitches(dipsw[i]);});

	QLabel* l = new QLabel("POKE 16388,255 : POKE 16389,255 : NEW",this);
	l->setFont(QFont("Lucida Grande",11));
	setColors(l,0xcccccc);
	l->move(10,145);
}












