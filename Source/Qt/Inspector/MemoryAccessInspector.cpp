// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MemoryAccessInspector.h"
#include "MemoryInspector.h"
#include <QImage>
#include <QPushButton>
#include "SimpleTerminal.h"
#include <QComboBox>
#include <QBoxLayout>
#include <QWidget>
#include <QCheckBox>
#include <QTimer>
#include "Qt/MyLineEdit.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include <Templates/Array.h>
#include <QMouseEvent>
#include <QToolTip>
#include <Application.h>
#include <QCursor>
#include "MachineController.h"
#include "Qt/Settings.h"
#include "kio/util/msbit.h"
#include <QApplication>
#include <QDesktopWidget>
#include "Templates/NVPtr.h"


// offset mouse pointer hotspot -> 'feeled' hotspot
#define mouse_x_offset	-2
#define mouse_y_offset	-2


static const int MIN_ROWS = 2;
static const int MAX_ROWS = 1000;
static const int MIN_BYTES_PER_ROW = 32;
static const int MAX_BYTES_PER_ROW = 512;
static const int MIN_PIXEL_SIZE = 3;
static const int MAX_PIXEL_SIZE = 5;



// ==================================================================================
// ============================   Graphics Widget   =================================
// ==================================================================================


class GWidgetRGB : public QWidget
{
public:
	QImage*	canvas;
	int		x,y,w;		// highlight position

protected:
VIR void	resizeEvent	(QResizeEvent*);
VIR void	paintEvent	(QPaintEvent*);

public:
			GWidgetRGB	(QWidget*parent, int w, int h);
			~GWidgetRGB	()					{ delete canvas; }
	QRgb*	scanLine	(int r)				{ return (QRgb*)canvas->scanLine(r); }
};

/*	Creator:
	GWidget gleich mit canvas in benötigter Größe erzeugen,
	weil wir das resizeEvent() erst sehr spät kriegen,
	insbes. erst nach Inspector.updateWidgets().
*/
GWidgetRGB::GWidgetRGB(QWidget*parent, int w, int h)
:	QWidget(parent),
	canvas(new QImage(QSize(w,h),QImage::Format_RGB32)),
	w(1)
{}

void GWidgetRGB::resizeEvent(QResizeEvent* e)
{
	if(e->size()==canvas->size()) return;
	delete canvas;
	canvas = new QImage(size(),QImage::Format_RGB32);		// pixel = 0xffRRGGBB
}

void GWidgetRGB::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.drawImage(0,0,*canvas);

	if(uint(x) < uint(width()) && uint(y) < uint(height()))
	{
		p.setPen(Qt::gray);
		p.drawRect(x/w*w, y/w*w, w, w);
	}
}



// ==================================================================================
// =======================   MemoryAccess Inspector   ===============================
// ==================================================================================


