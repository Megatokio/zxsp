#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MemoryInspector.h"
#include "Templates/Array.h"
#include "Z80/Z80.h"
#include <QWidget>
class QScrollBar;
class QPushButton;
class QLineEdit;
class QCheckBox;
class Machine;


namespace gui
{
class MemoryInspector;
class SimpleTerminal;
class MyScrollBar;

class MemoryHexInspector : public MemoryInspector
{
	SimpleTerminal* address_view;
	SimpleTerminal* hex_view;
	SimpleTerminal* ascii_view;

	//	QPushButton*	set16;
	//	QPushButton*	set32;
	QCheckBox* checkbox_words;
	bool	   show_words;

	QWidget*	 widget_edit_mode;
	QPushButton* button_breakpoint_r;
	QPushButton* button_breakpoint_w;
	QPushButton* button_breakpoint_x;
	QPushButton* button_edit_mode;

	//	QWidget*		widget_register;
	//	QPushButton*	button_goto_pc;
	//	QPushButton*	button_goto_sp;
	//	QPushButton*	button_goto_bc;
	//	QPushButton*	button_goto_de;
	//	QPushButton*	button_goto_hl;
	//	QPushButton*	button_goto_ix;
	//	QPushButton*	button_goto_iy;
	//	QPushButton*	button_goto_xsp;

	//	QPushButton*	button_register;

	uint32 ascii_edit_offset;
	uint32 hex_edit_offset;
	bool   hex_edit_nibble;
	bool   edit_flashphase;
	Time   edit_flashtime;

	int cw;							// Character width
	int rh;							// Row height
	int pc, bc, de, hl, ix, iy, sp; // last highlighted address for register

	//	bool			is_edit_mode;
	//	uint32			breakpoint_mode;
	uint edit_mode;
	enum { EDITMODE_VIEW, EDITMODE_EDIT, EDITMODE_BREAKPOINTS }; // as set by widget_edit_mode
	CoreByte breakpoint_mask;									 // bitmask, if edit_mode == EDITMODE_BREAKPOINTS

	// scroll position & currently displayed data values:
	Array<CoreByte> displayed_data; // bytes_per_row * rows
	int				first_valid_row;
	int				last_valid_row; // excl.

public:
	MemoryHexInspector(QWidget* parent, MachineController* mc, volatile IsaObject*);
	~MemoryHexInspector() override;

protected:
	void resizeEvent(QResizeEvent*) override;
	// void wheelEvent(QWheelEvent*) override;
	// void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	bool event(QEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	// void keyReleaseEvent(QKeyEvent*) override;
	void showEvent(QShowEvent*) override;

	void saveSettings() override;
	void setScrollOffset(int32) override;
	void adjustMaxSizeDuringResize() override;
	void updateWidgets() override;
	void adjustSize(QSize&) override;

	// void slotSetScrollPosition	(int) override;		// scrollbar
	void slotSetDataSource(int) override;
	void slotSetMemoryPage(int) override;
	void slotSet16BytesPerRow();
	void slotSet32BytesPerRow();
	void slotSetWordMode(bool); // checkbox_words
	void slotSetEditMode(bool);
	void slotSetBreakpointR(bool f) { setBreakpoint(cpu_break_r, f); }
	void slotSetBreakpointW(bool f) { setBreakpoint(cpu_break_w, f); }
	void slotSetBreakpointX(bool f) { setBreakpoint(cpu_break_x, f); }
	void slotFocusChanged(bool);

private:
	void print_byte(int row, int col, CoreByte byte);
	void print_rows(int row_a, int row_e, uint address, const volatile CoreByte* bytes, int cnt);
	void print_byte_seen_by_cpu(uint16 addr);
	int	 width_for_bytes(int n);
	int	 height_for_rows(int n);
	int	 rows_for_height(int h);
	int	 bytes_for_width(int w);
	void validate_rows();
	void validate_bytes_per_row();
	void validate_scrollposition();
	void updateTooltip();
	void show_hex_cursor(bool);
	void show_ascii_cursor(bool);
	void hide_cursor();
	void show_cursor();
	void step_left_in_hex();
	void step_right_in_hex();
	bool is_editing_in_ascii();
	bool is_editing_in_hex();
	void setBreakpoint(CoreByte mask, bool f);
};

} // namespace gui
