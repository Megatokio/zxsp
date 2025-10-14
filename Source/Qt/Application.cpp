// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Application.h"
#include "CheckUpdate.h"
#include "Files/RzxFile.h"
#include "Files/Z80Head.h"
#include "Machine.h"
#include "MachineController.h"
#include "OS/Dsp.h"
#include "Preferences.h"
#include "Qt/QEventTypes.h"
#include "Qt/Settings.h"
#include "UsbJoystick.h"
#include "WindowMenu.h"
#include "ZxInfo.h"
#include "about_text.h"
#include "kio/util/count1bits.h"
#include "kio/util/msbit.h"
#include "unix/FD.h"
#include "unix/files.h"
#include "unix/log.h"
#include "version.h"
#include "zasm/Source/Z80Assembler.h"
#include <QNetworkAccessManager>
#include <QPainter>
#include <QProxyStyle>
#include <QtGui>
#include <QtPlugin> // req. for static linking
#include <clocale>

#ifdef STATIC
//    Q_IMPORT_PLUGIN(qjpeg)
//    Q_IMPORT_PLUGIN(qgif)
#endif


/*	workaround for not paging scrollbars
	--> https://bugreports.qt-project.org/browse/QTBUG-36314
	should be fixed in Qt 5.3rc1
*/
#if QT_VERSION >= 0x050000 && QT_VERSION < 0x050300
class MyProxyStyle : public QProxyStyle
{
	int styleHint(StyleHint sh, const QStyleOption* opt, const QWidget* w, QStyleHintReturn* hret) const
	{
		int result = QProxyStyle::styleHint(sh, opt, w, hret);
		return sh == QStyle::SH_ScrollBar_LeftClickAbsolutePosition ? 0 : result;
	}
};
#endif


cstr appl_rsrc_path = nullptr; // set by main()


