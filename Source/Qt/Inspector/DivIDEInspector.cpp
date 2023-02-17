// Copyright (c) 2014 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "DivIDEInspector.h"
#include "Fdc/DivIDE.h"
#include "Machine/Machine.h"
#include "MachineController.h"
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "RecentFilesMenu.h"
#include "Templates/NVPtr.h"
#include "globals.h"
#include "unix/n-compress.h"
#include <QMenu>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QTimer>
#include <QWidget>


namespace gui
{

static QRect box_nmi_button(269 + 4, 61 - 2, 19, 11 + 10);
static QRect box_module(71, 2, 188, 85);
static QRect box_eeprom(199, 132, 122, 32);
static QRect box_jumper_E(202 + 1, 195, 20, 24);
static QRect box_jumper_A(220 + 1, 194, 20, 25);
static QRect box_jumper_EA(202 + 1, 194 + 1, 37, 24);

static QRect box_led_green_hi(167, 73, 23, 24);
static QRect box_led_yellow_hi(180, 74, 24, 23);
static QRect box_led_red_hi(193, 73, 23, 24);

static QRect box_label(102, 48, 128, 10); // module label
static QFont font_label("Geneva", 10);	  // Geneva (weiter), Arial oder Gill Sans (enger)


DivIDEInspector::DivIDEInspector(QWidget* o, MachineController* mc, volatile IsaObject* i) :
	Inspector(o, mc, i, "/Images/divide.jpg"), overlay_jumper_E(":/Icons/divide-j10.png"),
	overlay_jumper_A(":/Icons/divide-j01.png"), overlay_jumper_EA(":/Icons/divide-j11.png"),
	overlay_module(catstr(appl_rsrc_path, "Images/divide-module.jpg")),
	overlay_led_green_hi(":/Icons/led-green-highlight.png"), overlay_led_yellow_hi(":/Icons/led-yellow-highlight.png"),
	overlay_led_red_hi(":/Icons/led-red-highlight.png")
{
	assert(i->isA(isa_DivIDE));

	state.jumper_A	 = machine->isA(isa_MachineZxPlus2a); // wird in updateWidgets() nicht aktualisiert
	state.jumper_E	 = off;								  // wprot/enable jumper
	state.led_green	 = 1;		// power led: mostly on		// wird in updateWidgets() nicht aktualisiert
	state.led_yellow = 0;		// MAPRAM state
	state.led_red	 = 0;		// IDE busy: mostly off
	state.diskname	 = nullptr; // nullptr => no module; else name of disk

	// click area "insert/eject module":
	module = new QWidget(this);
	module->setGeometry(box_module);
	module->setCursor(QCursor(QPixmap(":Icons/mouse/arrow_down_black.png"), 10, 18));

	// click area "toggle jumper E":
	jumper_E = new QWidget(this);
	jumper_E->setGeometry(box_jumper_E);
	jumper_E->setCursor(Qt::PointingHandCursor);

	// click area "load DivIDE rom":
	eeprom = new QWidget(this);
	eeprom->setGeometry(box_eeprom);
	eeprom->setCursor(Qt::PointingHandCursor);

	// click area "nmi button":
	button = new QWidget(this);
	button->setGeometry(box_nmi_button);
	button->setCursor(Qt::PointingHandCursor);


	timer->start(1000 / 20);
}


DivIDEInspector::~DivIDEInspector() { delete[] state.diskname; }


void DivIDEInspector::paintEvent(QPaintEvent* e)
{
	xlogIn("DivIDEInspector:paintEvent");
	Inspector::paintEvent(e);
	QPainter p(this);

	if (state.diskname && e->rect().intersects(box_module))
	{
		p.drawPixmap(box_module.topLeft(), overlay_module);
		p.setFont(font_label);
		p.drawText(box_label, Qt::AlignTop | Qt::TextSingleLine, state.diskname);
	}

	if (state.led_green && e->rect().intersects(box_led_green_hi))
		p.drawPixmap(box_led_green_hi.topLeft(), overlay_led_green_hi);
	if (state.led_yellow && e->rect().intersects(box_led_yellow_hi))
		p.drawPixmap(box_led_yellow_hi.topLeft(), overlay_led_yellow_hi);
	if (state.led_red && e->rect().intersects(box_led_red_hi))
		p.drawPixmap(box_led_red_hi.topLeft(), overlay_led_red_hi);

	if (state.jumper_A && state.jumper_E && e->rect().intersects(box_jumper_EA))
		p.drawPixmap(box_jumper_EA.topLeft(), overlay_jumper_EA);
	if (state.jumper_A && !state.jumper_E && e->rect().intersects(box_jumper_A))
		p.drawPixmap(box_jumper_A.topLeft(), overlay_jumper_A);
	if (state.jumper_E && !state.jumper_A && e->rect().intersects(box_jumper_E))
		p.drawPixmap(box_jumper_E.topLeft(), overlay_jumper_E);
}


void DivIDEInspector::mousePressEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton)
	{
		Inspector::mousePressEvent(e);
		return;
	}

	xlogline("DivIDEInspector: mouse down at %i,%i", e->x(), e->y());

	QPoint p = e->pos();

	if (box_module.contains(p))
	{
		if (divide()->isDiskInserted())
			eject_disk();
		else
			insert_disk();
	}
	if (box_jumper_E.contains(p)) { NVPtr<DivIDE>(divide())->setJumperE(!state.jumper_E); }
	if (box_nmi_button.contains(p)) { NVPtr<Machine>(machine)->nmi(); }
	if (box_eeprom.contains(p)) { load_rom(); }
}

