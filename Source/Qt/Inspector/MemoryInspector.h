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


#ifndef MEMORYINSPECTOR_H
#define MEMORYINSPECTOR_H

#include "Inspector.h"
#include "SimpleTerminal.h"
#include <QScrollBar>
#include "Z80/Z80.h"

class QWidget;
class QBoxLayout;
class QRadioButton;
class QTextEdit;
class QComboBox;
class MyLineEdit;


extern const QColor paper_color_reg;	//gelb
extern const QColor paper_color_sp;		//grün
extern const QColor paper_color_hl;		//cyan
extern const QColor paper_color_pc;		//rot
extern const QColor color_white;
extern const QColor color_black;
extern const QColor color_light_grey;


// ==============================================================


/*	Memory Page Descriptor:
*/
struct Page
{
	int32 baseaddress;		//
	int32 baseoffset;		// inside mem[]
	int32 size;				//
	Page(int32 a,int32 o,int32 sz)	:baseaddress(a),baseoffset(o),size(sz){}
	Page()							:baseaddress(0),baseoffset(0),size(0){}
};


// ==============================================================


/*	Eine eigene Klasse für die ScrollBar
	damit wir wheelEvent(e) von außen aufrufen können:
*/
class MyScrollBar : public QScrollBar
{
	public:		MyScrollBar (Qt::Orientation orientation, QWidget*parent)	:QScrollBar(orientation,parent){}
	void		wheelEvent  (QWheelEvent*e) override						{ QScrollBar::wheelEvent(e); }
	void		mouseDoubleClickEvent(QMouseEvent* e) override				{ mousePressEvent(e); }
};


// ==============================================================

enum MIDisplayMode 	{ Bytes, Disass, MemGraph, MemAccess };				// displaymode


class MemoryInspector : public Inspector
{
protected:
	enum  { AsSeenByCpu, AllRom, RomPages, AllRam, RamPages };	// combobox_datasource
	enum  { regPC, regSP, regBC, regDE, regHL, regIX, regIY };	// combobox_register
	enum  { LEFT_MARGIN  = 1,
			RIGHT_MARGIN = 0,
			HOR_SPACING  = 4,
			TOP_MARGIN   = 0,
			BOT_MARGIN   = 1,
			VERT_MARGINS = TOP_MARGIN+BOT_MARGIN,
			HOR_MARGINS  = LEFT_MARGIN+RIGHT_MARGIN,
			TOOLBAR_WIDGET_HEIGHT = 26,
			TOOLBAR_BUTTON_HEIGHT = 32 };

protected:
	QComboBox*		combobox_datasource;
	QComboBox*		combobox_memorypage;
	QAction*		action_memorypage;			// for visibility of combobox
	QComboBox*		combobox_register;
	MyLineEdit*		lineedit_baseaddress;
	MyScrollBar*	scrollbar;

	int				old_baseaddress;			// for lineedit_baseaddress update
	int				scrollbar_width;			// for convenience
	bool			needs_aligned_addresses;

// data source:
	MIDisplayMode	display_mode;				// Bytes, Words, Disass, MemGraph, MemAccess
	int				data_source;				// AsSeenByCpu, AllRom, RomPages, AllRam, RamPages
	Page			data;						// base address, offset inside mem[] and page size
	int				ram_page_idx;				// combobox_memorypage idx if data_source = RamPages
	int				rom_page_idx;				// combobox_memorypage idx if data_source = RomPages
	int				bytes_per_row;				// displayed data
	int				rows;						// displayed data
	int32			scroll_offset;				// offset to data[base_offset] of first displayed byte
	bool			update_all;

protected:
	MemoryInspector(QWidget*, MachineController*, volatile IsaObject *, MIDisplayMode);
	~MemoryInspector();

	void		resizeEvent	(QResizeEvent*) override;
	//void		paintEvent(QPaintEvent*) override;
	void		wheelEvent(QWheelEvent*) override;
	//void		mousePressEvent(QMouseEvent*) override;
	//bool		event(QEvent*) override;
	//void		keyPressEvent(QKeyEvent*) override;
	//void		keyReleaseEvent(QKeyEvent*) override;

	void		saveSettings() override;
	void		updateWidgets() override;		// timer


	QComboBox*	newComboboxRegister();

VIR	void		setScrollOffset(int32);
VIR void		updateScrollbar();
	int32		pageOffsetForCpuAddress(uint16 addr);

VIR void		slotSetDataSource(int);				// combobox_datasource
VIR void		slotSetMemoryPage(int);				// combobox_memorypage
VIR void		slotSetScrollPosition(int32);		// scrollbar
VIR void		slotSetAddressFromRegister(int);	// combobox_register, follow_pc in MemoryDisassInspector
VIR void		slotMemoryConfigChanged(Memory*, uint how);

	FourBytes*	dataReadPtrForOffset(int32 offset);
	//uint8		peek(uint32 addr)				{ return rdPtr(addr)->data; }
	//uint16  	peek2(uint32 addr)				{ return rdPtr(addr)->data + rdPtr(addr+1)->data * 256; }
	//void		poke(uint32 addr, uint8 byte)	{ rdPtr(addr)->data = byte; }
	//void		poke2(uint32 addr, uint16 word)	{ rdPtr(addr)->data = word; rdPtr(addr+1)->data = word>>8; }

	Page		ramPage(int);
	Page		romPage(int);
	int 		ramPageIndexForCpuAddress(uint16);
	int 		romPageIndexForCpuAddress(uint16);

	void		updateAll()			{ update_all = true; }

private:
	void		init_combobox_memorypage();
	void		save_settings();
	void		set_address_from_textedit();		// textedit_baseaddress
};


#endif
















