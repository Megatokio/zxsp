// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MemoryGraphInspector.h"
#include "Application.h"
#include "Machine.h"
#include "MyLineEdit.h"
#include "kio/util/msbit.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QImage>
#include <QPushButton>
#include <QTimer>
#include <QToolTip>
#include <QWidget>


// offset mouse pointer hotspot -> 'feeled' hotspot
#define mouse_x_offset -1
#define mouse_y_offset -3


static const int MIN_BYTES_PER_ROW = 1; // min. bytes per row;  must be even
static const int MAX_BYTES_PER_ROW = 128;
static const int MIN_ROWS		   = 16;
static const int MAX_ROWS		   = 2000;


// ==================================================================================
// =========================   B&W Graphics Widget =================================
// ==================================================================================


class GWidget : public QWidget
{
public:
	QImage* canvas;
	int		x, y; // highlight position

protected:
	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent*);

public:
	GWidget(QWidget* parent) : QWidget(parent), canvas(nullptr) {}
	~GWidget() { delete canvas; }
};

void GWidget::resizeEvent(QResizeEvent*)
{
	if (canvas && canvas->size() == size()) return;
	delete canvas;
	canvas = new QImage(size(), QImage::Format_Mono);
	canvas->fill(Qt::black); // effektiv weiß
}

void GWidget::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	// draw the image:
	p.drawImage(0, 0, *canvas);

	// draw the byte under the mouse pointer:
	if (uint(x) < uint(width()) && uint(y) < uint(height()))
	{
		uint8 byte = canvas->scanLine(y)[x >> 3];
		int	  x0   = x & ~7;

		p.setPen(QColor(180, 0, 0)); // red
		p.drawLine(x0, y, x0 + 7, y);

		p.setPen(QColor(0, 240, 240)); // cyan
		for (int i = 0; i < 8; i++)
			if ((byte << i) & 0x80) p.drawPoint(x0 + i, y);
	}
}


// ==================================================================================
// ========================   MemoryGraph Inspector   ===============================
// ==================================================================================


MemoryGraphInspector::MemoryGraphInspector(QWidget* parent, MachineController* mc, volatile IsaObject* item) :
	MemoryInspector(parent, mc, item, MemGraph)
{
	xlogIn("new MemoryGraphInspector");

	validate_bytes_per_row();
	validate_rows();
	validate_scrollposition();

	// sub views:
	graphics_view = new GWidget(this);
	address_view  = new SimpleTerminal(this);
	rh = address_view->line_height = 16;
	cw							   = address_view->char_width;

	// resize:
	setMinimumWidth(width_for_bytes(MIN_BYTES_PER_ROW));
	setMaximumWidth(width_for_bytes(MAX_BYTES_PER_ROW));
	setMinimumHeight(height_for_rows(MIN_ROWS));
	setMaximumHeight(height_for_rows(MAX_ROWS));
	setBaseSize(minimumSize());
	setSizeIncrement(8, 1);
	xlogline(
		"new MemoryGraphInsp resized acc. to bpr+rows to %i x %i",
		width_for_bytes(bytes_per_row),
		height_for_rows(rows));
	resize(width_for_bytes(bytes_per_row), height_for_rows(rows));
	assert(width() == width_for_bytes(bytes_per_row));
	assert(height() == height_for_rows(rows));

	// toolbar
	assert(toolbar);
	toolbar->addWidget(newComboboxRegister());

	// Button "32 bytes/Zeile":
	// Buttons und Comboboxes sind vertikal schlecht ausgerichtet
	// und Buttons haben mehr Padding
	// => in eine Kiste einschneiden und ausrichten...
	QWidget* widget32 = new QWidget();
	widget32->setFixedSize(46, TOOLBAR_WIDGET_HEIGHT);
	QPushButton* buttonSet32 = new QPushButton("32", widget32);
	buttonSet32->setFixedSize(50, TOOLBAR_BUTTON_HEIGHT);
	buttonSet32->move(-2, -1);
	connect(buttonSet32, &QPushButton::clicked, this, &MemoryGraphInspector::slotSet32BytesPerRow);
	toolbar->addWidget(widget32);
}


// ==============================================================================
//			Helper
// ==============================================================================


int MemoryGraphInspector::width_for_bytes(int n)
{
	return HOR_MARGINS + HOR_SPACING + scrollbar_width + n * 8 + 5 * cw;
}

int MemoryGraphInspector::bytes_for_width(int w)
{
	w -= HOR_MARGINS + HOR_SPACING + scrollbar_width + 5 * cw; // verteilbarer Raum
	return w / 8;											   // Bytes pro Zeile
}

int MemoryGraphInspector::height_for_rows(int n) { return VERT_MARGINS + n; }

int MemoryGraphInspector::rows_for_height(int h) { return (h - VERT_MARGINS + 16 / 3) / 16 * 16; }


