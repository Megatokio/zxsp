/*	Copyright  (c)	Günter Woigk 2014 - 2018
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


#ifndef DIVIDEINSPECTOR_H
#define DIVIDEINSPECTOR_H

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

#endif
