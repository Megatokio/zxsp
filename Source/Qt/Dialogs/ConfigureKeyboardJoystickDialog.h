#pragma once
/*	Copyright  (c)	Günter Woigk 2016 - 2019
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




