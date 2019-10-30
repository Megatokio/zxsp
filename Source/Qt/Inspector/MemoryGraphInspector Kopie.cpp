/*	Copyright  (c)	Günter Woigk 2012 - 2015
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

#define SAFE 3
#define LOG 1
#include "MemoryGraphInspector.h"
#include <QPushButton>
#include <QBoxLayout>
#include <QWidget>
#include <QComboBox>
#include "MyLineEdit.h"
#include <QTimer>
#include "Machine.h"
#include <QImage>
#include <QToolTip>
#include "Application.h"


// offset mouse pointer hotspot -> 'feeled' hotspot
#define mouse_x_offset	-1
#define mouse_y_offset	-3


static const int MIN_BYTES_PER_ROW = 1;		// min. bytes per row;  must be even
static const int MAX_BYTES_PER_ROW = 128;
static const int MIN_ROWS = 16;
static const int MAX_ROWS = 2000;


// ==================================================================================
// =========================   B&W Graphics Widget =================================
// ==================================================================================


class GWidget : public QWidget
{
public:
	QImage*			canvas;
	int				x,y;		// highlight position

protected:
	virtual void	resizeEvent(QResizeEvent*);
	virtual void	paintEvent(QPaintEvent*);

public:
			GWidget(QWidget*parent)			:QWidget(parent),canvas(NULL){}
			~GWidget()						{ delete canvas; }
//	void	resize(int w,int h)				{ resize(QSize(w,h)); }
//	void	resize(const QSize&);
	void	resize_canvas(const QSize&);
};

//void GWidget::resize(const QSize& newsize)
//{
//	if(newsize==size()) return;
//	QWidget::resize(newsize);
//	resize_canvas(newsize);
//}

void GWidget::resize_canvas(const QSize& newsize)
{
	if(canvas && newsize==canvas->size()) return;
	delete canvas;
	canvas = new QImage(newsize,QImage::Format_Mono);
	canvas->fill(Qt::black);	// effektiv weiß
}

void GWidget::resizeEvent(QResizeEvent*)
{
	resize_canvas(size());
}

void GWidget::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.drawImage(0,0,*canvas);

	if(x>=0&&y>=0&&x<width()&&y<height())
	{
		uint8 byte = canvas->scanLine(y)[x>>3];
		int x0 = x&~7;

		p.setPen(QColor(180,0,0));	//red
		p.drawLine(x0,y,x0+7,y);

		p.setPen(QColor(0,240,240));	// cyan
		for(int i=0;i<8;i++)
			if((byte<<i)&0x80)
				p.drawPoint(x0+i,y);
	}
}



// ==================================================================================
// ========================   MemoryGraph Inspector   ===============================
// ==================================================================================



MemoryGraphInspector::MemoryGraphInspector( QWidget* parent, IsaObject* item )
:	MemoryInspector(parent,item,MemGraph)
{
	XLogIn("new MemoryGraphInspector");

	validate_bytes_per_row();
	validate_rows();
	validate_scrollposition();

// sub views:
	graphics_view	= new GWidget(this);
	address_view	= new SimpleTerminal(this);
	rh = address_view->line_height = 16;
	cw = address_view->char_width;

// resize:
	setMinimumWidth(width_for_bytes(MIN_BYTES_PER_ROW));
	setMaximumWidth(width_for_bytes(MAX_BYTES_PER_ROW));
	setMinimumHeight(height_for_rows(MIN_ROWS));
	setMaximumHeight(height_for_rows(MAX_ROWS));
	setBaseSize(minimumSize());
	setSizeIncrement(8,1);
	resize(width_for_bytes(bytes_per_row),height_for_rows(rows));

// toolbar
	XXXASSERT(toolbar);
	toolbar->addWidget(newComboboxRegister());

	// Button "32 bytes/Zeile":
	// Buttons und Comboboxes sind vertikal schlecht ausgerichtet
	// und Buttons haben mehr Padding
	// => in eine Kiste einschneiden und ausrichten...
	QWidget* widget32 = new QWidget();
	widget32->setFixedSize(46,TOOLBAR_WIDGET_HEIGHT);
	QPushButton* buttonSet32 = new QPushButton("32",widget32);
	buttonSet32->setFixedSize(50,TOOLBAR_BUTTON_HEIGHT);
	buttonSet32->move(-2,-1);
	bool f = connect(buttonSet32,SIGNAL(clicked()),this,SLOT(slotSet32BytesPerRow()));
	toolbar->addWidget(widget32);

	XXXASSERT(f);
}


int MemoryGraphInspector::width_for_bytes(int n)
{
	return HOR_MARGINS + HOR_SPACING + scrollbar_width + n*8 + 5*cw;
}

int MemoryGraphInspector::bytes_for_width(int w)
{
	w -= HOR_MARGINS + HOR_SPACING + scrollbar_width + 5*cw;	// verteilbarer Raum
	return w / 8;												// Bytes pro Zeile
}

int MemoryGraphInspector::height_for_rows(int n)
{
	return VERT_MARGINS + n;
}

int MemoryGraphInspector::rows_for_height(int h)
{
	return (h-VERT_MARGINS +16/3) / 16*16;
}



void MemoryGraphInspector::validate_rows()
{
	int maxrows = (data.size+bytes_per_row-1) / bytes_per_row;
	rows = min(rows,maxrows);
	limit(MIN_ROWS,rows,MAX_ROWS);
}

void MemoryGraphInspector::validate_bytes_per_row()
{
	limit(MIN_BYTES_PER_ROW,bytes_per_row,MAX_BYTES_PER_ROW);
}

void MemoryGraphInspector::validate_scrollposition()
{
	scroll_offset = min(scroll_offset, data.size - rows*bytes_per_row);
	scroll_offset = max(scroll_offset,0);
}


void MemoryGraphInspector::resizeEvent(QResizeEvent* e)
{
	XXLogIn("MemoryGraphInspector::resizeEvent");

	MemoryInspector::resizeEvent(e);

	bytes_per_row = bytes_for_width(width());			// bytes per row
	limit(MIN_BYTES_PER_ROW,bytes_per_row,MAX_BYTES_PER_ROW);

	rows = rows_for_height(height());
	limit(MIN_ROWS,rows,MAX_ROWS);

	// set child widget sizes:
	int w1 = 5*cw;								// Width of address field --> "$1234"
	int w2 = 8*bytes_per_row;					// Width of Graphics field
	int h1 = (rows+15)/16*16;
	int h2 = rows;
	address_view->setGeometry(LEFT_MARGIN,TOP_MARGIN, w1,h1);
	graphics_view->setGeometry(LEFT_MARGIN+HOR_SPACING+w1,TOP_MARGIN, w2,h2);

	updateScrollbar();
	updateWidgets();
}


void MemoryGraphInspector::showEvent(QShowEvent* e)
{
	MemoryInspector::showEvent(e);
	updateScrollbar();
	updateWidgets();
}


void MemoryGraphInspector::adjustSize(QSize& size)
{
	validate_bytes_per_row();
	validate_rows();
	validate_scrollposition();

	size.setWidth(width_for_bytes(bytes_per_row));
	size.setHeight(height_for_rows(rows));
}


//helper:
void MemoryGraphInspector::updateScrollbar()
{
	int current_base_row   = (scroll_offset+bytes_per_row-1) / bytes_per_row;
	int total_base_address = scroll_offset - current_base_row * bytes_per_row;
	int total_rows		   = (data.size-total_base_address+bytes_per_row-1) / bytes_per_row;

	scrollbar->blockSignals(true);
		scrollbar->setMinimum(0);
		scrollbar->setMaximum(total_rows - rows);		//LogLine("Scrollbar: total rows  = %i",total_rows - rows);
		scrollbar->setPageStep(rows/rh*rh);				//LogLine("Scrollbar: page step   = %i",rows/rh*rh);
		scrollbar->setSingleStep(max(1,rows/rh/16)*rh);	//LogLine("Scrollbar: single step = %i",rows/rh/16*rh);
		scrollbar->setValue(current_base_row);			//LogLine("Scrollbar: curr. value = %i",current_base_row);
	scrollbar->blockSignals(false);
}


void MemoryGraphInspector::slotSet32BytesPerRow()
{
	XLogIn("MemoryGraphInspector::set32BytesPerRow");

	bytes_per_row = 32;
	setScrollOffset(scroll_offset&~31);
	emit signalSizeConstraintsChanged();
}


void MemoryGraphInspector::updateWidgets()
{
	if(!machine||!object||!isVisible()) return;
	XXXASSERT(graphics_view->canvas);

// Feststellen, ob das Toolwindow sichtbar ist:
//	if(!window()->isVisible()) return;
//	if(!appl->isActiveApplication()) return;
//	if(machine->controller!=front_machine_controller) return;

// update parent:
	MemoryInspector::updateWidgets();

// update address_view:
	address_view->clearScreen();
	int lastrow = min(address_view->height()/rh, ((data.size-scroll_offset+bytes_per_row-1)/bytes_per_row +rh-1)/rh);
	int address = data.baseaddress+scroll_offset;
	for(int row=0; row<lastrow; row++)
	{
		char bu[8] = "$0000";
		sprintf(bu,address>>16?"%05X":"$%04X",address);
		address_view->printAt(row, 0, bu);
		address += bytes_per_row * rh;
	}

// update graphics_view:
	QImage* canvas = graphics_view->canvas;
	canvas->fill(Qt::black);	// effektiv weiß

	if(data_source==AsSeenByCpu)
	{
		Z80* cpu = machine->cpu;
		for(int n,r=0,a=scroll_offset; r<rows && a<data.size; r++,a+=n)
		{
			n = min(bytes_per_row, data.size-a);
			cpu->copyRamToBuffer(a,canvas->scanLine(r),n);
		}
	}
	else
	{
		CoreByte* q = data_source==AllRom || data_source==RomPages ? &machine->rom[data.baseoffset] : &machine->ram[data.baseoffset];
		for(int n,r=0,a=scroll_offset; r<rows && a<data.size; r++,a+=n)
		{
			n = min(bytes_per_row, data.size-a);
			Z80::c2b(q+a,canvas->scanLine(r),n);
		}
	}
	graphics_view->update();

// update tooltip:
	update_tooltip();
}


/*	Tooltip aktualisieren:
	aber nur, wenn die Maus über dem MemoryView hovert
	da QToolTip::showText(…) die App zwangsweise in den Vordergrund (zurück-) bringt.
*/
void MemoryGraphInspector::update_tooltip()
{
	XXLogIn("MemoryGraphInspector::updateTooltip");

	// test whether mouse is over this inspector:
	QPoint gpos = QCursor::pos();
	if(QApplication::topLevelAt(gpos)!=window()) { graphics_view->x=-99; return; }

	// test whether we are over graphics_view
	// this eliminates hits in the toolbar when it is overlaying portions of the inspector by expanding the ">>" button
	QWidget* top = QApplication::widgetAt(gpos);
	if(top!=graphics_view) return;

	// get the local coordinates
	QPoint pos  = graphics_view->mapFromGlobal(gpos);
	uint x = graphics_view->x = pos.x()+mouse_x_offset;	if( x >= (uint)graphics_view->width() ) return;
	uint y = graphics_view->y = pos.y()+mouse_y_offset;	if( y >= (uint)graphics_view->height() ) return;

	int32 offset = scroll_offset + x/8 + y*bytes_per_row;
	if(offset>=data.size) { QToolTip::showText(gpos, NULL, graphics_view, QRect()); graphics_view->x=-99; return; }

	uint byte = data_source==AsSeenByCpu
			? machine->cpu->peek(offset)
			: data_source==RomPages || data_source==AllRom
				? (uint8)machine->rom[data.baseoffset+offset]
				: (uint8)machine->ram[data.baseoffset+offset];

	QToolTip::showText(gpos, usingstr("$%04X: $%02X",data.baseaddress+offset,byte), graphics_view, QRect());
}















