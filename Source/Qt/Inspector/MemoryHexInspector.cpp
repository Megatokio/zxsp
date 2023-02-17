// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MemoryHexInspector.h"
#include "Application.h"
#include "Machine.h"
#include "MemoryInspector.h"
#include "MyLineEdit.h"
#include "Qt/Settings.h"
#include "SimpleTerminal.h"
#include "Uni/util.h"
#include "Z80/Z80.h"
#include <QCheckBox>
#include <QComboBox>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QRect>
#include <QScrollBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QWidget>


/*	The "MemoryBytesInspector"

	This widget displays 3 TextEdit fields (SimpleTerminal fields)
	for	Address
		Hex Bytes
		Ascii Characters
	and a vertical ScrollBar.

	The display data can come from the Machine's Rom pages, Ram pages or from the CPU 'as seen'.
	'data_start' and 'data_size' define the displayable range of data for this widget.
	Within this range the display can be scrolled using the scrollbar.
	The current scroll position is reflected in 'display_base_address', which can be negative up to -(bytes_per_row-1).
	The currently displayed data is backed in 'current_data',
	which is valid between rows 'first_valid_row' and 'last_valid_row' (excl.).

	By resizing the widget the amount of displayed rows and bytes_per_row can be varied.
	While resizing the widget the 'display_base_address' is kept const.
*/


static const int MIN_BYTES_PER_ROW = 8; // min. bytes per row;  must be even
static const int MAX_BYTES_PER_ROW = 64;
static const int MIN_ROWS		   = 2;
static const int MAX_ROWS		   = 100;


// offset mouse pointer hotspot -> 'feeled' hotspot
#define mouse_x_offset -2
#define mouse_y_offset -2


// ==============================================================================
//			Constructor & Destructor
// ==============================================================================


MemoryHexInspector::MemoryHexInspector(QWidget* parent, MachineController* mc, volatile IsaObject* item) :
	MemoryInspector(parent, mc, item, Bytes), address_view(nullptr), hex_view(nullptr), ascii_view(nullptr),
	checkbox_words(nullptr), show_words(no), widget_edit_mode(nullptr), button_breakpoint_r(nullptr),
	button_breakpoint_w(nullptr), button_breakpoint_x(nullptr), ascii_edit_offset(0), hex_edit_offset(0),
	hex_edit_nibble(0), edit_flashphase(0), edit_flashtime(0), cw(0), rh(0), pc(-1), bc(-1), de(-1), hl(-1), ix(-1),
	iy(-1), sp(-1), edit_mode(EDITMODE_VIEW), breakpoint_mask(0), first_valid_row(0), last_valid_row(0)
{
	xlogIn("new MemoryHexInspector");

	show_words = settings.get_bool(key_memoryview_hex_is_words, no);
	validate_bytes_per_row();
	validate_rows();
	validate_scrollposition();

	// child views:
	address_view = new SimpleTerminal(this);
	hex_view	 = new SimpleTerminal(this);
	ascii_view	 = new SimpleTerminal(this);

	cw = hex_view->char_width;	// Character width
	rh = hex_view->line_height; // Row height

	connect(hex_view, &SimpleTerminal::focusChanged, this, &MemoryHexInspector::slotFocusChanged);
	connect(ascii_view, &SimpleTerminal::focusChanged, this, &MemoryHexInspector::slotFocusChanged);

	// resize:
	setMinimumWidth(width_for_bytes(MIN_BYTES_PER_ROW));
	setMaximumWidth(width_for_bytes(MAX_BYTES_PER_ROW));
	setMinimumHeight(height_for_rows(MIN_ROWS));
	setMaximumHeight(height_for_rows(MAX_ROWS));
	setBaseSize(minimumSize());
	setSizeIncrement(show_words ? cw * 7 : cw * 4, rh);
	resize(width_for_bytes(bytes_per_row), height_for_rows(rows));

	// toolbar:
	checkbox_words = new QCheckBox("Words");
	checkbox_words->setMinimumWidth(20);
	checkbox_words->setMaximumWidth(60);
	checkbox_words->setChecked(show_words);
	connect(checkbox_words, &QCheckBox::clicked, this, &MemoryHexInspector::slotSetWordMode);

	const int W		 = 20;
	const int H		 = 18;
	widget_edit_mode = new QWidget();
	widget_edit_mode->setFixedSize(3 * W, 2 * H);

	button_edit_mode = new QPushButton(widget_edit_mode);
	connect(button_edit_mode, &QPushButton::toggled, this, &MemoryHexInspector::slotSetEditMode);
	button_edit_mode->setFixedSize(3 * W, H);
	button_edit_mode->setCheckable(1);
	button_edit_mode->setText("Edit");

	button_breakpoint_r = new QPushButton(widget_edit_mode);
	connect(button_breakpoint_r, &QPushButton::toggled, this, &MemoryHexInspector::slotSetBreakpointR);
	button_breakpoint_r->move(0, H);
	button_breakpoint_r->setFixedSize(W, H);
	button_breakpoint_r->setCheckable(1);
	button_breakpoint_r->setText("R");
	button_breakpoint_r->setStyleSheet("color: rgb(0,0,255)"); // blue

	button_breakpoint_w = new QPushButton(widget_edit_mode);
	connect(button_breakpoint_w, &QPushButton::toggled, this, &MemoryHexInspector::slotSetBreakpointW);
	button_breakpoint_w->move(W, H);
	button_breakpoint_w->setFixedSize(W, H);
	button_breakpoint_w->setCheckable(1);
	button_breakpoint_w->setText("W");
	button_breakpoint_w->setStyleSheet("color: rgb(0,180,0)"); // green

	button_breakpoint_x = new QPushButton(widget_edit_mode);
	connect(button_breakpoint_x, &QPushButton::toggled, this, &MemoryHexInspector::slotSetBreakpointX);
	button_breakpoint_x->move(2 * W, H);
	button_breakpoint_x->setFixedSize(W, H);
	button_breakpoint_x->setCheckable(1);
	button_breakpoint_x->setText("X");
	button_breakpoint_x->setStyleSheet("color: rgb(200,0,0)"); // red

	// Buttons "16" und "32 Bytes/Zeile":
	// Buttons und Comboboxes sind vertikal schlecht ausgerichtet
	// und Buttons haben mehr Padding
	// => in eine Kiste einschneiden und ausrichten...

	QWidget* widget16 = new QWidget();
	widget16->setFixedSize(44, TOOLBAR_WIDGET_HEIGHT);
	QPushButton* buttonSet16 = new QPushButton("16", widget16);
	buttonSet16->setFixedSize(50, TOOLBAR_BUTTON_HEIGHT);
	buttonSet16->move(-2, -1);
	connect(buttonSet16, &QPushButton::clicked, this, &MemoryHexInspector::slotSet16BytesPerRow);

	QWidget* widget32 = new QWidget();
	widget32->setFixedSize(46, TOOLBAR_WIDGET_HEIGHT);
	QPushButton* buttonSet32 = new QPushButton("32", widget32);
	buttonSet32->setFixedSize(50, TOOLBAR_BUTTON_HEIGHT);
	buttonSet32->move(-2, -1);
	connect(buttonSet32, &QPushButton::clicked, this, &MemoryHexInspector::slotSet32BytesPerRow);

	toolbar->addWidget(newComboboxRegister());
	toolbar->addWidget(checkbox_words);
	toolbar->addWidget(widget_edit_mode);
	toolbar->addWidget(widget16);
	toolbar->addWidget(widget32);
}

