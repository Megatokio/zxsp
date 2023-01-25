// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 1
#include <QPainter>
#include <QFont>
#include <QKeyEvent>
#include "Inspector.h"
#include "ToolWindow.h"
#include "Application.h"
#include "Machine.h"
#include "MachineController.h"
#include "ZxInfo.h"
#include "Keyboard.h"
#include "IsaObject.h"
#include <QTimer>
#include <QMenu>
#include "Mouse.h"
#include "Inspector/KempstonJoyInsp.h"
#include "Inspector/ZonxBoxInsp.h"
#include "Inspector/AyInsp.h"
#include "Inspector/UlaInsp.h"
#include "Inspector/IcTesterInsp.h"
#include "Inspector/UlaInsp.h"
#include "Inspector/Z80Insp.h"
#include "Inspector/KeyboardInspector.h"
#include "Inspector/Tc2048JoyInsp.h"
#include "Inspector/SinclairJoyInsp.h"
#include "Inspector/FullerBoxInsp.h"
#include "Inspector/KempstonMouseInsp.h"
#include "Inspector/UlaInsp.h"
#include "Inspector/WalkmanInspector.h"
#include "Inspector/CursorJoyInsp.h"
#include "Inspector/DktronicsDualJoyInsp.h"
#include "Inspector/InvesJoyInsp.h"
#include "Inspector/ZxIf2Insp.h"
#include "Inspector/ZxIf1Insp.h"
#include "Inspector/ZxPrinterInsp.h"
#include "Inspector/DidaktikMelodikInsp.h"
#include "Inspector/FdcBeta128Insp.h"
#include "Inspector/FdcD80Insp.h"
#include "Inspector/FdcJLOInsp.h"
#include "Inspector/FdcPlus3Insp.h"
#include "Inspector/FdcPlusDInsp.h"
#include "Inspector/Tc2068JoyInsp.h"
#include "Inspector/PrinterAercoInsp.h"
#include "Inspector/PrinterLprint3Insp.h"
#include "Inspector/PrinterPlus3Insp.h"
#include "Inspector/PrinterTs2040Insp.h"
#include "Inspector/GrafPadInsp.h"
#include "Inspector/Multiface128Insp.h"
#include "Inspector/Multiface3Insp.h"
#include "Inspector/Multiface1Insp.h"
#include "Inspector/Zx3kInsp.h"
#include "Inspector/Memotech64kRamInsp.h"
#include "Inspector/MachineInspector.h"
#include "Inspector/Machine50x60Inspector.h"
#include "Inspector/MemoryHexInspector.h"
#include "Inspector/MemoryDisassInspector.h"
#include "Inspector/MemoryGraphInspector.h"
#include "Inspector/MemoryAccessInspector.h"
#include "Inspector/Tk85JoyInsp.h"
#include "Inspector/SpectraVideoInspector.h"
#include "Inspector/TccDockInspector.h"
#include "Inspector/DivIDEInspector.h"
#include "Inspector/CurrahMicroSpeechInsp.h"




QLineEdit* Inspector::newLineEdit(cstr text, int min_width)
{
	QLineEdit* te = new QLineEdit(text);
	te->setFont(QFont("Andale Mono",13));
	te->setAlignment(Qt::AlignHCenter);
	te->setReadOnly(yes);
	te->setFrame(off);
	te->setMinimumWidth(min_width);
	return te;
}


Inspector::Inspector(QWidget* w, MachineController* mc, volatile IsaObject *item, cstr bg_file)
:
	QWidget(w),
	controller(mc),
	object(item),
	machine( mc==nullptr ? nullptr : mc->getMachine()),
	background( catstr(appl_rsrc_path,bg_file) ),
	is_visible(false),
	timer(new QTimer(this)),
	contextmenu(new QMenu(this)),
	toolbar(nullptr)
{
	xlogIn("new Inspector for %s",item?item->name:"nullptr");
	assert(machine!=nullptr || object==nullptr);
	this->setFixedSize( background.size() );	// also removes green resize button
	if(item) connect(NV(item), &IsaObject::destroyed, this, [=]{timer->stop(); object=nullptr;});
	if(machine) connect(NV(machine), &Machine::destroyed, this,[=]{timer->stop(); machine=nullptr;});
	//QWidget::setEnabled(true);                // enable mouse & kbd events (true=default)
	setFocusPolicy(Qt::ClickFocus);				// allow clicking "anywhere" to remove focus from input widgets

#if QT_VERSION!=0x050300
// funzt auch in 5.4 nicht: Childs werden immer mal wieder nicht gezeichnet
//	setAttribute(Qt::WA_OpaquePaintEvent,on);   // wir malen alle Pixel. Qt muss nicht vorher löschen.
#endif

	connect(timer,&QTimer::timeout,this,&Inspector::updateWidgets,Qt::AutoConnection);
	//timer->start(1000/10);					// done by child class c'tor if needed
}


