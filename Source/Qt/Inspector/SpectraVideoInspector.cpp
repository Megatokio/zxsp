// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QMenu>
#include <QTimer>
#include "SpectraVideoInspector.h"
#include "Machine.h"
#include "Items/SpectraVideo.h"
#include "Joystick.h"	// physical joysticks
#include "Items/Joy/Joy.h"
#include <QLabel>
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "RecentFilesMenu.h"
#include "Templates/NVPtr.h"
#include "Dialogs/ConfigureKeyboardJoystickDialog.h"


SpectraVideoInspector::SpectraVideoInspector(QWidget*w, MachineController* mc, volatile IsaObject *i )
:
	Inspector(w,mc,i,SpectraVideoPtr(i)->isRomInserted()?"/Images/Spectra_Loaded.jpg":"/Images/Spectra.jpg"),
	js_state(0),
	rom_file(nullptr)
{
	assert(object->isA(isa_SpectraVideo));
//	ula = UlaSpectraPtr(machine->ula);

//	this->setFixedSize( background.size() );

	rom_name = new QLabel(this);
	rom_name->move(155,50);
	rom_name->setFixedWidth(150);
	rom_name->setFont(QFont("Arial",13));
	rom_name->setAlignment(Qt::AlignTop);
	setColors(rom_name, 0xffffff/*foregroundcolor*/);

	js_display = new QLineEdit(this);
	js_display->setText("%--------");
	js_display->setAlignment(Qt::AlignHCenter);
	js_display->setReadOnly(yes);
	js_display->setFixedWidth(105);

	js_selector = new QComboBox(this);
	js_selector->setFocusPolicy(Qt::NoFocus);
	js_selector->setFixedWidth(110);
	connect(js_selector, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &SpectraVideoInspector::js_selector_selected);

	button_scan_usb = new QPushButton("Scan USB",this);
	button_scan_usb->setMinimumWidth(100);
	connect(button_scan_usb,&QPushButton::clicked,this,&SpectraVideoInspector::find_usb_joysticks);

	button_set_keys = new QPushButton("Set Keys",this);
	button_set_keys->setMinimumWidth(100);
	connect(button_set_keys,&QPushButton::clicked,this,&SpectraVideoInspector::set_keyboard_joystick_keys);

	uint x1=20-5, x2=130-7, x3=208-10, x4=290;

	uint y1=242,  y2=267;	// buttons
	uint dy=15;
	uint ya=227+3, yb=ya+dy, yc=yb+dy, yd=yc+dy;	// checkboxes

	checkbox_if1_rom_hooks		= new QCheckBox("IF1Rom hooks",this);	connect(checkbox_if1_rom_hooks,	 &QCheckBox::toggled,this,&SpectraVideoInspector::enable_if1_rom_hooks);
	checkbox_rs232				= new QCheckBox("RS232",this);			connect(checkbox_rs232,			 &QCheckBox::toggled,this,&SpectraVideoInspector::enable_rs232);
	checkbox_joystick			= new QCheckBox("Joystick",this);		connect(checkbox_joystick,		 &QCheckBox::toggled,this,&SpectraVideoInspector::enable_joystick);
	checkbox_new_video_modes	= new QCheckBox("New colours",this);	connect(checkbox_new_video_modes,&QCheckBox::toggled,this,&SpectraVideoInspector::enable_new_displaymodes);
	button_insert_rom			= new QPushButton(SpectraVideoPtr(i)->isRomInserted()?"Eject Rom":"Insert Rom",this);
																		connect(button_insert_rom,		 &QPushButton::clicked,this,&SpectraVideoInspector::insert_or_eject_rom);

	button_set_keys->setFixedWidth(80);
	button_scan_usb->setFixedWidth(80+12);
	button_insert_rom->setFixedWidth(80+12);

	js_selector->move(x1,y1+1);			button_set_keys->move(x2,y1);	button_scan_usb->move(x3,y1);
	js_display->move(x1+2,y2+3);										button_insert_rom->move(x3,y2);

	checkbox_if1_rom_hooks->move(x4,ya);
	checkbox_joystick->move(x4,yb);
	checkbox_rs232->move(x4,yc);
	checkbox_new_video_modes->move(x4,yd);

	bool f = spectra()->newVideoModesEnabled();
	checkbox_new_video_modes->setChecked(f);

	f = spectra()->rs232_enabled;
	checkbox_rs232->setChecked(f);

	f = spectra()->joystick_enabled;
	checkbox_joystick->setChecked(f);
	js_selector->setEnabled(f);
	js_display->setEnabled(f);
	button_set_keys->setEnabled(f);
	button_scan_usb->setEnabled(f);

	f = spectra()->if1_rom_hooks_enabled;
	checkbox_if1_rom_hooks->setChecked(f);

	update_js_selector();

	timer->start(1000/15);
}

SpectraVideoInspector::~SpectraVideoInspector()
{}

void SpectraVideoInspector::enable_if1_rom_hooks(bool f)
{
//	checkbox_if1_rom_hooks->setChecked(f);
	NVPtr<SpectraVideo>(spectra())->setIF1RomHooksEnabled(f);
}

void SpectraVideoInspector::enable_rs232(bool f)
{
//	checkbox_rs232->setChecked(f);
	NVPtr<SpectraVideo>(spectra())->setRS232Enabled(f);
}

void SpectraVideoInspector::enable_new_displaymodes(bool f)
{
//	checkbox_new_video_modes->setChecked(f);
	NVPtr<SpectraVideo>(spectra())->enableNewVideoModes(f);
}

