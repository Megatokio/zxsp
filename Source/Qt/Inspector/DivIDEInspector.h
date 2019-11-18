#pragma once
/*	Copyright  (c)	Günter Woigk 2014 - 2019
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

#include "Inspector.h"


class DivIDEInspector : public Inspector
{
	QPixmap		overlay_jumper_E;
	QPixmap		overlay_jumper_A;
	QPixmap		overlay_jumper_EA;
	QPixmap		overlay_module;
	QPixmap		overlay_led_green_hi;
	QPixmap		overlay_led_yellow_hi;
	QPixmap		overlay_led_red_hi;

	QWidget*	module;
	QWidget*	jumper_E;
	QWidget*	eeprom;
	QWidget*	button;

	struct
	{
		bool	jumper_A;		// set for +2A / +3
		bool	jumper_E;		// wprot eeprom
		bool	led_green;		// power on
		bool	led_yellow;		// MAPRAM activated
		bool	led_red;		// IDE busy
		cstr	diskname;
	}	state;


public:
	DivIDEInspector(QWidget*, MachineController *mc, volatile IsaObject*);
	~DivIDEInspector();

protected:
	void	paintEvent(QPaintEvent*) override;
	void	mousePressEvent(QMouseEvent*) override;

	void	fillContextMenu(QMenu*) override;
	cstr	getCustomTitle() override;
	void	updateWidgets() override;

private:
	void	load_rom(cstr);
	void	insert_new_disk(cstr basename);
	void	insert_disk(cstr filepath);
	void	load_default_rom();
	void	load_rom();
	void	save_rom();
	void	toggle_jumper_E();
	void	insert_new_16M();
	void	insert_new_128M();
	void	insert_disk();
	void	eject_disk();
	void	toggle_disk_wprot();
	void	set_ram_32k();
	void	set_ram_512k();
};


