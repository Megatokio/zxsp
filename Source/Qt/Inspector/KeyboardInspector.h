/*	Copyright  (c)	Günter Woigk 2000 - 2018
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


