MemoryHexInspector::~MemoryHexInspector()
{
	xlogIn("~MemoryHexInspector");
	save_settings();
}

void MemoryHexInspector::saveSettings()
{
	MemoryInspector::saveSettings();
	save_settings();
}

void MemoryHexInspector::save_settings() { settings.setValue(key_memoryview_hex_is_words, show_words); }


// ==============================================================================
//			Helper
// ==============================================================================


int MemoryHexInspector::width_for_bytes(int n)
{
	return HOR_MARGINS + 2 * HOR_SPACING + scrollbar_width + (show_words ? 5 + n / 2 * 7 - 1 : 5 + 4 * n - 1) * cw;
}

int MemoryHexInspector::height_for_rows(int n) { return VERT_MARGINS + n * rh; }

int MemoryHexInspector::rows_for_height(int h) { return (h - VERT_MARGINS + rh / 3) / rh; }

int MemoryHexInspector::bytes_for_width(int w)
{
	w -= HOR_MARGINS + 2 * HOR_SPACING + scrollbar_width + (5 - 1) * cw; // verteilbarer Raum
	return show_words ? w / cw / 7 * 2 : w / cw / 4;					 // Bytes pro Zeile
}

void MemoryHexInspector::validate_rows()
{
	int maxrows = (data.size + bytes_per_row - 1) / bytes_per_row;
	rows		= min(rows, maxrows);
	limit(MIN_ROWS, rows, MAX_ROWS);
}

void MemoryHexInspector::validate_bytes_per_row() { limit(MIN_BYTES_PER_ROW, bytes_per_row, MAX_BYTES_PER_ROW); }

void MemoryHexInspector::validate_scrollposition()
{
	int32 max = data.size - rows * bytes_per_row + bytes_per_row - 1;
	while (scroll_offset > max)
	{
		scroll_offset -= bytes_per_row;
		updateAll();
	}
	if (scroll_offset < 0)
	{
		scroll_offset = 0;
		updateAll();
	}
}

inline QColor pen_color_for_corebyte(CoreByte cb)
{
	return ~cb & cpu_break_rwx // not all bits set?
			   ?
			   QColor(cb & cpu_break_x ? 180 : 0, cb & cpu_break_w ? 160 : 0, cb & cpu_break_r ? 220 : 0) :
			   QColor(Qt::lightGray); // all bits set: light grey, not white for readability
}

/*	Print a single byte at row, col
	into the hex and ascii view
	Used to update data in the memory view
*/
void MemoryHexInspector::print_byte(int row, int col, CoreByte cb)
{
	if (cb & cpu_break_rwx)
	{
		hex_view->setPenColor(pen_color_for_corebyte(cb));
		hex_view->setAttributes(BOLD);
	}
	hex_view->printAt(row, show_words ? col & 1 ? (col - 1) / 2 * 5 : col / 2 * 5 + 2 : col * 3, hexstr(cb, 2));
	if (cb & cpu_break_rwx)
	{
		hex_view->setPenColor(Qt::black);
		hex_view->clearAttributes(BOLD);
	}

	if (cb & 0x80) ascii_view->setAttributes(INVERTED);
	ascii_view->printAt(row, col, printablechar(cb & 0x7F));
	if (cb & 0x80) ascii_view->clearAttributes(INVERTED);
}

