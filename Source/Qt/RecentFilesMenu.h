#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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




