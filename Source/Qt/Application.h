#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include <QApplication>
#include <QSplashScreen>


namespace gui
{

class Application : public QApplication
{
	Q_DISABLE_COPY(Application)

	static QString		  filepath; // during 'open file'
	static bool			  is_active_application;
	static QSplashScreen* about_screen;

public:
	Application(int, char**);
	~Application() override;

	static bool isActiveApplication() { return is_active_application; }
	static void showPreferences();
	static void showAbout() { about_screen->show(); }

	// void		commitData(QSessionManager&)	{}
	// void		saveState(QSessionManager&)		{}

private:
	virtual bool event(QEvent* e) override;
};


extern Application* appl;

} // namespace gui
