// Copyright (c) 2026 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Chroma81Inspector.h"
#include "Dialogs/ConfigureKeyboardJoystickDialog.h"
#include "Machine.h"
#include "MyLineEdit.h"
#include "RecentFilesMenu.h"
#include "Templates/NVPtr.h"
#include "UsbJoystick.h" // "OS/Mac/UsbJoystick.h"
#include "qt_util.h"
#include "zxsp_globals.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QTimer>

namespace zxsp
{

static constexpr char img_rom_color[] = "/Images/Chroma81-loaded.jpg";
static constexpr char img_color[]	  = "/Images/Chroma81.jpg";
static constexpr char img_sw[]		  = "/Images/Chroma81-sw.jpg";
static constexpr char img_rom_sw[]	  = "/Images/Chroma81-loaded.jpg";


class ColorIndicator : public QWidget
{
public:
	ColorIndicator(QWidget* owner, bool color_on, bool specci_on);
	void setColor(bool, bool);
	void paintEvent(QPaintEvent*) override;
	bool color_on;
	bool specci_on;
};
ColorIndicator::ColorIndicator(QWidget* owner, bool c, bool s) : QWidget(owner), color_on(c), specci_on(s) {}
void ColorIndicator::setColor(bool c, bool s)
{
	if (c == color_on && s == specci_on) return;
	color_on  = c;
	specci_on = s;
	update();
}
void ColorIndicator::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.setPen(Qt::NoPen);

	if (color_on)
	{
		int w = width() / 3;
		int h = height();

		p.setBrush(Qt::red);
		p.drawRect(w * 0, 0, w, h);
		p.setBrush(Qt::green);
		p.drawRect(w * 1, 0, w, h);
		p.setBrush(Qt::blue);
		p.drawRect(w * 2, 0, w, h);

		if (!specci_on) // colorized characters
		{
			h = h / 2;

			p.setBrush(Qt::cyan);
			p.drawRect(w * 0, h, w, h);
			p.setBrush(Qt::magenta);
			p.drawRect(w * 1, h, w, h);
			p.setBrush(Qt::yellow);
			p.drawRect(w * 2, h, w, h);
		}
	}
	else // b&w
	{
		int w = width() / 2;
		int h = height();

		p.setBrush(Qt::black);
		p.drawRect(0, 0, w, h);
		p.setBrush(Qt::white);
		p.drawRect(w, 0, w, h);
	}

	p.setPen(Qt::black);
	p.setBrush(Qt::transparent);
	p.drawRect(0, 0, width() - 1, height() - 1);
}


