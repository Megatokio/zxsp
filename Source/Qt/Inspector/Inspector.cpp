// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 1
#include "Inspector.h"
#include "Application.h"
#include "Ay/Ay.h"
#include "CurrahMicroSpeech.h"
#include "Fdc/FdcPlus3.h"
#include "Inspector/AyInsp.h"
#include "Inspector/CurrahMicroSpeechInsp.h"
#include "Inspector/CursorJoyInsp.h"
#include "Inspector/DidaktikMelodikInsp.h"
#include "Inspector/DivIDEInspector.h"
#include "Inspector/DktronicsDualJoyInsp.h"
#include "Inspector/FdcBeta128Insp.h"
#include "Inspector/FdcD80Insp.h"
#include "Inspector/FdcJLOInsp.h"
#include "Inspector/FdcPlus3Insp.h"
#include "Inspector/FdcPlusDInsp.h"
#include "Inspector/FullerBoxInsp.h"
#include "Inspector/GrafPadInsp.h"
#include "Inspector/IcTesterInsp.h"
#include "Inspector/InvesJoyInsp.h"
#include "Inspector/KempstonJoyInsp.h"
#include "Inspector/KempstonMouseInsp.h"
#include "Inspector/KeyboardInspector.h"
#include "Inspector/Machine50x60Inspector.h"
#include "Inspector/MachineInspector.h"
#include "Inspector/MemoryAccessInspector.h"
#include "Inspector/MemoryDisassInspector.h"
#include "Inspector/MemoryGraphInspector.h"
#include "Inspector/MemoryHexInspector.h"
#include "Inspector/Memotech64kRamInsp.h"
#include "Inspector/Multiface128Insp.h"
#include "Inspector/Multiface1Insp.h"
#include "Inspector/Multiface3Insp.h"
#include "Inspector/PrinterAercoInsp.h"
#include "Inspector/PrinterLprint3Insp.h"
#include "Inspector/PrinterPlus3Insp.h"
#include "Inspector/PrinterTs2040Insp.h"
#include "Inspector/SinclairJoyInsp.h"
#include "Inspector/SpectraVideoInspector.h"
#include "Inspector/Tc2048JoyInsp.h"
#include "Inspector/Tc2068JoyInsp.h"
#include "Inspector/TccDockInspector.h"
#include "Inspector/Tk85JoyInsp.h"
#include "Inspector/UlaInsp.h"
#include "Inspector/WalkmanInspector.h"
#include "Inspector/Z80Insp.h"
#include "Inspector/ZonxBoxInsp.h"
#include "Inspector/Zx3kInsp.h"
#include "Inspector/ZxIf1Insp.h"
#include "Inspector/ZxIf2Insp.h"
#include "Inspector/ZxPrinterInsp.h"
#include "IsaObject.h"
#include "Machine.h"
#include "MachineController.h"
#include "Mouse.h"
#include "ToolWindow.h"
#include "ZxInfo.h"
#include <QFont>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QTimer>