void MemoryGraphInspector::validate_bytes_per_row() { limit(MIN_BYTES_PER_ROW, bytes_per_row, MAX_BYTES_PER_ROW); }

void MemoryGraphInspector::validate_rows()
{
	int maxrows = (data.size + bytes_per_row - 1) / bytes_per_row;
	limit(MIN_ROWS, maxrows, MAX_ROWS);
	maxrows = (maxrows + 15) / 16 * 16;
	limit(MIN_ROWS, rows, maxrows);
}

void MemoryGraphInspector::validate_scrollposition()
{
	scroll_offset = min(scroll_offset, data.size - rows * bytes_per_row);
	scroll_offset = max(scroll_offset, 0);
}


// ==============================================================================
//			Interface
// ==============================================================================


void MemoryGraphInspector::resizeEvent(QResizeEvent* e)
{
	xlogIn("MemoryGraphInspector::resizeEvent: %i x %i", width(), height());

	MemoryInspector::resizeEvent(e);

	bytes_per_row = bytes_for_width(width()); // bytes per row
	limit(MIN_BYTES_PER_ROW, bytes_per_row, MAX_BYTES_PER_ROW);

	rows = rows_for_height(height());
	limit(MIN_ROWS, rows, MAX_ROWS);

	// set child widget sizes:
	int w1 = 5 * cw;			// Width of address field --> "$1234"
	int w2 = 8 * bytes_per_row; // Width of Graphics field
	int h1 = (rows + 15) / 16 * 16;
	int h2 = rows;
	address_view->setGeometry(LEFT_MARGIN, TOP_MARGIN, w1, h1);
	graphics_view->setGeometry(LEFT_MARGIN + HOR_SPACING + w1, TOP_MARGIN, w2, h2);

	validate_scrollposition();
	updateScrollbar();
	updateWidgets();
}


/*	we catch the showEvent to call updateWidgets
	because the child views were not resized by QT until now
	and their current contents is invalid
	and we don't want them to draw invalid contents in their first paintEvent
*/
void MemoryGraphInspector::showEvent(QShowEvent* e)
{
	xlogIn("MemoryGraphInspector::showEvent");

	MemoryInspector::showEvent(e);
	updateScrollbar();
	updateWidgets();
}


/*	callback for ToolWindow during user resize operation
	we have no fixed max width and height, instead max width and height always depend on current height and width.
	after a 1 sec timeout ToolWindow will call adjustSize so that we can set the max size for the Maximize button.
*/
void MemoryGraphInspector::adjustMaxSizeDuringResize()
{
	xxlogIn("MemoryGraphInspector.adjustMaxSizeDuringResize");

	int maxbytes = (data.size + rows - 1) / rows;
	int maxrows	 = (data.size + bytes_per_row - 1) / bytes_per_row;

	limit(MIN_BYTES_PER_ROW, maxbytes, MAX_BYTES_PER_ROW);
	limit(MIN_ROWS, maxrows, MAX_ROWS);
	maxrows = (maxrows + 15) / 16 * 16;

	setMaximumSize(
		width_for_bytes(MAX_BYTES_PER_ROW), // erlaube Maximalbreite
		max(height(),
			height_for_rows(
				maxrows))); // begrenze Höhe, aber nicht unter aktuelle Höhe, weil sonst die Fensteroberkante wandert...
}


// adjust size:
// called from toolwindow to finalize size after resizing
//
void MemoryGraphInspector::adjustSize(QSize& size)
{
	xlogIn("MemoryGraphInspector.adjustSize");

	validate_bytes_per_row();
	validate_rows();
	validate_scrollposition();

	/*	eigentlich würde ich gerne auf nächst-größeres 2^N verbreitern, aber dann kann es sein,
		dass die max. Zeilenzahl < aktuelle Zeilenzahl ist. Das schrumpelt mir dann sofort das Fenster. :-(
		Also versuchen wir die "Maximize maximiert nur vertikal"-Metapher und behalten die Breite bei.
		Damit der Benutzer einen bidir-Resize-Cursor sieht, geben wir als max. Breite ein Pixel mehr an.
	*/
	int maxbytes = (data.size + rows - 1) / rows;
	int maxrows	 = (data.size + bytes_per_row - 1) / bytes_per_row;

	limit(MIN_BYTES_PER_ROW, maxbytes, MAX_BYTES_PER_ROW); // limit for user resize
	limit(MIN_ROWS, maxrows, MAX_ROWS);					   // limit for user resize & height for Maximize
	maxrows = (maxrows + 15) / 16 * 16;

	setMaximumSize(width_for_bytes(bytes_per_row) + (bytes_per_row < MAX_BYTES_PER_ROW), height_for_rows(maxrows));

	size.setWidth(width_for_bytes(bytes_per_row));
	size.setHeight(height_for_rows(rows));
}


