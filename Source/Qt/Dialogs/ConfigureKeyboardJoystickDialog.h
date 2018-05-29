/*	Copyright  (c)	Günter Woigk 2016 - 2018
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

#ifndef CONFIGUREKEYBOARDJOYSTICKDIALOG_H
#define CONFIGUREKEYBOARDJOYSTICKDIALOG_H

#include "kio/kio.h"
#include "ConfigDialog.h"
class MachineController;
class QLineEdit;
class QLabel;
class QPushButton;
class QRadioButton;
class KbdLed;


extern void addKeyCap(uint8 key, cstr cap);
extern cstr getKeyCap(uint8 key);

extern cstr addKeyJoyFnmatchPattern(uint8 keys[5], cstr fnpattern);
extern cstr getKeyJoyFnmatchPattern(uint8 keys[5], cstr filename);


class ConfigureKeyboardJoystickDialog : public ConfigDialog
{
public:
	QLineEdit*	led_filenamepattern;
	QRadioButton* btn_default_for_all_files;
	QRadioButton* btn_for_match_pattern;
	QRadioButton* btn_use_just_now;
	KbdLed*	led_up;
	KbdLed*	led_down;
	KbdLed*	led_left;
	KbdLed*	led_right;
	KbdLed*	led_fire;
	QPushButton* btn_cancel;
	QPushButton* btn_ok;

	int		xm,ym;				// center position for joystick graphics

	uint8	old_keys[5];		// (RLDUF) Qt keycode to use for keyboard joystick
	cstr	old_matchpattern;	// the filename pattern, for which the keys were set
								// note: RLDUF array indexes == %000FUDLR bit numbers
	uint8	new_keys[5];		// (RLDUF) Qt keycode to use for keyboard joystick

public:
	explicit ConfigureKeyboardJoystickDialog(MachineController*);
	~ConfigureKeyboardJoystickDialog();

protected:
	void	paintEvent(QPaintEvent*);
};


#endif