/*	Print rows [a..[e to textedit widgets.
	Used to print data after clear or scroll screen.
	Prints:
		address into textedit_address,
		hex codes into textedit_hex and
		ascii characters into textedit_ascii.

	row_a	first row
	row_e	last row +1
	address	address to print for row_a
	bytes	pointer to first data byte
	cnt		data byte count

	if cnt < bytes_per_row * (row_e-row_a)
			-> the last line ends with some spaces
*/
void MemoryHexInspector::print_rows(int row_a, int row_e, uint address, const volatile CoreByte* bytes, int cnt)
{
	assert(row_a <= row_e);
	assert(row_a >= 0);
	assert(row_e <= rows);
	assert(show_words ? (bytes_per_row & 1) == 0 : true);
	//	assert(cnt >= (row_e-row_a-1)*bytes_per_row+1);
	if (cnt < (row_e - row_a - 1) * bytes_per_row + 1) // kann beim Umschalten auf kleinere Page passieren
		logline("MemoryHexInspector::print_rows: cnt < (row_e-row_a-1)*bytes_per_row+1"); // die implementierung hier
																						  // ist aber robust dagegen

	str as	   = spacestr(bytes_per_row); // ascii string
	int hs_len = show_words ? bytes_per_row / 2 * 5 - 1 : bytes_per_row * 3 - 1;
	str hs	   = spacestr(hs_len); // hex string
	int n	   = bytes_per_row;	   // bytes to print per row

	while (row_a < row_e && cnt > 0)
	{
		if (n > cnt)
		{
			memset(as, ' ', n);
			memset(hs, ' ', hs_len);
			n = cnt;
		}

		// print address:
		char bu[8] = "$0000";
		sprintf(bu, address >> 16 ? "%05X" : "$%04X", address);
		address_view->printAt(row_a, 0, bu);

		// print hex:
		for (int i = 0; i < n; i++)
		{
			int x	  = show_words ? i & 1 ? i / 2 * 5 : i / 2 * 5 + 2 : i * 3;
			hs[x]	  = hexchar(bytes[i] >> 4);
			hs[x + 1] = hexchar(bytes[i]);
		}
		hex_view->printAt(row_a, 0, hs); // print the black bytes
		for (int i = 0; i < n; i++)
			if (bytes[i] & cpu_break_rwx) print_byte(row_a, i, bytes[i]); // print the colored bytes

		// print ascii:
		for (int i = 0; i < n; i++) as[i] = printablechar(bytes[i]);
		ascii_view->printAt(row_a, 0, QLatin1String(as));
		ascii_view->setAttributes(INVERTED);
		for (int i = 0; i < n; i++)
			if (bytes[i] & 0x80) { ascii_view->printAt(row_a, i, printablechar(bytes[i] & 0x7F)); }
		ascii_view->clearAttributes(INVERTED);

		// increment:
		bytes += n;
		cnt -= n;
		address += bytes_per_row;
		row_a++;
	}
}

void MemoryHexInspector::print_byte_seen_by_cpu(uint16 addr)
{
	CoreByte* p		 = machine->cpu->rdPtr(addr);
	int32	  addr32 = addr; // may be 128K

	if (data_source != AsSeenByCpu)
	{
		const CoreByte* page = data_source == RamPages || data_source == AllRam ? &machine->ram[data.baseoffset] :
																				  &machine->rom[data.baseoffset];

		if (size_t(p - page) >= uint32(data.size)) return;

		addr32 = p - page;
	}

	if (addr32 < scroll_offset) return;

	int row = (addr32 - scroll_offset) / bytes_per_row;
	int col = (addr32 - scroll_offset) % bytes_per_row;

	if (row < rows) print_byte(row, col, *p);
}

void MemoryHexInspector::step_left_in_hex()
{
	if (!(hex_edit_nibble ^= 1)) return;

	uint32& addr = hex_edit_offset;
	if (show_words)
	{
		if ((addr - scroll_offset) & 1) // im high byte?
		{
			if (addr >= 3)
				addr -= 3; // step left => low byte des vorhergehenden wortes
			else if (addr >= 2)
				addr -= 2; // sonst high byte des vorhergehenden wortes
			else
				hex_edit_nibble ^= 1; // sonst stop
		}
		else // im low byte
		{
			addr = addr + 1 < uint32(data.size) ? addr + 1 // step left => high byte des selben wortes
												  :
												  addr - 2; // sonst low byte des vorhergehenden wortes
		}
	}
	else // bytes
	{
		if (addr)
			addr -= 1;
		else
			hex_edit_nibble ^= 1;
	}
}