void DivIDEInspector::updateWidgets()
{
	xxlogIn("DivIDEInspector:updateWidgets");
	if (!object)
	{
		xlogline("DivIDEInspector.updateWidgets: !item");
		return;
	}

	// state.jumper_A depends on machine type and machine type is fixed (for the lifetime of this inspector)
	assert(state.jumper_A == machine->isA(isa_MachineZxPlus2a));

	if (state.jumper_E != divide()->getJumperE())
	{
		state.jumper_E ^= 1;
		update(box_jumper_E);
	}

	// state.led_green depends on power on, and an intermediate power off state during power cycling is not yet
	// implemented
	assert(state.led_green == on);

	if (state.led_yellow != divide()->getMapRam())
	{
		state.led_yellow ^= 1;
		update(box_led_yellow_hi);
	}

	if (state.led_red != divide()->getIdeBusy())
	{
		state.led_red ^= 1;
		update(box_led_red_hi);
	}

	if (!eq(state.diskname, divide()->getDiskFilename()))
	{
		delete[] state.diskname;
		state.diskname = newcopy(divide()->getDiskFilename());
		update(box_module);
		module->setCursor(
			divide()->isDiskInserted() ? QCursor(QPixmap(":Icons/mouse/arrow_up_black.png"), 10, 1) :
										 QCursor(QPixmap(":Icons/mouse/arrow_down_black.png"), 10, 18));
	}
}


void DivIDEInspector::fillContextMenu(QMenu* menu)
{
	Inspector::fillContextMenu(menu); // NOP

	QAction* action_rom_wprot = new QAction("Write protected && enabled", menu);
	action_rom_wprot->setCheckable(true);
	action_rom_wprot->setChecked(divide()->getJumperE());
	connect(action_rom_wprot, &QAction::toggled, this, &DivIDEInspector::toggle_jumper_E);

	QAction* action_disk_wprot = new QAction("Write protected", menu);
	action_disk_wprot->setCheckable(true);
	action_disk_wprot->setChecked(!divide()->isDiskWritable());
	connect(action_disk_wprot, &QAction::toggled, this, &DivIDEInspector::toggle_disk_wprot);

	QAction* action_ram32 = new QAction("32 kByte Ram", menu);
	action_ram32->setCheckable(true);
	action_ram32->setChecked(divide()->getRam().count() == 32 * 1024);
	connect(action_ram32, &QAction::toggled, this, &DivIDEInspector::set_ram_32k);

	QAction* action_ram512 = new QAction("512 kByte Ram", menu);
	action_ram512->setCheckable(true);
	action_ram512->setChecked(divide()->getRam().count() == 512 * 1024);
	connect(action_ram512, &QAction::toggled, this, &DivIDEInspector::set_ram_512k);

	QActionGroup* ramGrp = new QActionGroup(this);
	ramGrp->addAction(action_ram32);
	ramGrp->addAction(action_ram512);


	menu->addAction("Insert ESXdos 0.8.5", this, &DivIDEInspector::load_default_rom);
	menu->addAction("Insert EEprom …", this, [=] { load_rom(); });
	menu->addAction("Recent EEproms …")->setMenu(new RecentFilesMenu(RecentDivideRoms, this, [=](cstr path) {
		load_rom(path);
	}));
	menu->addAction("Save EEprom as …", this, &DivIDEInspector::save_rom);
	menu->addAction(action_rom_wprot);
	menu->addSeparator();

	menu->addAction("Insert new empty 16M disc", this, &DivIDEInspector::insert_new_16M);
	menu->addAction("Insert new empty 128M disc", this, &DivIDEInspector::insert_new_128M);
	menu->addAction("Insert disc …", this, [=] { insert_disk(); }); // TODO: /dev/disk*
	menu->addAction("Recent discs …")->setMenu(new RecentFilesMenu(RecentDivideDisks, this, [=](cstr fpath) {
		insert_disk(fpath);
	}));
	menu->addAction("Eject disc", this, &DivIDEInspector::eject_disk);
	menu->addAction(action_disk_wprot);
	menu->addSeparator();

	menu->addAction(action_ram32);
	menu->addAction(action_ram512);
}


//	helper: load rom
//	if filepath==NULL load default rom
//
void DivIDEInspector::load_rom(cstr filepath)
{
	assert(divide()->getRom().count());

	bool f	 = machine->powerOff(); // new DOS needs to set up data
	int	 err = NV(divide())->insertRom(filepath);
	if (f) machine->powerOn();

	if (!err && filepath) addRecentFile(RecentDivideRoms, filepath);
	// addRecentFile(RecentFiles,filepath);	// würde später in's interne Rom laden
	emit updateCustomTitle();
}


