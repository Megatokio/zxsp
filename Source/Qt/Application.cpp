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

#include <clocale>
#include <QtPlugin>                 // req. for static linking
#include <QtGui>
#include <QPainter>
#include <QNetworkAccessManager>
#include "unix/files.h"
#include "unix/FD.h"
#include "Dsp.h"
#include "Application.h"
#include "Machine.h"
#include "MachineController.h"
#include "WindowMenu.h"
#include "ZxInfo.h"
#include "Qt/Settings.h"
#include "Files/Z80Head.h"
#include "kio/util/count1bits.h"
#include "kio/util/msbit.h"
#include <QProxyStyle>
#include "zasm/Source/Z80Assembler.h"
#include "about_text.h"
#include "Files/RzxFile.h"
#include "Preferences.h"


#ifdef STATIC
//    Q_IMPORT_PLUGIN(qjpeg)
//    Q_IMPORT_PLUGIN(qgif)
#endif


/*	workaround for not paging scrollbars
	--> https://bugreports.qt-project.org/browse/QTBUG-36314
	should be fixed in Qt 5.3rc1
*/
#if QT_VERSION>=0x050000 && QT_VERSION<0x050300
class MyProxyStyle : public QProxyStyle
{
	int styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *hret) const
	{
		int result = QProxyStyle::styleHint(sh, opt, w, hret);
		return sh == QStyle::SH_ScrollBar_LeftClickAbsolutePosition ? 0 : result;
	}
};
#endif



// ==========================================================
// global data
//
Application* appl			= nullptr;

cstr	appl_path			= nullptr;	// set by main()
cstr	appl_rsrc_path		= nullptr;	// set by main()

QString	Application::filepath;
bool	Application::is_active_application;
QSplashScreen* Application::about_screen;


// ==========================================================
// application instance, based on QApplication
//
Application::Application (int argc, char* argv[]) :
  QApplication(argc,argv)
{
	xlogIn("new Application");

	appl = this;
	is_active_application = yes;

#if QT_VERSION < 0x050000
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForCStrings( codec );	// Qt500: TODO
	QTextCodec::setCodecForTr( codec );			// Qt500: source is assumed to be utf-8. good!
#endif
//	QTextCodec::setCodecForLocale( codec );

	// required to use default ctor for QSettings:
	QCoreApplication::setOrganizationName("kio");
	QCoreApplication::setOrganizationDomain("k1.spdns.de");
	QCoreApplication::setApplicationName("zxsp");

// create 'About' == splash screen
	QPixmap pm = QPixmap(catstr(appl_rsrc_path,"Backgrounds/about.gif"));
	QPainter painter( &pm );
	painter.setPen( Qt::black );
	QFont font1("Arial",40,QFont::Black,true/*italic*/);
	//QFont font2("Arial",32,QFont::DemiBold,true/*italic*/);
	int x=210, y=60;//, x2=x+QFontMetrics(font1).width(APPL_VERSION_STR)+5;

	painter.setFont(font1);
	painter.drawText( QPoint(x,y), APPL_VERSION_STR );

//	painter.setFont(font2);
//	painter.drawText( QPoint(x2,y), appl_pre_version_str );

	about_screen = new QSplashScreen(pm,Qt::WindowStaysOnTopHint);
	about_screen->showMessage(ABOUT_TEXT,Qt::AlignLeft|Qt::AlignBottom);

// post 'open file' event if file path is passed as cmd line option (mostly during development)
	for(int i=1;i<argc;i++)
	{
		if(is_file(argv[i])) QCoreApplication::postEvent(this, new QFileOpenEvent(argv[i]));
	}

// catch 'open' events for double clicked file:
	processEvents();

// not started vie double click on snapshot file => show splash screen:
	if(filepath.isEmpty())
	{
		about_screen->show();
		QTimer::singleShot(2500, about_screen, SLOT(close()));
	}

// create 'recent files' menu:
//	recentFilesMenu = new RecentFilesMenu();
//	action_recentFiles = new RecentFilesMenuAction();
//	recent_files_controller = new RecentFilesController(this);

// create first machine instance:
	new MachineController(filepath);

// load double clicked file:
//	if(!filepath.isEmpty())
//	{
//		processEvents();	// process the queued signals in MachineController.itemAdded()
//		mc->loadSnapshot(filepath.toUtf8().data());
//	}

// start core audio == start running the machine:
	Dsp::startCoreAudio(settings.get_bool(key_warn_if_audio_in_fails,yes));

// open tool windows if set so in preferences:
	if(settings.get_bool(key_save_and_restore_session,no))
	{} //TODO

// start check for update in background:
	if( settings.get_bool(key_check_for_update,yes) &&
		now() > settings.get_double(key_check_update_timestamp,0.0) )
		checkUpdate(0/*silent*/);
}