void MemoryHexInspector::step_right_in_hex()
{
	if (hex_edit_nibble ^= 1) return;

	uint32& addr = hex_edit_offset;
	if (show_words)
	{
		if ((addr - scroll_offset) & 1) // im high byte?
		{
			addr = addr ? addr - 1 // step right => low byte des selben wortes
						  :
						  addr + 2; // sonst high byte des nächsten wortes
		}
		else // im low byte
		{
			if (addr + 3 < uint32(data.size))
				addr += 3; // step right => high byte des nächsten wortes
			else if (addr + 2 < uint32(data.size))
				addr += 2; // sonst low byte des nächsten wortes
			else
				hex_edit_nibble ^= 1; // sonst stop
		}
	}
	else // bytes
	{
		if (addr + 1 < uint32(data.size))
			addr += 1;
		else
			hex_edit_nibble ^= 1;
	}
}

void MemoryHexInspector::show_hex_cursor(bool show)
{
	uint rel_addr = hex_edit_offset - scroll_offset;
	if (rel_addr >= uint(rows * bytes_per_row)) return;

	uint row = rel_addr / bytes_per_row;
	uint col = rel_addr % bytes_per_row;
	col		 = show_words ? col & 1 ? (col - 1) / 2 * 5 : col / 2 * 5 + 2 : col * 3;

	CoreByte cb = displayed_data[rel_addr];

	if (show) hex_view->setAttributes(INVERTED);
	if (cb & cpu_break_rwx)
	{
		hex_view->setPenColor(pen_color_for_corebyte(cb));
		hex_view->setAttributes(BOLD);
	}
	hex_view->printAt(row, col + hex_edit_nibble, hexchar(hex_edit_nibble ? cb : cb >> 4));
	if (cb & cpu_break_rwx) { hex_view->setPenColor(Qt::black); }
	hex_view->clearAttributes(INVERTED | BOLD);
}

void MemoryHexInspector::show_ascii_cursor(bool show)
{
	uint rel_addr = ascii_edit_offset - scroll_offset;
	if (rel_addr >= uint(rows * bytes_per_row)) return;

	uint	 row = rel_addr / bytes_per_row;
	uint	 col = rel_addr % bytes_per_row;
	CoreByte cb	 = displayed_data[rel_addr];

	if (cb & 0x80) show = !show;
	ascii_view->setAttributes(show ? INVERTED : 0);
	ascii_view->printAt(row, col, printablechar(cb & 0x7f));
	ascii_view->clearAttributes(INVERTED);
}

inline bool MemoryHexInspector::is_editing_in_hex() { return hex_view->hasFocus(); }

inline bool MemoryHexInspector::is_editing_in_ascii() { return ascii_view->hasFocus(); }

void MemoryHexInspector::show_cursor()
{
	edit_flashphase = 1;
	edit_flashtime	= system_time;
	if (is_editing_in_hex())
		show_hex_cursor(1);
	else if (is_editing_in_ascii())
		show_ascii_cursor(1);
}

// remove cursor blob
// resets pen color and attributes in affected view
//
void MemoryHexInspector::hide_cursor()
{
	if (edit_flashphase)
	{
		if (is_editing_in_hex())
			show_hex_cursor(0);
		else if (is_editing_in_ascii())
			show_ascii_cursor(0);
		edit_flashphase = 0;
	}
}


// ==============================================================================
//			Interface
// ==============================================================================


void MemoryHexInspector::resizeEvent(QResizeEvent* e)
{
	xlogIn("MemoryHexInspector::resizeEvent: %i x %i", width(), height());

	MemoryInspector::resizeEvent(e); // scrollbar geometry, update_all

	int old_bytes_per_row = bytes_per_row;
	bytes_per_row		  = bytes_for_width(width());
	limit(MIN_BYTES_PER_ROW, bytes_per_row, MAX_BYTES_PER_ROW);

	rows = rows_for_height(height()); // new number of rows
	limit(MIN_ROWS, rows, MAX_ROWS);
	limit(0, first_valid_row, rows);
	limit(0, last_valid_row, rows);

	// set child widget sizes:
	int w1 = 5 * cw; // Width of address field --> "$1234"
	int w2 = (show_words ? bytes_per_row / 2 * 5 - 1 : bytes_per_row * 3 - 1) * cw; // Width of Hex field
	int w3 = bytes_per_row * cw;													// Width of Ascii field
	int h  = rows * rh;																// new view height
	address_view->setGeometry(LEFT_MARGIN, TOP_MARGIN, w1, h);
	hex_view->setGeometry(LEFT_MARGIN + HOR_SPACING + w1, TOP_MARGIN, w2, h);
	ascii_view->setGeometry(LEFT_MARGIN + 2 * HOR_SPACING + w1 + w2, TOP_MARGIN, w3, h);

	if (old_bytes_per_row != bytes_per_row) updateAll();
	validate_scrollposition();
	updateScrollbar();
	updateWidgets();
}

/*	we catch the showEvent to call updateWidgets
	because the child views were not resized by QT until now
	and their current contents is invalid
	and we don't want them to draw invalid contents in their first paintEvent
*/
void MemoryHexInspector::showEvent(QShowEvent* e)
{
	xlogIn("MemoryHexInspector::showEvent");

	MemoryInspector::showEvent(e);
	updateScrollbar();
	updateWidgets();
}

