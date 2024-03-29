// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "TccDockInspector.h"
#include "Machine.h"
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "RecentFilesMenu.h"
#include "Templates/NVPtr.h"
#include "Ula/MmuTc2048.h"
#include "globals.h"
#include "unix/files.h"
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QTimer>


namespace gui
{

// Positionen & Dimensionen für Unipolbrit 2086:
static QRect box_slot_u(92, 94, 138, 100);				 // Slot => "Insert Cartridge"
static QRect box_top_module_inserted_u(86, 89, 150, 62); // Cartridge Oberseite
static QRect box_top_module_ejected_u(83, 92, 160, 86);	 // ""
// static QRect box_front_module_inserted_u(91,150,140,16);	// Cartridge Vorderseite
static QRect box_front_module_ejected_u(88, 178, 148, 18); // ""
static QRect box_front_label_inserted_u(96, 153, 130, 16); // Beschriftung Vorderseite
static QRect box_front_label_ejected_u(93, 182, 138, 18);  // ""

// Positionen & Dimensionen für TC2086 / TS2086:
static QRect box_slot_tc(91, 91, 140, 100);
static QRect box_top_module_inserted_tc(87, 91, 147, 55);
static QRect box_top_module_ejected_tc(87, 91, 149, 83);
// static QRect box_front_module_inserted_tc(93,146,137,16);
static QRect box_front_module_ejected_tc(93, 173, 144, 17);
static QRect box_front_label_inserted_tc(101, 150, 125, 11);
static QRect box_front_label_ejected_tc(99, 176, 131, 11);

static QFont font_label("Arial Black", 9); // Geneva (weiter), Arial oder Gill Sans (enger)


TccDockInspector::TccDockInspector(QWidget* parent, MachineController* mc, volatile MmuTc2068* mmu) :
	Inspector(parent, mc, mmu, mmu->isA(isa_MmuU2086) ? "/Images/u2086/open.jpg" : "/Images/tc2068/open.jpg"),
	dock(mmu),
	u(mmu->isA(isa_MmuU2086)),
	button_insert(new QPushButton("Insert Cartridge", this)),
	x_overlay(u ? 72 : 73),
	y_overlay(89),
	imgdirpath(newcopy(catstr(appl_rsrc_path, "Images/", u ? "u2086/" : "tc2068/"))),
	overlay_zxspemu_ejected(catstr(imgdirpath, "zxspemu_ejected.jpg")),
	overlay_zxspemu_inserted(catstr(imgdirpath, "zxspemu_inserted.jpg")),
	overlay_game_ejected(catstr(imgdirpath, "game_ejected.jpg")),
	overlay_game_inserted(catstr(imgdirpath, "game_inserted.jpg")),
	//	box_slot(u ? box_slot_u : box_slot_tc),
	//	box_top_module_inserted(u ? box_top_module_inserted_u : box_top_module_inserted_tc),
	//	box_top_module_ejected (u ? box_top_module_ejected_u  : box_top_module_ejected_tc),
	//	box_front_module_inserted(u ? box_front_module_inserted_u : box_front_module_inserted_tc),
	//	box_front_module_ejected (u ? box_front_module_ejected_u  : box_front_module_ejected_tc),
	cartridge_state(Invalid),
	current_fpath(nullptr),
	current_id(TccUnknown)
{
	// Bereich des leeren Docks, in das man klicken kann, um ein neues Cartridge zu laden:
	dock_slot = new QWidget(this);
	dock_slot->setGeometry(u ? box_slot_u : box_slot_tc);
	dock_slot->setCursor(Qt::PointingHandCursor);
	dock_slot->setVisible(yes);

	// Oberseite des eingeschobenen Roms: "Herausziehen"
	module_top_inserted = new QWidget(this);
	module_top_inserted->setGeometry(u ? box_top_module_inserted_u : box_top_module_inserted_tc);
	module_top_inserted->setCursor(QCursor(QPixmap(":Icons/mouse/eject.png"), -1, -1));
	module_top_inserted->setVisible(no);

	// Oberseite des herausgezogenen Roms: "Entfernen"
	module_top_ejected = new QWidget(this);
	module_top_ejected->setGeometry(u ? box_top_module_ejected_u : box_top_module_ejected_tc);
	module_top_ejected->setCursor(QCursor(QPixmap(":Icons/mouse/eject.png"), -1, -1));
	module_top_ejected->setVisible(no);

	//	module_front_inserted = new QWidget(this);
	//	module_front_inserted->setGeometry(box_front_inserted);
	//	module_front_inserted->setCursor(Qt::PointingHandCursor);
	//	module_front_inserted->setVisible(no);

	// Vorderseite des herausgezogenen Roms: "Wieder einschieben"
	module_front_ejected = new QWidget(this);
	module_front_ejected->setGeometry(u ? box_front_module_ejected_u : box_front_module_ejected_tc);
	module_front_ejected->setCursor(Qt::PointingHandCursor);
	module_front_ejected->setVisible(no);

	button_insert->move(173, 207);
	connect(button_insert, &QPushButton::clicked, this, &TccDockInspector::insert_or_eject_cartridge);
	timer->start(1000 / 10);
}

TccDockInspector::~TccDockInspector()
{
	delete[] imgdirpath;
	delete[] current_fpath;
}

void TccDockInspector::updateWidgets()
{
	// timer

	assert(validReference(dock));

	CartridgeState new_state = dock->isLoaded() ? RomInserted : current_fpath ? RomEjected : NoCartridge;

	if (cartridge_state != new_state)
	{
		cartridge_state = new_state;

		button_insert->setText(
			new_state == RomInserted ? "Eject Cartridge" :
			new_state == RomEjected	 ? "Remove Cartridge" :
									   "Insert Cartridge");
		if (!current_fpath && dock->isLoaded())
		{
			current_fpath = newcopy(dock->getFilepath());
			current_id	  = dock->getTccId();
		}

		dock_slot->setVisible(new_state == NoCartridge);
		module_top_inserted->setVisible(new_state == RomInserted);
		//	module_front_inserted->setVisible(new_state==RomInserted);
		module_top_ejected->setVisible(new_state == RomEjected);
		module_front_ejected->setVisible(new_state == RomEjected);

		update();
	}
}

void TccDockInspector::paintEvent(QPaintEvent*)
{
	xlogIn("TccInspector:paintEvent");
	QPainter p(this);

	//	p.setPen(Qt::blue);
	//	p.setFont(QFont("Arial", 30));
	//	p.drawText(rect(), Qt::AlignCenter, "Qt");

	p.drawPixmap(0, 0, background);

	switch (cartridge_state)
	{
	case RomEjected:
		p.drawPixmap(x_overlay, y_overlay, current_id == TccZxspEmu ? overlay_zxspemu_ejected : overlay_game_ejected);
		if (current_id != TccZxspEmu)
		{
			p.setFont(font_label);
			p.setPen(QColor(Qt::white));
			p.drawText(
				u ? box_front_label_ejected_u : box_front_label_ejected_tc, Qt::AlignTop | Qt::TextSingleLine,
				basename_from_path(current_fpath));
		}
		break;
	case RomInserted:
		p.drawPixmap(x_overlay, y_overlay, current_id == TccZxspEmu ? overlay_zxspemu_inserted : overlay_game_inserted);
		if (current_id != TccZxspEmu)
		{
			p.setFont(font_label);
			p.setPen(QColor(Qt::white));
			p.drawText(
				u ? box_front_label_inserted_u : box_front_label_inserted_tc, Qt::AlignTop | Qt::TextSingleLine,
				basename_from_path(current_fpath));
		}
		break;
	case NoCartridge:
	case Invalid: break;
	}
}

cstr TccDockInspector::getSaveFilename()
{
	static cstr filter = "TCC Cartridges (*.dck);;All Files (*)";
	return selectSaveFile(this, "Save Cartridge as…", filter);
}

cstr TccDockInspector::getLoadFilename()
{
	cstr filter = "TCC Cartridge (*.dck);;All Files (*)";
	return selectLoadFile(this, "Insert Cartridge …", filter);
}

void TccDockInspector::fillContextMenu(QMenu* menu)
{
	Inspector::fillContextMenu(menu); // NOP

	switch (cartridge_state)
	{
	case RomInserted:
		menu->addAction("Eject cartridge", this, &TccDockInspector::eject_cartridge);
		menu->addAction("Save as …", this, &TccDockInspector::save_as);
		break;

	case RomEjected:
		menu->addAction("Remove cartridge", this, &TccDockInspector::remove_cartridge);
		menu->addAction("Insert again", this, &TccDockInspector::insert_again);
		FALLTHROUGH

	case NoCartridge:
		menu->addAction("Insert cartridge …", [=]() { insert_cartridge(); });
		menu->addAction("Recent cartridges …")->setMenu(new RecentFilesMenu(RecentTccRoms, this, [=](cstr fpath) {
			insert_cartridge(fpath);
		}));
		break;

	case Invalid: break;
	}
}

void TccDockInspector::mousePressEvent(QMouseEvent* e)
{
	// Mouse click handler:
	// - eject cartridge
	// - remove ejected cartridge
	// - insert ejected cartridge again

	if (e->button() != Qt::LeftButton)
	{
		Inspector::mousePressEvent(e);
		return;
	}

	xlogline("DockInspector: mouse down at %i,%i", e->x(), e->y());

	QPoint p = e->pos();
	switch (cartridge_state)
	{
	case NoCartridge:
		if ((u ? box_slot_u : box_slot_tc).contains(p)) insert_cartridge();
		break;

	case RomEjected:
		if ((u ? box_top_module_ejected_u : box_top_module_ejected_tc).contains(p)) remove_cartridge();
		if ((u ? box_front_module_ejected_u : box_front_module_ejected_tc).contains(p)) insert_again();
		break;

	case RomInserted:
		if ((u ? box_top_module_inserted_u : box_top_module_inserted_tc).contains(p)) eject_cartridge();
		break;

	case Invalid: break;
	}
}

void TccDockInspector::insert_or_eject_cartridge()
{
	// Slot für "Insert/Eject" Button:

	assert(validReference(dock));

	if (dock->isLoaded()) eject_cartridge();
	if (current_fpath) remove_cartridge();
	else insert_cartridge();
}

void TccDockInspector::insert_cartridge(cstr filepath)
{
	// called from menu: "Load..."        -> filepath==NULL -> query user
	// called from menu: "Load recent..." -> filepath!=NULL

	assert(validReference(dock));

	if (!filepath) filepath = getLoadFilename();
	if (!filepath) return;

	cartridge_state = Invalid;
	delete[] current_fpath;
	current_fpath = nullptr;

	bool f = machine->powerOff();

	NV(dock)->insertCartridge(filepath);

	current_fpath = newcopy(filepath);
	current_id	  = dock->getTccId();

	if (f) machine->powerOn();
}

void TccDockInspector::eject_cartridge()
{
	// Context menu:

	assert(validReference(dock));

	if (dock->isLoaded())
	{
		cartridge_state = Invalid;

		bool f = machine->powerOff();
		NV(dock)->ejectCartridge();
		if (f) machine->powerOn();
	}
}

void TccDockInspector::remove_cartridge()
{
	// Context menu:

	assert(validReference(dock));

	if (!dock->isLoaded())
	{
		cartridge_state = Invalid;
		delete[] current_fpath;
		current_fpath = nullptr;
	}
}

void TccDockInspector::insert_again()
{
	// Context menu:

	assert(validReference(dock));

	if (current_fpath)
	{
		bool f			= machine->powerOff();
		cartridge_state = Invalid;
		NV(dock)->insertCartridge(current_fpath);
		if (f) machine->powerOn();
	}
}

void TccDockInspector::save_as()
{
	// Context menu:

	assert(validReference(dock));

	cstr filepath = getSaveFilename();
	if (filepath)
	{
		bool f			= machine->suspend();
		cartridge_state = Invalid;
		NV(dock)->saveCartridgeAs(filepath);
		if (f) machine->resume();
	}
}

} // namespace gui


/*
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
*/
