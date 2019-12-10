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

#include <QPalette>
#include <QFileDialog>
#include <QTimer>
#include "kio/kio.h"
#include "zxsp_types.h"
#include "qt_util.h"
#include "util.h"
#include "Settings.h"
#include "unix/files.h"
#include "globals.h"
#include <QAudioFormat>
#include <QAudioDeviceInfo>

/*
QPalette::ColorGroup
	QPalette::Disabled	1
	QPalette::Active	0
	QPalette::Inactive	2
	QPalette::Normal	synonym for Active

QPalette::Window        A general background color.
QPalette::WindowText	A general foreground color.
QPalette::Base          Used mostly as the background color for text entry widgets, but can also be used
						for other painting - such as the background of combobox drop down lists and
						toolbar handles. It is usually white or another light color.
QPalette::AlternateBase	Used as the alternate background color in views with alternating row colors
						(see QAbstractItemView::setAlternatingRowColors()).
QPalette::ToolTipBase	Used as the background color for QToolTip and QWhatsThis.
						Tool tips use the Inactive color group of QPalette, because tool tips are not active windows.
QPalette::ToolTipText	Used as the foreground color for QToolTip and QWhatsThis.
						Tool tips use the Inactive color group of QPalette, because tool tips are not active windows.
QPalette::Text          The foreground color used with Base. This is usually the same as the WindowText,
						in which case it must provide good contrast with Window and Base.
QPalette::Button        The general button background color. This background can be different from Window
						as some styles require a different background color for buttons.
QPalette::ButtonText	A foreground color used with the Button color.
QPalette::BrightText	A text color that is very different from WindowText, and contrasts well with e.g. Dark.
						Typically used for text that needs to be drawn where Text or WindowText would
						give poor contrast, such as on pressed push buttons. Note that text colors can be
						used for things other than just words; text colors are usually used for text,
						but it's quite common to use the text color roles for lines, icons, etc.
*/
void setColors(QWidget* widget, QRgb foregroundcolor, QRgb backgroundcolor)
{


/*	TODO:
	Qt forum:
	QColor color = QColorDialog::getColor(Qt::white, this);
	QPalette palette = ui->label->palette();
	palette.setColor(QPalette::WindowText, color);
	ui->label->setPalette(palette);
*/



// sanity check for alpha channel:
	if((backgroundcolor>>24)==0 && backgroundcolor!=0) backgroundcolor |= 0xff000000;
	if((foregroundcolor>>24)==0) foregroundcolor |= 0xff000000;

// build a palette:
	QPalette palette;
	QBrush brush = QColor(foregroundcolor);
	QBrush brush1 = QColor(backgroundcolor);

	//set text color
	palette.setBrush(QPalette::Active, QPalette::Text, brush);
	palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
	palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
	palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);

	//set background color
	palette.setBrush(QPalette::Active, QPalette::Base, brush1);
	palette.setBrush(QPalette::Inactive, QPalette::Base, brush1);
	palette.setBrush(QPalette::Active, QPalette::Window, brush1);
	palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);

// apply palette to widget:
	widget->setPalette(palette);
}


/*	show "Save File" or "Load File" dialog and return file path.
 *	directory path is taken from and updated to settings.value(dirpath_key)
 *	default filetype is the first filetype in the filter list.
 *	example filter list: "Tapes (*.tap *.tzx);;All Files (*)"
 *
 *	NOTE: z.Zt. muss die Filterliste den Eintrag "All Files (*)" enthalten,
 *		  weil auf diesen als Default zurückgegriffen wird, und nur wenn es einen exact match gibt,
 *		  schaltet auch der angezeigte Filter im File-Dialog um.
 * TODO: Das sollte eigentlich der 1. Eintrag sein
 * TODO: wenn der Filter aus den Settings nicht (mehr) existiert, auf den ersten Eintrag umschalten
 */
