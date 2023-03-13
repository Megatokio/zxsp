// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "FdcPlus3Insp.h"
#include "Fdc/FdcPlus3.h"
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "RecentFilesMenu.h"
#include "Templates/NVPtr.h"
#include "unix/files.h"
#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QTimer>


namespace gui
{

// overlay images:
static const int x_overlay = 36;
static const int y_overlay = 110; // l/o ecke von oben

static const int x_led = 55;
static const int y_led = 176; // l/o ecke von oben

static cstr fname_A_inserted = "Images/disk/plus3_A_inserted.png";
static cstr fname_B_inserted = "Images/disk/plus3_B_inserted.png";
static cstr fname_A_ejected	 = "Images/disk/plus3_A_ejected.png";
static cstr fname_B_ejected	 = "Images/disk/plus3_B_ejected.png";
static cstr path_LED		 = ":/Icons/floppy_plus3_LED.png";

// disk label:
// static const int x_label		= 101;		// disk label, inserted
// static const int x_label_out_A	= 100;		// disk label, ejected, side A up
// static const int x_label_out_B	= 102;		// disk label, ejected, side B up
static const int x_label_B = 225; // disk label "side B"
// static const int y_label_in		= 135;		// disk label, inserted disk
// static const int y_label_out	= 150;		// disk label, ejected disk
static QFont font_label("Geneva", 10); // Geneva (weiter), Arial oder Gill Sans (enger)

// clickable and update areas:
static QRect box_slot(58, 128, 209, 18);						  // disk slot / inserted disk => "insert disk…"
static QRect box_disk_ejected_top(56, 126, 209, 14);			  // top side of ejected disk => "remove disk"
static QRect box_disk_ejected_front(56, 140, x_label_B - 56, 18); // front of ejected disk => "insert again"
static QRect box_disk_ejected_label_B(x_label_B, 140, 30, 18);	  // label "B" on ejected disk => "flip disk"
static QRect box_label_disk_out_side_A(100, 140, 125, 18);		  // disk label area on ejected disk
static QRect box_label_disk_out_side_B(102, 140, 125, 18);		  // disk label area on ejected disk
static QRect box_label_disk_in(101, 125, 125, 18);				  // disk label area on inserted disk
// static QRect box_button_in(220,170,47,14);				// front of eject button, in => "insert disk"
// static QRect box_button_out(224,183,47,13);				// front of eject button, ou" => "eject disk"
static QRect box_eject_button(220, 170, 51, 26); // combined area of eject button in and out
static QRect box_LED(x_led, y_led, 23, 9);		 // area for lit LED
static QRect box_overlay(x_overlay, y_overlay, 250, 85);


cstr FdcPlus3Insp::get_save_filename(cstr msg)
{
	static cstr filter = "ZX Spectrum +3 Discs (*.dsk);;All Files (*)";
	return selectSaveFile(this, msg, filter);
}

cstr FdcPlus3Insp::get_load_filename(cstr msg)
{
	cstr filter = "ZX Spectrum +3 Discs (*.dsk *.disk);;All Files (*)";
	return selectLoadFile(this, msg, filter);
}

FdcPlus3Insp::FdcPlus3Insp(QWidget* w, MachineController* mc, volatile FdcPlus3* fdc) :
	Inspector(w, mc, fdc, "Images/disk/plus3.jpg"),
	fdc(fdc),
	drive(NV(fdc)->getDrive(0)),
	overlay_disk_A_ejected(catstr(appl_rsrc_path, fname_A_ejected)),
	overlay_disk_A_inserted(catstr(appl_rsrc_path, fname_A_inserted)),
	overlay_disk_B_ejected(catstr(appl_rsrc_path, fname_B_ejected)),
	overlay_disk_B_inserted(catstr(appl_rsrc_path, fname_B_inserted)),
	overlay_led(path_LED),
	diskstate(NoDisk),
	current_disk(nullptr),
	led_on(no)
{
	slot = new QWidget(this);
	slot->setGeometry(box_slot);
	slot->setCursor(Qt::PointingHandCursor);

	eject_button = new QWidget(this);
	eject_button->setGeometry(box_eject_button);
	eject_button->setCursor(Qt::PointingHandCursor);

	disk_ejected_front = new QWidget(this);
	disk_ejected_front->setGeometry(box_disk_ejected_front);
	disk_ejected_front->setCursor(Qt::PointingHandCursor);

	disk_ejected_label_B = new QWidget(this);
	disk_ejected_label_B->setGeometry(box_disk_ejected_label_B);
	disk_ejected_label_B->setCursor(QCursor(QPixmap(":Icons/mouse/rotate.png"), -1, -1));

	disk_ejected_top = new QWidget(this);
	disk_ejected_top->setGeometry(box_disk_ejected_top);
	disk_ejected_top->setCursor(QCursor(QPixmap(":Icons/mouse/eject.png"), -1, -1));

	//	settings.readRecentFiles(RecentPlus3Disks);	// prefetch
	set_disk_state(NoDisk);
	timer->start(1000 / 20);
}

FdcPlus3Insp::~FdcPlus3Insp() { delete[] current_disk; }

void FdcPlus3Insp::paintEvent(QPaintEvent* e)
{
	xlogIn("FdcPlus3Insp:paintEvent");
	Inspector::paintEvent(e);
	QPainter p(this);

	switch (diskstate)
	{
	case Ejected:
		p.drawPixmap(x_overlay, y_overlay, drive->side_B_up ? overlay_disk_B_ejected : overlay_disk_A_ejected);
		if (e->rect().intersects(box_disk_ejected_front))
		{
			p.setFont(font_label);
			p.drawText(
				drive->side_B_up ? box_label_disk_out_side_B : box_label_disk_out_side_A,
				Qt::AlignTop | Qt::TextSingleLine, basename_from_path(current_disk));
		}
		break;
	case Loaded:
		p.drawPixmap(x_overlay, y_overlay, drive->side_B_up ? overlay_disk_B_inserted : overlay_disk_A_inserted);
		if (current_disk && e->rect().intersects(box_disk_ejected_front))
		{
			p.setFont(font_label);
			p.drawText(box_label_disk_in, Qt::AlignTop | Qt::TextSingleLine, basename_from_path(current_disk));
		}
		break;
	case NoDisk: break;
	}

	if (led_on && e->rect().intersects(box_LED)) p.drawPixmap(x_led, y_led, overlay_led);
}


void FdcPlus3Insp::mousePressEvent(QMouseEvent* e)
{
	// mouse down in widget:
	// virtual Qt override

	if (e->button() != Qt::LeftButton)
	{
		Inspector::mousePressEvent(e);
		return;
	}

	xlogline("FdcPlus3Insp: mouse down at %i,%i", e->x(), e->y());
	assert(validReference(fdc));

	QPoint p = e->pos();
	switch (diskstate)
	{
	case NoDisk:
		if (box_slot.contains(p)) insert_disk();
		if (box_eject_button.contains(p)) insert_disk();
		break;
	case Ejected:
		if (box_disk_ejected_top.contains(p)) remove_disk();
		if (box_disk_ejected_front.contains(p)) insert_again();
		if (box_eject_button.contains(p)) insert_disk();
		if (box_disk_ejected_label_B.contains(p)) flip_disk();
		break;
	case Loaded:
		if (box_eject_button.contains(p)) eject_disk();
		break;
	}
}

void FdcPlus3Insp::updateWidgets()
{
	// update widget:
	// regularly called by this->timer

	xlogIn("FdcPlus3Insp::updateWidgets");
	assert(validReference(fdc));

	// LED animieren:
	if (led_on != (fdc->isMotorOn() && drive.get() == NV(fdc)->getSelectedDrive().get()))
	{
		led_on = !led_on;
		update(box_LED);
	}

	// Prüfen, ob hinterrücks eine Disk eingelegt wurde:
	if (drive->disk && ne(drive->disk->filepath, current_disk))
	{
		delete[] current_disk;
		current_disk = newcopy(drive->disk->filepath);
		set_disk_state(Loaded);
	}
}

void FdcPlus3Insp::set_disk_state(DiskState newstate)
{
	// set state of display to NoDisk, Ejected or Inserted
	// updates the clickable areas which display custom mouse pointers
	// redraws the widget

	diskstate = newstate;

	// Über die "Visibility" wird die Anzeige des Hand-Cursors gesteuert:
	slot->setVisible(newstate == NoDisk);				   // -> insert file
	eject_button->setVisible(true);						   // -> eject / insert file
	disk_ejected_top->setVisible(newstate == Ejected);	   // -> remove disk
	disk_ejected_front->setVisible(newstate == Ejected);   // -> insert again
	disk_ejected_label_B->setVisible(newstate == Ejected); // -> flip disk
	update(box_overlay);								   // -> paintEvent()
}

void FdcPlus3Insp::fillContextMenu(QMenu* menu)
{
	// fill context menu for right-click
	// called by Inspector::contextMenuEvent()
	// items inserted here are inserted at the to of the popup menu

	Inspector::fillContextMenu(menu);

	if (diskstate == Loaded)
	{
		menu->addAction("Eject disc", this, &FdcPlus3Insp::eject_disk);
		menu->addAction("Save as …", this, &FdcPlus3Insp::save_as);
	}

	if (diskstate == Loaded || diskstate == Ejected)
	{
		QAction* action_wprot = new QAction("Write protected", menu);
		action_wprot->setCheckable(true); // (!drive->disk || !drive->disk->modified);
		action_wprot->setChecked(!is_writable(current_disk));
		connect(action_wprot, &QAction::toggled, this, &FdcPlus3Insp::toggle_wprot);
		menu->addAction(action_wprot);
	}

	if (diskstate == Ejected)
	{
		menu->addAction("Flip disc", this, &FdcPlus3Insp::flip_disk);
		menu->addAction("Remove disc", this, &FdcPlus3Insp::remove_disk);
		menu->addAction("Insert again", this, &FdcPlus3Insp::insert_again);
	}

	if (diskstate == Ejected || diskstate == NoDisk)
	{
		menu->addAction("Insert blank disc", this, &FdcPlus3Insp::insert_unformatted_disk);
		menu->addAction("Insert formatted disc", this, &FdcPlus3Insp::insert_formatted_disk);
		menu->addAction("Insert disc …", this, [=] { insert_disk(); });
		menu->addAction("Recent Discs …")->setMenu(new RecentFilesMenu(RecentPlus3Disks, this, [=](cstr fpath) {
			insert_disk(fpath);
		}));
	}
}

void FdcPlus3Insp::flip_disk()
{
	// flip disk sides
	// called from context menu and click on label "B" on disk

	drive->flipDisk();
	update();
}

void FdcPlus3Insp::toggle_wprot(bool wprot)
{
	// Toggle write protection state of disk
	// called from context menu
	// disk may be Loaded or Ejected
	// the wprot state is the wprot state of the disk file
	// TODO: set_file_writable() currently only checks the unix file mode

	// Schreibschützen geht nicht, wenn schon was auf die Disk geschrieben wurde:
	if (wprot && drive->disk && drive->disk->modified)
	{
		wprot = no;
		showWarning(
			"The disc is already modified.\n"
			"Use \"Save as…\" if you don't want to overwrite the original file.");
	}

	int err = drive->disk ? drive->disk->setWriteProtected(wprot) :
							set_file_writable(current_disk, wprot ? NOBODY : OWNER | GROUP);

	if (err) showAlert(errorstr(err));
}

void FdcPlus3Insp::eject_disk()
{
	// Eject disk
	// called from context menu and click on Eject Button
	// saves disk to file if modified.
	// note: disks must always have an associated file: in case the disk is modified
	// 	     and destroyed there must be a file for saving without asking the user.

	assert(drive->disk);
	assert(drive->disk->filepath);
	assert(eq(current_disk, drive->disk->filepath));

	if (drive->disk->modified) drive->disk->saveDisk();
	if (drive->disk->modified) return; // save disk failed

	drive->ejectDisk();
	set_disk_state(Ejected);
}

void FdcPlus3Insp::remove_disk()
{
	// Remove Loaded or Ejected disk
	// saves disk to file if modified
	// ejects and removes disk
	// called from context menu

	if (diskstate == Loaded) eject_disk();
	if (diskstate == Loaded) return; // eject failed

	delete[] current_disk;
	current_disk = nullptr;
	set_disk_state(NoDisk);
}

void FdcPlus3Insp::save_as()
{
	// Ask for new name and save disk this file
	// called from context menu
	// note: disks must always have an associated file: in case the disk is modified
	// 	     and destroyed there must be a file for saving without asking the user.

	assert(diskstate == Loaded);

	cstr filepath = get_save_filename();
	if (!filepath) return; // aborted

	try
	{
		drive->disk->saveAs(filepath);
		delete[] current_disk;
		current_disk = newcopy(filepath);
		update(box_slot);
		addRecentFile(RecentPlus3Disks, filepath);
		addRecentFile(RecentFiles, filepath);
	}
	catch (AnyError& e)
	{
		showAlert(e.what());
	}
}

void FdcPlus3Insp::insert_unformatted_disk()
{
	// Insert unformatted disk
	// asks for filename and inserts an unformatted disk
	// called from context menu
	// note: disks must always have an associated file: in case the disk is modified
	//       and destroyed there must be a file for saving without asking the user.

	if (diskstate == Loaded) eject_disk();
	if (diskstate == Loaded) return; // eject failed

	set_disk_state(NoDisk);
	cstr filepath = get_save_filename("Save new unformatted disk as…");
	if (!filepath)
	{
		if (current_disk) set_disk_state(Ejected);
		return;
	} // aborted

	delete[] current_disk;
	current_disk   = newcopy(filepath);
	FloppyDisk* fd = new FloppyDisk();
	fd->setFilepath(filepath);
	drive->insertDisk(fd);
	set_disk_state(Loaded);
}

void FdcPlus3Insp::insert_formatted_disk()
{
	// Insert formatted disk
	// asks for filename and inserts an IBM-formatted disk
	// called from context menu
	// note: disks must always have an associated file: in case the disk is modified
	// 	     and destroyed there must be a file for saving without asking the user.

	if (diskstate == Loaded) eject_disk();
	if (diskstate == Loaded) return; // failed

	set_disk_state(NoDisk);
	cstr filepath = get_save_filename("Save new formatted disk as…");
	if (!filepath)
	{
		if (current_disk) set_disk_state(Ejected);
		return;
	} // aborted

	delete[] current_disk;
	current_disk   = newcopy(filepath);
	FloppyDisk* fd = new FloppyDisk(1 /*sides*/, 40 /*tracks*/, 9 /*sectors*/, yes /*interleaved*/);
	fd->setFilepath(filepath);
	drive->insertDisk(fd);
	set_disk_state(Loaded);
}

void FdcPlus3Insp::insert_again()
{
	//	Insert ejected disk again

	assert(diskstate == Ejected);
	assert(current_disk);

	drive->insertDisk(current_disk, drive->side_B_up);
	set_disk_state(Loaded);
}

void FdcPlus3Insp::insert_disk()
{
	//	Insert disk from disk file with "open file" dialog

	if (diskstate == Loaded) eject_disk();
	if (diskstate == Loaded) return; // eject failed

	set_disk_state(NoDisk);
	cstr filepath = get_load_filename();
	if (filepath)
	{
		drive->insertDisk(filepath);
		if (drive->disk->filepath)
		{
			delete[] current_disk;
			current_disk = newcopy(filepath);
			set_disk_state(Loaded);
			return;
		}
	}

	// failed or aborted:
	// restore last state NoDisk or Ejected:
	if (current_disk) set_disk_state(Ejected);
}

void FdcPlus3Insp::insert_disk(cstr filepath)
{
	//	Insert disk from the "Recent Files" menu
	//	Called from QActions in the "recent files" menu

	if (diskstate == Loaded) eject_disk();
	if (diskstate == Loaded) return; // eject failed

	drive->insertDisk(filepath);
	if (drive->disk->filepath)
	{
		delete[] current_disk;
		current_disk = newcopy(drive->disk->filepath);
		set_disk_state(Loaded);
		return; // ok
	}

	// failed: keep old state
	// setState(current_disk ? Ejected : NoDisk);
}

} // namespace gui
