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


class CurrahMicroSpeechInsp : public Inspector
{
	class QRadioButton* button_8bit;
	class QRadioButton* button_hifi;

	// scroll text:
	uint8	scroller[64];	// circular buffer
	uint	rp;				// index of first allophone in scroller[] to print
	uint	wp;				// number of allophones in scroller[]
	int		xpos;			// xpos of first allophone in scroller
	int		width;			// total width [pixels] of scroller

public:
	CurrahMicroSpeechInsp( QWidget*, MachineController*, volatile IsaObject* );
	~CurrahMicroSpeechInsp();

protected:
	void	paintEvent(QPaintEvent*) override;	// Qt
	void	updateWidgets() override;			// Timer

private:
	void    set_8bit();
	void    set_hifi();
};























