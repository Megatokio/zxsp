#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
class QCheckBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;


class SpectraVideoInspector : public Inspector
{
	QCheckBox*	 checkbox_if1_rom_hooks;
	QCheckBox*	 checkbox_rs232;
	QCheckBox*	 checkbox_joystick;
	QCheckBox*	 checkbox_new_video_modes;
	QPushButton* button_insert_rom;
	QPushButton* button_scan_usb;
	QPushButton* button_set_keys;

	QLineEdit* js_display;
	uint8	   js_state;
	QComboBox* js_selector;
	QLabel*	   rom_name;
	cstr	   rom_file; // 2nd

public:
	SpectraVideoInspector(QWidget*, MachineController*, volatile IsaObject*);
	~SpectraVideoInspector();
	void insertRom(cstr filepath);

protected:
	void fillContextMenu(QMenu*) override;
	void updateWidgets() override;

private:
	void update_js_selector();
	void js_selector_selected();
	void find_usb_joysticks();
	void set_keyboard_joystick_keys();
	void enable_if1_rom_hooks(bool);
	void enable_rs232(bool);
	void enable_joystick(bool);
	void enable_new_displaymodes(bool);
	void insert_or_eject_rom();
};