static
cstr selectFile(QWidget* parent, cstr headline, cstr filefilterlist, bool isSaveDialog)
{
	assert(filefilterlist != nullptr);

/*	unter OSX klappt das Vorbesetzen des Directory-Pfades nicht.
 *	Deshalb muss der mit setDirectory() erneut gesetzt werden.
 *	Note: Fixed in Qt 5.1.0, broken again, even more broken in Qt 5.2
 *
 *	Deshalb kann man auch die statischen Convenience-Funktionen getOpenFileDialog() etc. nicht benutzen.
 *	Der Qt-Dialog hängt die Default-Filenameextension nicht an. :-(
 */
	QString s;	// keep-alive for the extracted c-strings

	// der settings-key für einen file dialog ergibt sich aus dem Text bis zum 1. "(" in der Filterliste
	cstr key = strchr(filefilterlist,'(');								// file filter key
		 key = key ? substr(filefilterlist,key) : filefilterlist;
		 //if(strchr(key,0)[-1] == ' ') strchr(key,0)[-1] = 0;
		 if(strchr(key,0)[-1] == ' ') key = substr(key,strchr(key,0)-1);
		 key = lowerstr(replacedstr(key,' ','_'));

	cstr ffkey = key_selectFile_filter_(isSaveDialog,key);				// key für gewählten Filter
	cstr vzkey = key_selectFile_directory_(isSaveDialog,key);			// key für gewähltes Verzeichnis

	cstr filter   = settings.get_str(ffkey,"All Files (*)");
	cstr filepath = settings.get_str(vzkey,fullpath("~/Desktop/",yes));

	if(0 && QT_VERSION>=0x050100)
	{
		s = isSaveDialog ?
					QFileDialog::getSaveFileName(parent, headline, filepath, filefilterlist)
				  : QFileDialog::getOpenFileName(parent, headline, filepath, filefilterlist);
		filepath = dupstr(s.toUtf8().data());

		parent->activateWindow();
		if(eq(filepath,"")) return nullptr;
	}
	else
	{
		QFileDialog dialog(parent, headline, filepath, filefilterlist );	// filepath ignored
		dialog.setAcceptMode(isSaveDialog ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen);
		dialog.setFileMode(isSaveDialog ? QFileDialog::AnyFile : QFileDialog::ExistingFile);
		xlogline("set dialog directory to: %s",filepath);
		dialog.setDirectory(filepath);										// ignored
		change_working_dir(filepath);										// <- da landen wir, egal wie, this works.

		xlogline("set file filter to: %s",filter);
		dialog.selectNameFilter(filter);
		bool ok = dialog.exec();

		change_working_dir("/");		// nicht den Fuß im cwd lassen..
		parent->activateWindow();
		if(!ok) return nullptr;

		s = dialog.selectedNameFilter(); filter = s.toUtf8().data();
		settings.setValue(ffkey,filter);
		xlogline("store file filter %s", filter);

		QStringList fileNames = dialog.selectedFiles();
		if(fileNames.count()<1) return nullptr;

		s = fileNames.at(0); filepath = dupstr(s.toUtf8().data());
		if(eq(filepath,"")) return nullptr;
	}

	if(is_dir(filepath))
	{
		showWarning("Qt is broken:\n"
		"You must select \"All files (*)\" from the popup in the file selector box to step into this folder.\n\n"
		"Vote up the bug at:\n"
		"https://bugreports.qt-project.org/browse/QTBUG-34187");
		return nullptr;
	}

	settings.setValue(vzkey, directory_from_path(filepath));
	return filepath;
}

cstr selectLoadFile(QWidget* parent, cstr headline, cstr filefilterlist)
{
	// der settings-key für einen file dialog ergibt sich aus dem Text bis zum 1. "(" in der Filterliste
	assert(filefilterlist != nullptr);

	return selectFile(parent,headline,filefilterlist,no);
}

cstr selectSaveFile(QWidget* parent, cstr headline, cstr filefilterlist)
{
	// der settings-key für einen file dialog ergibt sich aus dem Text bis zum 1. "(" in der Filterliste
	assert(filefilterlist != nullptr);

	return selectFile(parent,headline,filefilterlist,yes);
}


cstr tostr (QAudioFormat::Endian endi) noexcept  // helper
{
	return	endi==QAudioFormat::BigEndian ? "big endian" :
			endi==QAudioFormat::LittleEndian ? "little endian" : "illegal endianess";
}

cstr tostr (QAudioFormat::SampleType styp) noexcept  // helper
{
	return	styp==QAudioFormat::Float ? "float" :
			styp==QAudioFormat::SignedInt ? "signed int" :
			styp==QAudioFormat::UnSignedInt ? "unsigned int" : "unknown";
}