void SpectraVideoInspector::enable_joystick(bool f)
{
//	checkbox_joystick->setChecked(f);
	NVPtr<SpectraVideo>(spectra())->setJoystickEnabled(f);
//	js_selector->setEnabled(f);
//	js_display->setEnabled(f);
//	button_set_keys->setEnabled(f);
//	button_scan_usb->setEnabled(f);
}


void SpectraVideoInspector::find_usb_joysticks()
{
	xlogIn("SpectraVideoInspector::scanUSB");
	findUsbJoysticks();
	update_js_selector();
}

void SpectraVideoInspector::set_keyboard_joystick_keys()
{
	xlogIn("SpectraVideoInspector::setKeys");
//	getKbdJoystick()->setKeys();

	ConfigDialog* d = new ConfigureKeyboardJoystickDialog(controller);
	d->show();
}

void SpectraVideoInspector::js_selector_selected()
{
	xlogIn("SpectraVideoInspector::joySelected");

	int j = js_selector->currentIndex();
	spectra()->insertJoystick(js_selector->itemData(j).toInt());
}

void SpectraVideoInspector::update_js_selector()
{
	char f[max_joy]; int i;
	for( i=0; i<max_joy; i++ )			  f[i] = joysticks[i]->isConnected() ? '1' : '0';
	for( i=0; i<js_selector->count(); i++ )	  f[js_selector->itemData(i).toInt()] += 2;
	for( i=0; i<max_joy; i++ )			  if(f[i]!='0'&&f[i]!='3') break;
	if(i==max_joy) return;	// no change

	static cstr jname[5] = { "USB Joystick 1", "USB Joystick 2", "USB Joystick 3", "Keyboard", "no Joystick" };

	while( js_selector->count()) { js_selector->removeItem(0); }
	for( i=0;i<max_joy;i++) { if( f[i]!='0' ) js_selector->addItem(jname[i],i); }

	int id = spectra()->getJoystickID();
	for( i=0;i<js_selector->count();i++ ) { if( js_selector->itemData(i).toInt()==id ) { js_selector->setCurrentIndex(i); break; } }
	if(i==js_selector->count()) { js_selector->setCurrentIndex(i-1); }
}


//slot
void SpectraVideoInspector::insert_or_eject_rom()
{
	if(spectra()->isRomInserted())
	{
		xlogIn("SpectraVideoInspector::eject()");

		machine->powerOff();
			NV(spectra())->ejectRom();
		machine->powerOn();
	}
	else
	{
		xlogIn("SpectraVideoInspector::insert()");

		cstr filter = "IF2 Rom Cartridges (*.rom)"; //";;All Files (*)";
		cstr filepath = selectLoadFile(this, "Select Rom Cartridge", filter);
		if(!filepath) return;

		machine->powerOff();
			NV(spectra())->insertRom(filepath);
		machine->powerOn();
	}
}


void SpectraVideoInspector::updateWidgets()
{
	xlogIn("SpectraVideoInspector::updateWidgets");

	if(!object) return;
	volatile SpectraVideo* spectra = Inspector::spectra();

	uint8 newstate = spectra->joystick->getState(no);
	if(js_state!=newstate)
	{
		js_state = newstate;
		char s[] = "%111FUDLR";
		for(int j=0;j<8;j++){ if(((~newstate)<<j)&0x80) s[j+1]='-'; }
		js_display->setText(s);
	}

	cstr new_romfile = spectra->filepath;
	if(rom_file != new_romfile)
	{
		rom_name->setText(new_romfile?basename_from_path(new_romfile):nullptr);

		if(!rom_file)
		{
			background.load( catstr(appl_rsrc_path,"/Images/Spectra_Loaded.jpg") );
			button_insert_rom->setText("Eject Rom");
			update();
		}
		if(!new_romfile)
		{
			background.load( catstr(appl_rsrc_path,"/Images/Spectra.jpg") );
			button_insert_rom->setText("Insert Rom");
			update();
		}

		rom_file = new_romfile;
	}

	if(checkbox_if1_rom_hooks->isChecked() != spectra->if1_rom_hooks_enabled)
		checkbox_if1_rom_hooks->setChecked(spectra->if1_rom_hooks_enabled);

	if(checkbox_rs232->isChecked() != spectra->rs232_enabled)
		checkbox_rs232->setChecked(spectra->rs232_enabled);

	if(checkbox_new_video_modes->isChecked() != spectra->new_video_modes_enabled)
		checkbox_new_video_modes->setChecked(spectra->new_video_modes_enabled);

	bool f = spectra->joystick_enabled;
	if(checkbox_joystick->isChecked() != f)
	{
		checkbox_joystick->setChecked(f);
		js_selector->setEnabled(f);
		js_display->setEnabled(f);
		button_set_keys->setEnabled(f);
		button_scan_usb->setEnabled(f);
	}
}


/*	fill context menu for right-click
	called by Inspector::contextMenuEvent()
	items inserted here are inserted at the to of the popup menu
*/
void SpectraVideoInspector::fillContextMenu(QMenu* menu)
{
	Inspector::fillContextMenu(menu);	// NOP

	bool f = NVPtr<SpectraVideo>(spectra())->isRomInserted();
	if(f)
	{
		menu->addAction("Eject Rom", this, &SpectraVideoInspector::insert_or_eject_rom);
	}
	else
	{
		menu->addAction("Insert Rom", this, &SpectraVideoInspector::insert_or_eject_rom);
		menu->addAction("Recent Roms …")->setMenu(
				new RecentFilesMenu(RecentIf2Roms, this, [=](cstr fpath){insertRom(fpath);}));
	}
}


void SpectraVideoInspector::insertRom(cstr filepath)
{
	machine->powerOff();
		if(spectra()->isRomInserted()) NV(spectra())->ejectRom();
		NV(spectra())->insertRom(filepath);
	machine->powerOn();
}




























