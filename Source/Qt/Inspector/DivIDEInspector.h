#pragma once
// Copyright (c) 2014 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"


namespace gui
{

class DivIDEInspector : public Inspector
{
	volatile DivIDE* divide;

	QPixmap overlay_jumper_E;
	QPixmap overlay_jumper_A;
	QPixmap overlay_jumper_EA;
	QPixmap overlay_module;
	QPixmap overlay_led_green_hi;
	QPixmap overlay_led_yellow_hi;
	QPixmap overlay_led_red_hi;

	QWidget* module;
	QWidget* jumper_E;
	QWidget* eeprom;
	QWidget* button;

	struct
	{
		bool jumper_A;	 // set for +2A / +3
		bool jumper_E;	 // wprot eeprom
		bool led_green;	 // power on
		bool led_yellow; // MAPRAM activated
		bool led_red;	 // IDE busy
		cstr diskname;
	} state;


public:
	DivIDEInspector(QWidget*, MachineController* mc, volatile DivIDE*);
	~DivIDEInspector() override;

protected:
	void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent*) override;

	void fillContextMenu(QMenu*) override;
	cstr getCustomTitle() override;
	void updateWidgets() override;

private:
	void load_rom(cstr);
	void insert_new_disk(cstr basename);
	void insert_disk(cstr filepath);
	void load_default_rom();
	void load_rom();
	void save_rom();
	void toggle_jumper_E();
	void insert_disk();
	void eject_disk();
	void toggle_disk_wprot();
	void set_ram(uint size);
};

} // namespace gui