Chroma81Inspector::Chroma81Inspector(QWidget* w, MachineController* mc, volatile Chroma81* chroma) :
	Inspector(w, mc, chroma, chroma->isRomInserted() ? img_rom_color : img_color),
	chroma(chroma),
	bg_img_color(true)
{
	//	this->setFixedSize( background.size() );

	rom_name = new QLabel(this);
	rom_name->move(155, 50);
	rom_name->setFixedWidth(150);
	rom_name->setFont(QFont("Arial", 13));
	rom_name->setAlignment(Qt::AlignTop);
	setColors(rom_name, 0xffffff /*foregroundcolor*/);

	js_display = new MyLineEdit("%-----:-----", this);
	js_display->setAlignment(Qt::AlignHCenter);
	js_display->setReadOnly(yes);
	js_display->setFixedWidth(105);

	js_selector = new QComboBox(this);
	js_selector->setFocusPolicy(Qt::NoFocus);
	js_selector->setFixedWidth(110);
	update_joystick_selector();
	connect(
		js_selector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		&Chroma81Inspector::slotJoystickSelected);

	ir_display = new QLabel(this);
	ir_display->setFixedWidth(40);
	ir_display->setFont(QFont("Arial", 13));
	ir_display->setAlignment(Qt::AlignHCenter);
	ir_display->setEnabled(no); // --> dimmed

	button_scan_usb = new QPushButton("Scan USB", this);
	button_scan_usb->setMinimumWidth(100);
	connect(button_scan_usb, &QPushButton::clicked, this, &Chroma81Inspector::slotFindUsbJoysticks);

	button_set_keys = new QPushButton("Set Keys", this);
	button_set_keys->setMinimumWidth(100);
	connect(button_set_keys, &QPushButton::clicked, this, &Chroma81Inspector::slotSetKeyboardJoystickKeys);

	checkbox_wrx_graphics = new QCheckBox("WRX graphics", this);
	checkbox_wrx_graphics->setChecked(chroma->dip_wrx_enabled);
	connect(checkbox_wrx_graphics, &QCheckBox::toggled, this, &Chroma81Inspector::slotEnableWRXGraphics);

	checkbox_qs_char_board = new QCheckBox("QS char board", this);
	checkbox_qs_char_board->setChecked(chroma->dip_qs_enabled);
	connect(checkbox_qs_char_board, &QCheckBox::toggled, this, &Chroma81Inspector::slotEnableQSCharBoard);

	checkbox_ram_at_C000_and_color = new QCheckBox("Ram at $C000", this);
	checkbox_ram_at_C000_and_color->setChecked(chroma->dip_ram_at_C000_and_color_enabled);
	connect(checkbox_ram_at_C000_and_color, &QCheckBox::toggled, this, &Chroma81Inspector::slotEnableNewColorModes);

	checkbox_ram_at_2000 = new QCheckBox("Ram at $2000", this);
	checkbox_ram_at_2000->setChecked(chroma->dip_ram_at_2000);
	connect(checkbox_ram_at_2000, &QCheckBox::toggled, this, &Chroma81Inspector::slotEnable8kRam);

	checkbox_ram_at_4000 = new QCheckBox("Ram at $4000", this);
	checkbox_ram_at_4000->setChecked(chroma->dip_ram_at_4000);
	connect(checkbox_ram_at_4000, &QCheckBox::toggled, this, &Chroma81Inspector::slotEnable16kRam);

	checkbox_rs232 = new QCheckBox("RS232", this);
	checkbox_rs232->setChecked(chroma->dip_rs232_enabled);
	connect(checkbox_rs232, &QCheckBox::toggled, this, &Chroma81Inspector::slotEnableRS232);
	checkbox_rs232->setEnabled(no); // TODO

	button_insert_rom = new QPushButton(chroma->isRomInserted() ? "Eject Rom" : "Insert Rom", this);
	connect(button_insert_rom, &QPushButton::clicked, this, &Chroma81Inspector::slotInsertOrEjectRom);
	button_insert_rom->setEnabled(no); // TODO

	color_indicator = new ColorIndicator(this, chroma->color_modes_enabled, chroma->specci_color_mode);
	color_indicator->setFixedSize(42, 16);

	button_set_keys->setFixedWidth(80);
	button_scan_usb->setFixedWidth(80 + 12);
	button_insert_rom->setFixedWidth(80 + 12);

	int x1 = 14;  // selector, lineedit
	int x2 = 122; // buttons
	int x3 = 197; // buttons
	int x4 = 289; // checkboxes right

	int y1 = 243; // buttons
	int y2 = 268; // buttons
	int dy = 15;
	int yo = 5;	  // checkboxes top
	int yu = 231; // checkboxes bottom

	js_selector->move(x1, y1 + 1);
	button_set_keys->move(x2, y1);
	button_scan_usb->move(x3, y1);
	js_display->move(x1 + 2, y2 + 5);
	button_insert_rom->move(x3, y2);

	checkbox_ram_at_2000->move(x1, yo);
	checkbox_ram_at_4000->move(x1, yo + dy + 1);
	checkbox_ram_at_C000_and_color->move(x4, yo);
	color_indicator->move(width() - 45, yo + dy + 3);
	ir_display->move(width() - 45, yo + dy * 2 + 6);

	checkbox_wrx_graphics->move(x4, yu + dy);
	checkbox_qs_char_board->move(x4, yu + 2 * dy);
	checkbox_rs232->move(x4, yu + 3 * dy);

	timer->start(1000 / 15);
}

