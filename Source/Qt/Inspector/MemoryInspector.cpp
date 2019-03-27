/*	Copyright  (c)	Günter Woigk 2012 - 2018
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

#include "MemoryInspector.h"
#include "MemoryHexInspector.h"
#include <QComboBox>
#include <QTextEdit>
#include <QRadioButton>
#include <QGridLayout>
#include <QBoxLayout>
#include <QPainter>
#include "Qt/MyLineEdit.h"
#include "Inspector.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "SimpleTerminal.h"
#include <QTimer>
#include "Uni/util.h"
#include "Qt/qt_util.h"
#include "Qt/Settings.h"
#include "Templates/Array.h"
#include <QApplication>
#include "MachineController.h"


//
//	MemoryInspector ist die Basis-Klasse für Memory Display Widgets.
//


// highlight colors:
//
const QColor paper_color_reg(244,244,155);		//gelb
const QColor paper_color_sp(177,255,188);		//grün
const QColor paper_color_hl(177,255,255);		//cyan
const QColor paper_color_pc(255,199,199);		//rot
const QColor color_white(255,255,255);
const QColor color_black(0,0,0);
const QColor color_light_grey(Qt::lightGray);



// ==================================================================================
// ===========================   MemoryInspector   ==================================
// ==================================================================================



MemoryInspector::MemoryInspector(QWidget*p, MachineController* mc, volatile IsaObject *i, MIDisplayMode displaymode)
:
	Inspector(p,mc,i),
	combobox_datasource(NULL),
	combobox_memorypage(NULL),
	combobox_register(NULL),
	lineedit_baseaddress(NULL),
	scrollbar(new MyScrollBar(Qt::Vertical,this)),
	scrollbar_width(scrollbar->sizeHint().width()),
	needs_aligned_addresses(displaymode==MemAccess),
	display_mode(displaymode),
	data_source(settings.get_int(key_memoryview_datasource(displaymode),AsSeenByCpu)),
	ram_page_idx(settings.get_int(key_memoryview_ram_page(displaymode),0)),
	rom_page_idx(settings.get_int(key_memoryview_rom_page(displaymode),0)),
	bytes_per_row(settings.get_int(key_memoryview_bytes_per_row(displaymode),32)),	// must be validated by subclass
	rows(settings.get_int(key_memoryview_rows(displaymode),8)),						// must be validated by subclass
	scroll_offset(settings.get_int(key_memoryview_scrollposition(displaymode),0)),
	update_all(yes)
{
	xlogIn("new MemoryInspector");

	setMinimumSize(32,32);		// MemoryGraphInspector kann ziemlich klein sein
	setMaximumSize(4000,2000);
	QSize sz = settings.value(key_memoryview_size(displaymode),size()).toSize();
	if(!sz.isEmpty()) resize(sz);

	switch(data_source)
	{
	default:			data_source = AsSeenByCpu;
	case AsSeenByCpu:	data.size = 64 kB; break;
	case AllRom:		data.size = machine->rom.count(); break;
	case AllRam:		data.size = machine->ram.count(); if(data.size<=0xc000) data.baseaddress = 0x4000; break;
	case RamPages:		data = ramPage(ram_page_idx); if(!data.size) data = ramPage(ram_page_idx=0); break;
	case RomPages:		data = romPage(rom_page_idx); if(!data.size) data = romPage(rom_page_idx=0); break;
	}

// security:
	limit(1, bytes_per_row, 512);
	limit(0, scroll_offset, data.size-1);
	limit(1, rows, 2000);

// init:
	connect(scrollbar,&MyScrollBar::valueChanged,this,&MemoryInspector::slotSetScrollPosition);
	connect(controller,&MachineController::signal_memoryModified,this,&MemoryInspector::slotMemoryConfigChanged);

// toolbar:
	toolbar = new QToolBar();
	combobox_datasource = new QComboBox(NULL);
	combobox_datasource->setMinimumSize(80,TOOLBAR_WIDGET_HEIGHT);
	combobox_datasource->setMaximumSize(105,TOOLBAR_WIDGET_HEIGHT);
	combobox_datasource->addItems(QStringList()<<"As seen by CPU"<<"All Rom"<<"Rom Pages"<<"All Ram"<<"Ram Pages");
	combobox_datasource->setFocusPolicy(Qt::NoFocus);
	combobox_datasource->setCurrentIndex(data_source);
	connect(combobox_datasource, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &MemoryInspector::slotSetDataSource);
	toolbar->addWidget(combobox_datasource);

	combobox_memorypage = new QComboBox(NULL);
	combobox_memorypage->setFixedSize(63,TOOLBAR_WIDGET_HEIGHT);
	combobox_memorypage->setFocusPolicy(Qt::NoFocus);
	action_memorypage = toolbar->addWidget(combobox_memorypage);
	init_combobox_memorypage();
	connect(combobox_memorypage, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &MemoryInspector::slotSetMemoryPage);

	lineedit_baseaddress = new MyLineEdit(usingstr("$%04x",old_baseaddress=data.baseaddress),NULL);
	lineedit_baseaddress->setMinimumSize(50,TOOLBAR_WIDGET_HEIGHT);
	lineedit_baseaddress->setMaximumSize(70,TOOLBAR_WIDGET_HEIGHT);
	lineedit_baseaddress->setFocusPolicy(Qt::ClickFocus);
	connect(lineedit_baseaddress,&MyLineEdit::returnPressed,this,&MemoryInspector::set_address_from_textedit);
	toolbar->addWidget(lineedit_baseaddress);

	timer->start(1000/20);
}


MemoryInspector::~MemoryInspector()
{
	xlogIn("~MemoryInspector");
	save_settings();
}


void MemoryInspector::saveSettings()
{
	Inspector::saveSettings();
	save_settings();
}

void MemoryInspector::save_settings()
{
	settings.setValue(key_memoryview_datasource(display_mode), data_source);
	settings.setValue(key_memoryview_scrollposition(display_mode), scroll_offset);
	settings.setValue(key_memoryview_ram_page(display_mode), ram_page_idx);
	settings.setValue(key_memoryview_rom_page(display_mode), rom_page_idx);
	settings.setValue(key_memoryview_bytes_per_row(display_mode), bytes_per_row);
	settings.setValue(key_memoryview_rows(display_mode), rows);
	settings.setValue(key_memoryview_size(display_mode), size());
}



// ==============================================================================
//			Helper
// ==============================================================================


QComboBox* MemoryInspector::newComboboxRegister()
{
	combobox_register = new QComboBox(NULL);
	combobox_register->addItems(QStringList()<<"PC"<<"SP"<<"BC"<<"DE"<<"HL"<<"IX"<<"IY");
	combobox_register->setFixedSize(50,TOOLBAR_WIDGET_HEIGHT);
	combobox_register->setFocusPolicy(Qt::NoFocus);
	connect(combobox_register, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
			this, &MemoryInspector::slotSetAddressFromRegister);
	return combobox_register;
}


FourBytes* MemoryInspector::dataReadPtrForOffset(int32 offset)
{
	assert(offset>=0 && offset < data.size);

	return (FourBytes*)(data_source==AsSeenByCpu
			? machine->cpu->rdPtr(offset)
			: data_source==RomPages || data_source==AllRom
				? &machine->rom[offset]
				: &machine->ram[offset]);
}


// pass mouse wheel event to scrollbar:
void MemoryInspector::wheelEvent(QWheelEvent* e)
{
	scrollbar->wheelEvent(e);
}


// virtual: Qt
// resize head_widget
// reposition and resize data_widget
// data_widget contents must be updated by subclass
void MemoryInspector::resizeEvent(QResizeEvent* e)
{
	xlogIn("MemoryInspector.resizeEvent");

	Inspector::resizeEvent(e);
	scrollbar->setGeometry(width()-scrollbar_width, TOP_MARGIN, scrollbar_width, height()-VERT_MARGINS);
	updateAll();
}


//	Prüfe, ob Speicher wurde zur Laufzeit hinzugefügt / entfernt / geändert.
//	Wir prüfen, ob es die angezeigte Memory Page noch gibt
//	und ob Adresse und Länge noch stimmen.
//
void MemoryInspector::slotMemoryConfigChanged(Memory*, uint /*how*/)
{
	xlogIn("MemoryInspector.slotMemoryConfigChanged");
	if(!machine||!object) return;

	switch(data_source)
	{
	case RomPages:	init_combobox_memorypage(); data = romPage(rom_page_idx); break;
	case RamPages:	init_combobox_memorypage(); data = ramPage(ram_page_idx); break;
	case AllRom:	data.size = machine->rom.count(); break;
	case AllRam:	data.size = machine->ram.count(); break;
	case AsSeenByCpu: break;
	}

	updateAll();
	setScrollOffset(scroll_offset % data.size);
	emit signalSizeConstraintsChanged();
}


