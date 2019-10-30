/*	Copyright  (c)	Günter Woigk 2013 - 2018
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













