/*	Copyright  (c)	Günter Woigk 2009 - 2018
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


