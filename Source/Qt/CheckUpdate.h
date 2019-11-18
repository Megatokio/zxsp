/*	Copyright  (c)	Günter Woigk 2012 - 2019
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


#ifndef CHECKUPDATE_H
#define CHECKUPDATE_H


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



#endif // CHECKUPDATE_H







