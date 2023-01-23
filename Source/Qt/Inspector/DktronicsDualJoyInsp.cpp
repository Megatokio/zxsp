// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>
#include "DktronicsDualJoyInsp.h"
#include "Item.h"

DktronicsDualJoyInsp::DktronicsDualJoyInsp(QWidget* w, MachineController* mc, volatile IsaObject* item )
:
	JoyInsp(w,mc,item,"/Images/dktronics_dual_js_if.jpg")
{
	assert(item->isA(isa_DktronicsDualJoy));

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10,10,10,5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0,100);
	g->setColumnStretch(0,50);
	g->setColumnStretch(1,50);

	g->addWidget( joystick_selectors[1],1,0 );
	g->addWidget( joystick_selectors[0],1,1 );
	g->addWidget( lineedit_display[1],2,0 );
	g->addWidget( lineedit_display[0],2,1 );

	g->addWidget( button_scan_usb, 3,0, Qt::AlignHCenter|Qt::AlignVCenter );
	g->addWidget( button_set_keys, 3,1, Qt::AlignHCenter|Qt::AlignVCenter );
}
