/*	Copyright  (c)	Günter Woigk 2012 - 2018
                    mailto:kio@little-bat.de

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Permission to use, copy, modify, distribute, and sell this software and
    its documentation for any purpose is hereby granted without fee, provided
    that the above copyright notice appear in all copies and that both that
    copyright notice and this permission notice appear in supporting
    documentation, and that the name of the copyright holder not be used
    in advertising or publicity pertaining to distribution of the software
    without specific, written prior permission.  The copyright holder makes no
    representations about the suitability of this software for any purpose.
    It is provided "as is" without express or implied warranty.

    THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
    INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
    EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
    CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
    DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
    PERFORMANCE OF THIS SOFTWARE.
*/


#ifndef FDCPLUS3INSP_H
#define FDCPLUS3INSP_H

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

#endif











