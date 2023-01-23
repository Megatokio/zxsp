// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QMainWindow>
#include "WindowMenu.h"
#include "Application.h"



static uint windows=0;
static uint maxwindows=0;
static WindowMenu** windowlist=0;


static uint find_window(WindowMenu* w)
{
	for(uint i=0;i<windows;i++) { if(w==windowlist[i]) return i; }
	IERR(); 	// not in list
}

static void add_window(WindowMenu* w)
{
	if(windows==maxwindows)
	{
		maxwindows=windows+10;
		WindowMenu** newlist = new WindowMenu*[maxwindows];
		if(windows>0) memcpy(newlist,windowlist,windows*sizeof(WindowMenu*));
		delete[] windowlist; windowlist=newlist;
	}
	windowlist[windows++] = w;
}

static void rm_window(uint i)
{
	assert(i<windows);
	for(windows--;i<windows;i++) { windowlist[i]=windowlist[i+1]; }
}


// --------------------------------------------


WindowMenu::WindowMenu( QMainWindow* w )
: QMenu(tr("Window"),w)
{
	xlogIn("new WindowMenu");
	window = w;
	action = new QAction(w->windowTitle(),this);
	action->setCheckable(1);
//	bool f = connect( action, SIGNAL(triggered()), w, SLOT(activateWindow()) ); assert(f);
	connect( action, &QAction::triggered, w, &QMainWindow::activateWindow );
	separator = addSeparator();

	for( uint i=0; i<windows; i++ ) { addWindow(windowlist[i]->action); }
	add_window(this);
	for( uint i=0; i<windows; i++ ) { windowlist[i]->addWindow(action); }
}


WindowMenu::~WindowMenu()
{
	xlogIn("~WindowMenu");
	for(uint i=0; i<windows; i++) windowlist[i]->rmWindow(action);
	rm_window(find_window(this));
}


void WindowMenu::addWindow( QAction* w )
{
	insertAction(separator,w);
}

void WindowMenu::rmWindow( QAction* w )
{
	removeAction(w);
}


void WindowMenu::checkWindows()
{
	xlogIn("WindowMenu:checkWindows()");
	action->setChecked( window->isActiveWindow() );
}


void WindowMenu::setTitle()
{
	action->setText(window->windowTitle());
}












