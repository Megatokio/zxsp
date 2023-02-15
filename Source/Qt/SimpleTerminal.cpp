// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "SimpleTerminal.h"
#include "kio/kio.h"
#include <QChar>
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QRect>


/*	TODO

	Cursor Blob
	Print Graphics Char

	col bei dbl_width
	row bei dbl_height

	Durch Rundung kann dbl_width ein pixel zu schmal (oder zu breit?) sein
*/


// ---------------------------------------------------------------------
// Creator

SimpleTerminal::SimpleTerminal(QWidget* p, QString fontname, int fontsize, bool antialias) : QWidget(p)
{
	xlogIn("new SimpleTerminal");

	canvas	= new QPixmap(width(), height() /*,QImage::Format_RGB32*/);
	painter = new QPainter(canvas);

	setFont(fontname, fontsize, antialias);
	reset();

	xlogline("FONT FAMILY = %s", font.family().toUtf8().data());

	xlogline("SimpleTerminal: pointsize = %i", pointsize); // 12

	xlogline("SimpleTerminal: char_width  = %i", char_width);  // 7
	xlogline("SimpleTerminal: line_height = %i", line_height); // 16
	xlogline("SimpleTerminal: ascent  = %i", ascent);		   // 12
	xlogline("SimpleTerminal: descent = %i", descent);		   // 2
	xlogline("SimpleTerminal: height  = %i", char_height);	   // 15
	xlogline("SimpleTerminal: leading = %i", leading);		   // 1

	xlogline("SimpleTerminal: width  = %i", width());  // 100
	xlogline("SimpleTerminal: height = %i", height()); // 30
}

SimpleTerminal::~SimpleTerminal()
{
	delete painter;
	delete canvas;
}


// ---------------------------------------------------------------------
// Settings:

void SimpleTerminal::reset()
{
	pushed_paper = paper = Qt::white;
	pushed_pen = pen = Qt::black;
	clear_color		 = Qt::white;
	pushed_attr		 = 0;

	attr = 0xff;
	clearAllAttributes();
	clearScreen();
}


void SimpleTerminal::set_colors() // Helper
{
	painter->setBackgroundMode(attr & OVERPRINT ? Qt::TransparentMode : Qt::OpaqueMode);

	if (attr & INVERTED)
	{
		painter->setPen(paper);
		painter->setBackground(pen);
		painter->setBrush(pen);
	}
	else
	{
		painter->setPen(pen);
		painter->setBackground(paper);
		painter->setBrush(paper);
	}
}


void SimpleTerminal::setFont(QString fontname, int size, bool antialias)
{
	xlogIn("SimpleTerminal::setFont: %s", fontname.toUtf8().data());

	font = QFont(fontname, size);
	font.setFixedPitch(true);
	font.setStyleStrategy(
		antialias ? QFont::ForceIntegerMetrics :
					(QFont::StyleStrategy)(QFont::ForceIntegerMetrics | QFont::NoAntialias));
	pointsize = font.pointSize();

	painter->setFont(font);
	QFontMetrics m = painter->fontMetrics();
	char_width	   = m.averageCharWidth();
	line_height	   = m.lineSpacing();
	ascent		   = m.ascent();
	descent		   = m.descent();
	char_height	   = m.height();
	leading		   = m.leading();

	cols = (width() + 1) / char_width;
	rows = (height() + leading) / line_height;

	attr = 0;
	set_colors();
}


void SimpleTerminal::setAttributesTo(uint a)
{
	if (a == attr) return;
	attr = a;

	font.setUnderline(a & UNDERLINE);
	font.setBold(a & BOLD);
	font.setItalic(a & ITALIC);

	if (a & DOUBLE_HEIGHT)
	{
		font.setPointSize(pointsize * 2);
		font.setStretch(a & DOUBLE_WIDTH ? QFont::Unstretched : QFont::UltraCondensed);
	}
	else
	{
		font.setPointSize(pointsize);
		font.setStretch(a & DOUBLE_WIDTH ? QFont::UltraExpanded : QFont::Unstretched);
		// logline("XXX average char width = %i",painter->fontMetrics().averageCharWidth());
	}
	painter->setFont(font);

	set_colors();
}


void SimpleTerminal::setPaperColor(QColor c)
{
	paper = c;
	set_colors();
}


void SimpleTerminal::setPenColor(QColor c)
{
	pen = c;
	set_colors();
}


void SimpleTerminal::pushCursor()
{
	pushed_row	 = row;
	pushed_col	 = col;
	pushed_attr	 = attr;
	pushed_paper = paper;
	pushed_pen	 = pen;
	pushed_blob	 = blob;
}


void SimpleTerminal::popCursor()
{
	row	  = pushed_row;
	col	  = pushed_col;
	attr  = pushed_attr;
	paper = pushed_paper;
	pen	  = pushed_pen;
	blob  = pushed_blob;

	setAttributesTo(attr);
}