void Chroma81Inspector::updateWidgets()
{
	xlogIn("Chroma81Inspector::updateWidgets");
	assert(validReference(chroma));

	bool update_img = no;

	if (bg_img_color != chroma->dip_ram_at_C000_and_color_enabled)
	{
		bg_img_color = chroma->dip_ram_at_C000_and_color_enabled;
		update_img	 = yes;
		//color_indicator->setVisible(color_image_state);
	}

	cstr new_romfile = chroma->rom_filepath;
	if (rom_file != new_romfile)
	{
		rom_file   = new_romfile;
		update_img = yes;

		rom_name->setText(new_romfile ? basename_from_path(new_romfile) : nullptr);
		button_insert_rom->setText(rom_file ? "Eject Rom" : "Insert Rom");
	}

	if (update_img)
	{
		bool loaded = rom_file != nullptr;
		bool color	= bg_img_color;
		cstr bg_img = color ? (loaded ? img_rom_color : img_color) : (loaded ? img_rom_sw : img_sw);
		background.load(catstr(appl_rsrc_path, bg_img));
		update();
	}

	{
		char text[] = "I=0x00";
		int	 i		= machine->cpu->getRegisters().i;
		text[4]		= hexchar(i >> 4);
		text[5]		= hexchar(i);
		ir_display->setText(text);
	}

	color_indicator->setColor(chroma->color_modes_enabled, chroma->specci_color_mode);

	if (checkbox_rs232->isChecked() != chroma->dip_rs232_enabled) //
		checkbox_rs232->setChecked(chroma->dip_rs232_enabled);

	if (checkbox_ram_at_2000->isChecked() != chroma->dip_ram_at_2000) //
		checkbox_ram_at_2000->setChecked(chroma->dip_ram_at_2000);

	if (checkbox_ram_at_4000->isChecked() != chroma->dip_ram_at_4000) //
		checkbox_ram_at_4000->setChecked(chroma->dip_ram_at_4000);

	if (checkbox_ram_at_C000_and_color->isChecked() != chroma->dip_ram_at_C000_and_color_enabled) //
		checkbox_ram_at_C000_and_color->setChecked(chroma->dip_ram_at_C000_and_color_enabled);

	if (checkbox_qs_char_board->isChecked() != chroma->dip_qs_enabled) //
		checkbox_qs_char_board->setChecked(chroma->dip_qs_enabled);

	if (checkbox_wrx_graphics->isChecked() != chroma->dip_wrx_enabled) //
		checkbox_wrx_graphics->setChecked(chroma->dip_wrx_enabled);

	uint8 newstate = chroma->peekJoystickButtonsFUDLR(); // à la Kempston: %xxxFUDLR
	if (js_state != newstate)
	{
		js_state = newstate;

		// cursor keys:
		//   left  -> key "5" -> bit 4   port 0xf7fe
		//   down  -> key "6" -> bit 4   port 0xeffe
		//   up    -> key "7" -> bit 3   port 0xeffe
		//   right -> key "8" -> bit 2   port 0xeffe
		//   fire  -> key "0" -> bit 0   port 0xeffe

		uint mybyte = ((newstate & 2u) << 9)	 // left
					  + ((newstate & 1u) << 2)	 // right
					  + ((newstate & 4u) << 2)	 // down
					  + ((newstate & 8u) << 0)	 // up
					  + ((newstate & 16u) >> 4); // fire

		cstr s = binstr(mybyte, "%-----:-----", "%LxxxxxDURxF");
		js_display->setText(s);
	}
}