void DivIDEInspector::load_default_rom()
{
	xlogline("DivIDEInspector: slotLoadDefaultRom");
	load_rom(nullptr);
}

void DivIDEInspector::load_rom() // with requester
{
	xlogline("DivIDEInspector: slotLoadRom");

	cstr filter	  = "DivIDE Roms (*.rom *.bin);;All Files (*)";
	cstr filepath = selectLoadFile(this, "Select DivIDE Rom file", filter);
	if (filepath) load_rom(filepath);
}

void DivIDEInspector::save_rom()
{
	xlogline("DivIDEInspector: slotSaveRom");
	assert(divide()->getRom().count());

	static cstr filter	 = "DivIDE Roms (*.rom);;All Files (*)";
	cstr		filepath = selectSaveFile(this, "Save DivIDE Rom as:", filter);
	if (!filepath) return;

	try
	{
		FD	 fd(filepath, 'w');
		bool f = machine->suspend();
		NV(divide())->saveRom(fd);
		if (f) machine->resume();
		addRecentFile(RecentDivideRoms, filepath);
		// addRecentFile(RecentFiles,filepath);		würde in's interne Rom laden
		emit updateCustomTitle();
	}
	catch (FileError& e)
	{
		showAlert("File error:\n%s", e.what());
	}
}

/* toggle rom write protection & enable jumper 'E'
 */
void DivIDEInspector::toggle_jumper_E()
{
	xlogline("DivIDEInspector: slotToggleJumperE");

	bool f = machine->suspend();
	NV(divide())->setJumperE(!state.jumper_E);
	if (f) machine->resume();
}


void DivIDEInspector::insert_disk(cstr filepath)
{
	bool f = machine->powerOff(); // changing disks without reset is dangerous!
	NV(divide())->insertDisk(filepath);
	if (f) machine->powerOn();

	if (divide()->isDiskInserted()) { addRecentFile(RecentDivideDisks, filepath); }
}

void DivIDEInspector::insert_disk()
{
	xlogline("DivIDEInspector: slotInsertDisk");

	if (divide()->isDiskInserted())
	{
		bool f = machine->suspend();
		NV(divide())->ejectDisk();
		if (f) machine->resume();
	}
	if (divide()->isDiskInserted()) return; // eject failed
	updateWidgets();						// update state & display

	cstr filter	  = "Hard Disc images (*.img *.hdf *.dmg *.iso);;All Files (*)";
	cstr filepath = selectLoadFile(this, "Insert Hard Disc Image", filter);
	if (filepath) insert_disk(filepath);
}

void DivIDEInspector::eject_disk()
{
	xlogline("DivIDEInspector: slotEjectDisk");

	bool f = machine->suspend();
	NV(divide())->ejectDisk();
	if (f) machine->resume();
}

void DivIDEInspector::toggle_disk_wprot()
{
	xlogline("DivIDEInspector: slotToggleDiskWProt");

	divide()->setDiskWritable(!divide()->isDiskWritable());
}


void DivIDEInspector::set_ram_32k()
{
	xlogline("DivIDEInspector: slotSetRam32k");

	if (divide()->getRam().count() == 32 << 10) return;
	bool f = machine->suspend();
	NV(divide())->setRamSize(32 << 10);
	if (f) machine->resume();
}

void DivIDEInspector::set_ram_512k()
{
	xlogline("DivIDEInspector: slotSetRam512k");

	if (divide()->getRam().count() == 512 << 10) return;
	bool f = machine->suspend();
	NV(divide())->setRamSize(512 << 10);
	if (f) machine->resume();
}


void DivIDEInspector::insert_new_disk(cstr basename)
{
	static cstr filter	  = "Hard Disc images (*.img *.dmg *.iso);;All Files (*)";
	cstr		msg		  = usingstr("Save %s hard disc image as…", basename);
	cstr		zfilepath = selectSaveFile(this, msg, filter);
	if (!zfilepath) return;

	try
	{
		cstr qfilepath = usingstr("%sEmptyFiles/%s.img.Z", appl_rsrc_path, basename);
		decompress(qfilepath, zfilepath);
		insert_disk(zfilepath);
	}
	catch (FileError& e)
	{
		showAlert("File Error:\n%s", e.what());
	}
}

void DivIDEInspector::insert_new_16M()
{
	xlogline("DivIDEInspector: slotInsertNew16M");

	insert_new_disk("16MB");
}

void DivIDEInspector::insert_new_128M()
{
	xlogline("DivIDEInspector: slotInsertNew128M");

	insert_new_disk("128MB");
}


/*	callback provided for ToolWindow:
	either return a custom name
	or return nullptr for default/item name
*/
cstr DivIDEInspector::getCustomTitle()
{
	// wenn ein Rom geladen ist (was immer sein sollte)
	// dann hänge den Rom-Namen an den Item-Namen an:

	return divide()->getRomFilepath() ? catstr(item()->name, ": ", divide()->getRomFilename()) : nullptr;
}

} // namespace gui
