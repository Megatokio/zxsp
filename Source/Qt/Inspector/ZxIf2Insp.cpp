/*	Copyright  (c)	Günter Woigk 2009 - 2018
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

#include <QtGui>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include "ZxIf2Insp.h"
#include "Joy/ZxIf2.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "Item.h"
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "RecentFilesMenu.h"
#include "MachineController.h"


ZxIf2Insp::ZxIf2Insp(QWidget*w, MachineController* mc, volatile IsaObject *i )
:
	SinclairJoyInsp(w,mc,i,"/Images/zxif2.jpg"),
	old_romfilepath(NULL)
{
    assert(object->isA(isa_ZxIf2));

    button_insert_eject = new QPushButton("Insert",this);
    button_insert_eject->setMinimumWidth(100);
	connect(button_insert_eject,&QPushButton::clicked,this,&ZxIf2Insp::insert_or_eject_rom);

	label_romfilename = new QLabel(this);
	label_romfilename->move(150,51);
	label_romfilename->setFont(QFont("Arial",10));
//	rom_name->setMinimumWidth(100);
	label_romfilename->setAlignment(Qt::AlignTop);
    setColors(label_romfilename, 0xffffff/*foregroundcolor*/);

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10,10,10,5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0,100);
	g->setColumnStretch(0,50);
	g->setColumnStretch(1,50);
	g->setColumnStretch(2,50);
	g->setColumnStretch(3,50);
	g->setColumnStretch(4,50);
	g->setColumnStretch(5,50);

	g->addWidget( joystick_selectors[0],1,0,1,3 );
	g->addWidget( joystick_selectors[1],1,3,1,3 );
	g->addWidget( lineedit_display[0],2,0,1,3 );
	g->addWidget( lineedit_display[1],2,3,1,3 );

	g->addWidget( button_scan_usb,     3,0,1,2, Qt::AlignHCenter|Qt::AlignVCenter );
	g->addWidget( button_set_keys,     3,2,1,2, Qt::AlignHCenter|Qt::AlignVCenter );
	g->addWidget( button_insert_eject, 3,4,1,2, Qt::AlignHCenter|Qt::AlignVCenter );

//	timer->start(1000/15);			// started by JoyInsp()
}


//slot
void ZxIf2Insp::insert_or_eject_rom()
{
    if(zxif2()->isLoaded())
    {
        xlogIn("ZxIf2Insp::eject()");
        bool f = machine->powerOff();
	        NV(zxif2())->ejectRom();
		if(f) machine->powerOn();
    }
    else
    {
        xlogIn("ZxIf2Insp::insert()");

        cstr filter = "IF2 Rom Cartridges (*.rom)"; //";;All Files (*)";
		cstr filepath = selectLoadFile(this, "Select IF2 Rom Cartridge", filter);
		if(!filepath) return;

        bool f = machine->powerOff();
			NV(zxif2())->insertRom(filepath);
		if(f) machine->powerOn();
	}
}


void ZxIf2Insp::updateWidgets()
{
	xlogIn("ZxIf2Insp::updateWidgets");

	if(!object) return;

	SinclairJoyInsp::updateWidgets();

	cstr new_romfilepath = zxif2()->getFilepath();
	if(old_romfilepath != new_romfilepath)
	{
		label_romfilename->setText(new_romfilepath ? basename_from_path(new_romfilepath) : NULL);

		if(!old_romfilepath) { background.load( catstr(appl_rsrc_path,"/Images/zxif2_with_cart.jpg") ); button_insert_eject->setText("Eject Rom");  update(); }
		if(!new_romfilepath) { background.load( catstr(appl_rsrc_path,"/Images/zxif2.jpg") );		    button_insert_eject->setText("Insert Rom"); update(); }

		old_romfilepath = new_romfilepath;
	}
}


/*	fill context menu for right-click
	called by Inspector::contextMenuEvent()
	items inserted here are inserted at the to of the popup menu
*/
void ZxIf2Insp::fillContextMenu(QMenu* menu)
{
	Inspector::fillContextMenu(menu);	// NOP

    if(zxif2()->isLoaded())
	{
		menu->addAction("Eject Rom",this,&ZxIf2Insp::insert_or_eject_rom);
	}
	else
	{
		menu->addAction("Insert Rom",this,&ZxIf2Insp::insert_or_eject_rom);
		menu->addAction("Recent Roms …")->setMenu(
				new RecentFilesMenu(RecentIf2Roms, this, [=](cstr fpath){insertRom(fpath);}));
	}
}


void ZxIf2Insp::insertRom(cstr filepath)
{
	bool f = machine->powerOff();
		if(zxif2()->isLoaded()) NV(zxif2())->ejectRom();
		NV(zxif2())->insertRom(filepath);
	if(f) machine->powerOn();
}