MemoryAccessInspector::MemoryAccessInspector(QWidget* parent, MachineController* mc, volatile IsaObject *item)
:
	MemoryInspector(parent,mc,item,MemAccess),
	rom_pixels(machine->rom.count()),
	ram_pixels(machine->ram.count()),
	decay_mode(settings.get_int(key_memoryview_access_decaymode,modeDecayFast)),
	pixel_size(settings.get_int(key_memoryview_access_pixelsize, MIN_PIXEL_SIZE))
{
	xlogIn("new MemoryAccessInspector");

	assert(cpu_r_access==1<<16);
	assert(cpu_w_access==cpu_r_access<<1);
	assert(cpu_x_access==cpu_r_access<<2);
	assert(machine);
	assert(item);

	limit(int(modeFlash), decay_mode, int(modeAccumulate));
	limit(MIN_PIXEL_SIZE,pixel_size,MAX_PIXEL_SIZE);
	validate_bytes_per_row();
	validate_rows();
	validate_scrollposition();

	{	NVPtr<Machine> machine(this->machine);
		CoreByte* data;
		data = machine->ram.getData(); for(int32 i = machine->ram.count();i;) data[--i] |= cpu_access;
		data = machine->rom.getData(); for(int32 i = machine->rom.count();i;) data[--i] |= cpu_access;
	}	machine->cpu_options |= cpu_access;


// resize:
	setFixedWidth(width_for_bytes(bytes_per_row));
	setMinimumHeight(height_for_rows(MIN_ROWS));
	setMaximumHeight(height_for_rows(MAX_ROWS));
	setBaseSize(minimumSize());
	setSizeIncrement(pixel_size,pixel_size);
	resize(width(),height_for_rows(rows));

// child view:
	graphics_view = new GWidgetRGB(this, pixel_size*bytes_per_row+1, pixel_size*rows+1);
	graphics_view->move(LEFT_MARGIN, TOP_MARGIN);

// toolbar:
	assert(toolbar);

	combobox_decaymode = new QComboBox(NULL);
	combobox_decaymode->setFixedSize(75,TOOLBAR_WIDGET_HEIGHT);
	combobox_decaymode->addItems(QStringList()<<"Flash"<<"Decay fast"<<"Decay slow"<<"Accumulate");
	combobox_decaymode->setCurrentIndex(decay_mode);
	combobox_decaymode->setFocusPolicy(Qt::NoFocus);
	bool f = connect(combobox_decaymode,SIGNAL(currentIndexChanged(int)),this,SLOT(slotSetDecayMode(int)));
	toolbar->addWidget(combobox_decaymode);

	combobox_pixelzoom = new QComboBox(NULL);
	combobox_pixelzoom->setFixedSize(65,TOOLBAR_WIDGET_HEIGHT);
	combobox_pixelzoom->addItems(QStringList()<<"2x2"<<"3x3"<<"4x4");	// MIN_PIXEL_SIZE .. MAX_PIXEL_SIZE
	combobox_pixelzoom->setCurrentIndex(pixel_size-MIN_PIXEL_SIZE);
	combobox_pixelzoom->setFocusPolicy(Qt::NoFocus);
	f = f && connect(combobox_pixelzoom,SIGNAL(currentIndexChanged(int)),this,SLOT(slotSetPixelSize(int)));
	toolbar->addWidget(combobox_pixelzoom);

	combobox_bytes_per_row = new QComboBox(NULL);
	combobox_bytes_per_row->setFixedSize(60,TOOLBAR_WIDGET_HEIGHT);
	combobox_bytes_per_row->addItems(QStringList()<<"32"<<"64"<<"128"<<"256"<<"512");	// MIN_BYTES_PER_ROW .. MAX_BYTES_PER_ROW
	combobox_bytes_per_row->setCurrentIndex(msbit(bytes_per_row/MIN_BYTES_PER_ROW));
	combobox_bytes_per_row->setFocusPolicy(Qt::NoFocus);
	f = f && connect(combobox_bytes_per_row,SIGNAL(currentIndexChanged(int)),this,SLOT(slotSetBytesPerRow(int)));
	toolbar->addWidget(combobox_bytes_per_row);

	assert(f);
}


MemoryAccessInspector::~MemoryAccessInspector()
{
	save_settings();
}

void MemoryAccessInspector::saveSettings()
{
	MemoryInspector::saveSettings();
	save_settings();
}

void MemoryAccessInspector::save_settings()
{
	settings.setValue(key_memoryview_access_pixelsize, pixel_size);
	settings.setValue(key_memoryview_access_decaymode, decay_mode);
}



// ==============================================================================
//			Helper
// ==============================================================================


int MemoryAccessInspector::width_for_bytes(int n)
{
	assert(n==1<<msbit(n));

	return HOR_MARGINS + 1 + scrollbar_width + n * pixel_size;
}

int MemoryAccessInspector::height_for_rows(int n)
{
	return VERT_MARGINS + 1 + n * pixel_size;
}

int MemoryAccessInspector::rows_for_height(int h)
{
	return (h-VERT_MARGINS-1) / pixel_size;
}

int MemoryAccessInspector::bytes_for_width(int w)
{
	w -= HOR_MARGINS + 1 + scrollbar_width;
	w = 1 << msbit(w/pixel_size);
	return w;
}

void MemoryAccessInspector::validate_rows()
{
	int maxrows = data.size / bytes_per_row;
	rows = min(rows,maxrows);
	rows = max(MIN_ROWS,rows);
}

void MemoryAccessInspector::validate_bytes_per_row()
{
	bytes_per_row = 1 << msbit(bytes_per_row);
	limit(MIN_BYTES_PER_ROW,bytes_per_row,MAX_BYTES_PER_ROW);
}

void MemoryAccessInspector::validate_scrollposition()
{
	scroll_offset = min(scroll_offset, data.size - rows*bytes_per_row);
	scroll_offset = max(scroll_offset,0);
}