Application::~Application()
{
	xlogIn("~Application()");
	Dsp::stopCoreAudio();
	xlogline(".done");
}

// virtual
bool Application::event(QEvent*e)
{
	xlogIn("Application:event: %s",QEventTypeStr(e->type()));
	switch(int(e->type()))
	{
	case QEvent::Quit:		// nur nach Klick auf 'close' button des Fensters, nicht nach CMD-Q !!
		//Dsp::stopCoreAudio();		-->  void MachineController::unlink()
		break;
	case QEvent::FileOpen:
		{
			QFileOpenEvent* fop = static_cast<QFileOpenEvent*>(e);
			filepath = fop->file();
			MachineController* mc = front_machine_controller;
			if(mc) mc->loadSnapshot(filepath.toUtf8().data());	// else we are in the Application c'tor
			return 1; // processed
		}
	case QEvent::ApplicationActivate:   is_active_application = yes; break;
	case QEvent::ApplicationDeactivate: is_active_application = no;  break;
	default:
		break;
	}
	return QApplication::event(e);
//	return 0;	// not processed
}



bool cmdKeyDown()
{
#if QT_VERSION < 0x050000
	return QApplication::keyboardModifiers() & Qt::ControlModifier;
#else
	return QGuiApplication::keyboardModifiers() & Qt::ControlModifier;
#endif
}



