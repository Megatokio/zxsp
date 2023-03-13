// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZxIf2Insp.h"
#include "Item.h"
#include "Machine.h"
#include "MachineController.h"
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "RecentFilesMenu.h"
#include "Templates/NVPtr.h"
#include "Z80/Z80.h"
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QtGui>


namespace gui
{

ZxIf2Insp::ZxIf2Insp(QWidget* w, MachineController* mc, volatile ZxIf2* zxif2) :
	SinclairJoyInsp(w, mc, zxif2, "/Images/zxif2.jpg"),
	zxif2(zxif2),
	old_romfilepath(nullptr)
{
	button_insert_eject = new QPushButton("Insert", this);
	button_insert_eject->setMinimumWidth(100);
	connect(button_insert_eject, &QPushButton::clicked, this, &ZxIf2Insp::slotInsertEjectRom);

	label_romfilename = new QLabel(this);
	label_romfilename->move(150, 51);
	label_romfilename->setFont(QFont("Arial", 10));
	//	rom_name->setMinimumWidth(100);
	label_romfilename->setAlignment(Qt::AlignTop);
	setColors(label_romfilename, 0xffffff /*foregroundcolor*/);

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10, 10, 10, 5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0, 100);
	g->setColumnStretch(0, 50);
	g->setColumnStretch(1, 50);
	g->setColumnStretch(2, 50);
	g->setColumnStretch(3, 50);
	g->setColumnStretch(4, 50);
	g->setColumnStretch(5, 50);

	g->addWidget(joystick_selectors[0], 1, 0, 1, 3);
	g->addWidget(joystick_selectors[1], 1, 3, 1, 3);
	g->addWidget(lineedit_display[0], 2, 0, 1, 3);
	g->addWidget(lineedit_display[1], 2, 3, 1, 3);

	g->addWidget(button_scan_usb, 3, 0, 1, 2, Qt::AlignHCenter | Qt::AlignVCenter);
	g->addWidget(button_set_keys, 3, 2, 1, 2, Qt::AlignHCenter | Qt::AlignVCenter);
	g->addWidget(button_insert_eject, 3, 4, 1, 2, Qt::AlignHCenter | Qt::AlignVCenter);

	//	timer->start(1000/15);			// started by JoyInsp()
}

void ZxIf2Insp::slotInsertEjectRom()
{
	assert(validReference(zxif2));

	if (zxif2->isLoaded())
	{
		xlogIn("ZxIf2Insp::eject()");
		bool f = machine->powerOff();
		NV(zxif2)->ejectRom();
		if (f) machine->powerOn();
	}
	else
	{
		xlogIn("ZxIf2Insp::insert()");

		cstr filter	  = "IF2 Rom Cartridges (*.rom)"; //";;All Files (*)";
		cstr filepath = selectLoadFile(this, "Select IF2 Rom Cartridge", filter);
		if (!filepath) return;

		bool f = machine->powerOff();
		NV(zxif2)->insertRom(filepath);
		if (f) machine->powerOn();
	}
}

void ZxIf2Insp::updateWidgets()
{
	xlogIn("ZxIf2Insp::updateWidgets");
	assert(validReference(zxif2));

	SinclairJoyInsp::updateWidgets();

	cstr new_romfilepath = zxif2->getFilepath();
	if (old_romfilepath != new_romfilepath)
	{
		label_romfilename->setText(new_romfilepath ? basename_from_path(new_romfilepath) : nullptr);

		if (!old_romfilepath)
		{
			background.load(catstr(appl_rsrc_path, "/Images/zxif2_with_cart.jpg"));
			button_insert_eject->setText("Eject Rom");
			update();
		}
		if (!new_romfilepath)
		{
			background.load(catstr(appl_rsrc_path, "/Images/zxif2.jpg"));
			button_insert_eject->setText("Insert Rom");
			update();
		}

		old_romfilepath = new_romfilepath;
	}
}

void ZxIf2Insp::fillContextMenu(QMenu* menu)
{
	// fill context menu for right-click
	// called by Inspector::contextMenuEvent()
	// items inserted here are inserted at the to of the popup menu

	assert(validReference(zxif2));

	Inspector::fillContextMenu(menu); // NOP

	if (zxif2->isLoaded()) { menu->addAction("Eject Rom", this, &ZxIf2Insp::slotInsertEjectRom); }
	else
	{
		menu->addAction("Insert Rom", this, &ZxIf2Insp::slotInsertEjectRom);
		menu->addAction("Recent Roms …")->setMenu(new RecentFilesMenu(RecentIf2Roms, this, [=](cstr fpath) {
			insertRom(fpath);
		}));
	}
}

void ZxIf2Insp::insertRom(cstr filepath)
{
	assert(validReference(zxif2));

	bool f = machine->powerOff();
	NV(zxif2)->insertRom(filepath);
	if (f) machine->powerOn();
}

} // namespace gui