// slot: called from textedit_baseaddress
void MemoryInspector::set_address_from_textedit()
{
	xlogIn("MemoryInspector.setAddressFromTextEdit");

	int32 baseaddress = intValue(lineedit_baseaddress->text());
	if(errno==ok) setScrollOffset(baseaddress-data.baseaddress);
	else lineedit_baseaddress->setText(lineedit_baseaddress->oldText());
}


// virtual: set display base address
// note: may be called for data source change => don't fast quit if new addr == old addr
void MemoryInspector::setScrollOffset(int32 new_scrolloffset)
{
	xlogIn("MemoryInspector.setScrollOffset");

	limit( 0, new_scrolloffset, data.size-2*bytes_per_row );
	if(needs_aligned_addresses) new_scrolloffset -= new_scrolloffset % bytes_per_row;
	scroll_offset = new_scrolloffset;
	updateScrollbar();
}


// slot for scrollbar:
void MemoryInspector::slotSetScrollPosition(int32 row)
{
	xlogIn("MemoryInspector.setScrollPosition");

	int current_base_row = (scroll_offset+bytes_per_row-1) / bytes_per_row;

	setScrollOffset(scroll_offset + (row - current_base_row) * bytes_per_row);
}


//helper:
void MemoryInspector::updateScrollbar()
{
	xlogIn("MemoryInspector.updateScrollbar");

	int total_rows = (data.size + scroll_offset%bytes_per_row + bytes_per_row-1) / bytes_per_row;

	scrollbar->blockSignals(true);
		scrollbar->setMinimum(0);
		scrollbar->setMaximum(total_rows - rows);
		scrollbar->setPageStep(rows);
		scrollbar->setSingleStep(max(1,rows/16));
		scrollbar->setValue((scroll_offset+bytes_per_row-1)/bytes_per_row);
	scrollbar->blockSignals(false);
}