/*	callback for ToolWindow during user resize operation
	we have no fixed max width and height, instead max width and height always depend on current height and width.
	after a 1 sec timeout ToolWindow will call adjustSize so that we can set the max size for the Maximize button.
*/
void MemoryHexInspector::adjustMaxSizeDuringResize()
{
	xxlogIn("MemoryHexInspector::adjustMaxSizeDuringResize");

	/*	wir limitieren nur die Zeilenzahl auf das aktuelle Maximum gem. aktueller Breite
		und erlauben Verbreitern bis zu MAX_MYTES_PER_ROW
	*/
	int maxrows = (data.size + bytes_per_row - 1) / bytes_per_row;
	limit(MIN_ROWS, maxrows, MAX_ROWS);

	setMaximumSize(
		width_for_bytes(MAX_BYTES_PER_ROW), // erlaube Maximalbreite
		max(height(),
			height_for_rows(
				maxrows))); // begrenze Höhe, aber nicht unter aktuelle Höhe, weil sonst die Fensteroberkante wandert...
}

/*	adjust size:
	called from toolwindow to finalize size after resizing
	Justiert size auf ein Vielfaches der Zellengröße und
	setzt maxSize so dass Maximize nur vertikal vergrößert.
*/
void MemoryHexInspector::adjustSize(QSize& size)
{
	xlogIn("MemoryHexInspector::adjustSize");

	validate_bytes_per_row();
	validate_rows();
	validate_scrollposition();

	int maxbytes = (data.size + rows - 1) / rows;
	int maxrows	 = (data.size + bytes_per_row - 1) / bytes_per_row;

	limit(MIN_BYTES_PER_ROW, maxbytes, MAX_BYTES_PER_ROW); // limit for user resize
	limit(MIN_ROWS, maxrows, MAX_ROWS);					   // limit for user resize & height for Maximize

	setMaximumSize(width_for_bytes(bytes_per_row) + (bytes_per_row < MAX_BYTES_PER_ROW), height_for_rows(maxrows));

	size.setWidth(width_for_bytes(bytes_per_row));
	size.setHeight(height_for_rows(rows));
}

/*	slot für combobox_datasource:
	also clears focus in hex and ascii view
*/
void MemoryHexInspector::slotSetDataSource(int newdatasource)
{
	if (newdatasource == data_source) return;

	xlogIn("MemoryHexInspector.slotSetDataSource");

	MemoryInspector::slotSetDataSource(newdatasource);
	hex_view->clearFocus();
	ascii_view->clearFocus();
}

/*	slot for combobox_memorypage
	also clears focus in hex and ascii view
*/
void MemoryHexInspector::slotSetMemoryPage(int newpage)
{
	if (newpage < 0) return; // empty comboBox
	assert(data_source == RamPages || data_source == RomPages);
	if (newpage == (data_source == RamPages ? ram_page_idx : rom_page_idx)) return;

	xlogIn("MemoryHexInspector.setMemoryPage()");

	MemoryInspector::slotSetMemoryPage(newpage);

	hex_view->clearFocus();
	ascii_view->clearFocus();
}

void MemoryHexInspector::slotSet16BytesPerRow()
{
	xlogIn("MemoryHexInspector::set16BytesPerRow");

	bytes_per_row = 16;
	scroll_offset &= ~15;
	updateAll();
	emit signalSizeConstraintsChanged();
}

void MemoryHexInspector::slotSet32BytesPerRow()
{
	xlogIn("MemoryHexInspector::set32BytesPerRow");

	bytes_per_row = 32;
	scroll_offset &= ~31;
	updateAll();
	emit signalSizeConstraintsChanged();
}

void MemoryHexInspector::slotSetWordMode(bool words)
{
	xlogIn("MemoryHexInspector::setWordMode");

	show_words = words;
	if (words) bytes_per_row &= ~1;
	updateAll();
	setMinimumWidth(
		width_for_bytes(MIN_BYTES_PER_ROW)); // set limits => resize mit alter size auf queued connection => scheiße
	setMaximumWidth(width_for_bytes(MAX_BYTES_PER_ROW));
	emit signalSizeConstraintsChanged();
}

// note: may be called for data source change => don't fast quit if new addr == old addr
void MemoryHexInspector::setScrollOffset(int32 new_scroll_offset)
{
	int32 old_scroll_offset = scroll_offset;
	scroll_offset			= new_scroll_offset;
	validate_scrollposition();

	if (scroll_offset != old_scroll_offset && !update_all)
	{
		if ((scroll_offset - old_scroll_offset) % bytes_per_row)
		{
			updateAll(); // horizontaler Versatz => update all
		}
		else // if(!update_all)	// scroll up or down:
		{
			int d = (scroll_offset - old_scroll_offset) / bytes_per_row; // d>0 --> scroll up

			first_valid_row = minmax(0, first_valid_row - d, rows);
			last_valid_row	= minmax(0, last_valid_row - d, rows);

			if (first_valid_row < last_valid_row)
			{
				address_view->scrollScreen(d); // TODO: copyWindow
				hex_view->scrollScreen(d);
				ascii_view->scrollScreen(d);

				uint cnt   = bytes_per_row * (last_valid_row - first_valid_row);
				uint z_off = bytes_per_row * first_valid_row;
				uint q_off = bytes_per_row * (first_valid_row + d);

				memmove(&displayed_data[z_off], &displayed_data[q_off], cnt * sizeof(uint16));
			}
		}
	}

	updateScrollbar();
	updateWidgets();
}