void MemoryGraphInspector::updateScrollbar()
{
	xlogIn("MemoryGraphInspector.updateScrollbar");

	int current_base_row   = (scroll_offset + bytes_per_row - 1) / bytes_per_row;
	int total_base_address = scroll_offset - current_base_row * bytes_per_row;
	int total_rows		   = (data.size - total_base_address + bytes_per_row - 1) / bytes_per_row;

	scrollbar->blockSignals(true);
	scrollbar->setMinimum(0);
	scrollbar->setMaximum(total_rows - rows);
	scrollbar->setPageStep(rows / rh * rh);
	scrollbar->setSingleStep(max(1, rows / rh / 16) * rh);
	scrollbar->setValue(current_base_row);
	scrollbar->blockSignals(false);
}


void MemoryGraphInspector::slotSet32BytesPerRow()
{
	xlogIn("MemoryGraphInspector::slotSet32BytesPerRow");

	bytes_per_row = 32;
	scroll_offset &= ~31;
	updateAll();
	emit signalSizeConstraintsChanged();
}


// timer: refresh displayed data
void MemoryGraphInspector::updateWidgets()
{
	if (!machine || !object || !isVisible()) return;

	assert(graphics_view->canvas);
	assert(graphics_view->canvas->width() == bytes_per_row * 8);
	assert(graphics_view->canvas->height() >= rows);

	// update parent:
	MemoryInspector::updateWidgets();

	// update address_view:
	address_view->clearScreen();
	int lastrow = min(
		address_view->height() / rh, ((data.size - scroll_offset + bytes_per_row - 1) / bytes_per_row + rh - 1) / rh);
	int address = data.baseaddress + scroll_offset;
	for (int row = 0; row < lastrow; row++)
	{
		char bu[8] = "$0000";
		sprintf(bu, address >> 16 ? "%05X" : "$%04X", address);
		address_view->printAt(row, 0, bu);
		address += bytes_per_row * rh;
	}

	// update graphics_view:
	QImage* canvas = graphics_view->canvas;
	canvas->fill(Qt::black); // effektiv weiß

	if (data_source == AsSeenByCpu)
	{
		Z80* cpu = machine->cpu;
		for (int n, r = 0, a = scroll_offset; r < rows && a < data.size; r++, a += n)
		{
			n = min(bytes_per_row, data.size - a);
			cpu->copyRamToBuffer(a, canvas->scanLine(r), n);
		}
	}
	else
	{
		const CoreByte* q = data_source == AllRom || data_source == RomPages ? &machine->rom[data.baseoffset] :
																			   &machine->ram[data.baseoffset];
		for (int n, r = 0, a = scroll_offset; r < rows && a < data.size; r++, a += n)
		{
			n = min(bytes_per_row, data.size - a);
			Z80::c2b(q + a, canvas->scanLine(r), n);
		}
	}
	graphics_view->update();

	// update tooltip:
	updateTooltip();
}


/*	Tooltip aktualisieren:
	aber nur, wenn die Maus über dem MemoryView hovert
	da QToolTip::showText(…) die App zwangsweise in den Vordergrund (zurück-) bringt.
*/
void MemoryGraphInspector::updateTooltip()
{
	xlogIn("MemoryGraphInspector::updateTooltip");

	// test whether mouse is over this inspector:
	QPoint gpos = QCursor::pos();
	if (QApplication::topLevelAt(gpos) != window())
	{
		graphics_view->x = -99;
		return;
	}

	// test whether we are over graphics_view
	// this eliminates hits in the toolbar when it is overlaying portions of the inspector by expanding the ">>" button
	QWidget* top = QApplication::widgetAt(gpos);
	if (top != graphics_view) return;

	// get the local coordinates
	QPoint pos = graphics_view->mapFromGlobal(gpos);
	uint   x = graphics_view->x = pos.x() + mouse_x_offset;
	if (x >= uint(graphics_view->width())) return;
	uint y = graphics_view->y = pos.y() + mouse_y_offset;
	if (y >= uint(graphics_view->height())) return;

	int32 offset = scroll_offset + x / 8 + y * bytes_per_row;
	if (offset >= data.size)
	{
		QToolTip::showText(gpos, nullptr, graphics_view, QRect());
		graphics_view->x = -99;
		return;
	}

	uint byte = data_source == AsSeenByCpu						 ? machine->cpu->peek(offset) :
				data_source == RomPages || data_source == AllRom ? uint8(machine->rom[data.baseoffset + offset]) :
																   uint8(machine->ram[data.baseoffset + offset]);

	QToolTip::showText(gpos, usingstr("$%04X: $%02X", data.baseaddress + offset, byte), graphics_view, QRect());
}
