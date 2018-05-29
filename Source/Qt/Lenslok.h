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


#ifndef LENSLOK_H
#define LENSLOK_H

#include <QWidget>
#include "MachineController.h"
#include <QMenu>
#include <QAction>
#include <QTimer>

class Lenslok : public QWidget
{
	MachineController*	controller;

	QPixmap background_a;
	QPixmap background_b;
	QPixmap*background;
    QMenu*	contextmenu;
    QAction* actions[10];
    QTimer*	timer;
	uint	game_id;
	bool	ignore_focusout;

// state:
	int		click_dx,click_dy;	// after click, during move
	double	click_t0;			// after click
	QPoint	click_p0;			// after click

public:
	Lenslok(MachineController*, cstr name1, cstr name2);
	~Lenslok();

protected:
	void	paintEvent(QPaintEvent*) override;
	void	mousePressEvent(QMouseEvent*) override;
	void	mouseMoveEvent(QMouseEvent*) override;
	void	mouseReleaseEvent(QMouseEvent*) override;
	bool	event(QEvent*) override;
	void	focusOutEvent(QFocusEvent*) override;
	void	keyPressEvent(QKeyEvent*) override;
	void	keyReleaseEvent(QKeyEvent*) override;
	void	contextMenuEvent(QContextMenuEvent*) override;
	//void	enterEvent(QEvent*) override;
	//void	leaveEvent(QEvent*) override;
	void	moveEvent(QMoveEvent*) override;

private:
	void	draw_prism(QPainter&, QRectF, const QRectF&);
	void	select_game();
};

#endif