// copy cpu_access bits from memory to buffer
// and clear access bits in memory
void MemoryAccessInspector::copy_access_bits_to_pixels(MemoryPtr mem, Array<QRgb>& dest)
{
	assert(mem.count()==dest.count());

	// color definition for read, write, execute and alpha:
	const  QRgb r = 0x000000ff, w = 0x0000ff00, x = 0x00ff0000, a = 0xff000000;
	static QRgb rgb[] = {0,a+r,a+w,a+r+w,a+x,a+x+r,a+x+w,a+x+r+w};	// bit 2,1,0 = x,w,r

	QRgb* pixels = dest.getData();
	CoreByte* memptr = mem.getData();

	for(int bits,i=mem.count();--i>=0;)
	{
		if((bits = ~memptr[i] & cpu_access))		// wurde eines der Bits seit letztem Mal gelöscht?
		{
			pixels[i] |= rgb[bits/cpu_r_access];	// entsprechend gefärbtes Pixel im Puffer setzen
			memptr[i] |= cpu_access;				// und Bits wieder setzen
		}
	}
}

// prepare buffer for display in fast decay mode:
// fast decaying subtracts 5 and must hit 0 for end condition
void MemoryAccessInspector::prepare_for_fast_decay(Array<QRgb>& mem)
{
	QRgb pixel, *a = mem.getData(), *e = a+mem.count();

	while(a<e)
	{
		if((pixel = *--e))
		{
			QRgb r = ((pixel&0xff0000)>>16)/5*5;
			QRgb g = ((pixel&0x00ff00)>>8) /5*5;
			QRgb b = ((pixel&0x0000ff))    /5*5;
			*e = 0xff000000 + (r<<16) + (g<<8) + b;
		}
	}

}

// decay pixels in buffer:
// color components must be a multiple of 'decay'
// or overfow to other color component will occur
void MemoryAccessInspector::decay_pixel(Array<QRgb>& mem, int decay)
{
	QRgb pixel, *a = mem.getData(), *e = a+mem.count();

	QRgb r = decay<<16;
	QRgb g = decay<<8;
	QRgb b = decay;

	while(a<e)
	{
		if((pixel = *--e))
		{
			if(pixel&0x0000ff) pixel -= b;
			if(pixel&0x00ff00) pixel -= g;
			if(pixel&0xff0000) pixel -= r;
			*e = pixel;
		}
	}
}

// set pixels in Array to 0x00000000 (~ full transparent black)
//
inline void MemoryAccessInspector::clear_pixel(Array<QRgb>& mem)
{
	memset(mem.getData(), 0, mem.count()*sizeof(QRgb));
}

// adjust brightness of color components for display:
// green and red are much brighter than blue and are therefore dimmed
// for a balanced brightness experience
inline QRgb weighted_color(QRgb pixel)
{
	if(pixel&0x00ff0000) pixel -= (pixel&0x00fc0000)/4;							// red		 -1/4
	if(pixel&0x0000ff00) pixel -= (pixel&0x0000fc00)/4 + (pixel&0x0000f000)/16;	// green  ~  -1/3
	return pixel;
}



// ==============================================================================
//			Interface
// ==============================================================================



//	Qt callback: this widget was resized:
//
void MemoryAccessInspector::resizeEvent(QResizeEvent* e)
{
	xlogIn("MemoryAccessInspector::resizeEvent");

	MemoryInspector::resizeEvent(e);

	bytes_per_row = bytes_for_width(width());
	limit(MIN_BYTES_PER_ROW, bytes_per_row, MAX_BYTES_PER_ROW);

	rows = rows_for_height(height());
	limit(MIN_ROWS,rows,MAX_ROWS);

	graphics_view->resize(pixel_size*bytes_per_row+1, pixel_size*rows+1);

	validate_scrollposition();
	updateScrollbar();
	updateWidgets();
}


//	callback from ToolWindow:
//	please check, align and limit size
//	you'll be resized to it!
//
void MemoryAccessInspector::adjustSize(QSize& size)
{
	validate_rows();
	validate_scrollposition();

	setMinimumHeight(height_for_rows(MIN_ROWS));
	setMaximumHeight(height_for_rows(min(MAX_ROWS,data.size/bytes_per_row)));

	size.setWidth(width_for_bytes(bytes_per_row));
	size.setHeight(height_for_rows(rows));
}


//	some memory has been attached / removed / resized
//
void MemoryAccessInspector::slotMemoryConfigChanged(Memory* m, uint how)
{
	xlogIn("MemoryAccessInspector.slotMemoryConfigChanged");
	if(!machine||!object) return;

	rom_pixels.resize(machine->rom.count());
	ram_pixels.resize(machine->ram.count());
	MemoryInspector::slotMemoryConfigChanged(m,how);
}


//	slot for combobox_decaymode
//	argument is index in combobox
//
void MemoryAccessInspector::slotSetDecayMode(int m)
{
	if(m!=decay_mode)
	{
		if(m==modeDecayFast)
		{
			prepare_for_fast_decay(rom_pixels);
			prepare_for_fast_decay(ram_pixels);
		}
		decay_mode = m;
	}
}


