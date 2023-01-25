// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QString>
#include <QSettings>
#include "Templates/Array.h"
#include "globals.h"
#include "unix/FD.h"
#include "CheckUpdate.h"
#include "Application.h"
#include "Qt/Settings.h"
#include "cpp/cppthreads.h"
#include "unix/files.h"


static
char check_update_url[] = "http://zxsp.de/cgi-bin/zxsp-check-update.cgi?version=";


//extern
void checkUpdate(bool verbose )
{
	new CheckUpdate(appl,verbose);   // will self-destruct
	settings.setValue(key_check_update_timestamp,now()+12*60*60);
}


// ---------------------------------------------------------------
// Implementation:


CheckUpdate::CheckUpdate(QObject* parent, bool verbose)
:
	QObject(parent),
	verbose(verbose),
	state(dl_filelist),
	filename(nullptr),
	network_manager(nullptr),
	reply(nullptr),
	request()
{
	request.setUrl(QUrl(catstr(check_update_url, APPL_VERSION_STR)));
	network_manager = new QNetworkAccessManager(this);
	IFDEBUG( bool f = ) connect(network_manager, &QNetworkAccessManager::finished, this, &CheckUpdate::slot_finished);
	assert(f);
	reply = network_manager->get(request);
}


CheckUpdate::~CheckUpdate()
{
	delete[] filename; filename = nullptr;
}


//private slot
void CheckUpdate::slot_finished()
{
	xlogIn("CheckUpdate::slotFinished");

	reply->deleteLater();

	if(reply->error() != QNetworkReply::NoError)
	{
		if(verbose) showWarning( "CheckUpdate: error = %s", reply->errorString().toUtf8().data() );
		else xlogline("CheckUpdate: error = %s", reply->errorString().toUtf8().data());
		goto x;
	}

	{	QVariant qv = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
		if((QMetaType::Type)qv.type() == QMetaType::QUrl)
		{
			QUrl url = request.url().resolved(qv.toUrl());
			xlogline("CheckUpdate: redirected to %s",url.toString().toUtf8().data());
			request.setUrl(QUrl(url));
			reply = network_manager->get(request);
			//state = state; dl_filelist or dl_file: both may be redirected
			return;
		}
	}

	{	uint httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
		if(httpstatuscode != 200u/*HTTP_RESPONSE_OK*/)
		{
			if(verbose) showWarning( "CheckUpdate: http status = %u", httpstatuscode );
			else xlogline("CheckUpdate: http status = %u", httpstatuscode);
			goto x;
		}
	}

	if (!reply->isReadable())
	{
		if(verbose) showWarning( "CheckUpdate: reply not readable" );
		else xlogline("CheckUpdate: reply not readable");
		goto x;
	}

	try
	{
		switch(state)
		{
		case dl_filelist:   // step 1: retrieve download url completed
		{
			QByteArray data = reply->readAll();
			if(data.isEmpty()) goto x;	// empty reply => no new version

			cstr url = substr(data.data(), data.data() + data.count());
			if(*url=='<') goto x;		// html error message...

			filename = newcopy(last_component_from_path(url));
			xlogline("CheckUpdate: download new version: %s",filename);
			fd.open_file_n(catstr("~/Desktop/",filename));

			request.setUrl(QUrl(url));
			reply = network_manager->get(request);
			IFDEBUG( bool f = ) connect(reply, &QNetworkReply::readyRead, [this]()
			{
				if(!fd.is_valid()) return;
				QByteArray data = reply->readAll();
				fd.write_bytes(data.data(),data.count());
			});
			assert(f);				// now also readyRead() connected
			state = dl_file;			// state := download file
			return;						// -> slotReadyRead and slotFinished will be called (again)
		}
		case dl_file:       // step 2: download file completed
		{
			QByteArray data = reply->readAll();
			fd.write_bytes(data.data_ptr(),data.count());

			showInfo( "A new version of zxsp has been placed on your desktop:\n\n\"%s\"",filename );
			xlogline("CheckUpdate: New version of zxsp received successfully");
			state = dl_idle;
			break;
		}
		default: break;
		}
	}
	catch(AnyError& e)
	{
		if(verbose) showWarning("CheckUpdate: %s",e.what());
		else xlogline("CheckUpdate: %s",e.what());
	}


x:  deleteLater();  // self-destruct
}






















