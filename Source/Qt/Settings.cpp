// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QAction>
#include <QMenu>
#include "kio/kio.h"
#include "Qt/Settings.h"
#include "unix/FD.h"
#include "Templates/StrArray.h"
#include <QStringList>


//  Vault for all program settings:
Settings settings;





Settings::Settings()
{
	action_gifAnimateBorder = new QAction("Animated border in Gif movies",this);
	action_gifAnimateBorder->setCheckable(true);
	action_gifAnimateBorder->setChecked(value(key_gif_movies_animate_border,yes).toBool());
	connect( action_gifAnimateBorder, &QAction::toggled, this, &Settings::setGifAnimateBorder);
}

Settings::~Settings()
{
}


//private slot:
void Settings::setGifAnimateBorder(bool f)
{
	setValue(key_gif_movies_animate_border,f);
}


/*	get a string value from this settings
	returns a cstring
	either in tempmem
	or the dflt string, which is expected to be temp or const
*/
cstr Settings::get_cstr(cstr key, cstr dflt)
{
	QVariant r = value(key);
	if(r.canConvert(QMetaType::QString)) return dupstr(r.toString().toUtf8().data());
	else return dflt;
}

str Settings::get_str(cstr key, cstr dflt)
{
	QVariant r = value(key);
	if(r.canConvert(QMetaType::QString)) return dupstr(r.toString().toUtf8().data());
	else return dflt ? dupstr(dflt) : NULL;
}

int	Settings::get_int(cstr key, int dflt)
{
	QVariant r = value(key);
	if(r.canConvert(QMetaType::Int)) return r.toInt();
	else return dflt;
}

Model Settings::get_Model(cstr key, Model dflt)
{
	QVariant r = value(key);
	bool ok;
	int n = r.toInt(&ok);
	return ok && n>=0 && n<num_models ? Model(n) : dflt;
}

KbdMode Settings::get_KbdMode(cstr key, KbdMode dflt)
{
	QVariant r = value(key);
	bool ok;
	int n = r.toInt(&ok);
	return ok && n>=0 && n<num_kbdmodes ? KbdMode(n) : dflt;
}



uint Settings::get_uint(cstr key, uint dflt)
{
	QVariant r = value(key);
	if(r.canConvert(QMetaType::UInt)) return r.toUInt();
	else return dflt;
}

bool Settings::get_bool(cstr key, bool dflt)
{
	QVariant r = value(key);
	if(r.canConvert(QMetaType::Bool)) return r.toBool();
	else return dflt;
}

double Settings::get_double(cstr key, double dflt)
{
	QVariant r = value(key);
	if(r.canConvert(QMetaType::Double)) return r.toDouble();
	else return dflt;
}

void Settings::get_QStringList(cstr key, QStringList& qsl)
{
	QVariant r = value(key);
	if(r.canConvert(QMetaType::QStringList))
		qsl = r.toStringList();
}



void Settings::get_StrArray(cstr key, StrArray& sa)
{
	QVariant r = value(key);
	if(r.canConvert(QMetaType::QStringList))
	{
		QStringList qsl = r.toStringList();
		sa.purge();
		for(int i=0;i<qsl.length();i++)
		sa.append(qsl[i].toUtf8().data());
	}
}

void Settings::set_StrArray(cstr key, StrArray& sa)
{
	QStringList qsl;
	for(uint i=0;i<sa.count();i++)
	{
		qsl << sa[i];
	}
	setValue(key,qsl);
}

































