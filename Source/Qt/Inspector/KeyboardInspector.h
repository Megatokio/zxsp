/*	Copyright  (c)	Günter Woigk 2000 - 2019
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

#ifndef KBDINSP_H
#define KBDINSP_H

#include "Inspector.h"
#include "Keyboard.h"
#include <QRegion>


class KeyboardInspector : public Inspector
{
	Model		model;
	uint8		mousekey;
	Keymap		keymap;				// currently displayed keyboard state

public:
	KeyboardInspector( QWidget*, MachineController*, volatile IsaObject* );

private:
virtual QRect	keyRect(uint8);
	QRegion		keyRegion(uint8);
	uint8		findKeyForPoint(QPoint);

protected:
	void		paintEvent(QPaintEvent*) override;
	void		mouseDoubleClickEvent(QMouseEvent* e) override	{ mousePressEvent(e); }
	void		mousePressEvent(QMouseEvent*) override;
	void		mouseReleaseEvent(QMouseEvent*) override;
	void		mouseMoveEvent(QMouseEvent*) override;
	//void		showEvent(QShowEvent*) override;
	bool		event(QEvent*) override;

	void		fillContextMenu(QMenu*) override;
	void		updateWidgets() override;
};


class Tk90xKbdInsp : public KeyboardInspector
{
	QRect keyRect(uint8) override;

public: Tk90xKbdInsp( QWidget*p, MachineController*m, volatile IsaObject*i )	:KeyboardInspector(p,m,i){}
};


class Ts1000KbdInsp : public KeyboardInspector
{
	QRect keyRect(uint8) override;

public: Ts1000KbdInsp( QWidget*p, MachineController*m, volatile IsaObject*i )	:KeyboardInspector(p,m,i){}
};

class Ts1500KbdInsp : public KeyboardInspector
{
	QRect keyRect(uint8) override;

public: Ts1500KbdInsp( QWidget*p, MachineController*m, volatile IsaObject*i )	:KeyboardInspector(p,m,i){}
};

class Tk85KbdInsp : public KeyboardInspector
{
	QRect keyRect(uint8) override;

public: Tk85KbdInsp( QWidget*p, MachineController*m, volatile IsaObject*i )	:KeyboardInspector(p,m,i){}
};

class Tk95KbdInsp : public KeyboardInspector
{
	QRect keyRect(uint8) override;

public:	Tk95KbdInsp( QWidget*p, MachineController*m, volatile IsaObject*i )	:KeyboardInspector(p,m,i){}
};

#endif


