cstr tostr (const QAudioFormat& afmt) noexcept  // helper
{
	// "audio/pcm 44100hz, 16bit, 2ch, signed int (little endian)"

	if (!afmt.isValid()) return "invalid audio format";

	QString codec = afmt.codec();
	cstr endi  = tostr(afmt.byteOrder());
	cstr styp = tostr(afmt.sampleType());
	int cpf = afmt.channelCount();	// channels / frame
	int sps  = afmt.sampleRate();	// samples / sec
	int bps = afmt.sampleSize();	// bits / sample

	return usingstr("%s %ihz, %ibit, %ich, %s (%s)", codec.toUtf8().data(), sps, bps, cpf, styp, endi);

	//logline("byte order = %s", tostr(afmt.byteOrder()));
	//qint32 bytesForDuration(qint64 duration);
	//qint32 bytesForFrames(qint32 frameCount);
	//logline("bytes per frame = %i", afmt.bytesPerFrame());
	//logline("channel count = %i", afmt.channelCount());
	//logline("current codec = %s", afmt.codec().toUtf8().data());
	//qint64 durationForBytes(qint32 bytes);
	//qint64 durationForFrames(qint32 frameCount);
	//qint32 framesForBytes(qint32 byteCount);
	//qint32 framesForDuration(qint64 duration);
	//logline("sample rate = %i", afmt.sampleRate());
	//logline("sample size = %i", afmt.sampleSize());
	//logline("sample type = %s", tostr(afmt.sampleType()));
}

void print_QAudioFormat (cstr title, const QAudioFormat& afmt)
{
	logIn("%s:", title ? title : "QAudioFormat");

	if (!afmt.isValid()) logline("** invalid **");
	logline("byte order = %s", tostr(afmt.byteOrder()));
	//qint32 bytesForDuration(qint64 duration);
	//qint32 bytesForFrames(qint32 frameCount);
	logline("bytes per frame = %i", afmt.bytesPerFrame());
	logline("channel count = %i", afmt.channelCount());
	logline("current codec = %s", afmt.codec().toUtf8().data());
	//qint64 durationForBytes(qint32 bytes);
	//qint64 durationForFrames(qint32 frameCount);
	//qint32 framesForBytes(qint32 byteCount);
	//qint32 framesForDuration(qint64 duration);
	logline("sample rate = %i", afmt.sampleRate());
	logline("sample size = %i", afmt.sampleSize());
	logline("sample type = %s", tostr(afmt.sampleType()));
}

void print_QAudioDeviceInfo (cstr title, const QAudioDeviceInfo& info)
{
	assert(!info.isNull());

	logIn("%s:", title ? title : "QAudioDeviceInfo");

	logline("devcie name = %s",info.deviceName().toUtf8().data());
	cstr s;

	s = "";
	QStringList supported_codecs = info.supportedCodecs();
	if (supported_codecs.size() == 0)
	{
		logline("@@not usable"); return;
	}
	for (int i=0; i<supported_codecs.size();i++)
		s = catstr(s,", ", supported_codecs[i].toUtf8().data());
	logline("supported codecs:       %s", s+2);

	s= "";
	QList<QAudioFormat::Endian> supported_byteorders = info.supportedByteOrders();
	for (int i=0; i<supported_byteorders.size();i++)
		s = catstr(s, ", ", tostr(supported_byteorders[i]));
	logline("supported byte orders:  %s", s+2);

	s = "";
	QList<int> supported_channel_counts = info.supportedChannelCounts();
	for (int i=0; i<supported_channel_counts.size();i++)
		s = catstr(s, ", ", numstr(supported_channel_counts[i]));
	logline("supported num. channel: %s", s+2);

	s = "";
	QList<int> supported_samplerates = info.supportedSampleRates();
	for (int i=0; i<supported_samplerates.size();i++)
		s = catstr(s, ", ", numstr(supported_samplerates[i]));
	logline("supported sample rates: %s", s+2);

	s = "";
	QList<int> supported_samplesizes = info.supportedSampleSizes();
	for (int i=0; i<supported_samplesizes.size();i++)
		s = catstr(s, ", ", numstr(supported_samplesizes[i]));
	logline("supported sample sizes: %s", s+2);

	s = "";
	QList<QAudioFormat::SampleType> supported_sample_types = info.supportedSampleTypes();
	for (int i=0; i<supported_sample_types.size();i++)
		s = catstr(s, ", ", tostr(supported_sample_types[i]));
	logline("supported sample types: %s", s+2);

	//print_QAudioFormat("preferred format:", info.preferredFormat());
	logline("preferred format: %s", tostr(info.preferredFormat()));
}