// ---------------------------------------------------------------------
// Move Cursor:


/*	Zwinge 'col' in den Bereich [0 ... cols]
 */
void SimpleTerminal::handle_h_overflow()
{
	while (col < 0)
	{
		col += cols;
		row -= 1;
	}
	while (col > cols)
	{
		col -= cols;
		row += 1;
	}
}


/*	Zwinge 'row' in den Bereich [0 ... rows-1]
 */
void SimpleTerminal::handle_v_overflow()
{
	if (row >= rows)
		scrollScreen(row - rows + 1);
	else if (row < 0)
		scrollScreen(row);
}


/*	Remove the cursor blob; if visible
	Zwinge 'col' in den Bereich [0 ... cols-1]
	und 'row' in den Bereich [0 ... rows-1]
	so dass mindestens ein Zeichen oder der Blob gedruckt werden können
*/
void SimpleTerminal::hideBlob()
{
	if (blob)
	{
		// TODO
		blob = off;
	}
	else
	{
		if (uint16(col) > uint16(cols)) handle_h_overflow();
		if (col == cols)
		{
			col = 0;
			row++;
		}
		if (uint16(row) >= uint16(rows)) handle_v_overflow();
	}
}


/*	Show cursor blob
	Zwinge 'col' in den Bereich [0 ... cols-1]
	und 'row' in den Bereich [0 ... rows-1]
	so dass mindestens ein Zeichen oder der Blob gedruckt werden können
*/
void SimpleTerminal::showBlob()
{
	if (!blob)
	{
		if (uint16(col) > uint16(cols)) handle_h_overflow();
		if (col == cols)
		{
			col = 0;
			row++;
		}
		if (uint16(row) >= uint16(rows)) handle_v_overflow();
		// TODO();
		blob = on;
	}
}


void SimpleTerminal::moveTo(int row, int col)
{
	if (blob) hideBlob();
	this->row = (int16)row;
	this->col = (int16)col;
}

void SimpleTerminal::moveToColumn(int col)
{
	if (blob) hideBlob();
	this->col = (int16)col;
}

void SimpleTerminal::cursorLeft(int n)
{
	if (blob) hideBlob();
	col -= (int16)n;
}

void SimpleTerminal::cursorRight(int n)
{
	if (blob) hideBlob();
	col += (int16)n;
}

void SimpleTerminal::cursorUp(int n)
{
	if (blob) hideBlob();
	row -= (int16)n;
}

void SimpleTerminal::cursorDown(int n)
{
	if (blob) hideBlob();
	row += (int16)n;
}

void SimpleTerminal::cursorReturn()
{
	if (blob) hideBlob();
	col = 0;
}

void SimpleTerminal::cursorTab(int n)
{
	if (blob) hideBlob();
	col = ((int16)col / 8 + n) * 8;
}


// ----------------------------------------------------------
// Clear, scroll, copy:

void SimpleTerminal::clearScreen()
{
	canvas->fill(clear_color);
	row = col	= 0;
	pushed_blob = blob = off;
	update();
}


void SimpleTerminal::clearRect(int rows, int cols)
{
	if (blob) hideBlob();

	rows = min(rows, this->rows - row);
	cols = min(cols, this->cols - col);

	if (rows > 0 && cols > 0)
	{
		painter->fillRect(col * char_width, row * line_height, cols * char_width, rows * line_height, clear_color);
		update(col * char_width, row * line_height, cols * char_width, rows * line_height);
	}
}

void SimpleTerminal::clearToEndOfLine()
{
	if (blob) hideBlob();

	if (uint(row) < uint(rows) && uint(col) < uint(cols))
	{
		painter->fillRect(col * char_width, row * line_height, (cols - col) * char_width, line_height, clear_color);
		update(col * char_width, row * line_height, (cols - col) * char_width, line_height);
	}
}


/*	Scroll screen up (n>0) or down (n<0)
	takes care for cursor position and blob
*/
void SimpleTerminal::scrollScreen(int n)
{
	if (n > 0) // up
	{
		n		  = min(n, rows);
		QPixmap z = canvas->copy(0, n * line_height, width(), (rows - n) * line_height);
		painter->drawPixmap(QPoint(0, 0), z);
		painter->fillRect(0, (rows - n) * line_height, width(), n * line_height, clear_color);
		row -= n;
		if (row < 0) blob = off;
		pushed_row -= n;
		if (pushed_row < 0) pushed_blob = off;
		update();
	}
	else if (n < 0) // down
	{
		n		  = min(-n, rows);
		QPixmap z = canvas->copy(0, 0, width(), (rows - n) * line_height);
		painter->drawPixmap(QPoint(0, n * line_height), z);
		painter->fillRect(0, 0, width(), n * line_height, clear_color);
		row += n;
		if (row >= rows) blob = off;
		pushed_row += n;
		if (pushed_row >= rows) pushed_blob = off;
		update();
	}
}


