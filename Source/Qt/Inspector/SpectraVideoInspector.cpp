// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "SpectraVideoInspector.h"
#include "Dialogs/ConfigureKeyboardJoystickDialog.h"
#include "Items/Joy/Joy.h"
#include "Items/SpectraVideo.h"
#include "Machine.h"
#include "MachineController.h"
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "RecentFilesMenu.h"
#include "UsbJoystick.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTimer>

namespace gui
{

SpectraVideoInspector::SpectraVideoInspector(QWidget* w, MachineController* mc, volatile SpectraVideo* spectra) :
	Inspector(w, mc, spectra, spectra->isRomInserted() ? "/Images/Spectra_Loaded.jpg" : "/Images/Spectra.jpg"),
	spectra(spectra),
	js_state(0),
	rom_file(nullptr)
{
	//	this->setFixedSize( background.size() );

	rom_name = new QLabel(this);
	rom_name->move(155, 50);
	rom_name->setFixedWidth(150);
	rom_name->setFont(QFont("Arial", 13));
	rom_name->setAlignment(Qt::AlignTop);
	setColors(rom_name, 0xffffff /*foregroundcolor*/);

	js_display = new QLineEdit(this);
	js_display->setText("%--------");
	js_display->setAlignment(Qt::AlignHCenter);
	js_display->setReadOnly(yes);
	js_display->setFixedWidth(105);

	js_selector = new QComboBox(this);
	js_selector->setFocusPolicy(Qt::NoFocus);
	js_selector->setFixedWidth(110);
	update_joystick_selector();
	connect(
		js_selector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		&SpectraVideoInspector::slotJoystickSelected);

	button_scan_usb = new QPushButton("Scan USB", this);
	button_scan_usb->setMinimumWidth(100);
	connect(button_scan_usb, &QPushButton::clicked, this, &SpectraVideoInspector::slotFindUsbJoysticks);

	button_set_keys = new QPushButton("Set Keys", this);
	button_set_keys->setMinimumWidth(100);
	connect(button_set_keys, &QPushButton::clicked, this, &SpectraVideoInspector::slotSetKeyboardJoystickKeys);

	int x1 = 20 - 5, x2 = 130 - 7, x3 = 208 - 10, x4 = 290;

	int y1 = 242, y2 = 267; // buttons
	int dy = 15;
	int ya = 227 + 3, yb = ya + dy, yc = yb + dy, yd = yc + dy; // checkboxes

	checkbox_if1_rom_hooks = new QCheckBox("IF1Rom hooks", this);
	connect(checkbox_if1_rom_hooks, &QCheckBox::toggled, this, &SpectraVideoInspector::slotEnableIf1RomHooks);
	checkbox_rs232 = new QCheckBox("RS232", this);
	connect(checkbox_rs232, &QCheckBox::toggled, this, &SpectraVideoInspector::slotEnableRS232);
	checkbox_joystick = new QCheckBox("Joystick", this);
	connect(checkbox_joystick, &QCheckBox::toggled, this, &SpectraVideoInspector::slotEnableJoystick);
	checkbox_new_video_modes = new QCheckBox("New colours", this);
	connect(checkbox_new_video_modes, &QCheckBox::toggled, this, &SpectraVideoInspector::slotEnableNewDisplaymodes);
	button_insert_rom = new QPushButton(spectra->isRomInserted() ? "Eject Rom" : "Insert Rom", this);
	connect(button_insert_rom, &QPushButton::clicked, this, &SpectraVideoInspector::slotInsertOrEjectRom);

	button_set_keys->setFixedWidth(80);
	button_scan_usb->setFixedWidth(80 + 12);
	button_insert_rom->setFixedWidth(80 + 12);

	js_selector->move(x1, y1 + 1);
	button_set_keys->move(x2, y1);
	button_scan_usb->move(x3, y1);
	js_display->move(x1 + 2, y2 + 3);
	button_insert_rom->move(x3, y2);

	checkbox_if1_rom_hooks->move(x4, ya);
	checkbox_joystick->move(x4, yb);
	checkbox_rs232->move(x4, yc);
	checkbox_new_video_modes->move(x4, yd);

	bool f = spectra->newVideoModesEnabled();
	checkbox_new_video_modes->setChecked(f);

	f = spectra->rs232_enabled;
	checkbox_rs232->setChecked(f);

	f = spectra->joystick_enabled;
	checkbox_joystick->setChecked(f);
	js_selector->setEnabled(f);
	js_display->setEnabled(f);
	button_set_keys->setEnabled(f);
	button_scan_usb->setEnabled(f);

	f = spectra->if1_rom_hooks_enabled;
	checkbox_if1_rom_hooks->setChecked(f);

	timer->start(1000 / 15);
}

void SpectraVideoInspector::slotEnableIf1RomHooks(bool f)
{
	assert(validReference(spectra));

	nvptr(spectra)->setIF1RomHooksEnabled(f);
	settings.setValue(key_spectra_enable_if1_rom_hooks, f);
}

void SpectraVideoInspector::slotEnableRS232(bool f)
{
	assert(validReference(spectra));

	nvptr(spectra)->setRS232Enabled(f);
	settings.setValue(key_spectra_enable_rs232, f);
}

void SpectraVideoInspector::slotEnableNewDisplaymodes(bool f)
{
	assert(validReference(spectra));

	nvptr(spectra)->enableNewVideoModes(f);
	settings.setValue(key_spectra_enable_new_video_modes, f);
}

void SpectraVideoInspector::slotEnableJoystick(bool f)
{
	assert(validReference(spectra));

	nvptr(spectra)->setJoystickEnabled(f);

	js_selector->setEnabled(f);
	js_display->setEnabled(f);
	button_set_keys->setEnabled(f);
	button_scan_usb->setEnabled(f);

	settings.setValue(key_spectra_enable_joystick, f);
}

void SpectraVideoInspector::slotFindUsbJoysticks()
{
	xlogIn("SpectraVideoInspector::slotFindUsbJoysticks");

	findUsbJoysticks();
	update_joystick_selector();
}

void SpectraVideoInspector::slotSetKeyboardJoystickKeys()
{
	xlogIn("SpectraVideoInspector::slotSetKeyboardJoystickKeys");

	ConfigDialog* d = new ConfigureKeyboardJoystickDialog(controller);
	d->show();
}

void SpectraVideoInspector::slotJoystickSelected()
{
	xlogIn("SpectraVideoInspector::slotJoystickSelected");
	assert(validReference(spectra));

	spectra->insertJoystick(JoystickID(js_selector->currentIndex()));
	controller->addOverlayJoy(spectra);
}

void SpectraVideoInspector::update_joystick_selector()
{
	xlogIn("SpectraVideoInspector::update_js_selector");
	assert(validReference(spectra));

	int num_needed = 2 + int(num_usb_joysticks);

	while (js_selector->count() > num_needed) { js_selector->removeItem(num_needed); }

	for (int i = js_selector->count(); i < num_needed; i++)
	{
		if (i == 0) { js_selector->addItem("no Joystick"); }
		else if (i == 1) { js_selector->addItem("Keyboard"); }
		else
		{
			char idf[] = "USB Joystick #";
			idf[13]	   = char('0' + i - 2);
			js_selector->addItem(idf);
		}
	}

	js_selector->setCurrentIndex(spectra->getJoystickID());
}

void SpectraVideoInspector::slotInsertOrEjectRom()
{
	xlogIn("SpectraVideoInspector::slotInsertOrEjectRom()");
	assert(validReference(spectra));

	if (spectra->isRomInserted())
	{
		xlogIn(" -> eject");

		bool f = nvptr(machine)->powerOff();
		NV(spectra)->ejectRom();
		if (f) machine->powerOn();
	}
	else
	{
		xlogIn(" -> insert");

		cstr filter	  = "IF2 Rom Cartridges (*.rom)"; //";;All Files (*)";
		cstr filepath = selectLoadFile(this, "Select Rom Cartridge", filter);
		if (filepath) insertRom(filepath);
	}
}

void SpectraVideoInspector::updateWidgets()
{
	xlogIn("SpectraVideoInspector::updateWidgets");
	assert(validReference(spectra));

	uint8 newstate = spectra->peekJoystickButtonsFUDLR();
	if (js_state != newstate)
	{
		js_state = newstate;
		js_display->setText(binstr(newstate, "--------", "111FUDLR"));
	}

	cstr new_romfile = spectra->filepath;
	if (rom_file != new_romfile)
	{
		rom_name->setText(new_romfile ? basename_from_path(new_romfile) : nullptr);

		if (!rom_file)
		{
			background.load(catstr(appl_rsrc_path, "/Images/Spectra_Loaded.jpg"));
			button_insert_rom->setText("Eject Rom");
			update();
		}
		if (!new_romfile)
		{
			background.load(catstr(appl_rsrc_path, "/Images/Spectra.jpg"));
			button_insert_rom->setText("Insert Rom");
			update();
		}

		rom_file = new_romfile;
	}

	if (checkbox_if1_rom_hooks->isChecked() != spectra->if1_rom_hooks_enabled)
		checkbox_if1_rom_hooks->setChecked(spectra->if1_rom_hooks_enabled);

	if (checkbox_rs232->isChecked() != spectra->rs232_enabled) checkbox_rs232->setChecked(spectra->rs232_enabled);

	if (checkbox_new_video_modes->isChecked() != spectra->new_video_modes_enabled)
		checkbox_new_video_modes->setChecked(spectra->new_video_modes_enabled);

	bool f = spectra->joystick_enabled;
	if (checkbox_joystick->isChecked() != f)
	{
		checkbox_joystick->setChecked(f);
		//		js_selector->setEnabled(f);
		//		js_display->setEnabled(f);
		//		button_set_keys->setEnabled(f);
		//		button_scan_usb->setEnabled(f);
	}
}

void SpectraVideoInspector::fillContextMenu(QMenu* menu)
{
	// fill context menu for right-click
	// called by Inspector::contextMenuEvent()
	// items inserted here are inserted at the to of the popup menu

	xlogIn("SpectraVideoInspector::fillContextMenu");
	assert(validReference(spectra));
	Inspector::fillContextMenu(menu); // NOP

	if (spectra->isRomInserted())
	{
		menu->addAction("Eject Rom", this, &SpectraVideoInspector::slotInsertOrEjectRom); //
	}
	else
	{
		menu->addAction("Insert Rom", this, &SpectraVideoInspector::slotInsertOrEjectRom);
		menu->addAction("Recent Roms …")->setMenu(new RecentFilesMenu(RecentIf2Roms, this, [=](cstr fpath) {
			insertRom(fpath);
		}));
	}
}

void SpectraVideoInspector::insertRom(cstr filepath)
{
	xlogIn("SpectraVideoInspector::insertRom");
	assert(validReference(spectra));

	bool f = nvptr(machine)->powerOff();
	NV(spectra)->insertRom(filepath);
	if (f) machine->powerOn();
	addRecentFile(RecentIf2Roms, filepath);
	addRecentFile(RecentFiles, filepath);
}

} // namespace gui


/*






























*/
