/*	Copyright  (c)	Günter Woigk 2009 - 2019
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