Inspector::~Inspector()
{
	xlogIn("~Inspector");
	delete timer; timer=nullptr;
}


//virtual
void Inspector::paintEvent(QPaintEvent*)	// Qt callback
{
	xxlogIn("Inspector:paintEvent");
	QPainter p(this);
	p.drawPixmap(0, 0, width(), height(), background);
}


//virtual
bool Inspector::event(QEvent*e)
{
	xxlogIn("Inspector[%s]:event: %s",object?object->name:"nullptr",QEventTypeStr(e->type()));
	return QWidget::event(e);		// true if processed
}


//virtual
void Inspector::mousePressEvent( QMouseEvent* e )
{
	xlogIn("Inspector:mousePressEvent");
	if(e->button()==Qt::LeftButton) { xlogline("mouse down at %i,%i",e->x(),e->y()); }
	QWidget::mousePressEvent(e);
}


//void Inspector::resizeEvent(QResizeEvent* e)
//{
//	xlogIn("Inspector:resizeEvent");
//	QWidget::resizeEvent(e);
////	if(e->size() != e->oldSize()) emit sizeChanged();
//}


//virtual
void Inspector::contextMenuEvent(QContextMenuEvent* e)
{
	xlogIn("Inspector:contextMenuEvent");
	if(mouse.isGrabbed()) return;		// no context menu if mouse is grabbed

	contextmenu->clear();
	fillContextMenu(contextmenu);

	ToolWindow* toolwindow = dynamic_cast<ToolWindow*>(parent());
	if(toolwindow)
	{
		if(contextmenu->children().count()) contextmenu->addSeparator();
		toolwindow->fillContextMenu(contextmenu);
	}
	else logline("Inspector(%s).contextMenuEvent: parent is not a ToolWindow", this->object->name);

	contextmenu->popup(e->globalPos());
	e->accept();
}



// ###########################################################################################