//slot for combobox_register
void MemoryInspector::slotSetAddressFromRegister(int reg)
{
	xlogIn("MemoryInspector.slotSetAddressFromRegister");

	uint address;
	Z80Regs& registers = machine->cpu->getRegisters();

	switch(reg)
	{
	default: IERR();
	case regPC:	address = registers.pc; break;
	case regSP:	address = registers.sp; break;
	case regBC:	address = registers.bc; break;
	case regDE: address = registers.de; break;
	case regHL: address = registers.hl; break;
	case regIX: address = registers.ix; break;
	case regIY: address = registers.iy; break;
	}

	// AsSeenByCpu:
	if(data_source==AsSeenByCpu)
	{
		assert(data.size==64 kB);
		setScrollOffset(address-data.baseaddress);
		return;
	}

	// All/Ram/Pages oder All/Rom/Pages:

	CoreByte* rdptr = machine->cpu->rdPtr(address);

	if(rdptr >= machine->ram.getData() && rdptr < machine->ram.getData()+machine->ram.count())
	{
		if(data_source==RomPages) combobox_datasource->setCurrentIndex(RamPages);
		if(data_source==AllRom)	  combobox_datasource->setCurrentIndex(AllRam);
		if(data_source==RamPages) combobox_memorypage->setCurrentIndex(ramPageIndexForCpuAddress(address));
		setScrollOffset(rdptr - &machine->ram[0]);
	}
	else if(rdptr >= machine->rom.getData() && rdptr < machine->rom.getData()+machine->rom.count())
	{
		if(data_source==RamPages) combobox_datasource->setCurrentIndex(RomPages);
		if(data_source==AllRam)	  combobox_datasource->setCurrentIndex(AllRom);
		if(data_source==RomPages) combobox_memorypage->setCurrentIndex(romPageIndexForCpuAddress(address));
		setScrollOffset(rdptr - &machine->rom[0]);
	}
	else
	{
		cstr r = combobox_register->itemText(reg).toUtf8().data();
		showInfo("Register %s points to unmapped memory",r);
	}
}