void MemoryHexInspector::updateWidgets()
{
	if (!machine || !object || !isVisible()) return;

	assert(machine);
	assert(first_valid_row >= 0 || update_all);
	assert(last_valid_row <= rows || update_all);
	assert(scroll_offset >= 0);

	// update parent:
	MemoryInspector::updateWidgets();

	if (first_valid_row > last_valid_row && !update_all)
		logline("WARNING: MemoryByteInspector: first_valid_row>last_valid_row");

	if (rows * bytes_per_row > int(displayed_data.count())) displayed_data.grow(rows * bytes_per_row);

	if (update_all || first_valid_row >= last_valid_row)
	{
		address_view->clearScreen();
		hex_view->clearScreen();
		ascii_view->clearScreen();
		update_all		= false;
		first_valid_row = last_valid_row = 0;
	}

	int cnt = min(rows * bytes_per_row, data.size - scroll_offset); // real count; excl. tail

	const volatile CoreByte* newdata;
	if (data_source == AsSeenByCpu)
	{
		newdata = new CoreByte[cnt];
		machine->cpu->copyRamToBuffer(scroll_offset, (CoreByte*)newdata, cnt);
	}
	else
	{
		newdata = (data_source == RamPages || data_source == AllRam ? machine->ram.getData() : machine->rom.getData()) +
				  data.baseoffset + scroll_offset;
	}

	// zeilen vor first_valid_line schreiben:
	const volatile CoreByte* np = newdata;
	CoreByte*				 dp = displayed_data.getData();
	int						 r	= first_valid_row;
	uint					 n;
	if (r)
	{
		n = min(cnt, r * bytes_per_row);
		print_rows(0, r, data.baseaddress + scroll_offset, np, n);
		memcpy(dp, NV(np), n * sizeof(CoreByte));
		np += n;
		dp += n;
		cnt -= n;
	}

	// Zeilen im validen Mittelbereich aktualisieren:
	for (; r < last_valid_row; r++)
	{
		n = min(cnt, bytes_per_row);
		for (uint i = 0; i < n; i++)
		{
			if ((dp[i] ^ np[i]) & (0xff | cpu_break_rwx)) print_byte(r, i, dp[i] = np[i]);
		}
		np += n;
		dp += n;
		cnt -= n;
	}

	// Zeilen nach last_valid_row schreiben:
	if (r < rows)
	{
		n = cnt;
		print_rows(r, rows, data.baseaddress + scroll_offset + r * bytes_per_row, np, n);
		memcpy(dp, NV(np), n * sizeof(CoreByte));
	}

	if (data_source == AsSeenByCpu) delete[] newdata;
	first_valid_row = 0;
	last_valid_row	= rows;

	// register-highlight aktualisieren:
	Z80Regs& regs = machine->cpu->getRegisters();
	// remove old:
	print_byte_seen_by_cpu(pc);
	pc = regs.pc;
	print_byte_seen_by_cpu(bc);
	bc = regs.bc;
	print_byte_seen_by_cpu(de);
	de = regs.de;
	print_byte_seen_by_cpu(hl);
	hl = regs.hl;
	print_byte_seen_by_cpu(ix);
	ix = regs.ix;
	print_byte_seen_by_cpu(iy);
	iy = regs.iy;
	print_byte_seen_by_cpu(sp);
	sp = regs.sp;

	// print new:
	hex_view->setPaperColor(paper_color_reg); // gelb
	ascii_view->setPaperColor(paper_color_reg);
	print_byte_seen_by_cpu(bc);
	print_byte_seen_by_cpu(de);
	print_byte_seen_by_cpu(ix);
	print_byte_seen_by_cpu(iy);

	hex_view->setPaperColor(paper_color_sp); // grün
	ascii_view->setPaperColor(paper_color_sp);
	print_byte_seen_by_cpu(sp);

	hex_view->setPaperColor(paper_color_hl); // cyan
	ascii_view->setPaperColor(paper_color_hl);
	print_byte_seen_by_cpu(hl);

	hex_view->setPaperColor(paper_color_pc); // rot
	ascii_view->setPaperColor(paper_color_pc);
	print_byte_seen_by_cpu(pc);
	hex_view->setPaperColor(Qt::white);
	ascii_view->setPaperColor(Qt::white);

	// edit position aktualisieren:
	if (system_time >= edit_flashtime + 0.35)
	{
		edit_flashtime += 0.35;
		edit_flashphase ^= 1;
	}
	if (is_editing_in_hex())
		show_hex_cursor(edit_flashphase);
	else if (is_editing_in_ascii())
		show_ascii_cursor(edit_flashphase);

	// tooltip:
	updateTooltip();
}

