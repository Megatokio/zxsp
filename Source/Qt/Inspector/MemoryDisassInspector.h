#pragma once
/*	Copyright  (c)	Günter Woigk 2012 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include "kio/kio.h"
#include "MemoryInspector.h"
#include "Templates/Array.h"
class Machine;
class MemoryInspector;
class SimpleTerminal;
class QPushButton;
class QLineEdit;
class CoreByteDisassembler;


struct DisassData
{
	int32	 address;
	CoreByte data[4];
};


class MemoryDisassInspector : public MemoryInspector
{
	SimpleTerminal*	address_view;
	SimpleTerminal*	hex_view;
	SimpleTerminal*	disass_view;

	QWidget*		widget_edit_mode;
	QPushButton*	button_breakpoint_r;
	QPushButton*	button_breakpoint_w;
	QPushButton*	button_breakpoint_x;
	QPushButton*	button_edit_mode;

	CoreByteDisassembler*	disass;
	Array<DisassData>		displayed_data;

	int			cw;			// Character width
	int			rh;			// Row height

	int32		pc;
	bool		follow_pc;	// for follow-pc feature

	uint		edit_mode;	enum{EDITMODE_VIEW,EDITMODE_EDIT,EDITMODE_BREAKPOINTS}; // as set by combobox_editmode
	CoreByte	breakpoint_mask;	// bitmask, if edit_mode == EDITMODE_BREAKPOINTS

	bool		editing_in_hex();
	bool		editing_in_disass();

	bool		edit_flashphase;
	Time		edit_flashtime;

	int32		old_pc;						// -> row with colored background
	int32		old_disass_edit_address;

	QString		disass_edit_string;
	bool		disass_edit_string_valid;
	int32		disass_edit_address;		// hex or disass
	int			disass_edit_col;			// hex or disass
	int32		hex_edit_address;			// hex or disass
	int			hex_edit_col;				// hex or disass

public:
	MemoryDisassInspector( QWidget*, MachineController*, volatile IsaObject* );
	~MemoryDisassInspector();

protected:
	void		resizeEvent(QResizeEvent*) override;
	//void		wheelEvent(QWheelEvent*) override;
	//void		paintEvent(QPaintEvent*) override;
	void		mousePressEvent(QMouseEvent*) override;
	//bool		event(QEvent*) override;
	void		keyPressEvent(QKeyEvent*) override;
	//void		keyReleaseEvent(QKeyEvent*) override;
	//void		saveSettings() override;
	void		showEvent(QShowEvent*) override;

	void		updateScrollbar() override;
	//void		adjustMaxSizeDuringResize() override;
	void		adjustSize(QSize&) override;
	void		setScrollOffset (int32 new_base_address) override;

	void		updateWidgets() override;

	void		slotSetMemoryPage(int) override;
	void		slotSetDataSource(int) override;
	void		slotSetScrollPosition(int32) override;	// scrollbar
	void		slotSetAddressFromRegister(int) override; // combobox_register, follow_pc in MemoryDisassInspector

	void		slotSetEditMode		(bool);
	void		slotSetBreakpointR	(bool f)	{ setBreakpoint(cpu_break_r,f); }
	void		slotSetBreakpointW	(bool f)	{ setBreakpoint(cpu_break_w,f); }
	void		slotSetBreakpointX	(bool f)	{ setBreakpoint(cpu_break_x,f); }

private:
	void		print_rows		(int row_a, int row_e, int32 address);
	int			print_row		(int row, int32 address);
	void		print_row		(int32 address);
	void		hide_cursor		();
	void		show_hex_cursor	(bool);
	void		show_disass_cursor(bool);
	void		print_byte_at_address(int32 addr);
	int			displayed_data_row_for_address	(int32 address);
	void		step_left_in_hex();
	void		step_right_in_hex();
	int32		prev_opcode		(int32 address);
	int32		next_opcode		(int32 address);
	int32		prev_opcode		(int32 address, int n, bool f);
	int32		start_of_opcode	(int32 address);
	void		scroll_to_show_address(int32 address);
	void		assemble_and_store_opcode();
	void		setBreakpoint(CoreByte mask, bool f);
	void		validate_rows();
	void		validate_scrollposition();
	void		validate_disass_cols(int& n);
	int			height_for_rows(int n);
	int			rows_for_height(int h);
	int			disass_cols_for_width(int w);
	int			width_for_disass_cols(int n);
	void		slotFocusChanged(bool);
};


























