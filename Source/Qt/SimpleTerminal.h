﻿/*	Copyright  (c)	Günter Woigk 2012 - 2018
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


#ifndef SIMPLETERMINAL_H
#define SIMPLETERMINAL_H

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QFont>
#include <QFontMetrics>
#include "kio/kio.h"
#include <QSemaphore>
#include <QBrush>
#include <QPainter>
#include <QFontMetrics>

enum
{
	BOLD				= 1,
	UNDERLINE			= 2,
	INVERTED			= 4,
	ITALIC				= 8,
	OVERPRINT			= 16,
	DOUBLE_WIDTH		= 32,
	DOUBLE_HEIGHT		= 64,
	GRAPHICS_CHARACTERS	= 128		// NIMP
};


class SimpleTerminal : public QWidget
{
	Q_OBJECT

public:
	int 			rows, cols;		// size of terminal in character rows and columns
	int 			char_width,		// width and ..
					line_height;	// .. height of character cell
	int				pointsize;		// nominal font size [points]
	int				ascent,descent,char_height,leading;	// char_height = ascent+descent+1; line_height = char_height+leading
	QFont			font;			// font to use
	QColor			clear_color;	// color used to clear background in scroll and clear commands
	QPainter*		painter;		// painter in use
	QPixmap*		canvas;

	QColor			paper,	pushed_paper;	// paper color when printing in opaque mode
	QColor			pen,	pushed_pen;		// pen color for printing
	uint			attr,	pushed_attr;	// print style
	int 			row,	pushed_row;		// current print position
	int 			col,	pushed_col;		// ""
	bool			blob,	pushed_blob;	// cursor blob visible?

	void			handle_h_overflow();
	void			handle_v_overflow();
	void			draw_text(QString);
	void			set_colors();

					SimpleTerminal	(QWidget*parent, QString fontname="Menlo", int fontsize=12, bool antialias=yes);
					~SimpleTerminal	();

	void	resize(int w, int h);
	void	resize(const QSize &sz);

	void reset			();
	void setAttributesTo(uint);
	void setAttributes	(uint a)			{ setAttributesTo(attr|a); }
	void clearAttributes(uint a)			{ setAttributesTo(attr&~a); }
	void clearAllAttributes()				{ setAttributesTo(0); }
	void setFont		(QString name, int pointsize, bool antialias=yes);
	void setPaperColor	(QColor);
	void setPenColor	(QColor);

	void print(QString, int=1);
	void print(QChar,   int=1);

	void printAt(int row, int col, QString, int=1);
	void printAt(int row, int col, QChar, int=1);

	void clearScreen	();
	void moveTo			(int row, int col);
	void moveToColumn	(int col);
	void pushCursor		();
	void popCursor		();
	void showBlob		();
	void hideBlob		();
	void cursorLeft		(int=1);
	void cursorTab		(int=1);
	void cursorDown		(int=1);
	void cursorUp		(int=1);
	void cursorRight	(int=1);
	void cursorReturn	();
	void clearToEndOfLine ();
	void printGraphicsCharacter (uint16[],int=1);
	void scrollScreen	(int=1);
	void clearRect		(int rows, int cols);
	void copyRect		(int dest_row, int dest_col, int rows, int cols);

private:
	void	resize_canvas(int w, int h);

protected:
	void paintEvent		(QPaintEvent*);		// virtual protected
	void resizeEvent	(QResizeEvent*);	// virtual protected
	void focusInEvent	(QFocusEvent*);		// [virtual protected]
	void focusOutEvent	(QFocusEvent*);		// [virtual protected]

signals:
	void	focusChanged(bool);
public slots:
};


#endif // SIMPLETERMINAL_H





























