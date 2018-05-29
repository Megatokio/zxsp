/*	Copyright  (c)	Günter Woigk 2013 - 2018
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


#ifndef MYSIMPLETOGGLEBUTTON_H
#define MYSIMPLETOGGLEBUTTON_H

#include <functional>
#include <QPixmap>
#include <QImage>
#include <QWidget>
#include "kio/kio.h"


class MySimpleToggleButton : public QWidget
{
	Q_OBJECT

	QImage	image_up;
	QImage	image_down;
	bool	state;					enum { up=0, down=1 };
	bool	sticky;					// sticky: radio button behaviour, else the button can be toggle down and up again

public:
	MySimpleToggleButton(QWidget* parent, int x, int y, cstr filepath_up, cstr filepath_down, bool sticky = no);
	MySimpleToggleButton(QWidget* parent, int x, int y, cstr basepath, cstr ppath_up, cstr ppath_down, bool sticky = no);

	bool	isDown()				const	{ return state==down; }
	void	setDown(bool);			// no signal
	void	click();				// emits signal if successfully toggled (except if sticky & already down)

protected:
    void	paintEvent(QPaintEvent*) override;
    void	mousePressEvent(QMouseEvent*) override;
	void	mouseReleaseEvent(QMouseEvent*) override;
	//void	mouseMoveEvent(QMouseEvent*) override;
	//void	showEvent(QShowEvent*) override;
	//void	hideEvent (QHideEvent*) override;

signals:
	void	toggled(bool);
};

#endif























