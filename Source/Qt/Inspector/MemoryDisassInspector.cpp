// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MemoryDisassInspector.h"
#include "Application.h"
#include "Machine.h"
#include "MemoryInspector.h"
#include "MyLineEdit.h"
#include "Templates/NVPtr.h"
#include "Uni/util.h"
#include "Z80/Z80_Disassembler.h"
#include "Z80/Z80opcodes.h"
#include "zasm/Source/Z80Assembler.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QTimer>
#include <math.h>


namespace gui
{

// Z80/Z80disass.cpp
// extern int Z80OpcodeLength ( uint8 op1, uint8 op2 );

// offset mouse pointer hotspot -> 'feeled' hotspot
#define mouse_x_offset -2
#define mouse_y_offset -2

static QColor color_disass_ok(0, 160, 0);
static QColor color_disass_not_ok(200, 0, 0);

static const int MIN_ROWS		 = 2;
static const int MAX_ROWS		 = 100;
static const int MIN_DISASS_COLS = 16;
static const int MAX_DISASS_COLS = 40;


// ==================================================================================
// ======================   Disassembler Helper Classes   ===========================
// ==================================================================================


class CoreByteDisassembler : public Z80_Disassembler
{
public:
	virtual const CoreByte* pointer(uint32 address) = 0;
	CoreByte peek_cb(uint32 address) const { return *const_cast<CoreByteDisassembler*>(this)->pointer(address); }
	uint8	 peek(uint32 address) const { return *const_cast<CoreByteDisassembler*>(this)->pointer(address); }
};


class AsSeenByCpuDisass : public CoreByteDisassembler
{
	class Z80* cpu;

public:
	AsSeenByCpuDisass(class Z80* cpu) : cpu(cpu) {}
	const CoreByte* pointer(uint32 address) { return cpu->rdPtr(address); }
};


class CoreDisass : public CoreByteDisassembler
{
public:
	const CoreByte* core;
	uint32			size;
	uint32			baseaddress;

public:
	CoreDisass(const CoreByte* core, uint32 size, uint32 baseaddress) : core(core), size(size), baseaddress(baseaddress)
	{}
	const CoreByte* pointer(uint32 address) { return core + ((address - baseaddress) % size); }
};


// ==================================================================================
// ==================   Memory Disassemble Inspector Widget  ========================
// ==================================================================================


// ==============================================================================
//			Constructor & Destructor
// ==============================================================================


MemoryDisassInspector::MemoryDisassInspector(QWidget* parent, MachineController* mc, volatile IsaObject* item) :
	MemoryInspector(parent, mc, item, Disass),
	address_view(nullptr),
	hex_view(nullptr),
	disass_view(nullptr),
	widget_edit_mode(nullptr),
	button_breakpoint_r(nullptr),
	button_breakpoint_w(nullptr),
	button_breakpoint_x(nullptr),
	button_edit_mode(nullptr),
	displayed_data(MAX_ROWS * 4 + 1),
	cw(0),
	rh(0),
	pc(-1),
	follow_pc(no),
	edit_mode(EDITMODE_VIEW),
	breakpoint_mask(0),
	edit_flashphase(0),
	edit_flashtime(0),
	old_pc(-1),
	old_disass_edit_address(-1),
	//	disass_edit_string(""),
	disass_edit_string_valid(no),
	disass_edit_address(0),
	disass_edit_col(0),
	hex_edit_address(0),
	hex_edit_col(0)
{
	xlogIn("new MemoryDisassInspector");

	switch (data_source)
	{
	default: IERR();
	case AsSeenByCpu: disass = new AsSeenByCpuDisass(machine->cpu); break;
	case AllRom:
	case RomPages:
		disass = new CoreDisass(machine->rom.getData() + data.baseoffset, data.size, data.baseaddress);
		break;
	case AllRam:
	case RamPages:
		disass = new CoreDisass(machine->ram.getData() + data.baseoffset, data.size, data.baseaddress);
		break;
	}

	// create child widgets:
	address_view = new SimpleTerminal(this);
	hex_view	 = new SimpleTerminal(this);
	disass_view	 = new SimpleTerminal(this);

	cw = hex_view->char_width;
	rh = hex_view->line_height;

	bytes_per_row = 1;
	validate_rows();
	validate_scrollposition();
	int disass_cols = disass_cols_for_width(width());
	validate_disass_cols(disass_cols);
	address_view->setGeometry(LEFT_MARGIN, TOP_MARGIN, 5 * cw, rows * rh);
	hex_view->setGeometry(LEFT_MARGIN + HOR_SPACING + 5 * cw, TOP_MARGIN, 8 * cw, rows * rh);
	disass_view->setGeometry(LEFT_MARGIN + 2 * HOR_SPACING + (5 + 8) * cw, TOP_MARGIN, disass_cols * cw, rows * rh);

	bool f;
	f = connect(hex_view, &SimpleTerminal::focusChanged, this, &MemoryDisassInspector::slotFocusChanged);
	assert(f);
	f = connect(disass_view, &SimpleTerminal::focusChanged, this, &MemoryDisassInspector::slotFocusChanged);
	assert(f);

	// resize:
	setMinimumWidth(width_for_disass_cols(MIN_DISASS_COLS));
	setMaximumWidth(width_for_disass_cols(MAX_DISASS_COLS));
	setMinimumHeight(height_for_rows(MIN_ROWS));
	setMaximumHeight(height_for_rows(MAX_ROWS));
	setBaseSize(minimumSize());
	setSizeIncrement(cw, rh);
	resize(width_for_disass_cols(disass_cols), height_for_rows(rows));

	// toolbar:
	const int W		 = 20;
	const int H		 = 18;
	widget_edit_mode = new QWidget(nullptr);
	widget_edit_mode->setFixedSize(3 * W, 2 * H);

	button_edit_mode = new QPushButton("Edit", widget_edit_mode);
	f				 = connect(button_edit_mode, &QPushButton::toggled, this, &MemoryDisassInspector::slotSetEditMode);
	assert(f);
	button_edit_mode->setFixedSize(3 * W, H);
	button_edit_mode->setCheckable(1);

	button_breakpoint_r = new QPushButton("R", widget_edit_mode);
	f = connect(button_breakpoint_r, &QPushButton::toggled, this, &MemoryDisassInspector::slotSetBreakpointR);
	assert(f);
	button_breakpoint_r->move(0, H);
	button_breakpoint_r->setFixedSize(W, H);
	button_breakpoint_r->setCheckable(1);
	button_breakpoint_r->setStyleSheet("color: rgb(0,0,255)"); // blue

	button_breakpoint_w = new QPushButton("W", widget_edit_mode);
	f = connect(button_breakpoint_w, &QPushButton::toggled, this, &MemoryDisassInspector::slotSetBreakpointW);
	assert(f);
	button_breakpoint_w->move(W, H);
	button_breakpoint_w->setFixedSize(W, H);
	button_breakpoint_w->setCheckable(1);
	button_breakpoint_w->setStyleSheet("color: rgb(0,180,0)"); // green

	button_breakpoint_x = new QPushButton("X", widget_edit_mode);
	f = connect(button_breakpoint_x, &QPushButton::toggled, this, &MemoryDisassInspector::slotSetBreakpointX);
	assert(f);
	button_breakpoint_x->move(2 * W, H);
	button_breakpoint_x->setFixedSize(W, H);
	button_breakpoint_x->setCheckable(1);
	button_breakpoint_x->setStyleSheet("color: rgb(200,0,0)"); // red
	toolbar->addWidget(widget_edit_mode);

	toolbar->addWidget(newComboboxRegister());
}


MemoryDisassInspector::~MemoryDisassInspector() { delete disass; }


// ==============================================================================
//			Helper
// ==============================================================================


int MemoryDisassInspector::height_for_rows(int n) { return VERT_MARGINS + n * rh; }

int MemoryDisassInspector::rows_for_height(int h) { return (h - VERT_MARGINS + rh / 3) / rh; }

// calc char columns in disass view from inspector width:
int MemoryDisassInspector::disass_cols_for_width(int w)
{
	return (w - HOR_MARGINS - 2 * HOR_SPACING - scrollbar_width) / cw - 5 - 8;
}

// calc inspector width from columns in disass view:
int MemoryDisassInspector::width_for_disass_cols(int n)
{
	return HOR_MARGINS + 2 * HOR_SPACING + scrollbar_width + (5 + 8 + n) * cw;
}

void MemoryDisassInspector::validate_rows() { limit(MIN_ROWS, rows, MAX_ROWS); }

void MemoryDisassInspector::validate_disass_cols(int& n) { limit(MIN_DISASS_COLS, n, MAX_DISASS_COLS); }

/*	validate scroll_offset:
	asserts that all rows will be filled with text
	does not force-align to opcode boundary
	except if scroll_offset must be lowered
*/
void MemoryDisassInspector::validate_scrollposition()
{
	// max. opcode size is 4 bytes => if at least rows*4 bytes left then disassembly will fill all rows:
	if (scroll_offset < 0) scroll_offset = 0;
	if (scroll_offset <= data.size - 4 * rows) return;

	// else count remaining opcodes to see if disassembly will fill all rows:
	int r = 0;
	for (int32 a = scroll_offset; r < rows && a < data.size; a += disass->opcodeLength(a)) r++;
	if (r == rows) return;

	// else step back and find latest possible scroll_offset:
	int32 a0 = scroll_offset - (rows - r) * 4 - 16;
	r		 = 0;
	for (int32 a = a0; a < data.size; a += disass->opcodeLength(a)) r++;
	while (r-- > rows) a0 += disass->opcodeLength(a0);
	scroll_offset = a0;
}


inline bool MemoryDisassInspector::editing_in_hex() { return hex_view->hasFocus(); }

inline bool MemoryDisassInspector::editing_in_disass() { return disass_view->hasFocus(); }

inline QColor pen_color_for_corebyte(CoreByte cb)
{
	return (cb & cpu_break_rwx) == 0 ? color_black // no breakpoint
		   :
		   (~cb & cpu_break_rwx) == 0 ? color_light_grey // all rwx breakpoints: not white for readability
										:
										QColor(
											cb & cpu_break_x ? 180 : 0, cb & cpu_break_w ? 160 : 0,
											cb & cpu_break_r ? 220 : 0); // some breakpoints set
}

inline void MemoryDisassInspector::step_left_in_hex()
{
	if (hex_edit_address > data.baseaddress || hex_edit_col)
	{
		hex_edit_col = !hex_edit_col;
		hex_edit_address -= hex_edit_col;
	}
}

inline void MemoryDisassInspector::step_right_in_hex()
{
	if (hex_edit_col == 0 || hex_edit_address < data.baseaddress + data.size - 1)
	{
		hex_edit_address += hex_edit_col;
		hex_edit_col = !hex_edit_col;
	}
}


/*	find row for address in displayed_data[0..rows-1]
	returns -1 if row is out of screen
	returned row is unprecise if the machine is running and modifies the displayed data
*/
int MemoryDisassInspector::displayed_data_row_for_address(int32 address)
{
	if (address < displayed_data[0].address) return -1;

	int row_a = 0;
	int row_e = rows - 1;

	int32 addr_e = displayed_data[row_e].address;
	if (address >= addr_e)
	{
		if (address >= addr_e + 4) return -1;
		if (address >= addr_e + disass->opcodeLength(addr_e)) return -1;
		return row_e;
	}

	while (row_a < row_e)
	{
		int row_m = (row_e + row_a + 1) / 2;
		if (address < displayed_data[row_m].address) row_e = row_m - 1;
		else row_a = row_m;
	}

	return row_e;
}


/*	Note: Stepping back is ambiguous!
	Note: Unprecise if running machine modifies code!
*/
int32 MemoryDisassInspector::start_of_opcode(int32 address)
{
	for (int32 a = max(address - 16, data.baseaddress);;)
	{
		int n = disass->opcodeLength(a);
		if (a + n > address) return a;
		a += n;
	}
}


/*	step back one opcode
	assumes that displayed_data[] is valid
	assumes that address points to the start of an opcode
	if address points inside an opcode, the start of that opcode is returned
	note: stepping back is ambiguous!
*/
int32 MemoryDisassInspector::prev_opcode(int32 address)
{
	if (address > displayed_data[0].address && address <= displayed_data[rows - 1].address)
	{
		uint row = displayed_data_row_for_address(address);
		return displayed_data[address > displayed_data[row].address ? row : row - 1].address;
	}
	else
	{
		address		= min(address, data.baseaddress + data.size);
		int32 addr0 = data.baseaddress;
		int32 addr1 = max(address - 16, data.baseaddress);
		while (addr1 < address)
		{
			addr0 = addr1;
			addr1 += disass->opcodeLength(addr1);
		}
		return addr0;
	}
}


/*	• Step back n Opcodes
	• Wenn 'address' nicht auf den Anfang eines Opcodes zeigt,
		count_partial_opcode==0: wird der angebrochene Opcode nicht mitgezählt.
		count_partial_opcode==1: wird der angebrochene Opcode mitgezählt.
	• Stops at address 0.
	• Note: Stepping back is ambiguous!
*/
int32 MemoryDisassInspector::prev_opcode(int32 address, int n, bool count_partial_opcode)
{
	assert(n >= 0);

	if (data.baseaddress + n >= address) return data.baseaddress;
	if (address > data.baseaddress + data.size) address = data.baseaddress + data.size;

	int	  sz	= n * 4 + 12;
	int32 addr1 = max(address - sz, data.baseaddress);
	int32 addr[sz];
	int	  i = 0;

	while (addr1 < address)
	{
		addr[i++] = addr1;
		addr1 += disass->opcodeLength(addr1);
	}

	if (addr1 > address && !count_partial_opcode) n += 1;

	return i > n ? addr[i - 1 - n] : data.baseaddress;
}


// note: modifies Pen Color!
void MemoryDisassInspector::print_byte_at_address(int32 addr)
{
	int row = displayed_data_row_for_address(addr);
	if (row == -1) return;
	int		 col  = addr - displayed_data[row].address;
	CoreByte byte = displayed_data[row].data[col];

	hex_view->setPenColor(pen_color_for_corebyte(byte));
	if (byte & cpu_break_rwx) hex_view->setAttributes(BOLD);
	hex_view->printAt(row, col * 2, hexstr(byte, 2));
	if (byte & cpu_break_rwx) hex_view->clearAttributes(BOLD);
}

void MemoryDisassInspector::show_hex_cursor(bool show)
{
	int row = displayed_data_row_for_address(hex_edit_address);
	if (row == -1) return;
	int		 col = hex_edit_address - displayed_data[row].address;
	CoreByte cb	 = displayed_data[row].data[col];

	hex_view->setPenColor(pen_color_for_corebyte(cb));
	hex_view->setAttributesTo(show ? INVERTED : 0);
	if (cb & cpu_break_rwx) hex_view->setAttributes(BOLD);
	hex_view->printAt(row, col * 2 + hex_edit_col, hexchar(hex_edit_col ? cb : cb >> 4));
	if (cb & cpu_break_rwx) hex_view->setPenColor(color_black);
	hex_view->clearAllAttributes();
}

void MemoryDisassInspector::show_disass_cursor(bool show)
{
	int row = displayed_data_row_for_address(disass_edit_address);
	if (row == -1 || disass_edit_address != displayed_data[row].address) return;
	int col = disass_edit_col;
	if (col >= disass_view->cols) return;
	QChar c = col < disass_edit_string.length() ? disass_edit_string[col] : QChar(' ');

	const QColor& color = !show && editing_in_disass() && displayed_data[row].address == old_disass_edit_address ?
							  disass_edit_string_valid ? color_disass_ok : color_disass_not_ok :
							  color_black;

	disass_view->setPenColor(color);
	disass_view->setAttributesTo(show ? INVERTED : 0);
	disass_view->printAt(row, col, c);
	disass_view->clearAllAttributes();
	disass_view->setPenColor(color_black);
}

// remove cursor blob
// resets pen color and attributes in affected view
void MemoryDisassInspector::hide_cursor()
{
	if (edit_flashphase)
	{
		if (editing_in_hex()) show_hex_cursor(0);
		else if (editing_in_disass()) show_disass_cursor(0);
	}
	edit_flashphase = 1;
	edit_flashtime	= system_time;
}


/*	assembles opcode into buffer[4]
	returns size of opcode (1..4) or 0 if error
*/
int assemble(uint16 address, QString source_str, char buffer[])
{
	Z80Assembler ass;
	return ass.assembleSingleLine(address, catstr(" ", source_str.toUtf8().data()), buffer);
}


/*	test whether the source string assembles without errors
 */
bool assembles_ok(uint16 address, QString source_str)
{
	char bu[4];
	return assemble(address, source_str, bu) != 0;
}

void MemoryDisassInspector::assemble_and_store_opcode()
{
	// assembler new opcode:
	char opcode_buffer[4];
	int	 opcode_size = assemble(disass_edit_address, disass_edit_string, opcode_buffer);
	opcode_size		 = min(opcode_size, data.baseaddress + data.size - disass_edit_address);

	// count size of overwritten opcode(s):
	int old_opcode_size = 0;
	while (old_opcode_size < opcode_size)
	{
		old_opcode_size += disass->opcodeLength(disass_edit_address + old_opcode_size);
	}

	// store new opcode:
	for (int i = 0; i < opcode_size; i++)
	{
		((FourBytes*)disass->pointer(disass_edit_address + i))->data = opcode_buffer[i];
	}

	// blank remainder of old opcodes with NOPs:
	old_opcode_size = min(old_opcode_size, data.baseaddress + data.size - disass_edit_address);
	for (int i = opcode_size; i < old_opcode_size; i++)
	{
		((FourBytes*)disass->pointer(disass_edit_address + i))->data = NOP;
	}
}


/*	Print one row:
	- Address
	- Hex
	- Disassemble
	Update displayed_data[]
	Colorizes line with new_pc
	Decolorizes line with old pc

	Returns opcode size: 1 .. 4
*/
int MemoryDisassInspector::print_row(int row, int32 address)
{
	DisassData& data = displayed_data[row];

	if (editing_in_disass() && address == disass_edit_address && old_disass_edit_address != disass_edit_address &&
		old_disass_edit_address != -1)
	{
		print_row(old_disass_edit_address);
	}

	bool paper_color_for_pc = address == pc;
	bool paper_color_change = (address == old_pc) != paper_color_for_pc;

	if (paper_color_for_pc)
	{
		address_view->setPaperColor(paper_color_pc);
		disass_view->setPaperColor(paper_color_pc);
		disass_view->clear_color = paper_color_pc;
		hex_view->setPaperColor(paper_color_pc);
		hex_view->clear_color = paper_color_pc;
	}

	bool pen_color_for_disass_edit_address = editing_in_disass() && (address == disass_edit_address);
	bool pen_color_change =
		(/*editing_in_disass() &&*/ (address == old_disass_edit_address)) != pen_color_for_disass_edit_address;

	if (pen_color_change)
	{
		if (address == old_disass_edit_address) old_disass_edit_address = -1;
		if (address == disass_edit_address && editing_in_disass()) old_disass_edit_address = disass_edit_address;
	}

	if (pen_color_for_disass_edit_address)
	{
		disass_view->setPenColor(disass_edit_string_valid ? color_disass_ok : color_disass_not_ok);
	}

	if (data.address != address || paper_color_change || update_all)
	{
		char bu[8] = "$0000";
		sprintf(bu, address >> 16 ? "%05X" : "$%04X", address);
		address_view->printAt(row, 0, bu);
		data.address = address;
	}

	int n = disass->opcodeLength(address);
	assert(n >= 1 && n <= 4);

	bool u = update_all || paper_color_change || pen_color_change;
	for (int i = 0; !u && i < n; i++) u = data.data[i] != (disass->peek_cb(address + i) & (255 | cpu_break_rwx));

	if (u)
	{
		CoreByte corebyte;
		CoreByte flags = 0;

		for (int i = 0; i < n; i++)
		{
			data.data[i] = corebyte = disass->peek_cb(address + i) & (255 | cpu_break_rwx);

			if (flags != (corebyte & cpu_break_rwx))
			{
				flags = corebyte & cpu_break_rwx;
				if (flags)
				{
					hex_view->setAttributes(BOLD);
					hex_view->setPenColor(pen_color_for_corebyte(corebyte));
				}
				else
				{
					hex_view->clearAttributes(BOLD);
					hex_view->setPenColor(color_black);
				}
			}

			hex_view->printAt(row, i * 2, hexstr(corebyte, 2));
			hex_view->clearToEndOfLine();
		}

		if (flags)
		{
			hex_view->clearAttributes(BOLD);
			hex_view->setPenColor(color_black);
		}

		disass_view->printAt(
			row, 0,
			pen_color_for_disass_edit_address ? disass_edit_string.left(disass_view->cols) :
												disass->disassemble(address));
		disass_view->clearToEndOfLine();
	}

	if (pen_color_for_disass_edit_address) { disass_view->setPenColor(color_black); }

	if (paper_color_for_pc)
	{
		address_view->setPaperColor(color_white);
		disass_view->setPaperColor(color_white);
		disass_view->clear_color = color_white;
		hex_view->setPaperColor(color_white);
		hex_view->clear_color = color_white;
	}

	return n;
}


void MemoryDisassInspector::print_row(int32 address)
{
	int row = displayed_data_row_for_address(address);
	if (row != -1) print_row(row, address);
}


/*	Print rows [a..[e to textedit widgets.
	Used to print data after clear or scroll screen.
	Prints address into textedit_address,
		hex codes into textedit_hex and
		disassembly into textedit_disass.
	Updates displayed_data[]

	row_a	first row
	row_e	last row +1
	address	address to print for row_a
*/
void MemoryDisassInspector::print_rows(int row_a, int row_e, int32 address)
{
	for (int row = row_a; row < row_e; row++) { address += print_row(row, address); }
	old_pc = pc; // remember line with colored background
}


// step forward one opcode
inline int32 MemoryDisassInspector::next_opcode(int32 address)
{
	address = prev_opcode(address + 1);
	return address + disass->opcodeLength(address);
}


void MemoryDisassInspector::scroll_to_show_address(int32 address)
{
	if (address < data.baseaddress + scroll_offset) // scroll down:
	{
		scroll_offset = prev_opcode(address + 1) - data.baseaddress;
		return;
	}

	int32 display_end_address = next_opcode(displayed_data[rows - 1].address);
	if (address < display_end_address) return; // ok: don't scroll

	// scroll up:

	if (data.baseaddress + scroll_offset + rows * 4 < address)
	{
		display_end_address = prev_opcode(address - rows * 4);
		scroll_offset		= display_end_address - data.baseaddress;
		for (int i = 0; i < rows; i++) display_end_address = next_opcode(display_end_address);
	}

	while (address >= display_end_address)
	{
		scroll_offset		= next_opcode(data.baseaddress + scroll_offset) - data.baseaddress;
		display_end_address = next_opcode(display_end_address);
	}
}


// ==============================================================================
//			Interface
// ==============================================================================


void MemoryDisassInspector::resizeEvent(QResizeEvent* e)
{
	xlogIn("MemoryDisassInspector::resizeEvent: %i x %i", width(), height());

	MemoryInspector::resizeEvent(e);

	rows = rows_for_height(height());
	limit(MIN_ROWS, rows, MAX_ROWS);
	validate_scrollposition();

	// set child widget sizes:
	int h  = rows * rh;							  // new view height
	int w1 = 5 * cw;							  // address feldbreite
	int w2 = 8 * cw;							  // hexbyte feldbreite
	int w3 = disass_cols_for_width(width()) * cw; // disass feldbreite

	address_view->resize(w1, h);
	hex_view->resize(w2, h);
	disass_view->resize(w3, h);

	validate_scrollposition();
	updateAll();
	updateScrollbar();
	updateWidgets();

	setMaximumWidth(width_for_disass_cols(MAX_DISASS_COLS));
}


/*	we catch the showEvent to call updateWidgets
	because the child views were not resized by QT until now
	and their current contents is invalid
	and we don't want them to draw invalid contents in their first paintEvent
*/
void MemoryDisassInspector::showEvent(QShowEvent* e)
{
	xlogIn("MemoryDisassInspector::showEvent");

	MemoryInspector::showEvent(e);

	updateAll();
	updateScrollbar();
	updateWidgets();
}


/*	adjust size:
	called from toolwindow to finalize size after resizing
	Justiert size auf ein Vielfaches der Zellengröße und
	setzt maxSize so dass Maximize nur vertikal vergrößert.
*/
void MemoryDisassInspector::adjustSize(QSize& size)
{
	xlogIn("MemoryDisassInspector::adjustSize");

	int disass_cols = disass_cols_for_width(size.width());
	validate_disass_cols(disass_cols);
	validate_rows();
	validate_scrollposition();

	setMaximumWidth(width_for_disass_cols(disass_cols) + (disass_cols < MAX_DISASS_COLS));

	size.setWidth(width_for_disass_cols(disass_cols));
	size.setHeight(height_for_rows(rows));
}


/*	slot für combobox_datasource:
	we also create a new disassembler here
	and clear focus in hex and disass view
	and clear follow_pc
*/
void MemoryDisassInspector::slotSetDataSource(int newdatasource)
{
	if (newdatasource == data_source) return;

	xlogIn("MemoryDisassInspector.slotSetDataSource");

	MemoryInspector::slotSetDataSource(newdatasource);

	delete disass;
	disass = nullptr;
	switch (data_source)
	{
	case AsSeenByCpu: disass = new AsSeenByCpuDisass(machine->cpu); break;
	case AllRom:
	case RomPages:
		disass = new CoreDisass(machine->rom.getData() + data.baseoffset, data.size, data.baseaddress);
		break;
	case AllRam:
	case RamPages:
		disass = new CoreDisass(machine->ram.getData() + data.baseoffset, data.size, data.baseaddress);
		break;
	}

	hex_view->clearFocus();
	disass_view->clearFocus();
	follow_pc = no;
}


/*	slot for combobox_memorypage
	we also point disass to the new memory page
	and clear focus in hex and disass view
	and clear follow_pc
*/
void MemoryDisassInspector::slotSetMemoryPage(int newpage)
{
	if (newpage < 0) return; // empty comboBox
	assert(data_source == RamPages || data_source == RomPages);
	if (newpage == (data_source == RamPages ? ram_page_idx : rom_page_idx)) return;

	xlogIn("MemoryDisassInspector.setMemoryPage()");

	MemoryInspector::slotSetMemoryPage(newpage);

	volatile MemoryPtr& mem = data_source == RomPages ? machine->rom : machine->ram;
	delete disass;
	disass = new CoreDisass(&mem[data.baseoffset], data.size, data.baseaddress);

	hex_view->clearFocus();
	disass_view->clearFocus();
	follow_pc = no;
}


/*	slot for combobox_register
	if reg==regPC then switch on follow_pc else off
	if PC points to unmapped memory then switch off follow_pc to suppress subsequent "unmapped memory" alerts
*/
void MemoryDisassInspector::slotSetAddressFromRegister(int reg)
{
	xlogIn("MemoryDisassInspector.slotSetAddressFromRegister");

	MemoryInspector::slotSetAddressFromRegister(reg);

	follow_pc = no;
	if (reg != regPC) return;

	follow_pc = yes;
	if (data_source == AsSeenByCpu) return;

	Z80Regs& registers = machine->cpu->getRegisters();
	uint	 address   = registers.pc;
	follow_pc		   = pageOffsetForCpuAddress(address) != -1;
}


void MemoryDisassInspector::slotSetEditMode(bool f)
{
	if (f)
	{
		button_breakpoint_r->setChecked(0);
		button_breakpoint_w->setChecked(0);
		button_breakpoint_x->setChecked(0);
	}

	edit_mode = f ? EDITMODE_EDIT : EDITMODE_VIEW;
	hex_view->setFocusPolicy(f ? Qt::StrongFocus : Qt::NoFocus);
	disass_view->setFocusPolicy(f ? Qt::StrongFocus : Qt::NoFocus);
	hex_view->setCursor(f ? Qt::IBeamCursor : Qt::ArrowCursor);
	disass_view->setCursor(f ? Qt::IBeamCursor : Qt::ArrowCursor);
}


/* Common Handler for Slots: setBreakpoint…()
 */
void MemoryDisassInspector::setBreakpoint(CoreByte mask, bool f)
{
	assert((mask & ~cpu_break_rwx) == 0);
	breakpoint_mask &= ~mask;
	if (f)
	{
		breakpoint_mask |= mask;
		button_edit_mode->setChecked(0);
	}
	edit_mode = breakpoint_mask ? EDITMODE_BREAKPOINTS : EDITMODE_VIEW;
	hex_view->setCursor(edit_mode ? Qt::PointingHandCursor : Qt::ArrowCursor);
	disass_view->setCursor(edit_mode ? Qt::PointingHandCursor : Qt::ArrowCursor);
}


//	set display base address
//	note: may be called for data source change => don't fast quit if new addr == old addr
//	reimplemented for adjusted limit test
void MemoryDisassInspector::setScrollOffset(int32 new_scroll_offset)
{
	xlogIn("MemoryDisassInspector.setScrollOffset(%i)", new_scroll_offset);

	scroll_offset = new_scroll_offset;
	validate_scrollposition();
	updateScrollbar();
}


//	slot for scrollbar:
//	reimplemented to lock to opcode boundaries
//	note: new_scrollposition is measured in rows
//		  bytes_per_row was determined by screen contents
void MemoryDisassInspector::slotSetScrollPosition(int32 new_scrollposition)
{
	xlogIn("MemoryDisassInspector.setScrollPosition(%i)", new_scrollposition);

	float bytes_per_row = data.size / (scrollbar->maximum() + rows);			// calculate bytes/row
	float delta_rows	= (new_scrollposition - scroll_offset / bytes_per_row); // distance of movement [rows]

	if (fabs(delta_rows) > rows + 0.5) // beyond paging
	{
		scroll_offset = start_of_opcode(scroll_offset + (delta_rows + 0.5) * bytes_per_row);
	}
	else if (delta_rows < 0) // small movement or paging down
	{
		scroll_offset = prev_opcode(data.baseaddress + scroll_offset, floorf(-delta_rows), yes) - data.baseaddress;
	}
	else // small movement or paging up
	{
		for (int r = 0; r < ceilf(delta_rows); r++) scroll_offset += disass->opcodeLength(scroll_offset);
	}

	xlogline("--> display_base_address = %i", int(scroll_offset));

	validate_scrollposition();
	updateScrollbar();
}


//	reimplemented due to varying page size
void MemoryDisassInspector::updateScrollbar()
{
	xlogIn("MemoryDisassInspector.updateScrollbar");

	int visible_bytes = 0;
	for (int r = 0; r < rows; r++) visible_bytes += disass->opcodeLength(scroll_offset + visible_bytes);
	float bytes_per_row = (float)visible_bytes / rows;

	scrollbar->blockSignals(true);
	scrollbar->setMinimum(0);
	scrollbar->setMaximum((data.size - visible_bytes + 0.5f) / bytes_per_row);
	scrollbar->setValue((scroll_offset + 0.5f) / bytes_per_row);
	scrollbar->setPageStep(rows);
	scrollbar->setSingleStep(max(1, rows / 16));
	scrollbar->blockSignals(false);
}


void MemoryDisassInspector::updateWidgets()
{
	if (!machine || !object || !isVisible()) return;

	// follow PC:
	Z80Regs& regs = machine->cpu->getRegisters();
	pc			  = pageOffsetForCpuAddress(regs.pc);

	if (follow_pc)
	{
		if ((machine->isRunning() && machine->cpu_clock > 1000) || this->combobox_register->currentIndex() != regPC ||
			editing_in_disass() || editing_in_hex())
			follow_pc = no;
		else
		{
			if (pc < displayed_data[0].address || pc > displayed_data[rows - 1].address) // TODO: or misaligned PC
				slotSetAddressFromRegister(regPC);
		}
	}

	// update parent:
	MemoryInspector::updateWidgets();

	// update display:
	print_rows(0, rows, scroll_offset);
	update_all = false;

	// hilite register addresses:
	hex_view->setPaperColor(paper_color_reg); // gelb
	print_byte_at_address(pageOffsetForCpuAddress(regs.bc));
	print_byte_at_address(pageOffsetForCpuAddress(regs.de));
	print_byte_at_address(pageOffsetForCpuAddress(regs.ix));
	print_byte_at_address(pageOffsetForCpuAddress(regs.iy));
	hex_view->setPaperColor(paper_color_sp); // grün
	print_byte_at_address(pageOffsetForCpuAddress(regs.sp));
	hex_view->setPaperColor(paper_color_hl); // cyan
	print_byte_at_address(pageOffsetForCpuAddress(regs.hl));
	hex_view->setPaperColor(color_white);
	hex_view->setPenColor(color_black);

	// edit position aktualisieren:
	if (system_time >= edit_flashtime + 0.35)
	{
		edit_flashtime = max(system_time, edit_flashtime + 0.35);
		edit_flashphase ^= 1;
	}
	if (editing_in_hex()) show_hex_cursor(edit_flashphase);
	else if (editing_in_disass()) show_disass_cursor(edit_flashphase);
}


/*	mouse button pressed
	if edit_mode == EDITMODE_EDIT start editing in hex_view or disass_view
	if edit_mode == EDITMODE_BREAKPOINTS set / clear breakpoint
*/
void MemoryDisassInspector::mousePressEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton)
	{
		MemoryInspector::mousePressEvent(e);
		return;
	}
	if (edit_mode == 0) return;

	QPoint gpos		   = e->globalPos();
	QPoint lpos_hex	   = hex_view->mapFromGlobal(gpos);
	QPoint lpos_disass = disass_view->mapFromGlobal(gpos);

	bool in_hex	   = hex_view->rect().contains(lpos_hex, true);
	bool in_disass = disass_view->rect().contains(lpos_disass, true);
	if (!in_hex && !in_disass) return;

	hide_cursor();

	int row, col;

	if (in_hex)
	{
		col = (lpos_hex.x() + mouse_x_offset) / cw;
		row = (lpos_hex.y() + mouse_y_offset) / rh;
	}

	if (in_disass)
	{
		col = (lpos_disass.x() + mouse_x_offset) / cw;
		row = (lpos_disass.y() + mouse_y_offset) / rh;
	}

	if (edit_mode == EDITMODE_EDIT)
	{
		if (in_hex)
		{
			hex_edit_address = displayed_data[row].address;
			int opcodelen	 = disass->opcodeLength(hex_edit_address);
			if (col >= opcodelen * 2) col = opcodelen * 2 - 1;
			hex_edit_col = col & 1;
			hex_edit_address += col / 2;
			if (hex_edit_address >= data.baseaddress + data.size)
			{
				hex_edit_address = data.baseaddress + data.size - 1;
				hex_edit_col	 = 1;
			}
		}
		else
		{
			disass_edit_address		 = displayed_data[row].address;
			disass_edit_string		 = disass->disassemble(disass_edit_address);
			disass_edit_col			 = min(col, disass_edit_string.length());
			disass_edit_string_valid = assembles_ok(disass_edit_address, disass_edit_string); // yes;
			print_row(old_disass_edit_address);
		}
	}
	else // if(edit_mode==EDITMODE_BREAKPOINTS)
	{
		int32 address	= displayed_data[row].address;
		int	  opcodelen = disass->opcodeLength(address);

		if (in_hex)
		{
			if (col < opcodelen * 2)
			{
				NVPtr<Machine> z(machine);
				CoreByte*	   cb = (CoreByte*)(disass->pointer(address + col / 2));
				if ((*cb & breakpoint_mask) == breakpoint_mask) { *cb &= ~breakpoint_mask; }
				else { *cb |= breakpoint_mask; }
			}
		}
		else // in disass
		{
			CoreByte sum = disass->peek_cb(address);
			for (int i = 1; i < opcodelen; i++) sum &= disass->peek_cb(address + i);

			NVPtr<Machine> z(machine);
			if ((sum & breakpoint_mask) == breakpoint_mask)
				for (int i = 0; i < opcodelen; i++) { *(CoreByte*)disass->pointer(address + i) &= ~breakpoint_mask; }
			else
				for (int i = 0; i < opcodelen; i++) { *(CoreByte*)disass->pointer(address + i) |= breakpoint_mask; }
		}
	}
}


