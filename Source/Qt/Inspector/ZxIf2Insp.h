#pragma once
/*	Copyright  (c)	Günter Woigk 2009 - 2019
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

#include "SinclairJoyInsp.h"
class QMouseEvent;
class QLabel;
class QMenu;


class ZxIf2Insp : public SinclairJoyInsp
{
	QPushButton*	button_insert_eject;
	QLabel*         label_romfilename;
	cstr			old_romfilepath;		// 2nd

public:
	ZxIf2Insp( QWidget*, MachineController*, volatile IsaObject* );

	void	insertRom(cstr filepath);

protected:
	void	fillContextMenu(QMenu* menu) override;
	void	updateWidgets() override;

private:
	void    insert_or_eject_rom();
};