namespace gui
{

// ==========================================================
// global data
//
Application* appl = nullptr;

QString		   Application::filepath;
bool		   Application::is_active_application;
QSplashScreen* Application::about_screen;


// ==========================================================
// application instance, based on QApplication
//
Application::Application(int argc, char* argv[]) : QApplication(argc, argv)
{
	xlogIn("new Application");

	appl				  = this;
	is_active_application = yes;

#if QT_VERSION < 0x050000
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForCStrings(codec); // Qt500: TODO
	QTextCodec::setCodecForTr(codec);		// Qt500: source is assumed to be utf-8. good!
#endif
	//	QTextCodec::setCodecForLocale( codec );

	// required to use default ctor for QSettings:
	QCoreApplication::setOrganizationName("kio");
	QCoreApplication::setOrganizationDomain("k1.spdns.de");
	QCoreApplication::setApplicationName("zxsp");

	// create 'About' == splash screen
	QPixmap	 pm = QPixmap(catstr(appl_rsrc_path, "Backgrounds/about.gif"));
	QPainter painter(&pm);
	painter.setPen(Qt::black);
	QFont font1("Arial", 40, QFont::Black, true /*italic*/);
	int	  x = 210, y = 60;

	painter.setFont(font1);
	painter.drawText(QPoint(x, y), APPL_VERSION_STR);

	if (APPL_VERSION_BETA)
	{
		QFont font2("Arial", 32, QFont::DemiBold, true /*italic*/);
#if QT_VERSION < 0x050b00
		x += QFontMetrics(font1).width(APPL_VERSION_STR) + 5;
#else
		x += QFontMetrics(font1).horizontalAdvance(APPL_VERSION_STR) + 5;
#endif
		painter.setFont(font2);
		painter.drawText(QPoint(x, y), "beta");
	}

	about_screen = new QSplashScreen(pm, Qt::WindowStaysOnTopHint);
	about_screen->showMessage(ABOUT_TEXT, Qt::AlignLeft | Qt::AlignBottom);

	findUsbJoysticks();

	// post 'open file' event if file path is passed as cmd line option (mostly during development)
	for (int i = 1; i < argc; i++)
	{
		if (is_file(argv[i])) QCoreApplication::postEvent(this, new QFileOpenEvent(argv[i]));
	}

	// catch 'open' events for double clicked file:
	processEvents();

	// not started via double click on snapshot file => show splash screen:
	if (filepath.isEmpty())
	{
		about_screen->show();
		QTimer::singleShot(2500, [] {
			about_screen->close();
			if (ne(settings.get_str(key_new_version_info), APPL_VERSION_STR))
			{
				settings.setValue(key_new_version_info, APPL_VERSION_STR);
				showInfo(startup_info_message);
			}
		});
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
	os::startCoreAudio(settings.get_bool(key_warn_if_audio_in_fails, yes));

	// open tool windows if set so in preferences:
	if (settings.get_bool(key_save_and_restore_session, no)) {} // TODO

	// start check for update in background:
	if (settings.get_bool(key_check_for_update, yes) && now() > settings.get_double(key_check_update_timestamp, 0.0))
		checkUpdate(0 /*silent*/);
}


Application::~Application()
{
	xlogIn("~Application()");
	os::stopCoreAudio();
	xlogline(".done");
}

// virtual
bool Application::event(QEvent* e)
{
	xlogIn("Application:event: %s", QEventTypeStr(e->type()));
	switch (int(e->type()))
	{
	case QEvent::Quit: // nur nach Klick auf 'close' button des Fensters, nicht nach CMD-Q !!
		// Dsp::stopCoreAudio();		-->  void MachineController::unlink()
		break;
	case QEvent::FileOpen:
	{
		QFileOpenEvent* fop	  = static_cast<QFileOpenEvent*>(e);
		filepath			  = fop->file();
		MachineController* mc = front_machine_controller;
		if (mc) mc->loadSnapshot(filepath.toUtf8().data()); // else we are in the Application c'tor
		return 1;											// processed
	}
	case QEvent::ApplicationActivate: is_active_application = yes; break;
	case QEvent::ApplicationDeactivate: is_active_application = no; break;
	default: break;
	}
	return QApplication::event(e);
	//	return 0;	// not processed
}

void Application::showPreferences()
{
	xlogIn("Application:showPreferences");

	QWidget*	 parent		 = nullptr;
	QMainWindow* window		 = new QMainWindow(parent, Qt::Tool);
	QWidget*	 preferences = new Preferences(nullptr);
	window->setCentralWidget(preferences);
	window->setWindowTitle("zxsp Settings");
	preferences->setFocus();
	window->setAttribute(Qt::WA_DeleteOnClose, 1);
	window->setFocusPolicy(Qt::StrongFocus);
	window->show();
}

} // namespace gui


// ==========================================================
// main() entry point:
//
int main(int argc, char* argv[])
{
	openLogfile(APPL_NAME, "/var/log/", LogRotation::DAILY, 10, debug, no, yes);
	logline("Welcome to zxsp\n"); // no LogIn() because indentation would last forever

#ifdef _MACOSX
	extern double kCFCoreFoundationVersionNumber; // CFBase.h
	// 299 ~ 10.3
	// 368 ~ 10.4
	// 476 ~ 10.5
	// 744 ~ 10.8 Mountain Lion
	// 855 ~ 10.9 Maverics
	logline("  Core Foundation version: %.2lf", kCFCoreFoundationVersionNumber);
#endif

	logline("  Qt version: %s (0x%06X)", QT_VERSION_STR, QT_VERSION);

	xlogline("  Compiler:   %s", _compiler_str);
	xlogline("  Platform:   %s", _platform_str);
	xlogline("  Processor:  %s", _processor_str);
	xlogline("  Byte order: %s", _byteorder_str);

	xlogline("  bits per byte:      %i", _bits_per_byte);
	xlogline("  sizeof char:        %i", _sizeof_char);
	xlogline("  sizeof short:       %i", _sizeof_short);
	xlogline("  sizeof int:         %i", _sizeof_int);
	xlogline("  sizeof long:        %i", _sizeof_long);
	xlogline("  sizeof pointer:     %i", _sizeof_pointer);
	xlogline("  sizeof double:		%i", _sizeof_double);
	xlogline("  sizeof short float: %i", _sizeof_float);
	xlogline("  sizeof long double: %i", _sizeof_long_double);
	xlogline("  native alignment:   %i", native_alignment);

	// validate byte order:
	const char abcd[5] = "abcd";
	assert(peek4X(abcd) == 'abcd');
	assert(peek4Z(abcd) == 'dcba');

	// appl name:
	xlogline("  appl_name: %s", APPL_NAME);

	// invocation:
	assert(argc);
	cstr appl_path = argv[0];
	assert(is_file(appl_path));
	logline("  appl_path: %s", appl_path);
	if (argc > 1)
	{
		logline("  arguments:");
		for (int i = 1; i < argc; i++) { logline("    %s", argv[i]); }
	}

#if defined(_MACOSX)
	// Resource path:
	cstr appl_bundle_path = substr(appl_path, rfind(appl_path, "Contents/"));
	assert(is_dir(appl_bundle_path));
	logline("  bndl_path: %s", appl_bundle_path);
	appl_rsrc_path = catstr(appl_bundle_path, "Contents/Resources/");
	assert(is_dir(appl_rsrc_path));
	logline("  rsrc_path: %s", appl_rsrc_path);

	// sdcc C Compiler: executable, headers and libraries:
	sdcc_compiler_path = catstr(directory_from_path(appl_path), "sdcc");
	assert(is_file(sdcc_compiler_path));
	sdcc_include_path = catstr(appl_bundle_path, "Contents/sdcc/include/");
	assert(is_dir(sdcc_include_path));
	sdcc_library_path = catstr(appl_bundle_path, "Contents/sdcc/lib/");
	assert(is_dir(sdcc_library_path));

#elif defined(_LINUX)
	// Resource path:
	if (cptr p = find(appl_path, "/build-"))
	{
		appl_rsrc_path = catstr(substr(appl_path, p), "/Resources/");
		assert(is_dir(appl_rsrc_path));
		logline("  rsrc_path: %s", appl_rsrc_path);
	}
	else
	{
		TODO(); // final location in /usr/local/bin/ ?
	}

	// zasm must find the C Compiler sdcc:
	// sdcc_compiler_path = catstr(directory_from_path(appl_path),"sdcc");		assert(is_file(sdcc_compiler_path));
	// sdcc_include_path = catstr(appl_bundle_path,"Contents/sdcc/include/");	assert(is_dir(sdcc_include_path));
	// sdcc_library_path = catstr(appl_bundle_path,"Contents/sdcc/lib/");		assert(is_dir(sdcc_library_path));
#endif

	// version:
	// formerly the version was read from the Info.plist file
	// which was generated by the script makemacstuff.vh
	// but now they are defined in settings.h and  makemacstuff.vh shall read it from settings.h too
	logline("  version: %s", APPL_VERSION_STR);
	logline("  version: %i.%i.%i", APPL_VERSION_H, APPL_VERSION_M, APPL_VERSION_L);
	logNl();

	// *DOIT*
	TempMemPool		 t; // new flushable pool, preserving some temp strings from above
	gui::Application app(argc, argv);
	std::setlocale(LC_ALL, "en_US"); // decimal POINT!  ((bestimmt kann man Application selbst tweaken…))

#if QT_VERSION >= 0x050000 && QT_VERSION < 0x050300
	qApp->setStyle(new MyProxyStyle); // fix for not paging scrollbars
#endif

	try
	{
		return app.exec();
	}
	catch (AnyError& e)
	{
		logline("\nzxsp: Unhandled exception:\n%s (%i)\n", e.what(), e.error());
		return 112; // error
	}
}
