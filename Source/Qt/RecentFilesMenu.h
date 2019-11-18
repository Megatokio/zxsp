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


#ifndef RECENTFILESMENU_H
#define RECENTFILESMENU_H

#include <QMenu>
#include "kio/kio.h"


/*	List of implemented "recent files" lists:
	must match static QList<RecentFiles*> recent_files.		<-- !!!
*/
enum ListId
{
	RecentFiles,
	RecentPlus3Disks,
	RecentZxspTapes,
	RecentZx80Tapes,
	RecentZx81Tapes,
	RecentJupiterTapes,
	RecentIf2Roms,
	RecentTccRoms,
	RecentDivideRoms,
	RecentDivideDisks
};


class RecentFilesMenu : public QMenu
{
//	Q_OBJECT

	friend class RecentFilesList;

	Q_DISABLE_COPY(RecentFilesMenu)

	ListId	_list_id;
	std::function<void(cstr)> _callback;

	void	clear_menu();
	void	add_file(QString, int oldidx);

public:
	RecentFilesMenu(ListId, QWidget* owner, std::function<void(cstr)> open_file);
	~RecentFilesMenu();
};


extern void		addRecentFile(ListId, QString fpath);
extern QString	getRecentFile(ListId, int position=0);
extern void		clearRecentFiles(ListId);



#endif // RECENTFILESMENU_H