//	slot for combobox_pixelzoom
//	argument is index in combobox
//
void MemoryAccessInspector::slotSetPixelSize(int i)
{
	if(MIN_PIXEL_SIZE+i == pixel_size) return;

	pixel_size = MIN_PIXEL_SIZE+i;

	updateAll();
	setFixedWidth(width_for_bytes(bytes_per_row));
	emit signalSizeConstraintsChanged();
}


//	slot for combobox_bytes_per_row
//	argument is index in combobox
//
void MemoryAccessInspector::slotSetBytesPerRow(int i)
{
	if(MIN_BYTES_PER_ROW<<i == bytes_per_row) return;

	bytes_per_row = MIN_BYTES_PER_ROW<<i;
	scroll_offset -= scroll_offset % bytes_per_row;

	updateAll();
	setFixedWidth(width_for_bytes(bytes_per_row));
	emit signalSizeConstraintsChanged();
}


// timer: refresh displayed data
void MemoryAccessInspector::updateWidgets()
{
	if(!machine || !object || !isVisible()) return;

	assert(rom_pixels.count()==machine->rom.count());
	assert(ram_pixels.count()==machine->ram.count());

	// these might happen in a race condition after slotSetXxx:
	if(graphics_view->canvas->width()!=bytes_per_row*pixel_size+1) { xlogline("MemoryAccessInspector::updateWidgets: wrong graphics_view->canvas->width()"); return; }
	if(graphics_view->canvas->height()!=rows*pixel_size+1)		   { xlogline("MemoryAccessInspector::updateWidgets: wrong graphics_view->canvas->height()"); return; }
	if(rows*bytes_per_row+scroll_offset>data.size)				   { xlogline("MemoryAccessInspector::updateWidgets: rows*bytes_per_row+scroll_offset > data.size"); return; }	// <-- happens
	if(uint32(scroll_offset)>uint32(data.size-2*bytes_per_row))    { xlogline("MemoryAccessInspector::updateWidgets: scroll_offset > data.size-2*bytes_per_row"); return; }

// update parent:
	MemoryInspector::updateWidgets();

// fade old pixels according to 'mode' setting:
	switch(decay_mode)
	{
	case modeFlash:		// show only what was accessed since last update
		clear_pixel(ram_pixels);		// pixel=0 => black, no decaying color here,
		clear_pixel(rom_pixels);		// needs not (and must not: alpha=0!) be drawn
		break;
	case modeDecayFast:	// decay previous access
		decay_pixel(rom_pixels,5);
		decay_pixel(ram_pixels,5);
		break;
	case modeDecaySlow:	// decay previous access
		decay_pixel(rom_pixels,1);
		decay_pixel(ram_pixels,1);
		break;
	case modeAccumulate:// don't decay/clear old access
		break;
	}

// update with new access bits:
	{	NVPtr<Machine> machine(this->machine);
		copy_access_bits_to_pixels(machine->rom,rom_pixels);
		copy_access_bits_to_pixels(machine->ram,ram_pixels);
	}

// create map for row -> pixels[]
	Array<QRgb*> pixelrows(rows);
	switch(data_source)
	{
	case AsSeenByCpu:
		{
			const volatile CoreByte* romptr = machine->rom.getData();	int32 romsize = machine->rom.count();
			const volatile CoreByte* ramptr = machine->ram.getData();	int32 ramsize = machine->ram.count();
			Z80* cpu = machine->cpu;
			for(int r = min(rows,(0x10000-scroll_offset)/bytes_per_row); r-- ;)
			{
				CoreByte* p = cpu->rdPtr(scroll_offset+r*bytes_per_row);
				if(p>=romptr&&p<romptr+romsize) pixelrows[r] = rom_pixels.getData() + (p-romptr); else
				if(p>=ramptr&&p<ramptr+ramsize) pixelrows[r] = ram_pixels.getData() + (p-ramptr); // else unmapped cpu address -> NULL
			}
			break;
		}
	case AllRam:
	case RamPages:
		{
			QRgb* p = &ram_pixels[data.baseoffset+scroll_offset];
			int r = min( rows, (data.size-scroll_offset)/bytes_per_row );
			while(r--) { pixelrows[r] = p+r*bytes_per_row; }
			break;
		}
	case AllRom:
	case RomPages:
		{
			QRgb* p = &rom_pixels[data.baseoffset+scroll_offset];
			int r = min( rows, (data.size-scroll_offset)/bytes_per_row );
			while(r--) { pixelrows[r] = p+r*bytes_per_row; }
			break;
		}
	}

// paint pixels into canvas:
	graphics_view->canvas->fill(Qt::black);
	switch(pixel_size)
	{
	case 3:		// 2*2 pixel
		{
			QRgb pixel;
			const int d = 3;
			assert(graphics_view->canvas->width()>=d*bytes_per_row);
			assert(graphics_view->canvas->height()>=d*rows);

			for(int r=0;r<rows;r++)
			{
				QRgb* q = pixelrows[r]; if(q==NULL) continue;
				QRgb* z0 = graphics_view->scanLine(r*d+1) +1;
				QRgb* z1 = graphics_view->scanLine(r*d+2) +1;

				for(int zi=0; zi<d*bytes_per_row; zi+=d)
				{
					if(( pixel = *q++ ))
					{
						z0[zi]   = z1[zi]   =
						z0[zi+1] = z1[zi+1] = weighted_color(pixel);
					}
				}
			}
			break;
		}

	case 4:		// 3*3 pixel
		{
			QRgb pixel;
			const int d = 4;
			assert(graphics_view->canvas->width()>=d*bytes_per_row);
			assert(graphics_view->canvas->height()>=d*rows);

			for( int r=0; r<rows; r++ )
			{
				QRgb* q  = pixelrows[r]; if(q==NULL) continue;
				QRgb* z0 = graphics_view->scanLine(r*d+1) +1;
				QRgb* z1 = graphics_view->scanLine(r*d+2) +1;
				QRgb* z2 = graphics_view->scanLine(r*d+3) +1;

				for(int zi=0;zi<d*bytes_per_row;zi+=d)
				{
					if(( pixel = *q++ ))
					{
						z0[zi]   = z1[zi]   = z2[zi]   =
						z0[zi+1] = z1[zi+1] = z2[zi+1] =
						z0[zi+2] = z1[zi+2] = z2[zi+2] = weighted_color(pixel);
					}
				}
			}
			break;
		}

	case 5:		// 4*4 pixel
		{
			QRgb pixel;
			const int d = 5;
			assert(graphics_view->canvas->width()>=d*bytes_per_row);
			assert(graphics_view->canvas->height()>=d*rows);

			for( int r=0; r<rows; r++ )
			{
				QRgb* q  = pixelrows[r]; if(q==NULL) continue;
				QRgb* z0 = graphics_view->scanLine(r*d+1) +1;
				QRgb* z1 = graphics_view->scanLine(r*d+2) +1;
				QRgb* z2 = graphics_view->scanLine(r*d+3) +1;
				QRgb* z3 = graphics_view->scanLine(r*d+4) +1;

				for(int zi=0;zi<d*bytes_per_row;zi+=d)
				{
					if(( pixel = *q++ ))
					{
						z0[zi]   = z1[zi]   = z2[zi]   = z3[zi]   =
						z0[zi+1] = z1[zi+1] = z2[zi+1] = z3[zi+1] =
						z0[zi+2] = z1[zi+2] = z2[zi+2] = z3[zi+2] =
						z0[zi+3] = z1[zi+3] = z2[zi+3] = z3[zi+3] = weighted_color(pixel);
					}
				}
			}
			break;
		}
	}

// update tooltip:
	updateTooltip();

// paint canvas into graphics widget:
// note: after update_tooltip, because update_tooltip updates graphics_view.x, y and w
	graphics_view->update();
}


