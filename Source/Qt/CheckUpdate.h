#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

extern void checkUpdate  (bool verbose);


// ---------------------------------------------------------
// Implementation, private:


#include <QObject>
#include "kio/kio.h"
#include "unix/FD.h"
#include <QNetworkRequest>
class QNetworkAccessManager;
class QNetworkReply;


class CheckUpdate : public QObject
{
//    Q_OBJECT

	enum DlState { dl_filelist, dl_file, dl_idle };

	bool        verbose;
	DlState     state;
	cstr        filename;
	FD			fd;
	QNetworkAccessManager* network_manager;
	QNetworkReply*  reply;
	QNetworkRequest request;

public:
	CheckUpdate(QObject*, bool verbose);
	~CheckUpdate();

//signals:
//private slots:
private:
	void    slot_finished();
};