//slot für combobox_datasource:
void MemoryInspector::slotSetDataSource(int newdatasource)
{
	if(newdatasource==data_source) return;

	xlogIn("MemoryInspector.slotSetDataSource");

	data_source = newdatasource;
	init_combobox_memorypage();

	switch(newdatasource)
	{
	default:			IERR();
	case AsSeenByCpu:	data = Page(0,0,64 kB); break;
	case AllRam:		data = Page(0,0,machine->ram.count()); if(data.size<=0xc000) data.baseaddress=0x4000; break;
	case AllRom:		data = Page(0,0,machine->rom.count()); break;
	case RamPages:		data = ramPage(ram_page_idx); assert(data.size); break;
	case RomPages:		data = romPage(rom_page_idx); assert(data.size); break;
	}

	updateAll();
	setScrollOffset(0);
	emit signalSizeConstraintsChanged();
}


void MemoryInspector::init_combobox_memorypage()
{
	xlogIn("MemoryInspector.init_combobox_memorypage");

	combobox_memorypage->clear();

	if(data_source==RomPages)
	{
		action_memorypage->setVisible(yes);
		for(int i=0;;i++)
		{
			Page page = romPage(i);
			if(page.size==0) break;
			combobox_memorypage->addItem(
				usingstr("Pg %i: $%04X - $%04X", i, page.baseaddress, page.baseaddress+page.size-1));
		}
		if(rom_page_idx>=combobox_memorypage->count()) rom_page_idx = 0;
		combobox_memorypage->setCurrentIndex(rom_page_idx);
	}
	else if(data_source==RamPages)
	{
		action_memorypage->setVisible(yes);
		for(int i=0;;i++)
		{
			Page page = ramPage(i);
			if(page.size==0) break;
			combobox_memorypage->addItem(
				usingstr("Pg %i: $%04X - $%04X", i, page.baseaddress, page.baseaddress+page.size-1));
		}
		if(ram_page_idx>=combobox_memorypage->count()) ram_page_idx = 0;
		combobox_memorypage->setCurrentIndex(ram_page_idx);
	}
	else
	{
		action_memorypage->setVisible(no);
	}
}


// slot für combobox_memorypage:
// diese Funktion ist bei gleichem Index nicht 'abweisend',
// weil sie auch nach Data Source Change oder Memory Config Change aufgerufen werden kann.
void MemoryInspector::slotSetMemoryPage(int newpage)
{
	if(newpage<0) return;	// empty comboBox
	assert(data_source==RamPages||data_source==RomPages);
	if(newpage==(data_source==RamPages?ram_page_idx:rom_page_idx)) return;

	xlogIn("MemoryInspector.slotSetMemoryPage");

	if(data_source==RomPages)
	{
		rom_page_idx = newpage;
		data = romPage(newpage);
	}
	else
	{
		ram_page_idx = newpage;
		data = ramPage(newpage);
	}

	assert(data.size);

	updateAll();
	setScrollOffset(0);
	emit signalSizeConstraintsChanged();
}


void MemoryInspector::updateWidgets()
{
	xxlogIn("MemoryInspector.updateWidgets");

	if(!machine||!object) return;

	if(old_baseaddress != data.baseaddress+scroll_offset)
	{
		old_baseaddress = data.baseaddress+scroll_offset;
		lineedit_baseaddress->setText(usingstr("$%04X",old_baseaddress));
	}

	assert(data_source == combobox_datasource->currentIndex());
	assert(data_source!=RomPages || data.baseoffset == romPage(combobox_memorypage->currentIndex()).baseoffset);
	assert(data_source!=RamPages || data.baseoffset == ramPage(combobox_memorypage->currentIndex()).baseoffset);
}