void MemoryAccessInspector::updateTooltip()
{
	graphics_view->x = -99;

	// test whether mouse is over this inspector:
	QPoint gpos = QCursor::pos();
	if(QApplication::topLevelAt(gpos)!=window()) return;

	// test whether we are over graphics_view
	// this eliminates hits in the toolbar when it is overlaying portions of the inspector by expanding the ">>" button
	QWidget* top = QApplication::widgetAt(gpos);
	if(top!=graphics_view) return;

	// get the local coordinates
	QPoint pos  = graphics_view->mapFromGlobal(gpos);
	uint x = graphics_view->x = pos.x()+mouse_x_offset;	if(x >= uint(graphics_view->width())) return;
	uint y = graphics_view->y = pos.y()+mouse_y_offset;	if(y >= uint(graphics_view->height())) return;
	graphics_view->w = pixel_size;

	int32 offset = scroll_offset + x/pixel_size + y/pixel_size*bytes_per_row;
	if(offset>=data.size) return;

	uint byte = data_source==AsSeenByCpu
			? machine->cpu->peek(offset)
			: data_source==RomPages || data_source==AllRom
				? uint8(machine->rom[data.baseoffset+offset])
				: uint8(machine->ram[data.baseoffset+offset]);

	QToolTip::showText(gpos, usingstr("$%04X: $%02X",data.baseaddress+offset,byte), graphics_view, QRect());
}




