// virtual
void MemoryDisassInspector::keyPressEvent(QKeyEvent* e)
{
	if (!editing_in_hex() && !editing_in_disass())
	{
		MemoryInspector::keyPressEvent(e);
		return;
	}

	hide_cursor();

	if (editing_in_hex())
	{
		switch (e->key())
		{
			//	case Qt::Key_Backtab:
			//	case Qt::Key_Return:
			//	case Qt::Key_Enter:
		case Qt::Key_Tab:
		case Qt::Key_Escape: hex_view->clearFocus(); break;
		case Qt::Key_Up:
			hex_edit_address = prev_opcode(hex_edit_address);
			hex_edit_col	 = 0;
			goto X;
		case Qt::Key_Left:
			step_left_in_hex();
			if (hex_edit_col) goto X;
			else break;
		case Qt::Key_Down:
			hex_edit_address = next_opcode(hex_edit_address);
			hex_edit_col	 = 0;
			if (hex_edit_address >= data.baseaddress + data.size)
			{
				hex_edit_address = data.baseaddress + data.size - 1;
				hex_edit_col	 = 1;
			}
			goto X;
		case Qt::Key_Right:
			step_right_in_hex();
			if (hex_edit_col) break;
			else goto X;
		default:
			if (uint32(hex_edit_address - data.baseaddress) < uint32(data.size) && is_hex_digit(e->key()))
			{
				uint8& byte = dataReadPtrForOffset(hex_edit_address)->data;
				if (hex_edit_col) byte = (byte & 0xF0) + (hex_digit_value(e->key()));
				else byte = (byte & 0x0F) + (hex_digit_value(e->key()) << 4);
				step_right_in_hex();
				goto X;
			}
			// scroll window to show cursor pos:
		X:
			scroll_to_show_address(hex_edit_address);
			show_hex_cursor(1);
			break;
		}
	}

	else // editing_in_disass
	{
		switch (e->key())
		{
		case Qt::Key_Escape: // abort & clear focus:
			disass_view->clearFocus();
			break;

		case Qt::Key_Return: // enter opcode (if valid) and goto next line:
		case Qt::Key_Enter: disass_edit_col = 0; assemble_and_store_opcode();
		case Qt::Key_Down:
			if (next_opcode(disass_edit_address) < data.baseaddress + data.size)
				disass_edit_address = next_opcode(disass_edit_address);
			disass_edit_string		 = disass->disassemble(disass_edit_address);
			disass_edit_string_valid = assembles_ok(disass_edit_address, disass_edit_string); // yes;
			goto Y;
		case Qt::Key_Up: // enter opcode (if valid) and goto previous line:
			if (disass_edit_address > data.baseaddress) disass_edit_address = prev_opcode(disass_edit_address);
			disass_edit_string		 = disass->disassemble(disass_edit_address);
			disass_edit_string_valid = assembles_ok(disass_edit_address, disass_edit_string); // yes;
			goto Y;

		case Qt::Key_Left: // move left, stays in line!
			disass_edit_col = minmax(0, disass_edit_col - 1, disass_edit_string.length());
			goto Z;
		case Qt::Key_Right: // move right, stays in line!
			disass_edit_col = minmax(0, disass_edit_col + 1, disass_edit_string.length());
			goto Z;
		case Qt::Key_Backspace: // delete char left to cursor
			if (disass_edit_col)
			{
				disass_edit_string =
					disass_edit_string.left(disass_edit_col - 1) + disass_edit_string.mid(disass_edit_col);
				disass_edit_string_valid = assembles_ok(disass_edit_address, disass_edit_string);
				disass_edit_col--;
			}
			goto Z;
		default: // input key, stay in line:
			if (e->text().length() == 1 && e->text().at(0) >= 0x20 && e->text().at(0) < 0x80)
			{
				if (disass_edit_col > disass_edit_string.length())
					disass_edit_string += spacestr(disass_edit_col - disass_edit_string.length());
				disass_edit_string =
					disass_edit_string.left(disass_edit_col) + e->text() + disass_edit_string.mid(disass_edit_col + 1);
				disass_edit_string_valid = assembles_ok(disass_edit_address, disass_edit_string);
				disass_edit_col++;
			}
			else logline("MemoryDisassInspector::keyPressEvent: %i not handled", e->key());
		Z:
			if (old_disass_edit_address == disass_edit_address) old_disass_edit_address = -1;
			print_row(disass_edit_address);
		Y:
			scroll_to_show_address(disass_edit_address);
			show_disass_cursor(1);
			break;
		}
		// TODO
	}
}


// slot for hex_view and disass_view:
void MemoryDisassInspector::slotFocusChanged(bool f)
{
	QObject* sender = (QObject::sender());

	if (sender == disass_view)
	{
		if (f) // disass view gains focus:
		{
			disass_edit_string_valid = uint32(disass_edit_address - data.baseaddress) < uint32(data.size);
			if (disass_edit_string_valid)
			{
				disass_edit_string = disass->disassemble(disass_edit_address);
				disass_edit_col	   = min(disass_edit_col, disass_edit_string.length());
				//				print_row(disass_edit_address);
			}
		}
		else // disass view loses focus:
		{
			//			print_row(disass_edit_address);
		}
	}

	if (edit_flashphase)
	{
		if (sender == hex_view) show_hex_cursor(f);
		else if (sender == disass_view) show_disass_cursor(f);
	}
}

} // namespace gui