/*	calculate start and size of ram page #i
*/
Page MemoryInspector::ramPage(int i)
{
	assert(i>=0);

	int32 ramsize = machine->ram.count();

	if(machine->isA(isa_MachineJupiter))	// Jupiter: 3kB + n*16 kB
	{
		assert((ramsize&0x3FFF) == 3 kB);
		if(i==0) return Page(0x2000,0x0000,0x400);
		if(i==1) return Page(0x2800,0x0400,0x400);
		if(i==2) return Page(0x3000,0x0800,0x400);
		if(i==3 && ramsize>=19 kB) return Page(0x4000,0x0C00,0x4000);
		if(i==4 && ramsize>=35 kB) return Page(0x8000,0x4C00,0x4000);
		if(i==5 && ramsize>=51 kB) return Page(0xc000,0x8C00,0x4000);
	}

	else if(ramsize>=64 kB)			// paged memory
	{
		assert((ramsize&0x3fff)==0);
		if(i<ramsize/0x4000) return Page(i*0x4000,i*0x4000,0x4000);
	}

	else if(ramsize>=16 kB)			// ≥ 16 kB: ZX Spectrum or ZX80 etc. with Ram Extension ≥ 16 kB
	{
		assert((ramsize&0x3fff)==0);
		if(i<ramsize/0x4000) return Page(0x4000+i*0x4000,i*0x4000,0x4000);
	}

	else // if(ramsize <= 4 kB)		// ZX80 etc. with ≤ 4kB ram: 1 page only
	{
		if(i==0) return Page(0x4000,0,ramsize);
	}

	return Page();
}


/*	calculate start and size of rom page #i
*/
Page MemoryInspector::romPage(int i)
{
	assert(i>=0);

	int32 romsize = machine->rom.count();

	if(romsize<=16 kB)
	{
		if(i==0) return Page(0,0,romsize);	// 1 page only
	}
	else if(romsize<32 kB)	// TC2068
	{
		if(i==0) return Page(0,0,0x4000);
		if(i==1) return Page(0,0x4000,romsize-0x4000);
	}
	else
	{
		assert((romsize&0x3fff)==0);
		if(i<romsize/0x4000) return Page(0,i*0x4000,0x4000);
	}

	return Page();
}


/*	find ram page index for CPU address a
	returns -1 if address a does not point into ram
	used by setAddressFromRegister()
*/
int MemoryInspector::ramPageIndexForCpuAddress(uint16 a)
{
	CoreByte* rdptr = machine->cpu->rdPtr(a);
	volatile const CoreByte* ram   = &machine->ram[0];

	for(int i=0;;i++)
	{
		Page page = ramPage(i);
		if(page.size==0) return -1;		// nicht im Ram => return -1
		if(rdptr>=ram+page.baseoffset && rdptr<ram+(page.baseoffset+page.size)) return i;
	}
}


/*	find rom page index for CPU address a
	returns -1 if address a does not point into rom
	used by setAddressFromRegister()
*/
int MemoryInspector::romPageIndexForCpuAddress(uint16 a)
{
	CoreByte* rdptr = machine->cpu->rdPtr(a);
	volatile const CoreByte* rom   = &machine->rom[0];

	for(int i=0;;i++)
	{
		Page page = romPage(i);
		if(page.size==0) return -1;		// nicht im Rom => return Page(0,0)
		if(rdptr>=rom+page.baseoffset && rdptr<rom+(page.baseoffset+page.size)) return i;
	}
}


//returns -1 if outside of this page
int32 MemoryInspector::pageOffsetForCpuAddress(uint16 addr)
{
	if(data_source==AsSeenByCpu) return addr;	// 64kB => always inside

	CoreByte* p    = machine->cpu->rdPtr(addr);
	volatile const CoreByte* page = data_source==RamPages || data_source==AllRam ?
					 &machine->ram[data.baseoffset] :
					 &machine->rom[data.baseoffset];

	return p>=page && p<page+data.size ? p-page : -1;
}











