/*	Tooltip aktualisieren:
	aber nur, wenn die Maus über dem MemoryView hovert
	da QToolTip::showText(…) die App zwangsweise in den Vordergrund (zurück-) bringt.
*/
void MemoryHexInspector::updateTooltip()
{
	xlogIn("updateTooltip");

	return; // TODO: remove

	// test whether mouse is over this inspector:
	// note: bug in Qt: Qt always thinks the last opened tool window is the top level window
	QPoint gpos = QCursor::pos();
	if (QApplication::topLevelAt(gpos) != window()) return;

	// test whether we are over hex or ascii view
	// this eliminates hits in the toolbar when it's overlaying portions of the inspector by expanding the ">>" button
	QWidget* top = QApplication::widgetAt(gpos);
	if (top != hex_view && top != ascii_view) return;

	// get the local coordinates inside hex and ascii view:
	QPoint lpos_hex	  = hex_view->mapFromGlobal(gpos);
	QPoint lpos_ascii = ascii_view->mapFromGlobal(gpos);

	int	 x, y;
	bool in_hex;

	if (hex_view->rect().contains(lpos_hex, true))
	{
		x = (lpos_hex.x() + mouse_x_offset) / cw;
		y = (lpos_hex.y() + mouse_y_offset) / rh;

		if (show_words)
		{
			if (x % 5 == 4)
				goto X;
			else
				x = x / 5 * 2 + (x % 5 < 2);
		}
		else
		{
			if (x % 3 == 2)
				goto X;
			else
				x = x / 3;
		}
		in_hex = yes;
	}
	else if (ascii_view->rect().contains(lpos_ascii, true))
	{
		x	   = (lpos_ascii.x() + mouse_x_offset) / cw;
		y	   = (lpos_ascii.y() + mouse_y_offset) / rh;
		in_hex = no;
	}
	else
	{
	X:
		QToolTip::showText(gpos, nullptr, ascii_view, QRect());
		return;
	}

	int32 offset = scroll_offset + x + y * bytes_per_row;

	if (offset < data.size)
	{
		CoreByte byte = data_source == AsSeenByCpu						 ? *machine->cpu->rdPtr(offset) :
						data_source == RomPages || data_source == AllRom ? machine->rom[data.baseoffset + offset] :
																		   machine->ram[data.baseoffset + offset];

		cstr fmt = "$%04X";
		if (pageOffsetForCpuAddress(pc) == offset)
			fmt = "pc -> $%04X";
		else if (pageOffsetForCpuAddress(sp) == offset)
			fmt = "sp -> $%04X";
		else if (pageOffsetForCpuAddress(hl) == offset)
			fmt = "hl -> $%04X";
		else if (pageOffsetForCpuAddress(de) == offset)
			fmt = "de -> $%04X";
		else if (pageOffsetForCpuAddress(ix) == offset)
			fmt = "ix -> $%04X";
		else if (pageOffsetForCpuAddress(iy) == offset)
			fmt = "iy -> $%04X";
		else if (pageOffsetForCpuAddress(bc) == offset)
			fmt = "bc -> $%04X";

		if (in_hex)
		{
			assert(cpu_break_x * 2 == cpu_break_w);
			assert(cpu_break_x * 4 == cpu_break_r);

			if (byte & cpu_break_rwx)
				fmt = catstr(fmt, ": breakpoints: ", binstr((byte & cpu_break_rwx) / cpu_break_x, "---", "rwx"));

			QToolTip::showText(gpos, usingstr(fmt, data.baseaddress + offset), hex_view, QRect());
		}
		else
		{
			fmt = catstr(fmt, ": $%02X");
			QToolTip::showText(gpos, usingstr(fmt, data.baseaddress + offset, uint(uint8(byte))), ascii_view, QRect());
		}
	}
}

void MemoryHexInspector::slotSetEditMode(bool f)
{
	if (f)
	{
		button_breakpoint_r->setChecked(0);
		button_breakpoint_w->setChecked(0);
		button_breakpoint_x->setChecked(0);
	}

	edit_mode = f ? EDITMODE_EDIT : EDITMODE_VIEW;
	hex_view->setFocusPolicy(f ? Qt::StrongFocus : Qt::NoFocus);
	ascii_view->setFocusPolicy(f ? Qt::StrongFocus : Qt::NoFocus);
	hex_view->setCursor(f ? Qt::IBeamCursor : Qt::ArrowCursor);
	ascii_view->setCursor(f ? Qt::IBeamCursor : Qt::ArrowCursor);
}

/* Common handler for slots: setBreakpoint…()
 */
