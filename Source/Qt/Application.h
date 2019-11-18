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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QSplashScreen>
#include <QApplication>
#include "kio/kio.h"
#include "globals.h"



class Application : public QApplication
{
	Q_OBJECT
	Q_DISABLE_COPY(Application)

	static QString		  filepath;					// during 'open file'
	static bool			  is_active_application;
	static QSplashScreen* about_screen;

public:

	Application(int,char**);
	~Application();

	static bool	isActiveApplication()			{ return is_active_application; }
	static void	showPreferences();
	static void	showAbout()						{ about_screen->show(); }

	//void		commitData(QSessionManager&)	{}
	//void		saveState(QSessionManager&)		{}

private:
virtual bool    event(QEvent*e);

};


#endif


