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


namespace gui
{

class SpectraVideoInspector : public Inspector
{
	volatile SpectraVideo* const spectra;

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
	SpectraVideoInspector(QWidget*, MachineController*, volatile SpectraVideo*);
	void insertRom(cstr filepath);

protected:
	void fillContextMenu(QMenu*) override;
	void updateWidgets() override;

private:
	void update_js_selector();
	void slot_js_selector_selected();
	void slot_find_usb_joysticks();
	void slot_set_keyboard_joystick_keys();
	void slot_enable_if1_rom_hooks(bool);
	void slot_enable_rs232(bool);
	void slot_enable_joystick(bool);
	void slot_enable_new_displaymodes(bool);
	void slot_insert_or_eject_rom();
};

} // namespace gui
