/*	Copyright  (c)	Günter Woigk 2016 - 2018
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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "kio/kio.h"
#include <QWidget>
#include <QBrush>
#include <QPen>
class MachineController;
class QTimer;


class ConfigDialog : public QWidget
{
public:
	QWidget* controller;

	QBrush	bg_brush;
	QPen	upper_rim_pen;
	QPen	lower_rim_pen;
	QPen	border_pen;

	uint	style;
	int		outer_padding;		// outer padding width
	int		border_width;		// border line width, may be 0
	int		inner_padding;		// inner padding width

	int		w,h;				// inner dimensions
	int		x0,y0;				// total border width = offset for origin (0,0)

// mouse state:
	int		click_dx, click_dy;	// after click, during move
	double	click_t0;			// after click
	QPoint	click_p0;			// after click

	static const int
		NoBorder		= 0<<0,
		Border1			= 1<<0,
		Border2			= 2<<0,
		Border3			= 3<<0,
		Border4			= 4<<0,
		Border5			= 5<<0,
		Border6			= 6<<0,
		Border7			= 7<<0,
		Neutral			= 0<<3,		// default
		Red				= 1<<3,
		Green			= 2<<3,
		Yellow			= 3<<3,
		Blue			= 4<<3,
		Magenta			= 5<<3,
		Cyan			= 6<<3,
		White			= 7<<3,
		Dark			= 0<<6,		// default
		Bright			= 1<<6,
		IgnoreFocusOut	= 0<<7,		// default
		CloseOnFocusOut = 1<<7,
		EatAllKeys		= 0<<8,		// default
		PropagateKeys	= 1<<8,
		DefaultStyle		= Dark+Neutral+Border2+IgnoreFocusOut+EatAllKeys;

private:
	static QColor	color_for_style(uint s, int h, int l, int n, int a=0xff);
	static QBrush	bg_brush_for_style(uint);
	static QPen		upper_rim_pen_for_style(uint);
	static QPen		lower_rim_pen_for_style(uint);
	static QPen		border_pen_for_style(uint);

public:
	ConfigDialog(QWidget* mc, int w, int h, uint style=DefaultStyle);
	ConfigDialog(QWidget* mc, QWidget* the_widget, uint style=DefaultStyle);
	virtual ~ConfigDialog();

protected:
	void	paintEvent(QPaintEvent*) override;
	void	mousePressEvent(QMouseEvent*) override;
	void	mouseMoveEvent(QMouseEvent *) override;
	void	mouseReleaseEvent(QMouseEvent*) override;
	void	focusOutEvent(QFocusEvent*) override;
	void	keyPressEvent(QKeyEvent*) override;
	void	keyReleaseEvent(QKeyEvent*) override;
};



extern void showDialog		 (QWidget*, cstr title, cstr text, uint style);
extern void showInfoDialog   (QWidget*, cstr title, cstr text);
extern void showWarningDialog(QWidget*, cstr title, cstr text);
extern void showAlertDialog  (QWidget*, cstr title, cstr text);
extern void	showAlert  (cstr msg, ...);
extern void	showWarning(cstr msg, ...);
extern void	showInfo   (cstr msg, ...);


#endif



















