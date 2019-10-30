/*	Copyright  (c)	Günter Woigk 2009 - 2018
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












