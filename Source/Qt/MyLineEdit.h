/*	Copyright  (c)	Günter Woigk 2012 - 2018
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


#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

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



#endif // MYLINEEDIT_H