/*	copy r with size rows * cols from current cursor position to destination position z
 */
void SimpleTerminal::copyRect(int z_row, int z_col, int z_rows, int z_cols)
{
	if (blob) hideBlob();

	z_rows = min(z_rows, rows - row);
	z_rows = min(z_rows, rows - z_row);
	z_cols = min(z_cols, cols - col);
	z_cols = min(z_cols, cols - z_col);

	if (z_rows > 0 && z_cols > 0)
	{
		QPixmap z = canvas->copy(col * char_width, row * line_height, z_cols * char_width, z_rows * line_height);
		painter->drawPixmap(QPoint(z_col * char_width, z_row * line_height), z);
		update(z_col * char_width, z_row * line_height, z_cols * char_width, z_rows * line_height);
	}
}


// ----------------------------------------------------------
// Print:

void SimpleTerminal::draw_text(QString s)
{
	int x = col * char_width;
	int y = row * line_height;
	int n = s.length();

	painter->drawText(x, y + ascent, s);

	if (leading && (~attr & OVERPRINT))
	{
		painter->setPen(Qt::NoPen);
		// painter->setBrush(paper);
		painter->drawRect(x, y + char_height, n * char_width, leading);
		painter->setPen(attr & INVERTED ? paper : pen);
	}

	update(x, y, n * char_width, line_height);

	col += n;
}

void SimpleTerminal::print(QChar c, int n)
{
	hideBlob();	  // and force inside
	n = (int16)n; // security

	if (n == 1)
		draw_text(QString(c));
	else if (n > 1)
		print(QString(n, c));
}

void SimpleTerminal::print(QString s, int n)
{
	hideBlob();	  // and force inside
	n = (int16)n; // security

	if (n == 1)
	{
		while (s.length() > cols - col)
		{
			n = cols - col;
			draw_text(s.left(n));
			s	= s.mid(n);
			col = 0;
			row++;
			hideBlob();
		}

		draw_text(s);
	}
	else if (n > 1)
	{
		if (s.length() < cols)
		{
			print(s + s, n / 2);
			if (n & 1) print(s, 1);
		}
		else
			while (n) print(s, n--);
	}
}


void SimpleTerminal::printAt(int row, int col, QString s, int n)
{
	moveTo(row, col);
	print(s, n);
}

void SimpleTerminal::printAt(int row, int col, QChar c, int n)
{
	moveTo(row, col);
	print(c, n);
}


void SimpleTerminal::printGraphicsCharacter(uint16 bmp[], int n)
{
	TODO();
	(void)bmp;
	(void)n;
}


void SimpleTerminal::paintEvent(QPaintEvent*) // virtual protected
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, *canvas);

	if (hasFocus())
	{
		QPen pen(QColor(0x88, 0xff, 0x00, 0x88));
		pen.setWidth(6);
		painter.setPen(pen);
		painter.setCompositionMode(QPainter::CompositionMode_Darken);
		painter.drawRect(rect());
	}
}


void SimpleTerminal::focusInEvent(QFocusEvent* e)
{
	QWidget::focusInEvent(e);
	emit focusChanged(1);
}

void SimpleTerminal::focusOutEvent(QFocusEvent* e)
{
	QWidget::focusOutEvent(e);
	emit focusChanged(0);
}


void SimpleTerminal::resizeEvent(QResizeEvent*) // virtual protected
{
	xlogIn("SimpleTerminal::resizeEvent(w=%i,h=%i)", width(), height());
	resize_canvas(width(), height());
}


// replacement for QWidget::resize()
// because QWidget::resize() may call resizeEvent() on a queued connection
// by when the caller may already have printed into it (and lost…)
//
void SimpleTerminal::resize(int w, int h)
{
	xlogIn("SimpleTerminal::resize(w=%i,h=%i)", w, h);
	QWidget::resize(w, h);
	resize_canvas(w, h);
}

void SimpleTerminal::resize(const QSize& sz)
{
	xlogIn("SimpleTerminal::resize(w=%i,h=%i)", sz.width(), sz.height());
	QWidget::resize(sz);
	resize_canvas(sz.width(), sz.height());
}

// resize (grow) canvas
//
void SimpleTerminal::resize_canvas(int w, int h)
{
	rows = h / line_height;
	cols = w / char_width;

	if (canvas->width() < w || canvas->height() < h)
	{
		w		  = max(canvas->width(), w);
		h		  = max(canvas->height(), h);
		QPixmap p = canvas->copy();
		painter->end();
		delete canvas;
		canvas = new QPixmap(w + char_width, h + line_height /*,QImage::Format_RGB32*/);
		painter->begin(canvas);
		canvas->fill(clear_color);
		painter->drawPixmap(0, 0, p);
		painter->setFont(font); // gets reset by begin()
		set_colors();			// get reset by begin()
	}
}
