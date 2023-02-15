// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MyLineEdit.h"
#include "kio/kio.h"
#include <QEvent>
#include <QFont>
#include <QKeyEvent>


QFont monaco12("Monaco", 12);


MyLineEdit::MyLineEdit(QString s, QWidget* parent) : QLineEdit(s, parent)
{
	setAlignment(Qt::AlignHCenter);
	setFrame(0);
	setFixedHeight(16);
	setFont(monaco12);
}

void MyLineEdit::focusInEvent(QFocusEvent* event)
{
	oldtext = text();
	QLineEdit::focusInEvent(event);
}

void MyLineEdit::focusOutEvent(QFocusEvent* event)
{
	setText(oldtext);
	QLineEdit::focusOutEvent(event);
}

bool MyLineEdit::event(QEvent* e)
{
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = (QKeyEvent*)e;
		if (ke->key() == Qt::Key_Enter)
		{
			emit returnPressed();
			return 1;
		}
		if (ke->key() == Qt::Key_Return)
		{
			emit returnPressed();
			return 1;
		}
		if (ke->key() == Qt::Key_Escape)
		{
			clearFocus();
			return 1;
		}
		if (ke->key() == Qt::Key_Tab) { emit returnPressed(); }
	}
	return QLineEdit::event(e);
}

void MyLineEdit::setText(QString s)
{
	oldtext = s;
	if (!hasFocus()) QLineEdit::setText(s);
}
