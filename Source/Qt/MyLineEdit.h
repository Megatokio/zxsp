#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include <QFont>
#include <QLineEdit>


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
	MyLineEdit(QString, QWidget* parent = nullptr);

	void	setText(QString);
	QString oldText() { return oldtext; }

protected:
	void focusInEvent(QFocusEvent*) override;
	void focusOutEvent(QFocusEvent*) override;
	bool event(QEvent*) override;
};