/*	static Inspector Factory:
*/
Inspector* Inspector::newInspector(QWidget* p, MachineController* mc, volatile IsaObject* item)
{
	assert(isMainThread());
	xlogIn("Inspector::newInspector");

// new empty inspector:
	if(item==nullptr) return new Inspector(p,mc,nullptr);

// new machine inspector:
	volatile Machine* machine = mc->getMachine();
	if(item==machine)
	{
		switch(int(machine->model))
		{
		case jupiter:
		case tk85:
		case tk90x:
		case tk95:
		case zx80:	return new Machine50x60Inspector(p,mc,machine);	// has the 50/60Hz switch
		default:	return new MachineInspector(p,mc,machine);		// plain image
		}
	}

// new item inspector:
	switch(int(item->id))
	{
	case isa_KbdTk95:			return new Tk95KbdInsp(p,mc,item);
	case isa_KbdTk90x:			return new Tk90xKbdInsp(p,mc,item);
	case isa_KbdTs1000:			return new Ts1000KbdInsp(p,mc,item);
	case isa_KbdTs1500:			return new Ts1500KbdInsp(p,mc,item);
	case isa_KbdTk85:			return new Tk85KbdInsp(p,mc,item);
	case isa_KbdJupiter:
	case isa_KbdZx80:
	case isa_KbdZx81:
	case isa_KbdZxsp:
	case isa_KbdZxPlus:
	case isa_KbdTimex:			return new KeyboardInspector(p,mc,item);

	case isa_Z80:				return new Z80Insp(p,mc,item);
	case isa_TS2020:			return new TS2020Inspector(p,mc,item);
	case isa_Plus2Tapedeck:		return new Plus2TapeRecorderInsp(p,mc,item);
	case isa_Plus2aTapedeck:	return new Plus2aTapeRecorderInsp(p,mc,item);
	case isa_Walkman:			return new WalkmanInspector(p,mc,item);

	case isa_MemHex:			return new MemoryHexInspector(p,mc,item);
	case isa_MemDisass:			return new MemoryDisassInspector(p,mc,item);
	case isa_MemGraphical:		return new MemoryGraphInspector(p,mc,item);
	case isa_MemAccess:			return new MemoryAccessInspector(p,mc,item);

	case isa_FdcPlus3:			return new FdcPlus3Insp(p,mc,item);
	case isa_FdcBeta128:		return new FdcBeta128Insp(p,mc,item);
	case isa_FdcPlusD:			return new FdcPlusDInsp(p,mc,item);
	case isa_FdcD80:			return new FdcD80Insp(p,mc,item);
	case isa_FdcJLO:			return new FdcJLOInsp(p,mc,item);

//	case isa_CursorJoy:			return new CursorJoyInsp(p,item);
	case isa_DktronicsDualJoy:  return new DktronicsDualJoyInsp(p,mc,item);
	case isa_InvesJoy:			return new InvesJoyInsp(p,mc,item);
	case isa_KempstonJoy:		return new KempstonJoyInsp(p,mc,item);
	case isa_ProtekJoy:			return new ProtekJoyInsp(p,mc,item);
	case isa_ZxPlus2Joy:		return new SinclairJoyInsp(p,mc,item,"/Images/zxplus2_sideview.jpg");
	case isa_ZxPlus2AJoy:		return new SinclairJoyInsp(p,mc,item,"/Images/zxplus2a_sideview.jpg");
	case isa_ZxPlus3Joy:		return new SinclairJoyInsp(p,mc,item,"/Images/zxplus3_sideview.jpg");
	case isa_Tk90xJoy:			return new SinclairJoyInsp(p,mc,item,"/Images/tk90x_joy.jpg");
	case isa_Tk95Joy:			return new SinclairJoyInsp(p,mc,item,"/Images/tk95_joy.jpg");
	case isa_Tc2048Joy:			return new Tc2048JoyInsp(p,mc,item);
	case isa_Tc2068Joy:			return new Tc2068JoyInsp(p,mc,item,"/Images/tc2068/side_view.jpg");
	case isa_Ts2068Joy:			return new Tc2068JoyInsp(p,mc,item,"/Images/ts2068_side_view.jpg");
	case isa_U2086Joy:			return new Tc2068JoyInsp(p,mc,item,"/Images/u2086/side_view.jpg");
	case isa_Tk85Joy:			return new Tk85JoyInsp(p,mc,item);
	case isa_SpectraVideo:		return new SpectraVideoInspector(p,mc,item);
	case isa_DivIDE:			return new DivIDEInspector(p,mc,item);
	case isa_CurrahMicroSpeech:	return new CurrahMicroSpeechInsp(p,mc,item);

	case isa_PrinterAerco:		return new PrinterAercoInsp(p,mc,item);
	case isa_PrinterLprint3:	return new PrinterLprint3Insp(p,mc,item);
	case isa_PrinterPlus3:		return new PrinterPlus3Insp(p,mc,item);
	case isa_PrinterTs2040:		return new PrinterTs2040Insp(p,mc,item);
	case isa_ZxPrinter:			return new ZxPrinterInsp(p,mc,item);

	case isa_InternalAy:		return new AyInsp(p,mc,item);
	case isa_ZonxBox:			return new ZonxBoxInsp(p,mc,item);
	case isa_ZonxBox81:			return new ZonxBoxInsp(p,mc,item);
	case isa_DidaktikMelodik:   return new DidaktikMelodikInsp(p,mc,item);

	case isa_Cheetah32kRam:     return new Inspector(p,mc,item,"/Images/cheetah_32k.jpg");
	case isa_Jupiter16kRam:     return new Inspector(p,mc,item,"/Images/jupiter_16k.jpg");
	case isa_Zx16kRam:          return new Inspector(p,mc,item,"/Images/zx16k.jpg");
	case isa_Ts1016Ram:         return new Inspector(p,mc,item,"/Images/ts1016.jpg");
	case isa_Stonechip16kRam:   return new Inspector(p,mc,item,"/Images/stonechip16k.jpg");
	case isa_Memotech16kRam:    return new Inspector(p,mc,item,"/Images/memopak16k.jpg");

	case isa_Memotech64kRam:    return new Memotech64kRamInsp(p,mc,item);
	case isa_Zx3kRam:           return new Zx3kInsp(p,mc,item);

	case isa_FullerBox:			return new FullerBoxInsp(p,mc,item);
	case isa_GrafPad:			return new GrafPadInsp(p,mc,item);
	case isa_IcTester:			return new IcTesterInsp(p,mc,item);

	case isa_KempstonMouse:		return new KempstonMouseInsp(p,mc,item);
	case isa_Multiface1:		return new Multiface1Insp(p,mc,item);
	case isa_Multiface128:		return new Multiface128Insp(p,mc,item);
	case isa_Multiface3:		return new Multiface3Insp(p,mc,item);
	case isa_ZxIf2:				return new ZxIf2Insp(p,mc,item);
	case isa_ZxIf1:				return new ZxIf1Insp(p,mc,item);

	case isa_MmuTc2068:
	case isa_MmuTs2068:
	case isa_MmuU2086:			return new TccDockInspector(p,mc,item);
	}

	switch(int(item->grp_id))
	{
	case isa_Ay:				return new AyInsp(p,mc,item);
	case isa_Ula:				return new UlaInsp(p,mc,machine);
	}

	showAlert("TODO: Inspector::newInspector() for: %s",item->name);
	return new Inspector(p,mc,nullptr);
}









