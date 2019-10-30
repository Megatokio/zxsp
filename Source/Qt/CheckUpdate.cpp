/*	Copyright  (c)	Günter Woigk 2012 - 2018
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
	filename(NULL),
	network_manager(NULL),
	reply(NULL),
	request()
{
	request.setUrl(QUrl(catstr(check_update_url, appl_version_str)));
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
	catch(any_error& e)
	{
		if(verbose) showWarning("CheckUpdate: %s",e.what());
		else xlogline("CheckUpdate: %s",e.what());
	}


x:  deleteLater();  // self-destruct
}






















