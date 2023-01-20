#pragma once
/*	Copyright  (c)	Günter Woigk 2015 - 2019
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
class QRect;
class QMouseEvent;
class QLabel;


class MultifaceInsp : public Inspector
{
protected:
	QRect		buttonbox;
	QLabel*		label_nmi_pending;
	QLabel*		label_paged_in;

	static const int l_x=4,l_y=3,l_d=14;

public:
	MultifaceInsp(QWidget*, MachineController*, volatile IsaObject *, cstr image, const QRect& redbuttonbox);

	void	pressRedButton();

protected:
	void	mousePressEvent(QMouseEvent*) override;
	void	updateWidgets() override;
};