void MemoryHexInspector::setBreakpoint(CoreByte mask, bool f)
{
	breakpoint_mask &= ~mask;
	if (f)
	{
		breakpoint_mask |= mask;
		button_edit_mode->setChecked(0);
	}
	edit_mode = breakpoint_mask ? EDITMODE_BREAKPOINTS : EDITMODE_VIEW;
	hex_view->setCursor(edit_mode ? Qt::PointingHandCursor : Qt::ArrowCursor);
	ascii_view->setCursor(edit_mode ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

bool MemoryHexInspector::event(QEvent* e)
{
	xlogIn("MemoryHexInspector::event: %s", QEventTypeStr(e->type()));
	return MemoryInspector::event(e);
}

void MemoryHexInspector::mousePressEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton)
	{
		MemoryInspector::mousePressEvent(e);
		return;
	}
	if (edit_mode == EDITMODE_VIEW) return;

	QPoint gpos		  = e->globalPos();
	QPoint lpos_hex	  = hex_view->mapFromGlobal(gpos);
	QPoint lpos_ascii = ascii_view->mapFromGlobal(gpos);

	bool in_hex	  = hex_view->rect().contains(lpos_hex, true);
	bool in_ascii = ascii_view->rect().contains(lpos_ascii, true);
	if (!in_hex && !in_ascii) return;

	int x, y;

	if (in_hex)
	{
		x = (lpos_hex.x() + mouse_x_offset) / cw;
		y = (lpos_hex.y() + mouse_y_offset) / rh;

		if (show_words)
		{
			if (x % 5 == 4)
				return;
			else
				x = x / 5 * 2;
		}
		else
		{
			if (x % 3 == 2)
				return;
			else
				x = x / 3;
		}
	}
	else // if(in_ascii)
	{
		x = (lpos_ascii.x() + mouse_x_offset) / cw;
		y = (lpos_ascii.y() + mouse_y_offset) / rh;
	}

	int32 offset = scroll_offset + x + y * bytes_per_row;
	if (offset >= data.size) return;

	CoreByte* byte1 = (CoreByte*)dataReadPtrForOffset(offset);
	CoreByte* byte2 =
		show_words && in_hex && (offset + 1 < data.size) ? (CoreByte*)dataReadPtrForOffset(offset + 1) : byte1;

	if (edit_mode == EDITMODE_BREAKPOINTS)
	{
		if ((*byte1 & breakpoint_mask) == breakpoint_mask)
		{
			*byte1 &= ~breakpoint_mask;
			*byte2 &= ~breakpoint_mask;
		}
		else
		{
			*byte1 |= breakpoint_mask;
			*byte2 |= breakpoint_mask;
		}
	}
	else // edit_mode==1: EDIT
	{
		hide_cursor();
		if (is_editing_in_hex())
		{
			hex_edit_offset = offset;
			hex_edit_nibble = 0;
		}
		else { ascii_edit_offset = offset; }
		show_cursor();
	}
}

void MemoryHexInspector::keyPressEvent(QKeyEvent* e)
{
	if (is_editing_in_hex())
	{
		hide_cursor();

		switch (e->key())
		{
			//	case Qt::Key_Backtab:
			//	case Qt::Key_Return:
			//	case Qt::Key_Enter:
		case Qt::Key_Tab:
		case Qt::Key_Escape: hex_view->clearFocus(); break;
		case Qt::Key_Up:
			hex_edit_offset = max(hex_edit_offset, uint32(bytes_per_row)) - bytes_per_row;
			goto X;									   // break;
		case Qt::Key_Left: step_left_in_hex(); goto X; // break;
		case Qt::Key_Down:
			hex_edit_offset = min(hex_edit_offset + bytes_per_row, uint32(data.size) - 1);
			goto X;										 // break;
		case Qt::Key_Right: step_right_in_hex(); goto X; // break;
		default:
			if (is_hex_digit(e->key()))
			{
				uint8& byte = dataReadPtrForOffset(hex_edit_offset)->data;
				if (hex_edit_nibble)
					byte = (byte & 0xF0) + (hex_digit_value(e->key()));
				else
					byte = (byte & 0x0F) + (hex_digit_value(e->key()) << 4);
				step_right_in_hex();
			}
		X:
			while (hex_edit_offset < uint32(scroll_offset)) { setScrollOffset(scroll_offset - bytes_per_row); }
			while (hex_edit_offset >= uint32(scroll_offset) + rows * bytes_per_row)
			{
				setScrollOffset(scroll_offset + bytes_per_row);
			}
			show_cursor();
			break;
		}
	}
	else if (is_editing_in_ascii())
	{
		hide_cursor();

		switch (e->key())
		{
			//	case Qt::Key_Backtab:
			//	case Qt::Key_Return:
			//	case Qt::Key_Enter:
		case Qt::Key_Tab:
		case Qt::Key_Escape: ascii_view->clearFocus(); break;
		case Qt::Key_Up: ascii_edit_offset -= bytes_per_row; goto Y;   // break;
		case Qt::Key_Left: ascii_edit_offset -= 1; goto Y;			   // break;
		case Qt::Key_Down: ascii_edit_offset += bytes_per_row; goto Y; // break;
		case Qt::Key_Right: ascii_edit_offset += 1; goto Y;			   // break;
		default:
			if (e->key() >= 0x20 && e->key() < 0x7F)
			{
				char c = e->key();
				if (~e->modifiers() & Qt::ShiftModifier) c = tolower(c);
				if (e->modifiers() & Qt::MetaModifier) c |= 0x80;

				uint8& byte = dataReadPtrForOffset(ascii_edit_offset)->data;
				byte		= c;
				ascii_edit_offset++;
			}
		Y:
			if (int32(ascii_edit_offset) < 0) ascii_edit_offset = 0;
			if (int32(ascii_edit_offset) >= data.size) ascii_edit_offset = data.size - 1;
			while (ascii_edit_offset < uint32(scroll_offset)) { setScrollOffset(scroll_offset - bytes_per_row); }
			while (ascii_edit_offset >= uint32(scroll_offset) + rows * bytes_per_row)
			{
				setScrollOffset(scroll_offset + bytes_per_row);
			}
			show_cursor();
			break;
		}
	}
	else
		MemoryInspector::keyPressEvent(e);
}

void MemoryHexInspector::slotFocusChanged(bool f)
{
	QObject* sender = QObject::sender();

	if (edit_flashphase)
	{
		if (sender == hex_view)
			show_hex_cursor(f);
		else if (sender == ascii_view)
			show_ascii_cursor(f);
	}
}