// ==========================================================
// main() entry point:
//
int main( int argc, char *argv[] )
{
	logline("\nWelcome to zxsp\n");		// no LogIn() because indentation would last forever

#ifdef _MACOSX
	extern double kCFCoreFoundationVersionNumber;	// CFBase.h
	// 299 ~ 10.3
	// 368 ~ 10.4
	// 476 ~ 10.5
	// 744 ~ 10.8 Mountain Lion
	// 855 ~ 10.9 Maverics
	logline("  Core Foundation version: %.2lf",kCFCoreFoundationVersionNumber);
#endif

	logline("  Qt version: %s (0x%06X)",QT_VERSION_STR,QT_VERSION);

	xlogline("  Compiler:   %s",_COMPILER);
	xlogline("  Platform:   %s",_PLATFORM);
	xlogline("  Processor:  %s",_PROCESSOR);
	xlogline("  Byte order: %s",_BYTEORDER);

	xlogline("  bits per byte:      %i", _bits_per_byte);		{ assert(uchar(0xffffffff)==(1<<_bits_per_byte)-1); }
	xlogline("  sizeof char:        %i", _sizeof_char);			{ assert(sizeof(char)==_sizeof_char); }
	xlogline("  sizeof short:       %i", _sizeof_short);		{ assert(sizeof(short)==_sizeof_short); }
	xlogline("  sizeof int:         %i", _sizeof_int);			{ assert(sizeof(int)==_sizeof_int); }
	xlogline("  sizeof long:        %i", _sizeof_long);			{ assert(sizeof(long)==_sizeof_long); }
	xlogline("  sizeof pointer:     %i", _sizeof_pointer);		{ assert(sizeof(char*)==_sizeof_pointer); }

	#ifdef _sizeof_double
	xlogline("  sizeof double:		%i", _sizeof_double);		{ assert(sizeof(double)==_sizeof_double); }
	#endif
	#ifdef _sizeof_float
	xlogline("  sizeof short float: %i", _sizeof_float);		{ assert(sizeof(float)==_sizeof_float); }
	#endif
	#ifdef _sizeof_long_double
	xlogline("  sizeof long double: %i", _sizeof_long_float);	{ assert(sizeof(long double)==_sizeof_long_double); }
	#endif

	xlogline("  max. alignment:     %i", int(_MAX_ALIGNMENT) );
	xlogline("  alignment %s", _ALIGNMENT_REQUIRED?"required!":"not required" );

// validate byte order:
	IFDEBUG( char const abcd[5]="abcd"; )
	assert( peek4X(abcd)=='abcd' );
	assert( peek4Z(abcd)=='dcba' );


// appl name:
	xlogline("  appl_name: %s", APPL_NAME);

// invocation:
	assert(argc);
	appl_path = argv[0];													assert(is_file(appl_path));
	logline("  appl_path: %s",appl_path);
	if( argc>1 ) { logline("  arguments:"); for(int i=1;i<argc;i++) { logline("    %s", argv[i]); } }

#if defined(_MACOSX)
	// Resource path:
	cstr appl_bundle_path = substr(appl_path,rfind(appl_path,"Contents/") ); assert(is_dir(appl_bundle_path));
	logline("  bndl_path: %s",appl_bundle_path);
	appl_rsrc_path = catstr(appl_bundle_path,"Contents/Resources/");		assert(is_dir(appl_rsrc_path));
	logline("  rsrc_path: %s",appl_rsrc_path);

	// sdcc C Compiler: executable, headers and libraries:
	sdcc_compiler_path = catstr(directory_from_path(appl_path),"sdcc");		assert(is_file(sdcc_compiler_path));
	sdcc_include_path = catstr(appl_bundle_path,"Contents/sdcc/include/");	assert(is_dir(sdcc_include_path));
	sdcc_library_path = catstr(appl_bundle_path,"Contents/sdcc/lib/");		assert(is_dir(sdcc_library_path));

#elif defined(_LINUX)
	// Resource path:
	if (cptr p=find(appl_path,"/build-"))
	{
		appl_rsrc_path = catstr(substr(appl_path,p),"/Resources/");		assert(is_dir(appl_rsrc_path));
		logline("  rsrc_path: %s",appl_rsrc_path);
	}
	else
	{
		TODO();		// final location in /usr/local/bin/ ?
	}

	// zasm must find the C Compiler sdcc:
	//sdcc_compiler_path = catstr(directory_from_path(appl_path),"sdcc");		assert(is_file(sdcc_compiler_path));
	//sdcc_include_path = catstr(appl_bundle_path,"Contents/sdcc/include/");	assert(is_dir(sdcc_include_path));
	//sdcc_library_path = catstr(appl_bundle_path,"Contents/sdcc/lib/");		assert(is_dir(sdcc_library_path));
#endif

	// version:
	// formerly the version was read from the Info.plist file
	// which was generated by the script makemacstuff.vh
	// but now they are defined in settings.h and  makemacstuff.vh shall read it from settings.h too
	logline("  version: %s",APPL_VERSION_STR);
	logline("  version: %i.%i.%i", APPL_VERSION_H, APPL_VERSION_M, APPL_VERSION_L);
	logNl();

// *DOIT*
	TempMemPool t;				// new flushable pool, preserving some temp strings from above
	Application app(argc,argv);
	std::setlocale(LC_ALL, "en_US");		// decimal POINT!  ((bestimmt kann man Application selbst tweaken…))

#if QT_VERSION>=0x050000 && QT_VERSION<0x050300
	qApp->setStyle(new MyProxyStyle);		// fix for not paging scrollbars
#endif

	try
	{
		return app.exec();
	}
	catch(any_error& e)
	{
		logline("\nzxsp: Unhandled exception:\n%s (%i)\n", e.what(), e.error);
		return 112; // error
	}
}


void Application::showPreferences()
{
	xlogIn("Application:showPreferences");

	QWidget* parent = nullptr; // front_machine_controller;
	QMainWindow* window = new QMainWindow(parent,Qt::Tool);
	QWidget* preferences = new Preferences(nullptr);
	window->setCentralWidget(preferences);
	window->setWindowTitle("zxsp Settings");
	preferences->setFocus();
	window->setAttribute(Qt::WA_DeleteOnClose,1);
	window->setFocusPolicy(Qt::StrongFocus);
	window->show();
}

