void Chroma81Inspector::slotEnableNewColorModes(bool f)
{
	assert(validReference(chroma));
	nvptr(chroma)->setRamAtC000AndColorEnabled(f);
}
void Chroma81Inspector::slotEnable16kRam(bool f)
{
	assert(validReference(chroma));
	nvptr(chroma)->set16kRamAt4000Enabled(f);
}
void Chroma81Inspector::slotEnable8kRam(bool f)
{
	assert(validReference(chroma));
	nvptr(chroma)->set8kRamAt2000Enabled(f);
}
void Chroma81Inspector::slotEnableQSCharBoard(bool f)
{
	assert(validReference(chroma));
	nvptr(chroma)->setQSEnabled(f);
}
void Chroma81Inspector::slotEnableWRXGraphics(bool f)
{
	assert(validReference(chroma));
	nvptr(chroma)->setWRXEnabled(f);
}
void Chroma81Inspector::slotEnableRS232(bool f)
{
	assert(validReference(chroma));
	nvptr(chroma)->setRS232Enabled(f);
}

void Chroma81Inspector::slotFindUsbJoysticks()
{
	xlogIn("Chroma81Inspector::slotFindUsbJoysticks");

	findUsbJoysticks();
	update_joystick_selector();
}

void Chroma81Inspector::update_joystick_selector()
{
	xlogIn("Chroma81Inspector::update_js_selector");
	assert(validReference(chroma));

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

	js_selector->setCurrentIndex(chroma->getJoystickID());
}

void Chroma81Inspector::slotJoystickSelected()
{
	xlogIn("Chroma81Inspector::slotJoystickSelected");
	assert(validReference(chroma));

	chroma->insertJoystick(JoystickID(js_selector->currentIndex()));
}

void Chroma81Inspector::slotSetKeyboardJoystickKeys()
{
	xlogIn("Chroma81Inspector::slotSetKeyboardJoystickKeys");

	ConfigDialog* d = new ConfigureKeyboardJoystickDialog(controller);
	d->show();
}

void Chroma81Inspector::fillContextMenu(QMenu* menu)
{
	// fill context menu for right-click
	// called by Inspector::contextMenuEvent()
	// items inserted here are inserted at the to of the popup menu

	xlogIn("Chroma81Inspector::fillContextMenu");
	assert(validReference(chroma));
	Inspector::fillContextMenu(menu); // NOP

	if (chroma->isRomInserted())
	{
		menu->addAction("Eject Rom", this, &Chroma81Inspector::slotInsertOrEjectRom); //
	}
	else
	{
		menu->addAction("Insert Rom", this, &Chroma81Inspector::slotInsertOrEjectRom);
		menu->addAction("Recent Roms …")
			->setMenu(new RecentFilesMenu( //
				RecentChroma81Roms, this, [=](cstr fpath) { insertRom(fpath); }));
	}
	// TODO: enable color and set color mode?
}

void Chroma81Inspector::slotInsertOrEjectRom()
{
	xlogIn("Chroma81Inspector::slotInsertOrEjectRom()");
	assert(validReference(chroma));

	if (chroma->isRomInserted())
	{
		xlogIn(" -> eject");

		bool f = nvptr(machine)->powerOff();
		NV(chroma)->ejectRom();
		if (f) machine->powerOn();
	}
	else
	{
		xlogIn(" -> insert");

		cstr filter	  = "Chroma81 Rom Cartridges (*.rom);;All Files (*)";
		cstr filepath = selectLoadFile(this, "Select Rom Cartridge", filter);
		if (filepath) insertRom(filepath);
	}
}

void Chroma81Inspector::insertRom(cstr filepath)
{
	xlogIn("Chroma81Inspector::insertRom");
	assert(validReference(chroma));

	bool f = nvptr(machine)->powerOff();
	NV(chroma)->insertRom(filepath);
	if (f) machine->powerOn();
	addRecentFile(RecentChroma81Roms, filepath);
	addRecentFile(RecentFiles, filepath);
}

} // namespace zxsp

/*


























*/