namespace gui
{

bool Inspector::validReference(volatile Item* item)
{
	// helper for the various Inspector subclasses to assert valid pointers
	// usage: assert(validReference(my_item));

	assert(isMainThread());
	assert(controller->getMachine() == machine);
	return NV(machine)->contains(item);
}

QLineEdit* Inspector::newLineEdit(cstr text, int min_width)
{
	QLineEdit* te = new QLineEdit(text);
	te->setFont(QFont("Andale Mono", 13));
	te->setAlignment(Qt::AlignHCenter);
	te->setReadOnly(yes);
	te->setFrame(off);
	te->setMinimumWidth(min_width);
	return te;
}

Inspector::Inspector(QWidget* p, MachineController* mc, cstr bg_file) :
	QWidget(p),
	controller(mc),
	machine(mc->getMachine()),
	object(nullptr),
	background(catstr(appl_rsrc_path, bg_file))
{
	// empty Inspector

	assert(mc);

	xlogIn("new empty Inspector");

	this->setFixedSize(background.size()); // also removes green resize button
	setFocusPolicy(Qt::NoFocus);
	QWidget::setEnabled(false);
}

Inspector::Inspector(QWidget* w, MachineController* mc, volatile IsaObject* item, cstr bg_file) :
	QWidget(w),
	controller(mc),
	machine(mc->getMachine()),
	object(item),
	background(catstr(appl_rsrc_path, bg_file)),
	is_visible(false),
	timer(new QTimer(this)),
	contextmenu(new QMenu(this)),
	toolbar(nullptr)
{
	assert(mc);
	assert(machine);
	assert(item);

	xlogIn("new Inspector for %s", item->name);

	this->setFixedSize(background.size()); // also removes green resize button

	// QWidget::setEnabled(true);   // enable mouse & kbd events (true=default)
	setFocusPolicy(Qt::ClickFocus); // allow clicking "anywhere" to remove focus from input widgets

#if QT_VERSION != 0x050300
// funzt auch in 5.4 nicht: Childs werden immer mal wieder nicht gezeichnet
//	setAttribute(Qt::WA_OpaquePaintEvent,on);   // wir malen alle Pixel. Qt muss nicht vorher löschen.
#endif

	if (machine)
		connect(
			timer, &QTimer::timeout, this,
			[this] {
				assert(isMainThread());
				if (!is_visible) return;
				if (unlikely(controller->getMachine() != machine))
				{
					// can happen if event sent just before Machine destroyed
					showWarning("Inspector::timer called for invalid Machine");
					//logline("*** Inspector::timer called for invalid Machine ***");
					return;
				}
				if (auto* item = dynamic_cast<volatile Item*>(object)) { assert(NV(machine)->contains(item)); }
				updateWidgets();
			},
			Qt::AutoConnection);

	// timer->start(1000/10);	<-- done by child class c'tor if needed
}

Inspector::~Inspector()
{
	xlogIn("~Inspector");
	//delete timer;
	//timer = nullptr;
}

void Inspector::paintEvent(QPaintEvent*) // Qt callback
{
	xxlogIn("Inspector:paintEvent");
	QPainter p(this);
	p.drawPixmap(0, 0, width(), height(), background);
}

bool Inspector::event(QEvent* e)
{
	xxlogIn("Inspector[%s]:event: %s", object ? object->name : "nullptr", QEventTypeStr(e->type()));
	return QWidget::event(e); // true if processed
}

void Inspector::mousePressEvent(QMouseEvent* e)
{
	xlogIn("Inspector:mousePressEvent");
	if (e->button() == Qt::LeftButton) { xlogline("mouse down at %i,%i", e->x(), e->y()); }
	QWidget::mousePressEvent(e);
}

void Inspector::contextMenuEvent(QContextMenuEvent* e)
{
	xlogIn("Inspector:contextMenuEvent");
	if (mouse.isGrabbed()) return; // no context menu if mouse is grabbed
	if (!contextmenu) return;	   // empty Inspector

	contextmenu->clear();
	fillContextMenu(contextmenu);

	if (auto* toolwindow = dynamic_cast<ToolWindow*>(parent()))
	{
		if (contextmenu->children().count()) contextmenu->addSeparator();
		toolwindow->fillContextMenu(contextmenu);
	}
	else logline("Inspector(%s).contextMenuEvent: parent is not a ToolWindow", object->name);

	contextmenu->popup(e->globalPos());
	e->accept();
}


// ###########################################################################################


#define ITEM(T) &dynamic_cast<volatile T&>(*item)


Inspector* Inspector::newInspector(QWidget* p, MachineController* mc, volatile IsaObject* item)
{
	// static Inspector Factory:

	xlogIn("Inspector::newInspector");
	assert(isMainThread());
	assert(item);
	assert(mc != nullptr);
	volatile Machine* machine = mc->getMachine();
	assert(machine);

	if (!item) return new Inspector(p, mc); // empty Inspector

	// new machine inspector:
	if (item == machine)
	{
		switch (int(machine->model))
		{
		case jupiter:
		case tk85:
		case tk90x:
		case tk95:
		case zx80: return new Machine50x60Inspector(p, mc, machine); // has the 50/60Hz switch
		default: return new MachineInspector(p, mc, machine);		 // plain image
		}
	}

	// new item inspector:
	switch (int(item->id))
	{
	case isa_KbdTk95: return new Tk95KbdInsp(p, mc, ITEM(Keyboard));
	case isa_KbdTk90x: return new Tk90xKbdInsp(p, mc, ITEM(Keyboard));
	case isa_KbdTs1000: return new Ts1000KbdInsp(p, mc, ITEM(Keyboard));
	case isa_KbdTs1500: return new Ts1500KbdInsp(p, mc, ITEM(Keyboard));
	case isa_KbdTk85: return new Tk85KbdInsp(p, mc, ITEM(Keyboard));
	case isa_KbdJupiter:
	case isa_KbdZx80:
	case isa_KbdZx81:
	case isa_KbdZxsp:
	case isa_KbdZxPlus:
	case isa_KbdTimex: return new KeyboardInspector(p, mc, ITEM(Keyboard));

	case isa_Z80: return new Z80Insp(p, mc, ITEM(Z80));

	case isa_TS2020: return new TS2020Inspector(p, mc, ITEM(TS2020));
	case isa_Plus2Tapedeck: return new Plus2TapeRecorderInsp(p, mc, ITEM(Plus2TapeRecorder));
	case isa_Plus2aTapedeck: return new Plus2aTapeRecorderInsp(p, mc, ITEM(Plus2aTapeRecorder));
	case isa_Walkman: return new WalkmanInspector(p, mc, ITEM(Walkman));

	case isa_MemHex: return new MemoryHexInspector(p, mc, item);
	case isa_MemDisass: return new MemoryDisassInspector(p, mc, item);
	case isa_MemGraphical: return new MemoryGraphInspector(p, mc, item);
	case isa_MemAccess: return new MemoryAccessInspector(p, mc, item);

	case isa_FdcPlus3: return new FdcPlus3Insp(p, mc, ITEM(FdcPlus3));
	case isa_FdcBeta128: return new FdcBeta128Insp(p, mc, ITEM(FdcBeta128));
	case isa_FdcPlusD: return new FdcPlusDInsp(p, mc, ITEM(FdcPlusD));
	case isa_FdcD80: return new FdcD80Insp(p, mc, ITEM(FdcD80));
	case isa_FdcJLO: return new FdcJLOInsp(p, mc, ITEM(FdcJLO));

	case isa_CursorJoy: return new CursorJoyInsp(p, mc, ITEM(CursorJoy), "/Backgrounds/light-grey-75.jpg");
	case isa_DktronicsDualJoy: return new DktronicsDualJoyInsp(p, mc, ITEM(DktronicsDualJoy));
	case isa_InvesJoy: return new InvesJoyInsp(p, mc, ITEM(InvesJoy));
	case isa_KempstonJoy: return new KempstonJoyInsp(p, mc, ITEM(KempstonJoy));
	case isa_ProtekJoy: return new ProtekJoyInsp(p, mc, ITEM(ProtekJoy));
	case isa_ZxPlus2Joy: return new SinclairJoyInsp(p, mc, ITEM(SinclairJoy), "/Images/zxplus2_sideview.jpg");
	case isa_ZxPlus2AJoy: return new SinclairJoyInsp(p, mc, ITEM(SinclairJoy), "/Images/zxplus2a_sideview.jpg");
	case isa_ZxPlus3Joy: return new SinclairJoyInsp(p, mc, ITEM(SinclairJoy), "/Images/zxplus3_sideview.jpg");
	case isa_Tk90xJoy: return new SinclairJoyInsp(p, mc, ITEM(SinclairJoy), "/Images/tk90x_joy.jpg");
	case isa_Tk95Joy: return new SinclairJoyInsp(p, mc, ITEM(SinclairJoy), "/Images/tk95_joy.jpg");
	case isa_Tc2048Joy: return new Tc2048JoyInsp(p, mc, ITEM(Tc2048Joy));
	case isa_Tc2068Joy: return new Tc2068JoyInsp(p, mc, ITEM(Tc2068Joy), "/Images/tc2068/side_view.jpg");
	case isa_Ts2068Joy: return new Tc2068JoyInsp(p, mc, ITEM(Tc2068Joy), "/Images/ts2068_side_view.jpg");
	case isa_U2086Joy: return new Tc2068JoyInsp(p, mc, ITEM(Tc2068Joy), "/Images/u2086/side_view.jpg");
	case isa_Tk85Joy: return new Tk85JoyInsp(p, mc, ITEM(Tk85Joy));

	case isa_SpectraVideo: return new SpectraVideoInspector(p, mc, ITEM(SpectraVideo));
	case isa_DivIDE: return new DivIDEInspector(p, mc, ITEM(DivIDE));
	case isa_CurrahMicroSpeech: return new CurrahMicroSpeechInsp(p, mc, ITEM(CurrahMicroSpeech));

	case isa_PrinterAerco: return new PrinterAercoInsp(p, mc, ITEM(PrinterAerco));
	case isa_PrinterLprint3: return new PrinterLprint3Insp(p, mc, ITEM(PrinterLprint3));
	case isa_PrinterPlus3: return new PrinterPlus3Insp(p, mc, ITEM(PrinterPlus3));
	case isa_PrinterTs2040: return new PrinterTs2040Insp(p, mc, ITEM(PrinterTs2040));
	case isa_ZxPrinter: return new ZxPrinterInsp(p, mc, ITEM(ZxPrinter));

	case isa_InternalAy: return new AyInsp(p, mc, ITEM(Ay));
	case isa_ZonxBox: return new ZonxBoxInsp(p, mc, ITEM(Ay));
	case isa_ZonxBox81: return new ZonxBoxInsp(p, mc, ITEM(Ay));
	case isa_DidaktikMelodik: return new DidaktikMelodikInsp(p, mc, ITEM(DidaktikMelodik));

	case isa_Cheetah32kRam: return new Inspector(p, mc, item, "/Images/cheetah_32k.jpg");
	case isa_Jupiter16kRam: return new Inspector(p, mc, item, "/Images/jupiter_16k.jpg");
	case isa_Zx16kRam: return new Inspector(p, mc, item, "/Images/zx16k.jpg");
	case isa_Ts1016Ram: return new Inspector(p, mc, item, "/Images/ts1016.jpg");
	case isa_Stonechip16kRam: return new Inspector(p, mc, item, "/Images/stonechip16k.jpg");
	case isa_Memotech16kRam: return new Inspector(p, mc, item, "/Images/memopak16k.jpg");

	case isa_Memotech64kRam: return new Memotech64kRamInsp(p, mc, ITEM(Memotech64kRam));
	case isa_Zx3kRam: return new Zx3kInsp(p, mc, ITEM(Zx3kRam));

	case isa_FullerBox: return new FullerBoxInsp(p, mc, ITEM(FullerBox));
	case isa_GrafPad: return new GrafPadInsp(p, mc, ITEM(GrafPad));
	case isa_IcTester: return new IcTesterInsp(p, mc, ITEM(IcTester));

	case isa_KempstonMouse: return new KempstonMouseInsp(p, mc, ITEM(KempstonMouse));
	case isa_Multiface1: return new Multiface1Insp(p, mc, ITEM(Multiface1));
	case isa_Multiface128: return new Multiface128Insp(p, mc, ITEM(Multiface128));
	case isa_Multiface3: return new Multiface3Insp(p, mc, ITEM(Multiface3));
	case isa_ZxIf2: return new ZxIf2Insp(p, mc, ITEM(ZxIf2));
	case isa_ZxIf1: return new ZxIf1Insp(p, mc, ITEM(ZxIf1));

	case isa_MmuTc2068:
	case isa_MmuTs2068:
	case isa_MmuU2086: return new TccDockInspector(p, mc, ITEM(MmuTc2068));
	}

	switch (int(item->grp_id))
	{
	case isa_Ay: return new AyInsp(p, mc, &dynamic_cast<volatile Ay&>(*item));
	case isa_Ula: return new UlaInsp(p, mc, ITEM(Ula), &dynamic_cast<volatile Mmu&>(*machine->mmu));
	}

	showAlert("TODO: Inspector::newInspector() for: %s", item->name);
	return new Inspector(p, mc, item);
}

} // namespace gui
