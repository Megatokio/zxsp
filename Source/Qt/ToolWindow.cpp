// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 1
#include "ToolWindow.h"
#include "Inspector/Inspector.h"
#include "Items/Ay/Ay.h"
#include "Items/Fdc/Fdc.h"
#include "Items/Item.h"
#include "Items/Joy/Joy.h"
#include "Items/Keyboard.h"
#include "Items/Printer/Printer.h"
#include "Items/Ula/Mmu.h"
#include "Machine.h"
#include "MachineController.h"
#include "Mouse.h"
#include "Qt/Settings.h"
#include "Templates/NVPtr.h"
#include "Z80/Z80.h"
#include "ZxInfo.h"
#include <QApplication>
#include <QContextMenuEvent>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QMenu>
#include <QSettings>
#include <QTimer>
#include <QToolBar>
#include <QVector>


namespace gui
{

// -------------------------------------------------------
//			c'tor, d'tor etc.
// -------------------------------------------------------

ToolWindow::ToolWindow(MachineController* mc, volatile IsaObject* item, QAction* showaction) :
	QMainWindow(mc, Qt::Tool),
	machine_controller(mc)
{
	xlogIn("new ToolWindow");
	assert(mc != nullptr);

	setAttribute(Qt::WA_DeleteOnClose, 1);
	setAttribute(Qt::WA_ShowWithoutActivating, 1);
	setAttribute(Qt::WA_MacAlwaysShowToolWindow, 0); // TODO: doesn't work (Qt530rc45)

	adjust_size_timer.setSingleShot(true);
	adjust_size_timer.setInterval(1000);
	connect(&adjust_size_timer, &QTimer::timeout, this, &ToolWindow::adjust_window_size);

	init(item, showaction);
	restore_window_position();
}

ToolWindow::~ToolWindow()
{
	xlogIn("~ToolWindow");

	save_window_position();
	kill();
}

void ToolWindow::save_window_position()
{
	settings.setValue(catstr(key_toolwindow_position, tostr(grp_id)), pos());
	if (toolbar_height) settings.setValue(catstr(key_toolwindow_toolbar_height, tostr(grp_id)), toolbar_height);
}

void ToolWindow::restore_window_position()
{
	QPoint p(settings.value(catstr(key_toolwindow_position, tostr(grp_id)), QPoint()).toPoint());
	if (!p.isNull()) move(p);
}

void ToolWindow::set_window_title()
{
	cstr special = inspector->getCustomTitle();
	QMainWindow::setWindowTitle(special ? special : item ? item->name : "Empty inspector");
}

void ToolWindow::kill()
{
	adjust_size_timer.stop();

	if (toolbar)
	{
		removeToolBar(toolbar);		   // hide, but does not delete
		toolbar->setParent(inspector); // keep alive as long as inspector lives because inspector relies on existance of
									   // toolbar widgets
		toolbar		   = nullptr;
		toolbar_height = 0; // forget it
	}

	if (show_action)
	{
		const volatile IsaObject* object = item;
		item							 = nullptr;
		QAction* showaction				 = show_action;
		show_action						 = nullptr;

		if (machine_controller->findToolWindowForItem(object) == nullptr)
		{
			showaction->blockSignals(true);
			showaction->setChecked(off);
			showaction->blockSignals(false);
		}
	}

	// Prefs speichern, damit z.B. nach Modellwechsel der Inspector im alten Zustand geöffnet werden kann:
	inspector->saveSettings();
}

void ToolWindow::init()
{
	// setup the toolwindow with an empty inspector

	item		   = nullptr;
	show_action	   = nullptr;
	toolbar		   = nullptr;
	toolbar_height = 0;
	// grp_id = preserve grp_id

	inspector = new Inspector(this, machine_controller);
	setWindowTitle("Empty inspector");

	setFixedSize(inspector->size());
	setCentralWidget(inspector);
}

void ToolWindow::init(volatile IsaObject* object, QAction* showaction)
{
	assert(object);
	assert(showaction);

	item   = object; // item or machine
	grp_id = item->grp_id;

	show_action = showaction;
	showaction->blockSignals(true);
	showaction->setChecked(on);
	showaction->blockSignals(false);

	inspector = Inspector::newInspector(this, machine_controller, object);
	set_window_title();

	toolbar		   = inspector->toolbar;
	toolbar_height = 0;
	if (toolbar)
	{
		toolbar->setAllowedAreas(Qt::TopToolBarArea);
		connect(toolbar, &QToolBar::topLevelChanged, toolbar, [=] { adjust_size_timer.start(); });
		addToolBar(toolbar);
		setUnifiedTitleAndToolBarOnMac(1);
		toolbar_height = settings.value(catstr(key_toolwindow_toolbar_height, tostr(grp_id)), 38).toInt();
	}

	xlogline(
		"Toolwindow resized acc. to inspector to %i x %i + %i", inspector->width(), inspector->height(),
		toolbar_height);
	setMinimumSize(inspector->minimumSize() + QSize(0, toolbar_height));
	xlogline("ToolWindow min size = %i x %i", minimumWidth(), minimumHeight());
	setMaximumSize(inspector->maximumSize() + QSize(0, toolbar_height));
	xlogline("ToolWindow max size = %i x %i", maximumWidth(), maximumHeight());
	resize(inspector->width(), inspector->height() + toolbar_height);
	xlogline("Toolwindow new size = %i x %i", width(), height());
	xlogline("Inspector  new size = %i x %i", inspector->width(), inspector->height());
	xlogline("ToolWindow: toolbar height = %i", toolbar_height);
	// assert(width()==insp->width() && height()==insp->height()+toolbar_height);	might limited by display size

	setCentralWidget(inspector);
	adjust_size_timer.start();

	connect(inspector, &Inspector::signalSizeConstraintsChanged, this, &ToolWindow::adjust_window_size);
	connect(inspector, &Inspector::updateCustomTitle, this, &ToolWindow::set_window_title); // Tape, Disk, Rom, etc.
}


// -------------------------------------------------------
//			Qt Interface
// -------------------------------------------------------

void ToolWindow::keyPressEvent(QKeyEvent* e)
{
	xlogIn("ToolWindow[%s]:keyPressEvent", item ? item->name : "nullptr");

	machine_controller->keyPressEvent(e);
}

void ToolWindow::keyReleaseEvent(QKeyEvent* e)
{
	xlogIn("ToolWindow[%s]:keyReleaseEvent", item ? item->name : "nullptr");

	machine_controller->keyReleaseEvent(e);
}

void ToolWindow::resizeEvent(QResizeEvent* e)
{
	xlogIn("ToolWindow::resizeEvent: %i,%i -> %i,%i", width(), height(), e->size().width(), e->size().height());
	xlogline(e->spontaneous() ? "SPONTAN" : "NICHT SPONTAN");

	QMainWindow::resizeEvent(e);

	// spontane Events kommen direkt vom OS: idR. während der User das Fenster resizet.
	// dann JETZT maxSize auf Limits für user resize ändern und SPÄTER maxSize auf Größe für Maximize:
	if (e->spontaneous())
	{
		adjust_size_timer.start();

		inspector->adjustMaxSizeDuringResize();
		QSize max_size = inspector->maximumSize();
		//		toolbar_height = !toolbar || toolbar->isFloating() || toolbar->height()>50 ? 0 : toolbar->height();
		if (toolbar_height) max_size.setHeight(max_size.height() + toolbar_height);
		if (maximumSize() != max_size) setMaximumSize(max_size);
	}
}

void ToolWindow::adjust_window_size()
{
	//	adjust window size:
	//	called from adjust_size_timer
	//	called from inspector.signalSizeConstraintsChanged ((queued connection))
	//	ruft inspector->adjustSize(the_size) auf und passt danach min. size, max. size an
	//	und resizet das Fenster auf the_size.
	//	sollte mal sizeIncrement und baseSize gehen, dann auch das.

	xlogIn("ToolWindow::adjust_size");

	QSize new_size = inspector->size();
	inspector->adjustSize(new_size);
	QSize max_size = inspector->maximumSize();
	QSize min_size = inspector->minimumSize();

	toolbar_height = !toolbar || toolbar->isFloating() ? 0 : toolbar->height();
	if (toolbar_height)
	{
		new_size.setHeight(new_size.height() + toolbar_height);
		min_size.setHeight(min_size.height() + toolbar_height);
		max_size.setHeight(max_size.height() + toolbar_height);
	}

	if (maximumSize() != max_size) setMaximumSize(max_size);
	if (minimumSize() != min_size) setMinimumSize(min_size);

	//	if(inspector->sizeIncrement()!=sizeIncrement())
	//		setSizeIncrement(inspector->sizeIncrement());

	//	if(baseSize()!=inspector->baseSize())
	//	{
	//		// caveat: broken Qt!  -->  moves and resizes window! :-((
	//		QPoint p = pos();
	//		setBaseSize(inspector->baseSize());
	//		move(p);
	//	}

	if (size() != new_size)
	{
		xlogline(
			"Toolwindow resized acc. to inspector.adjustSize() to %i x %i", inspector->width(), inspector->height());
		resize(new_size);
	}
}

void ToolWindow::fillContextMenu(QMenu* contextmenu)
{
	//	add items to context menu:
	//	called from this.contextMenuEvent()
	//	called from Inspector.contextMenuEvent()

	xlogIn("ToolWindow:fillContextMenu");

	Machine* machine = NVPtr<Machine>(machine_controller->getMachine());
	QAction* action	 = new QAction("Machine image", contextmenu);
	contextmenu->addAction(action);
	connect(action, &QAction::triggered, this, [=] {
		xlogIn("ToolWindow.contextmenu.showMachineImage");

		kill();
		init(machine, machine_controller->action_showMachineImage);
	});

	for (uint i = 0; i < NELEM(machine_controller->mem); i++)
	{
		IsaObject* item = machine_controller->mem[i];
		action			= new QAction(item->name, contextmenu);
		contextmenu->addAction(action);
		connect(action, &QAction::triggered, this, [=] {
			xlogIn("ToolWindow.contextmenu.showMemoryItem");

			QAction* action = machine_controller->findShowActionForItem(item);
			assert(action != nullptr);

			kill();
			init(item, action);
		});
	}

	for (uint i = 0; i < machine->all_items.count(); i++)
	{
		Item* item = machine->all_items[i].get();
		if (item->grp_id == isa_Mmu) continue; // mmu+ula=ula

		action = new QAction(item->name, contextmenu);
		contextmenu->addAction(action);
		connect(action, &QAction::triggered, this, [=] {
			xlogIn("ToolWindow.contextmenu.showItem");

			QAction* action = machine_controller->findShowActionForItem(item);
			assert(action != nullptr);

			kill();
			init(item, action);
		});
	}
}

void ToolWindow::contextMenuEvent(QContextMenuEvent* e)
{
	//	show context menu:
	//	wird z.Zt. nie aufgerufen, weil der Inspector das gesamte ToolWindow ausfüllt.
	//	Statt dessen wird immer Inspector::contextMenuEvent() aufgerufen,
	//	und wir bekommen nur den Callback zu fillContextMenu().
	//	Aber vielleicht ändert sich das ja mal.

	xlogIn("ToolWindow:contextMenuEvent");

	//	if(mouse.isGrabbed()) return;		// no context menu if mouse is grabbed
	contextmenu.clear();
	fillContextMenu(&contextmenu);
	contextmenu.popup(e->globalPos());
	e->accept();
}

} // namespace gui
