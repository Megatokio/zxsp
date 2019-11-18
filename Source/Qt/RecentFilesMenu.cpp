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

#include "RecentFilesMenu.h"
#include "Qt/Settings.h"
#include <QSettings>
#include <QList>
#include "unix/files.h"


/*	Max. files per list:
*/
#define MAX_ENTRIES	20



// *******************************************************************
//						RecentFilesList
// *******************************************************************

/*	Class representing one recent files list:
	- preferences key
	- list of file paths
	- list of menus with this list (usually one)
*/
class RecentFilesList : public QObject
{
public:
	cstr			key;
	QStringList		fpaths;
	QList<RecentFilesMenu*>	menus;

	RecentFilesList(cstr key);
	void clear_list();
	void add_file(QString);
};


/*	Create a recent files list:
	- reads in the preferences
	- removes any disappeared files from the list
	- truncates list to MAX_ENTRIES
*/
RecentFilesList::RecentFilesList(cstr key)
:	key(key),
	fpaths(settings.value(key).toStringList())
{
	for(int i=0;i<fpaths.count();i++) { if(!is_file(fpaths.at(i).toUtf8().data())) fpaths.removeAt(i--); }
	while(fpaths.count()>MAX_ENTRIES) { fpaths.removeLast(); }
}


/*	Clear list
	- clear list
	- write empty list back to file
	- update all menues
*/
void RecentFilesList::clear_list()
{
	fpaths.clear();
	for(int i=0;i<menus.count();i++) { menus.at(i)->clear_menu(); }
	settings.setValue(key,fpaths);
}


/*	Add filepath to list
	- add filepath at start of list
	- truncate list to MAX_ENTRIES
	- update all menues
*/
void RecentFilesList::add_file(QString fpath)
{
	int oldidx = fpaths.indexOf(fpath);
	if(oldidx==0) return;						// ist schon drin und ist auch schon ganz oben
	if(oldidx!=-1) fpaths.removeAt(oldidx);		// von alter position entfernen
	fpaths.insert(0,fpath);						// ganz oben einfügen
	while(fpaths.count()>MAX_ENTRIES) { fpaths.removeLast(); }					// Listenlänge begrenzen
	for(int i=0;i<menus.count();i++) { menus.at(i)->add_file(fpath,oldidx); }	// Menüs informieren
	settings.setValue(key,fpaths);				// settings aktualisieren
}


/*	All known recent files lists:
	Must match enum ListId.
*/
static QList<RecentFilesList*> recent_files = QList<RecentFilesList*>()
	<< new RecentFilesList("recent_files")
	<< new RecentFilesList("recent_plus3_disks")
	<< new RecentFilesList("recent_zxsp_tapes")
	<< new RecentFilesList("recent_zx80_tapes")
	<< new RecentFilesList("recent_zx81_tapes")
	<< new RecentFilesList("recent_ace_tapes")
	<< new RecentFilesList("recent_if2_roms")
	<< new RecentFilesList("recent_tcc_roms")
	<< new RecentFilesList("recent_divide_roms")
	<< new RecentFilesList("recent_divide_disks")
	;


/*	global function: clear recent files list[id]
*/
void clearRecentFiles(ListId id)
{
	recent_files.at(id)->clear_list();
}

/*	global function: Add file to recent files list[id]
*/
void addRecentFile(ListId id, QString fpath)
{
	recent_files.at(id)->add_file(fpath);
}

/*	global function: Get file from recent files list[id]
*/
QString getRecentFile(ListId id, int position)
{
	QStringList& list = recent_files.at(id)->fpaths;
	if(list.count()>position) return list.at(position); else return "";
}


// *******************************************************************
//						RecentFilesMenu
// *******************************************************************


/*	Create a "recent files" QMenu
	id   = recent files list id
	callback = function (closure) to be called to open a recent file

	populates itself with filenames and a "clear menu" entry
	adds itself to it's RecentFilesList's list of menus
*/
RecentFilesMenu::RecentFilesMenu(ListId id, QWidget* owner, std::function<void(cstr)> callback)
:	QMenu(owner),
	_list_id(id),
	_callback(callback)
{
	RecentFilesList* mylist = recent_files.at(id);

	for(int i=0;i<mylist->fpaths.count();i++)
	{
		QString fpath = mylist->fpaths.at(i);
		QString fname = fpath.mid(fpath.lastIndexOf('/')+1);
		QAction* a = new QAction(fname,this);
		connect(a, &QAction::triggered, [=](){_callback(fpath.toUtf8().data());});
		addAction(a);
	}

	addSeparator();
	QAction* a = new QAction("Clear menu",this);
	connect(a,&QAction::triggered,mylist,&RecentFilesList::clear_list);
	addAction(a);

	mylist->menus.append(this);
}


/*	Destructor
	remove myself from my RecentFilesList's list of menus
*/
RecentFilesMenu::~RecentFilesMenu()
{
	RecentFilesList* mylist = recent_files.at(_list_id);
	mylist->menus.removeOne(this);
}

/*	clear this menu
	only leaving the separator and the "clear menu" entry
	called from my RecentFilesList.clear_list()
*/
void RecentFilesMenu::clear_menu()
{
	clear();
	addSeparator();
	QAction* a = new QAction("Clear menu",this);
	connect(a,&QAction::triggered,recent_files.at(_list_id),&RecentFilesList::clear_list);
	addAction(a);
}

/*	Add filename to start of this menu
	truncates list to MAX_ENTRIES
	called from my RecentFilesList.add_file()
*/
void RecentFilesMenu::add_file(QString fpath, int oldidx)
{
	if(oldidx!=-1) removeAction(actions().at(oldidx));

	QString fname = fpath.mid(fpath.lastIndexOf('/')+1);
	QAction* a = new QAction(fname,this);
	connect(a, &QAction::triggered, [=](){_callback(fpath.toUtf8().data());});

	assert(actions().count()!=0);
	insertAction(actions().at(0),a);

	while(actions().count()>MAX_ENTRIES+2) removeAction(actions().at(MAX_ENTRIES));
}















