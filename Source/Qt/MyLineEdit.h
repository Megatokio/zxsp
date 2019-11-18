#pragma once
/*	Copyright  (c)	Günter Woigk 2012 - 2019
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

#include <QFont>
#include <QLineEdit>
#include "kio/kio.h"


/*	Unterschiede zu QLineEdit:

	Bestätigte Änderungen (ENTER/RETURN/TAB) werden mit Signal returnPressed() gemeldet.
	ENTER/RETURN wird NICHT an das Parent Widget durchgereicht.
	ESCAPE bricht Eingabe ab (-> clearFokus)
	Click outside (Verlust des Fokus) bricht Eingabe ab
	Nach Abbruch der Eingabe wird der alte Text wieder angezeigt
	setText() während Fokus aktiv ist, wird gespeichert, aber nicht angezeigt.

	Achtung: setText() ist nicht virtuell, also immer mit Type MyLineEdit arbeiten!

	Default Style:
		Alignment = Qt::AlignHCenter
		Frame = 0
		Fixed Height = 16
		Font = Monaco 12pt
*/


class MyLineEdit : public QLineEdit
{
	QString oldtext;

public:
	MyLineEdit( QString, QWidget* parent=NULL );

	void	setText		(QString);
	QString	oldText		()				{ return oldtext; }

protected:
	void focusInEvent	(QFocusEvent*) override;
	void focusOutEvent	(QFocusEvent*) override;
	bool event			(QEvent*) override;
};




