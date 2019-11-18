/*	Copyright  (c)	Günter Woigk 2013 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/


#ifndef SPECTRAINSP_H
#define SPECTRAINSP_H

#include "Inspector.h"
class QCheckBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;


class SpectraVideoInspector : public Inspector
{
	QCheckBox*		checkbox_if1_rom_hooks;
	QCheckBox*		checkbox_rs232;
	QCheckBox*		checkbox_joystick;
	QCheckBox*		checkbox_new_video_modes;
	QPushButton*	button_insert_rom;
	QPushButton*	button_scan_usb;
	QPushButton*	button_set_keys;

	QLineEdit*		js_display;
	uint8			js_state;
	QComboBox*		js_selector;
	QLabel*         rom_name;
	cstr			rom_file;		// 2nd

public:
	SpectraVideoInspector(QWidget*, MachineController*, volatile IsaObject*);
	~SpectraVideoInspector();
	void	insertRom(cstr filepath);

protected:
	void	fillContextMenu(QMenu*) override;
	void	updateWidgets() override;

private:
	void	update_js_selector();
	void	js_selector_selected();
	void	find_usb_joysticks();
	void	set_keyboard_joystick_keys();
	void	enable_if1_rom_hooks(bool);
	void	enable_rs232(bool);
	void	enable_joystick(bool);
	void	enable_new_displaymodes(bool);
	void    insert_or_eject_rom();
};

#endif













