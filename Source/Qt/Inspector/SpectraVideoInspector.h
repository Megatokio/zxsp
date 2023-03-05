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
	void update_joystick_selector();
	void slotJoystickSelected();
	void slotFindUsbJoysticks();
	void slotSetKeyboardJoystickKeys();
	void slotEnableIf1RomHooks(bool);
	void slotEnableRS232(bool);
	void slotEnableJoystick(bool);
	void slotEnableNewDisplaymodes(bool);
	void slotInsertOrEjectRom();
};

} // namespace gui
