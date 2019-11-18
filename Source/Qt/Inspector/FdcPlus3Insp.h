#pragma once
/*	Copyright  (c)	Günter Woigk 2012 - 2019
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

#include <QObject>
#include "Inspector.h"
#include "Items/Fdc/FloppyDiskDrive.h"


class FdcPlus3Insp : public Inspector
{
	volatile FdcPlus3*	fdc;
	FloppyDiskDrive*	drive;

	QPixmap		overlay_disk_A_ejected;
	QPixmap		overlay_disk_A_inserted;
	QPixmap		overlay_disk_B_ejected;
	QPixmap		overlay_disk_B_inserted;
	QPixmap		overlay_led;

	QWidget*	slot;
	QWidget*	disk_ejected_front;
	QWidget*	eject_button;
	QWidget*	disk_ejected_label_B;
	QWidget*	disk_ejected_top;

	enum DiskState { NoDisk, Ejected, Loaded };
	DiskState	diskstate;
	cstr		current_disk;

// displayed state:
	bool		led_on;

public:
	FdcPlus3Insp( QWidget*, MachineController*, volatile IsaObject* );
	~FdcPlus3Insp();

protected:
	void	paintEvent(QPaintEvent*) override;
	void	mousePressEvent(QMouseEvent*) override;

	void	updateWidgets() override;
	void	fillContextMenu(QMenu*) override;

private:
	bool	motor_on();
	bool	side_B_up();
	cstr	get_save_filename(cstr msg = "Save +3 disc as…") throws;
	cstr	get_load_filename(cstr msg = "Load +3 disc file…") throws;
	void	set_disk_state(DiskState);
	void	insert_disk();
	void	insert_disk(cstr filepath);
	void	insert_unformatted_disk();
	void	insert_formatted_disk();
	void	eject_disk();
	void	remove_disk();
	void	insert_again();
	void	flip_disk();
	void	save_as();
	void	toggle_wprot(bool);
};













