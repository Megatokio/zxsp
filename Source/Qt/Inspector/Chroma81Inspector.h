// Copyright (c) 2026 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Chroma81.h"
#include "Inspector.h"
class QCheckBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;


namespace zxsp
{

class Chroma81Inspector : public Inspector
{
public:
	Chroma81Inspector(QWidget*, MachineController*, volatile Chroma81*);

	void insertRom(cstr filepath);

private:
	void fillContextMenu(QMenu*) override;
	void updateWidgets() override;

	void update_joystick_selector();
	void slotJoystickSelected();
	void slotFindUsbJoysticks();
	void slotSetKeyboardJoystickKeys();

	void slotEnable16kRam(bool);
	void slotEnableRS232(bool);
	void slotEnable8kRam(bool);
	void slotEnableWRXGraphics(bool);
	void slotEnableQSCharBoard(bool);
	void slotEnableNewColorModes(bool);
	void slotInsertOrEjectRom();

private:
	volatile Chroma81* chroma;

	QCheckBox* checkbox_ram_at_4000			  = nullptr;
	QCheckBox* checkbox_rs232				  = nullptr;
	QCheckBox* checkbox_ram_at_2000			  = nullptr;
	QCheckBox* checkbox_ram_at_C000_and_color = nullptr;
	QCheckBox* checkbox_wrx_graphics		  = nullptr;
	QCheckBox* checkbox_qs_char_board		  = nullptr;

	class ColorIndicator* color_indicator = nullptr;
	bool				  bg_img_color	  = false;

	QPushButton* button_insert_rom = nullptr;
	QPushButton* button_scan_usb   = nullptr;
	QPushButton* button_set_keys   = nullptr;

	QLineEdit* js_display  = nullptr;
	uint8	   js_state	   = 0;
	QComboBox* js_selector = nullptr;

	QLabel* ir_display = nullptr; // show I register value

	QLabel* rom_name = nullptr;
	cstr	rom_file = nullptr; // 2nd
};

} // namespace zxsp


/*























*/
