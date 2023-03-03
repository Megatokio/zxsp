// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 2
#include "MachineController.h"
#include "Application.h"
#include "Dialogs/ConfigureKeyboardJoystickDialog.h"
#include "Fdc/DivIDE.h"
#include "Fdc/Fdc.h"
#include "Files/RzxFile.h"
#include "Files/Z80Head.h"
#include "Items/Ay/Ay.h"
#include "Items/Joy/Joy.h"
#include "Items/TapeRecorder.h"
#include "Items/Ula/Mmu.h"
#include "Joy/KempstonJoy.h"
#include "Joy/ZxIf2.h"
#include "Joystick.h"
#include "KempstonMouse.h"
#include "Keyboard.h"
#include "Lenslok.h"
#include "Machine.h"
#include "MachineInves.h"
#include "MachineJupiter.h"
#include "MachinePentagon128.h"
#include "MachineTc2048.h"
#include "MachineTc2068.h"
#include "MachineTk85.h"
#include "MachineTk90x.h"
#include "MachineTk95.h"
#include "MachineTs1000.h"
#include "MachineTs1500.h"
#include "MachineZx128.h"
#include "MachineZx80.h"
#include "MachineZx81.h"
#include "MachineZxPlus2.h"
#include "MachineZxPlus2a.h"
#include "MachineZxPlus3.h"
#include "MachineZxsp.h"
#include "MemObject.h"
#include "Preferences.h"
#include "Printer/ZxPrinter.h"
#include "Qt/Screen/Screen.h"
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "Ram/Zx3kRam.h"
#include "RecentFilesMenu.h"
#include "Screen/ScreenMono.h"
#include "Settings.h"
#include "SpectraVideo.h"
#include "Templates/NVPtr.h"
#include "Templates/RCPtr.h"
#include "ToolWindow.h"
#include "Ula/MmuTc2068.h"
#include "WindowMenu.h"
#include "Z80/Z80.h"
#include "ZxIf1.h"
#include "ZxInfo.h"
#include "ZxInfo/ZxInfo.h"
#include "globals.h"
#include "unix/FD.h"
#include "zasm/Source/Z80Assembler.h"
#include <QAction>
#include <QDesktopWidget>
#include <QEvent>
#include <QGLWidget>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QtGui>


namespace gui
{

MachineController* front_machine_controller = nullptr;

static constexpr int CTRL  = Qt::CTRL; // for use with binary operators wg. Warning
static constexpr int SHIFT = Qt::SHIFT;


// ============================================================
//			Create & Destroy:
// ============================================================

std::shared_ptr<Machine> MachineController::newMachineForModel(Model model)
{
	// create Machine instance for model
	// if ramsize==0 then default ramsize is used.
	// machine.parent := this
	// the machine is not powered on.
	// the machine is not suspended.

	assert(in_machine_ctor);

	std::shared_ptr<Machine> m = Machine::newMachine(this, model);

	switch (model)
	{
	case jupiter: m->set60Hz(settings.get_bool(key_framerate_jupiter_60hz, false)); break;
	case zx80: m->set60Hz(settings.get_bool(key_framerate_zx80_60hz, false)); break;
	case tk85: m->set60Hz(settings.get_bool(key_framerate_tk85_60hz, false)); break;
	case tk90x: m->set60Hz(settings.get_bool(key_framerate_tk90x_60hz, false)); break;
	case tk95: m->set60Hz(settings.get_bool(key_framerate_tk95_60hz, false)); break;
	case zxplus_span: showWarning("spanisch Sinclair ZX Spectrum+: TODO\nusing a Sinclair ZX Spectrum+"); break;
	case scorpion: showWarning("Scorpion: TODO\nusing a Sinclair ZX Spectrum 48k"); break;
	default: break;
	}

	bool auto_start = settings.get_bool(key_auto_start_stop_tape, true);
	bool fast_load	= settings.get_bool(key_fast_load_tape, true);

	m->taperecorder->setAutoStartStopTape(auto_start);
	m->taperecorder->setInstantLoadTape(fast_load);

	return m;
}

Screen* MachineController::newScreenForModel(Model model)
{
	// create Screen instance for model
	// screen.parent := this
	// currently there are ScreenZxsp and ScreenMono

	assert(in_machine_ctor);

	switch (model)
	{
	case zx80:
	case zx81:
	case ts1000:
	case ts1500:
	case tk85:
	case jupiter: return new ScreenMono(this);

	case zxsp_i1:
	case zxsp_i2:
	case zxsp_i3:
	case zxplus:
	case inves:
	case zx128:
	case zx128_span:
	case zxplus2:
	case zxplus2_frz:
	case zxplus2_span:
	case zxplus3:
	case zxplus3_span:
	case zxplus2a:
	case zxplus2a_span:
	case tk90x:
	case tk95:
	case pentagon128:
	case zxplus_span:
	case scorpion: return new ScreenZxsp(this);

	case u2086:
	case tc2048:
	case tc2068:
	case ts2068: return new ScreenZxsp(this, isa_ScreenTc2048);

	case samcoupe:
	case unknown_model:
	case num_models: break;
	}
	IERR();
}

void MachineController::loadSnapshot(cstr filename)
{
	// load snapshot file
	// --> machine powered up and running

	xlogIn("MachineController:loadSnapshot");

	in_machine_ctor = yes;

	cstr org_filename = filename;
	cstr ext		  = lowerstr(extension_from_path(filename));

	bool s = settings.get_bool(key_always_attach_soundchip, yes);
	bool j = settings.get_bool(key_always_attach_joystick, no);
	bool r = settings.get_bool(key_always_attach_rampack, yes);
	bool d = settings.get_bool(key_always_attach_divide, no);

	this->machine->suspend();			  // machine is powered on and suspended
	Machine* machine = NV(this->machine); // we need the machine powered on because we may load helper snapshots

	if (machine->rzxIsRecording() && action_RzxRecordAppendSna->isChecked()) {}
	else
	{
		action_RzxRecord->setChecked(false);
		machine->rzxDispose();
		// screen->removeAllOverlays();
	}
	RzxFile* rzx = nullptr;

	try
	{
		if (eq(ext, ".ass") || eq(ext, ".asm") || eq(ext, ".src") || eq(ext, ".s"))
		{
			filename = fullpath(filename);
			if (errno) throw FileError(filename, errno);

			Z80Assembler ass;

			cstr destdir   = nullptr;	  // nullptr => same as source
			cstr listfile  = nullptr;	  // nullptr => same as dest, ext = ".lst"
			cstr tempdir   = "/tmp/zxsp"; // for temp files: c sources will be assembled into /tmp/zxsp/s/
			int	 liststyle = 6;			  // with opcodes & labels list
			int	 deststyle = 'b';		  // binary
			bool clean	   = no;		  // TODO: clean /tmp/zxsp/ on app start? option in preferences?
			create_dir(tempdir);

			ass.assembleFile(filename, destdir, listfile, tempdir, liststyle, deststyle, clean);

			if (ass.numErrors())
				throw AnyError("your assembly had %i error%s", ass.numErrors(), ass.numErrors() == 1 ? "" : "s");
			filename = dupstr(ass.targetFilepath());
			ext		 = lowerstr(extension_from_path(filename));
			// TODO: store path to listfile for disassembler
			// NOTE: the listfile can be found at filename -ext +".lst"
		}

		if (eq(ext, ".hdf") || eq(ext, ".img") || eq(ext, ".dmg") || eq(ext, ".iso"))
		{
			if (!model_info->canAttachDivIDE()) machine = initMachine(zx128, 0, s, j, r, yes); // powered on & suspended
			action_addDivIDE->setChecked(true);
			if (DivIDE* divide = machine->find<DivIDE>()) divide->insertDisk(filename);

			setFilepath(org_filename);
			setKeyboardMode(settings.get_KbdMode(key_new_snapshot_keyboard_mode, kbdgame));

			assert(machine->isPowerOn());
			assert(machine->isSuspended());
			machine->resume();
			in_machine_ctor = no;
			return;
		}

		if (eq(ext, ".rzx"))
		{
			action_RzxRecord->setChecked(false);
			machine->rzxDispose();
			//			screen->removeAllOverlays();

			rzx = new RzxFile;
			try
			{
				rzx->readFile(filename);
			}
			catch (AnyError& e)
			{
				rzx->rewind(); // versuche, das File bis zur Fehlerposition abspielbar zu machen
				if (!rzx->isSnapshot())
				{
					delete rzx;
					rzx = nullptr;
				} // defekt ab Start
				showAlert("%s", e.what());
			}
			if (rzx && rzx->isSnapshot())
			{
				filename = fullpath(rzx->getSnapshot());
				ext		 = lowerstr(extension_from_path(filename));
			}
		}

		FD fd(filename, 'r');

		if (eq(ext, ".o") || eq(ext, ".80"))
		{
			if (!machine->isA(isa_MachineZx80)) machine = initMachine(zx80, 0 /*ramsize*/, s, j, r, no);
			machine->loadO80(fd);
		}

		else if (eq(ext, ".p") || eq(ext, ".81") || eq(ext, ".p81"))
		{
			if (!machine->isA(isa_MachineZx81)) machine = initMachine(zx81, 0 /*ramsize*/, s, j, r, no);
			machine->loadP81(fd, eq(ext, ".p81"));
		}

		else if (eq(ext, ".sna"))
		{
			Model id = modelForSna(fd);
			if (model != id) machine = initMachine(id, 0 /*ramsize*/, s, j, r, d);
			machine->loadSna(fd);
		}

		else if (eq(ext, ".z80"))
		{
			Model id = modelForZ80(fd);
			if (id == unknown_model) throw DataError("illegal model in file");
			if (model != id) machine = initMachine(id, 0, s, j, no, d);
			machine->loadZ80(fd);
		}

		else if (eq(ext, ".ace"))
		{
			Model id = jupiter;
			if (model != id)
				machine = initMachine(
					id, 0, s, j, no, no); // note: don't add rampack now: rampack will be added by loadAce(),
			machine->loadAce(fd);		  // while removing a just added item will crash in the queued signal!
		}

		else if (eq(ext, ".scr"))
		{
			if (!machine->isA(isa_MachineZxsp)) machine = initMachine(zxsp_i3, 0, s, j, r, d);
			machine->loadScr(fd);
		}

		else if (eq(ext, ".rom"))
		{
			off_t	 fsz	  = fd.file_size();
			Model	 m		  = settings.get_Model(key_startup_model, zxsp_i3);
			Language language = zx_info[m].language;

			if (fsz != 4 kB && fsz != 8 kB && fsz != 10 kB && fsz != 16 kB && fsz != 24 kB && fsz != 32 kB &&
				fsz != 64 kB)
				throw DataError("The rom file size does not match any supported machine.");

			if (fsz < 16 kB) // zx80/81/clone or jupiter ace: load into interal rom
			{
				if (fsz != machine->rom.count())
				{
					if (machine->isA(isa_MachineZx80) && fsz == 8 kB) machine->rom.grow(8 kB);
					else if (machine->isA(isa_MachineZx80) && fsz == 4 kB) machine->rom.shrink(4 kB);
					else
					{
						if (fsz != zx_info[m].rom_size)
							m = fsz == 4 kB ? zx80 : fsz == 10 kB ? tk85 : language == american ? ts1000 : zx81;

						machine = initMachine(m, 0, s, j, r, d); // powered up & suspended
						assert(fsz == machine->rom.count());
					}
				}
				machine->loadRom(fd);
			}

			else if (fsz > 16 kB) // +128k, +2, +3/+2A, Pentagon, tc2068: load into internal rom
			{
				if (fsz != machine->rom.count())
				{
					if (fsz != zx_info[m].rom_size)
						m = fsz == 24 kB ? (language == american ? ts2068 : tc2068) :
							fsz == 32 kB ? (language == spanish ? zx128_span : zx128) :
										   (language == spanish ? zxplus3_span : zxplus3);

					machine = initMachine(m, 0, s, j, r, d); // powered up & suspended
					assert(fsz == machine->rom.count());
				}
				machine->loadRom(fd);
			}

			else // fsz == 16 kB: load into cartridge if possible
			{
				ZxIf2*		  zxif2	  = machine->findZxIf2();
				SpectraVideo* spectra = machine->findSpectraVideo();

				if (!zxif2 && !spectra)
				{
					if (!machine->model_info->canAttachZxIf2() && !machine->model_info->canAttachSpectraVideo() &&
						machine->rom.count() != fsz)
					{
						if (!model_info[m].canAttachZxIf2() && !model_info[m].canAttachSpectraVideo() &&
							model_info[m].rom_size != fsz)
							m = language == portuguese ? tc2048 : language == spanish ? inves : zxsp_i3;

						machine = initMachine(m, 0, s, j, r, d); // powered up & suspended
					}

					if (machine->model_info->canAttachZxIf2())
					{
						machine->addExternalItem(isa_ZxIf2);
						zxif2 = machine->findZxIf2();
					}
					else if (machine->model_info->canAttachSpectraVideo())
					{
						action_addSpectraVideo->setChecked(true);
						spectra = machine->findSpectraVideo();
						assert(spectra);
					}
				}

				if (zxif2) // if attached, use Interface 2
				{
					fd.close_file(0);
					zxif2->insertRom(filename);
					machine->powerCycle();
				}
				else if (spectra) // if attached, load into spectra video module
				{
					fd.close_file(0);
					spectra->insertRom(filename);
					machine->powerCycle();
				}
				else { machine->loadRom(fd); }
			}
		}

		else if (eq(ext, ".tap") || eq(ext, ".tape"))
		{
			bool   f	 = fd.file_size(); // f = not an empty tape
			uint16 bsize = f ? fd.read_uint16_z() : model == jupiter ? 0x1a : 0;
			uint8  btype = f ? fd.read_uint8() : 0;

			if (bsize == 0x1b && btype == 0x00) // Jupiter Ace program in a Spectrum-style .tap file
				btype = fd.read_uint8();		// btype = header type (0x00/0x20 for dictionary/binary data)

			if ((bsize == 0x1a || bsize == 0x1b) && btype == 0) // Jupiter Ace Tape
			{
				uint8 bname[10];
				fd.read_bytes(bname, 10);
				cstr loader = catstr(appl_rsrc_path, "Snapshots/load_tape_jupiter.z80");

				fd.close_file(0);
				if (model != jupiter) machine = initMachine(jupiter, 0, s, j, no, no); // powered up & suspended
				fd.open_file_r(loader);
				machine->loadZ80(fd); // powered up & suspended
				Z80::b2c(bname, &machine->ram[0x005], 10);
				Z80::b2c(bname, &machine->ram[0x302], 10);
			}
			else // ZX Spectrum Tape
			{
				Model m = model;
				if (!zx_info[m].isA(isa_MachineZxsp)) m = settings.get_Model(key_startup_model, zxsp_i3);
				if (!zx_info[m].isA(isa_MachineZxsp)) m = zxsp_i3;

				cstr loader = usingstr("%sSnapshots/load_tape_%s.z80", appl_rsrc_path, zx_info[m].nickname);
				if (!is_file(loader))
					throw AnyError("Sorry, i have no loader for loading tapes into a %s", zx_info[m].name);

				fd.close_file(0);
				if (m != model) machine = initMachine(m, 0, s, j, no, d);
				fd.open_file_r(loader);
				machine->loadZ80(fd);
			}

			TapeRecorder* tr = machine->taperecorder;
			assert(tr);
			tr->setAutoStartStopTape(0); // autoSS drückt Pause rein => Tape stoppt bei 1. custom Block
			tr->setInstantLoadTape(1);	 //							   und lädt nicht weiter => kein autoSS
			tr->insert(filename);
			tr->play();
		}

		else if (
			eq(ext, ".tzx") || eq(ext, ".aiff") || eq(ext, ".aif") || eq(ext, ".aifc") || eq(ext, ".wav") ||
			eq(ext, ".mp3") || eq(ext, ".mp2") || eq(ext, ".m4a"))
		{
			cstr loader = catstr(appl_rsrc_path, "Snapshots/load_tape_", model_info->nickname, ".z80");
			if (!is_file(loader)) throw AnyError("Sorry, i have no loader to load tapes into a %s", model_info->name);

			fd.close_file(0);
			fd.open_file_r(loader);
			machine->loadZ80(fd);

			TapeRecorder* tr = machine->taperecorder;
			assert(tr);
			tr->setAutoStartStopTape(0); // autoSS drückt Pause rein => Tape stoppt bei 1. custom Block
			tr->setInstantLoadTape(1);	 //							   und lädt nicht weiter => kein autoSS
			tr->insert(filename);
			tr->play();
		}

		else if (eq(ext, ".dsk"))
		{
			Model m = model;
			if (!zx_info[m].isA(isa_MachineZxPlus3)) m = settings.get_Model(key_startup_model, zxplus3);
			if (!zx_info[m].isA(isa_MachineZxPlus3)) m = zx_info[m].language == spanish ? zxplus3_span : zxplus3;

			cstr loader = catstr(appl_rsrc_path, "Snapshots/load_disk_", zx_info[m].nickname, ".z80");
			if (!is_file(loader)) throw AnyError("Sorry, i have no loader to load discs into a %s", zx_info[m].name);

			fd.close_file(0);
			if (m != model) machine = initMachine(m, 0, s, j, 0, d);
			dynamic_cast<MachineZxPlus3&>(*machine).insertDisk(filename);
		}

		else if (eq(ext, ".dck"))
		{
			Model m = model;
			if (!zx_info[m].isA(isa_MachineTc2068)) m = settings.get_Model(key_startup_model, zxplus3);
			if (!zx_info[m].isA(isa_MachineTc2068)) m = zx_info[m].language == spanish ? tc2068 : ts2068;

			fd.close_file(0);
			if (m != model) machine = initMachine(m, 0, 0, 0, 0, 0);
			dynamic_cast<MachineTc2068&>(*machine).insertCartridge(filename);
		}

		else // unsupported snapshot format:
		{
			if (rzx) showWarning("Sorry, your file contains a \"%s\" snapshot. This is not yet supported.", ext);
			else showAlert("No handler for \"%s\" files in \"loadSnapshot()\"", ext);
			delete rzx;
			rzx			 = nullptr;
			org_filename = nullptr;
		}

		setFilepath(org_filename);
		setKeyboardMode(settings.get_KbdMode(key_new_snapshot_keyboard_mode, kbdgame));

		if (rzx) // wir haben ein rzx file und der snapshot wurde geladen
		{		 // der state nach getSnapshot() ist playing oder endoffile
			assert(!machine->rzxIsLoaded());
			machine->rzxPlayFile(rzx);
			if (rzx->isPlaying())
			{
				action_RzxRecord->setChecked(false);
				assert(machine->isPowerOn());
				assert(machine->isRunning());
				in_machine_ctor = no;
				return;
			}
			else
			{
				showWarning("The rzx file only contained a snapshot. Recording to rzx file started!");
				action_RzxRecord->setChecked(false);
			}
		}
	}
	catch (AnyError& e)
	{
		showAlert("Error: %s", e.what());
		if (!machine) machine = initMachine(zxsp_i3, 0, s, j, r, d);
		delete rzx;
		rzx = nullptr;
		machine->rzxDispose();
		action_RzxRecord->setChecked(false);
		//		screen->removeAllOverlays();
		setFilepath(nullptr);
	}

	assert(machine->isPowerOn());
	assert(machine->isSuspended());
	machine->resume();
	in_machine_ctor = no;
}

class MyAction : public QAction
{
public:
	MyAction(QActionGroup* p) : QAction(p) {}
	MyAction(cstr label, QActionGroup* p) : QAction(label, p) {}
	MyAction(cstr label, QObject* p) : QAction(label, p) {}
	MyAction(cstr icon, cstr label, QObject* p) : QAction(QIcon(catstr(":/Icons/", icon)), label, p) {}

protected:
	virtual bool event(QEvent* e) override
	{
		xxlogIn("MyAction:event: %s", QEventTypeStr(e->type()));
		return QAction::event(e);
	}
};

class MyMenu : public QMenu
{
public:
	MyMenu(QWidget* parent) : QMenu(parent) {}
	MyMenu(cstr title, QWidget* parent) : QMenu(title, parent) {}

protected:
	virtual bool event(QEvent* e) override
	{
		xxlogIn("MyMenu:event: %s", QEventTypeStr(e->type()));
		return QMenu::event(e);
	}
};

class MyMenuBar : public QMenuBar
{
public:
	MyMenuBar(QWidget* parent) : QMenuBar(parent) {}

protected:
	virtual bool event(QEvent* e) override
	{
		xxlogIn("MyMenuBar:event: %s", QEventTypeStr(e->type()));
		return QMenuBar::event(e);
	}
};

QAction*
MachineController::newAction(cstr icon, cstr title, const QKeySequence& key, std::function<void(bool)> fu, isa_id id)
{
	// helper: create QAction for checkable menu entry:
	// fu(bool) called when 'triggered'.

	xxlogIn("new_action(%s, %s)", title, isa_names[id]);

	QAction* a = icon ? new MyAction(icon, title, this) : new MyAction(title, this);
	if (!key.isEmpty()) a->setShortcut(CTRL | key[0]);
	a->setCheckable(yes);
	if (id != isa_none) a->setData(QVariant(id));
	connect(a, &QAction::toggled, fu);
	return a;
}

QAction*
MachineController::newAction(cstr icon, cstr title, const QKeySequence& key, std::function<void()> fu, isa_id id)
{
	// helper: create QAction for non-checkable menu entry:
	// fu() called when 'triggered'.

	xxlogIn("new_action(%s, %s)", title, isa_names[id]);

	QAction* a = icon ? new MyAction(icon, title, this) : new MyAction(title, this);
	if (!key.isEmpty()) a->setShortcut(CTRL | key[0]);
	if (id != isa_none) a->setData(QVariant(id));
	connect(a, &QAction::triggered, fu);
	return a;
}

void MachineController::createActions()
{
	// create all actions and action groups:

	separator = new QAction(this);
	separator->setSeparator(true);
	separator2 = new QAction(this);
	separator2->setSeparator(true);
	separator3 = new QAction(this);
	separator3->setSeparator(true);
	action_dummy	   = new QAction(this);
	action_recentFiles = new QAction("Recent files…", this);
	action_recentFiles->setMenu(new RecentFilesMenu(RecentFiles, this, [=](cstr fpath) { loadSnapshot(fpath); }));

#define NOKEY		 QKeySequence()
#define NOICON		 nullptr
#define ADDITEM(ISA) [=](bool f) { addExternalItem(ISA, f); }, ISA // note: need to setData(ISA) in QAction
#define ADDRAM(ISA)	 [=](bool f) { addExternalRam(ISA, f); }, ISA  // for findActionForItem() in slot_item_added()

	action_showAbout	   = newAction(NOICON, "About …", Qt::Key_Question, [] { Application::showAbout(); });
	action_showPreferences = newAction(NOICON, "Settings …", Qt::Key_Comma, [] { Application::showPreferences(); });
	action_showAbout->setMenuRole(QAction::AboutRole);
	action_showPreferences->setMenuRole(QAction::PreferencesRole);

	action_newMachine =
		newAction("new-window.png", "&New window", QKeySequence::New, [=] { new MachineController(""); });
	action_openFile	   = newAction("open.png", "&Open…", QKeySequence::Open, [=] { openFile(); });
	action_reloadFile  = newAction("reload.png", "&Reload current file", Qt::Key_R, [=] { reloadFile(); });
	action_closeWindow = newAction(NOICON, "Close top window", Qt::Key_W, [=] { QWidget::close(); });
	action_saveAs	   = newAction("save.png", "&Save as…", QKeySequence::Save, [=] { saveAs(); });
	action_screenshot  = newAction("camera.png", "Save screenshot", Qt::Key_P, [=] { saveScreenshot(); });
	action_recordMovie =
		newAction("video.gif", "Record Gif movie", Qt::Key_P | int(Qt::ALT), [=](bool f) { recordMovie(f); });
	action_pwrOnReset =
		newAction("power_button.gif", "Power-On reset", Qt::Key_R | SHIFT, [=] { powerResetMachine(); });
	action_reset	= newAction("reset_button.gif", "Push reset", NOKEY, [=] { resetMachine(); });
	action_nmi		= newAction("nmi_button.gif", "Push NMI", Qt::Key_N | SHIFT, [=] { nvptr(machine)->nmi(); });
	action_suspend	= newAction("run-pause.png", "Halt CPU", Qt::Key_H | SHIFT, [=](bool f) { haltMachine(f); });
	action_stepIn	= newAction("arrow-dn.png", "Step in", Qt::Key_I | SHIFT, [=] {
		  assert(machine->isSuspended());
		  nvptr(machine)->stepIn();
	  });
	action_stepOver = newAction("run-r.png", "Step over", Qt::Key_S | SHIFT, [=] {
		assert(machine->isSuspended());
		nvptr(machine)->stepOver();
	});
	action_stepOut	= newAction("arrow-up.png", "Step out", Qt::Key_O | SHIFT, [=] {
		 assert(machine->isSuspended());
		 nvptr(machine)->stepOut();
	 });
	action_enable_breakpoints =
		newAction(NOICON, "Enable breakpoints", Qt::Key_B | SHIFT, [=](bool f) { enableBreakpoints(f); });

	action_minimize		= newAction(NOICON, "Minimize", NOKEY, [=]() { showMinimized(); });
	action_zoom[0]		= newAction(NOICON, "Size x 1", Qt::Key_1, [=]() { setWindowZoom(1); });
	action_zoom[1]		= newAction(NOICON, "Size x 2", Qt::Key_2, [=]() { setWindowZoom(2); });
	action_zoom[2]		= newAction(NOICON, "Size x 3", Qt::Key_3, [=]() { setWindowZoom(3); });
	action_zoom[3]		= newAction(NOICON, "Size x 4", Qt::Key_4, [=]() { setWindowZoom(4); });
	action_fullscreen	= newAction(NOICON, "Fullscreen", Qt::Key_F, [=]() { QWidget::showFullScreen(); });
	action_showLenslok	= newAction(NOICON, "Lenslok", NOKEY, [=](bool f) { showLenslok(f); });
	action_newInspector = newAction(NOICON, "Inspector", NOKEY, [=] { newToolwindow()->show(); });

	// fixed show_actions: these are never removed from showActions[] (except in dtor)
	action_showMachineImage = newAction(
		NOICON, "Machine image", Qt::Key_I,
		[=](bool f) { toggleToolwindow(machine.get(), action_showMachineImage, f); }, isa_Machine);
	action_showMemHex = newAction(
		NOICON, "Memory hexview", Qt::Key_M, [=](bool f) { toggleToolwindow(mem[0], action_showMemHex, f); },
		isa_MemHex);
	action_showMemDisass = newAction(
		NOICON, "Memory disassemble", Qt::Key_M | SHIFT,
		[=](bool f) { toggleToolwindow(mem[1], action_showMemDisass, f); }, isa_MemDisass);
	action_showMemGraphical = newAction(
		NOICON, "Memory graphical", Qt::Key_M | int(Qt::ALT),
		[=](bool f) { toggleToolwindow(mem[2], action_showMemGraphical, f); }, isa_MemGraphical);
	action_showMemAccess = newAction(
		NOICON, "Memory access", Qt::Key_M | int(Qt::META),
		[=](bool f) { toggleToolwindow(mem[3], action_showMemAccess, f); }, isa_MemAccess);

	// add external item:
	action_addKempstonJoy = newAction("joystick-k.gif", "Kempston joystick interface", NOKEY, ADDITEM(isa_KempstonJoy));
	action_addKempstonMouse	  = newAction("mouse.png", "Kempston mouse interface", NOKEY, ADDITEM(isa_KempstonMouse));
	action_addDidaktikMelodik = newAction("ay.gif", "Didaktik Melodik [ACB]", NOKEY, ADDITEM(isa_DidaktikMelodik));
	// action_addZaxonAyMagic = newAction("ay.gif",         "Zaxon AY-Magic",               NOKEY,
	// ADDITEM(isa_ZaxonAyMagic));
	action_addZonxBox	 = newAction("ay.gif", "Bi-Pak ZON X", NOKEY, ADDITEM(isa_ZonxBox));
	action_addZonxBox81	 = newAction("ay.gif", "Bi-Pak ZON X-81", NOKEY, ADDITEM(isa_ZonxBox81));
	action_addZxIf2		 = newAction("joystick-2.gif", "Sinclair ZX Interface 2", NOKEY, ADDITEM(isa_ZxIf2));
	action_addZxPrinter	 = newAction("printer.gif", "Sinclair ZX Printer", NOKEY, ADDITEM(isa_ZxPrinter));
	action_addFdcBeta128 = newAction("save.png", "Beta 128 disc interface", NOKEY, ADDITEM(isa_FdcBeta128));
	action_addFdcD80	 = newAction("save.png", "Datel 80 disc interface", NOKEY, ADDITEM(isa_FdcD80));
	action_addFdcJLO	 = newAction("save.png", "JLO disc interface", NOKEY, ADDITEM(isa_FdcJLO));
	action_addFdcPlusD	 = newAction("save.png", "Plus D disc interface", NOKEY, ADDITEM(isa_FdcPlusD));
	action_addDktronicsDualJoy =
		newAction("joystick-2.gif", "dk'tronics dual joystick", NOKEY, ADDITEM(isa_DktronicsDualJoy));
	action_addProtekJoy	   = newAction("joystick-1.gif", "Protek joystick interface", NOKEY, ADDITEM(isa_ProtekJoy));
	action_addPrinterAerco = newAction("printer.gif", "Aerco printer", NOKEY, ADDITEM(isa_PrinterAerco));
	action_addPrinterLprint3 =
		newAction("printer.gif", "LPrintIII printer interface", NOKEY, ADDITEM(isa_PrinterLprint3));
	action_addPrinterTs2040 = newAction("printer.gif", "TS2040 printer", NOKEY, ADDITEM(isa_PrinterTs2040));
	action_addMultiface1	= newAction(
		   "nmi_button.gif", "Romantic Robots Multiface ONE", NOKEY, [this](bool f) { addMultiface1(f); }, isa_Multiface1);
	action_addMultiface128 =
		newAction("nmi_button.gif", "Romantic Robots Multiface 128", NOKEY, ADDITEM(isa_Multiface128));
	action_addMultiface3 = newAction("nmi_button.gif", "Romantic Robots Multiface 3", NOKEY, ADDITEM(isa_Multiface3));
	action_addFullerBox	 = newAction("ay.gif", "Fuller box", NOKEY, ADDITEM(isa_FullerBox));
	action_addZxIf1		 = newAction(NOICON, "Sinclair ZX Interface 1", NOKEY, ADDITEM(isa_ZxIf1));
	action_addGrafPad	 = newAction(NOICON, "Grafpad", NOKEY, ADDITEM(isa_GrafPad));
	action_addIcTester	 = newAction(NOICON, "Kio's Ic Tester", NOKEY, ADDITEM(isa_IcTester));
	action_addCurrahMicroSpeech = newAction(NOICON, "Currah µSpeech", NOKEY, ADDITEM(isa_CurrahMicroSpeech));
	action_addCheetah32kRam		= newAction(NOICON, "Cheetah 32k rampack", NOKEY, ADDRAM(isa_Cheetah32kRam));
	action_addJupiter16kRam		= newAction(NOICON, "Jupiter 16K RAM", NOKEY, ADDRAM(isa_Jupiter16kRam));
	action_addZx16kRam			= newAction(NOICON, "Sinclair ZX 16K RAM", NOKEY, ADDRAM(isa_Zx16kRam));
	action_addTs1016Ram			= newAction(NOICON, "Timex Sinclair 1016", NOKEY, ADDRAM(isa_Ts1016Ram));
	action_addMemotech16kRam	= newAction(NOICON, "MEMOPAK 16k", NOKEY, ADDRAM(isa_Memotech16kRam));
	action_addStonechip16kRam	= newAction(NOICON, "Stonechip 16K Expandable Ram", NOKEY, ADDRAM(isa_Stonechip16kRam));

	action_addMemotech64kRam = newAction(
		NOICON, "MEMOPAK 64k", NOKEY, [this](bool f) { addMemotech64kRam(f); }, isa_Memotech64kRam);

	action_addZx3kRam = newAction(
		NOICON, "Sinclair ZX80 1-3K BYTE RAM PACK", NOKEY, [this](bool f) { addZx3kRam(f); }, isa_Zx3kRam);

	action_addDivIDE = newAction(
		NOICON, "DivIDE 57c CF card interface", NOKEY, [this](bool f) { addDivIDE(f); }, isa_DivIDE);

	action_addSpectraVideo = newAction(
		NOICON, "SPECTRA video interface", NOKEY, [=](bool f) { addSpectraVideo(f); }, isa_SpectraVideo);

	action_gifAnimateBorder = settings.action_gifAnimateBorder;

	action_setKbdBasic =
		newAction("mini_zxsp.gif", "Keyboard BASIC mode", Qt::Key_B, [=] { setKeyboardMode(kbdbasic); });
	action_setKbdGame = newAction("mini_zxsp.gif", "Keyboard game mode", Qt::Key_G, [=] { setKeyboardMode(kbdgame); });
	action_setKbdBtZXKbd = newAction(
		"mini_zxsp.gif", "Recreated ZXKeyboard game mode", Qt::Key_G | SHIFT, [=] { setKeyboardMode(kbdbtzxkbd); });

	action_audioin_enabled = newAction(NOICON, "Enable audio-in", NOKEY, [=](bool f) { enableAudioIn(f); });
	action_RzxRecord	   = newAction(NOICON, "Record RZX data", NOKEY, [=](bool f) { setRzxRecording(f); });
	action_RzxRecordAutostart =
		newAction(NOICON, "Switch to recording on any key", NOKEY, [=](bool f) { setRzxAutostartRecording(f); });
	action_RzxRecordAppendSna =
		newAction(NOICON, "Append snapshots to RZX file", NOKEY, [=](bool f) { setRzxAppendSnapshots(f); });

	action_setSpeed100_50 = newAction(NOICON, "100% 50Hz", Qt::Key_5, [=](bool f) {
		if (f)
		{
			nvptr(machine)->set50Hz();
			switch (uint(model))
			{
			case zx80: gui::settings.setValue(key_framerate_zx80_60hz, false); break;
			case jupiter: gui::settings.setValue(key_framerate_jupiter_60hz, false); break;
			case tk85: gui::settings.setValue(key_framerate_tk85_60hz, false); break;
			case tk90x: gui::settings.setValue(key_framerate_tk90x_60hz, false); break;
			case tk95: gui::settings.setValue(key_framerate_tk95_60hz, false); break;
			}
		}
	});
	action_setSpeed100_60 = newAction(NOICON, "100% 60Hz", Qt::Key_6, [=](bool f) {
		if (f)
		{
			nvptr(machine)->set60Hz();
			switch (uint(model))
			{
			case zx80: gui::settings.setValue(key_framerate_zx80_60hz, true); break;
			case jupiter: gui::settings.setValue(key_framerate_jupiter_60hz, true); break;
			case tk85: gui::settings.setValue(key_framerate_tk85_60hz, true); break;
			case tk90x: gui::settings.setValue(key_framerate_tk90x_60hz, true); break;
			case tk95: gui::settings.setValue(key_framerate_tk95_60hz, true); break;
			}
		}
	});
	action_setSpeed120	  = newAction(NOICON, "120% 60Hz", Qt::Key_6, [=] { nvptr(machine)->speedupTo60fps(); });
	action_setSpeed200	  = newAction(NOICON, "200% 60Hz", NOKEY, [=] { nvptr(machine)->setSpeedAnd60fps(2.0); });
	action_setSpeed400	  = newAction(NOICON, "400% 60Hz", NOKEY, [=] { nvptr(machine)->setSpeedAnd60fps(4.0); });
	action_setSpeed800	  = newAction(NOICON, "800% 60Hz", Qt::Key_8, [=] { nvptr(machine)->setSpeedAnd60fps(8.0); });

	QActionGroup* speedGrp = new QActionGroup(this);
	speedGrp->addAction(action_setSpeed100_50);
	action_setSpeed100_50->setCheckable(1);
	speedGrp->addAction(action_setSpeed100_60);
	action_setSpeed100_60->setCheckable(1);
	speedGrp->addAction(action_setSpeed120);
	action_setSpeed120->setCheckable(1);
	speedGrp->addAction(action_setSpeed200);
	action_setSpeed200->setCheckable(1);
	speedGrp->addAction(action_setSpeed400);
	action_setSpeed400->setCheckable(1);
	speedGrp->addAction(action_setSpeed800);
	action_setSpeed800->setCheckable(1);

	QActionGroup* kbdGrp = new QActionGroup(this);
	kbdGrp->addAction(action_setKbdBasic);
	action_setKbdBasic->setCheckable(true);
	kbdGrp->addAction(action_setKbdGame);
	action_setKbdGame->setCheckable(true);
	kbdGrp->addAction(action_setKbdBtZXKbd);
	action_setKbdBtZXKbd->setCheckable(true);

	QActionGroup* zoomGrp = new QActionGroup(this);
	zoomGrp->addAction(action_zoom[0]);
	action_zoom[0]->setCheckable(1);
	zoomGrp->addAction(action_zoom[1]);
	action_zoom[1]->setCheckable(1);
	zoomGrp->addAction(action_zoom[2]);
	action_zoom[2]->setCheckable(1);
	zoomGrp->addAction(action_zoom[3]);
	action_zoom[3]->setCheckable(1);
	zoomGrp->addAction(action_fullscreen);
	action_fullscreen->setCheckable(1);
}

void MachineController::createMenus()
{
	// create mbar menus, context menu and model actiongroup:

	// menus:
	context_menu = new MyMenu("_oOo_", this);
	file_menu	 = new MyMenu("File", this);
	model_menu	 = new MyMenu("Model", this);
	items_menu	 = new MyMenu("Extensions", this);
	options_menu = new MyMenu("Options", this);
	control_menu = new MyMenu("Control", this);
	window_menu	 = new WindowMenu(this);
	memory_menu	 = new MyMenu("Memory", this);
	speed_menu	 = new MyMenu("Speed", this);

	file_menu->addActions(
		QList<QAction*>() // <-- void QCocoaMenu::insertNative … Menu item is already in a menu …
		<< action_newMachine << separator << action_openFile << action_recentFiles << action_reloadFile << separator2
		<< action_closeWindow << action_saveAs << separator3 << action_screenshot << action_recordMovie
		<< action_gifAnimateBorder << action_showAbout << action_showPreferences);

	control_menu->addActions(
		QList<QAction*>() << action_pwrOnReset << action_reset << action_nmi << separator << action_enable_breakpoints
						  << action_suspend << action_stepIn << action_stepOver << action_stepOut);

	options_menu->addAction(action_audioin_enabled);
	options_menu->addMenu(speed_menu);

	options_menu->addActions(
		QList<QAction*>() << separator << action_setKbdBasic << action_setKbdGame << action_setKbdBtZXKbd << separator2
						  << action_RzxRecord << action_RzxRecordAutostart << action_RzxRecordAppendSna);

	window_menu->addActions(
		QList<QAction*>() << action_minimize << action_zoom[0] << action_zoom[1] << action_zoom[2] << action_zoom[3]
						  << action_fullscreen << separator2 << action_showMachineImage);

	context_menu->addActions(
		QList<QAction*>() // <-- void QCocoaMenu::insertNative … Menu item is already in a menu …
		<< action_setKbdBasic << action_setKbdGame << action_setKbdBtZXKbd << separator << action_openFile
		<< action_recentFiles << action_reloadFile << action_saveAs << separator2 << action_newInspector << separator3
		<< action_zoom[0] << action_zoom[1] << action_zoom[2] << action_zoom[3] << action_fullscreen);

	memory_menu->addActions(
		QList<QAction*>() << action_showMemHex << action_showMemDisass << action_showMemGraphical
						  << action_showMemAccess);

	window_menu->addMenu(memory_menu);

	// actiongroup for model:
	model_actiongroup = new QActionGroup(this);
	for (int model = 0; model < num_models; model++)
	{
		QAction* a;
		ZxInfo&	 info = zx_info[model];
		if (eq(info.name, "(separator)"))
		{
			a = new MyAction(model_actiongroup); // wir brauchen eine richtige, eigene Action,
			a->setSeparator(true); // damit die Indexe in der ActionGroup mit den Modelnummern übereinstimmen
		}
		else
		{
			a = new MyAction(info.name, model_actiongroup);
			a->setCheckable(true);
			a->setEnabled(info.is_supported);
			a->setData(QVariant(model));
		}
		model_menu->addAction(a);
	}

	connect(model_actiongroup, &QActionGroup::triggered, this, [=](QAction* a) {
		xlogIn("MachineController:SetModel");

		Model newmodel = Model(a->data().toInt());
		assert(newmodel >= 0 && newmodel < num_models);
		if (newmodel == model) return;

		bool s = settings.get_bool(key_always_attach_soundchip, yes);
		bool j = settings.get_bool(key_always_attach_joystick, no);
		bool r = settings.get_bool(key_always_attach_rampack, yes);
		bool d = settings.get_bool(key_always_attach_divide, no);

		initMachine(newmodel, 0, s, j, r, d)->resume(); // machine is powered up and running
	});
}

void MachineController::createMainmenubar()
{
	createActions();
	createMenus();

	QMenuBar* mbar = new MyMenuBar(this);
	mbar->addMenu(file_menu); // <-- void QCocoaMenu::insertNative … Menu item is already in a menu …
	mbar->addMenu(model_menu);
	mbar->addMenu(items_menu);
	mbar->addMenu(options_menu);
	mbar->addMenu(control_menu);
	mbar->addMenu(window_menu);

	setMenuBar(mbar);
}

void MachineController::startupOpenToolwindows()
{
	// open toolwindows for new window as set in preferences
	// this must be delayed because creation of showactions in itemAdded() is also delayed

	QTimer::singleShot(0, this, [=] {
		if (settings.get_bool(key_startup_open_keyboard, no))
		{
			QAction* action = findShowActionForItem(machine->keyboard);
			assert(action); // if(!action) logline("***no showaction for keyboard!***"); else
			action->setChecked(yes);
		}

		if (settings.get_bool(key_startup_open_taperecorder, no))
		{
			QAction* action = findShowActionForItem(machine->taperecorder);
			assert(action); // if(!action) logline("***no showaction for taperecorder!***"); else
			action->setChecked(yes);
		}

		if (settings.get_bool(key_startup_open_machine_image, no)) { action_showMachineImage->setChecked(yes); }

		if (settings.get_bool(key_startup_open_disk_drive, no))
		{
			if (machine->fdc) findShowActionForItem(machine->fdc)->setChecked(yes);
			else if (machine->mmu->isA(isa_MmuTc2068)) findShowActionForItem(machine->mmu)->setChecked(yes);
			else
			{
				Item* divide = NV(machine)->find<DivIDE>();
				if (divide) findShowActionForItem(divide)->setChecked(yes);
			}
		}
	});
}

MachineController::~MachineController()
{
	xlogIn("~MachineController");

	in_dtor = yes;

	if (this == front_machine_controller)
	{
		// note: pos() as required for move() and size() as required for resize()
		//       this is neither geometry() nor frameGeometry().    :-/
		xlogline("pos = %i,%i", pos().x(), pos().y());
		xlogline("geo = %i,%i", geometry().x(), geometry().y());
		xlogline("frm = %i,%i", frameGeometry().x(), frameGeometry().y());
		settings.setValue(catstr(key_mainwindow_position, tostr(screen->getZoom())), QRect(pos(), size()));

		front_machine_controller = nullptr;
		front_machine			 = nullptr;
	}

	while (tool_windows.count()) { delete tool_windows.last(); }

	killMachine();
}

MachineController::MachineController(QString filepath) :
	QMainWindow(nullptr),
	in_ctor(yes),
	in_dtor(no),
	in_machine_ctor(no),
	in_machine_dtor(no),
	model(unknown_model),
	model_info(nullptr),
	filepath(nullptr),
	machine(nullptr),
	screen(nullptr),
	mem {nullptr, nullptr, nullptr, nullptr},
	lenslok(nullptr),
	keyjoy_keys {0, 0, 0, 0, 0},
	keyjoy_fnmatch_pattern(nullptr)
{
	// setup window
	// setup menubar
	// init with default machine: zxsp_i3
	// link into MachineList => GO!

	xlogIn("new MachineController(\"%s\")", filepath.toUtf8().data());

	createMainmenubar();

	Model model = settings.get_Model(key_startup_model, zxsp_i3);
	if (!filepath.isEmpty()) model = bestModelForFile(filepath.toUtf8().data(), model);

	uint32 ramsize = 0; // default
	bool   ay	   = settings.get_bool(key_always_attach_soundchip, yes);
	bool   joy	   = settings.get_bool(key_always_attach_joystick, no);
	bool   ram	   = settings.get_bool(key_always_attach_rampack, yes);
	bool   divide  = settings.get_bool(key_always_attach_divide, no);

	initMachine(model, ramsize, ay, joy, ram, divide); // create and init Machine: powered on & suspended

	// init window:
	setMinimumSize(256, 192); // --> Screen
	setBaseSize(320, 240);	  //
	setSizeIncrement(2, 2);	  // seems to have no effect
	setAttribute(Qt::WA_DeleteOnClose, 1);
	setFocusPolicy(Qt::StrongFocus);

	int zoom = settings.get_int(key_startup_screen_size, 2);
	if (zoom)
	{
		if (zoom < 0 || zoom > 4) zoom = 2;
		QRect main_screen = QApplication::primaryScreen()->availableGeometry();
		QRect r(settings.value(catstr(key_mainwindow_position, tostr(zoom)), QRect()).toRect());

		if (!r.topLeft().isNull() && !r.size().isEmpty())
		{
			move(r.left(), r.top());
			resize(min(r.width(), main_screen.width()), min(r.height(), main_screen.height()));
		}
		else { resize(min(320 * zoom, main_screen.width()), min(zoom * 240, main_screen.height())); }
		show(); // show this QMainWindow
		action_zoom[zoom - 1]->setChecked(1);
	}
	else
	{
		showFullScreen();
		action_fullscreen->setChecked(1);
	}

	startupOpenToolwindows(); // acc. to preferences

	assert(machine->isPowerOn());
	assert(machine->isSuspended());
	if (filepath.isEmpty()) machine->resume();
	else loadSnapshot(filepath.toUtf8().data());

	in_ctor = no;
}

void MachineController::killMachine()
{
	// destroy Machine and Screen controlled by this Controller
	// delete machine and screen
	// used to switch model for this machine instance
	// not required for ~MachineController(), because this is parent of screen and machine

	xlogIn("MachineController:kill_machine");

	in_machine_dtor = yes;
	machine->powerOff();

	// unchecked menu items:
	action_showLenslok->setChecked(off);
	action_recordMovie->setChecked(off);
	action_suspend->setChecked(off);
	action_enable_breakpoints->setChecked(off);
	// keep action_zoom
	// keep action_fullscreen
	// action_showMachineImage will be reset in init()
	// checkmark in model menu will be reset in init()
	// kbd mode will be reset in init()

	for (int i = NELEM(mem); i--;)
	{
		hideInspector(mem[i], no);
		delete mem[i];
		mem[i] = nullptr;
	}
	hideInspector(NV(machine), no);

	//delete machine:
	machine.reset();

	delete screen;
	screen = nullptr;
	if (XSAFE)
		foreach (ToolWindow* toolwindow, tool_windows) { assert(toolwindow->item == nullptr); }

	while (show_actions.count() > 5) { window_menu->removeAction(show_actions.takeLast()); }
	items_menu->clear();
	items_menu->addAction(
		action_dummy); // Trick damit das Extensions-Menü bei Wechsel des Modells nicht sporadisch verschwindet
	add_actions.clear();

	// keyjoy_fnmatch_pattern = nullptr;

	in_machine_dtor = no;
}

Machine* MachineController::initMachine(
	Model model, uint32 ramsize, bool alwaysAddAy, bool alwaysAddJoy, bool alwaysAddRam, bool alwaysAddDivide)
{
	// create Machine and Screen for model
	// set keyboard to "Basic" mode
	// set window title
	// setup "Items" menu
	// checkmark machine in "Model" menu
	// set central widget to screen & show self
	// the machine is powered up but suspended

	xlogIn("MachineController:initMachine(%i)", model);

	bool was_in_machine_ctor = in_machine_ctor;
	in_machine_ctor			 = yes;

	if (machine) killMachine();

	if (!zx_info[model].is_supported)
	{
		model	= zxsp_i3;
		ramsize = 0;
	}

	if (ramsize != 0) TODO();


	// Reset checkable menu entries: (done in killMachine())
	assert(!action_recordMovie->isChecked());
	assert(!action_suspend->isChecked());

	// not yet supported items:
	action_addZxPrinter->setEnabled(off);
	action_addPrinterTs2040->setEnabled(off);
	action_addZxIf1->setEnabled(off);
	action_addFullerBox->setEnabled(off);
	action_addGrafPad->setEnabled(off);
	action_addIcTester->setEnabled(off);
	action_addFdcBeta128->setEnabled(off);
	action_addFdcD80->setEnabled(off);
	action_addFdcJLO->setEnabled(off);
	action_addFdcPlusD->setEnabled(off);
	action_addPrinterAerco->setEnabled(off);
	action_addPrinterLprint3->setEnabled(off);

	// Menu entries which are disabled by default:
	action_stepIn->setEnabled(no);
	action_stepOut->setEnabled(no);
	action_stepOver->setEnabled(no);

	// Model menu:
	// add checkmark to current model in model menu
	// other machines become unchecked because they are in the same QActionGroup
	model_actiongroup->actions().at(model)->setChecked(1);

	// Create machine:
	screen		  = newScreenForModel(model);
	auto machine  = newMachineForModel(model); // not powered on, not suspended
	this->machine = machine;				   // volatile
	this->model = model = machine->model;
	model_info			= machine->model_info;
	if (XSAFE)
		foreach (ToolWindow* toolwindow, tool_windows) { assert(toolwindow->item == nullptr); }
	machine->installRomPatches();
	action_enable_breakpoints->setChecked(true);

	setFilepath(nullptr);
	if (this == front_machine_controller) front_machine = machine.get();

	setKeyboardMode(settings.get_KbdMode(key_new_machine_keyboard_mode, kbdbasic));
	enableAudioIn(settings.get_bool(key_new_machine_audioin_enabled, machine->audio_in_enabled));
	setCentralWidget(screen);
	if (this == front_machine_controller) activateWindow(); // else no focus on special conditions. Qt-bug?
	action_setKbdBasic->setText(model == jupiter ? "Keyboard FORTH mode" : "Keyboard BASIC mode");

	// init speed menu:

	bool is50hz = machine->ula->is50Hz();
	bool is60hz = machine->ula->is60Hz();
	speed_menu->clear();
	if (model_info->has_50_60hz_switch)
	{
		speed_menu->addAction(action_setSpeed100_50);
		speed_menu->addAction(action_setSpeed100_60);
		action_setSpeed100_50->setChecked(is50hz);
		action_setSpeed100_60->setChecked(is60hz);
	}
	else if (is50hz) // 50Hz model
	{
		speed_menu->addAction(action_setSpeed100_50);
		speed_menu->addAction(action_setSpeed120);
		action_setSpeed100_50->setChecked(is50hz);
	}
	else if (is60hz) // 60Hz model
	{
		speed_menu->addAction(action_setSpeed100_60);
		action_setSpeed100_60->setChecked(is60hz);
	}
	else { IERR(); }
	speed_menu->addAction(action_setSpeed200);
	speed_menu->addAction(action_setSpeed400);
	speed_menu->addAction(action_setSpeed800);


	// init RZX recording menu:

	action_RzxRecord->setChecked(off);
	action_RzxRecordAutostart->setChecked(settings.get_bool(key_rzx_autostart_recording, off));
	action_RzxRecordAppendSna->setChecked(off);


	// init items menu
	// and add AY, Joystick, more Ram etc. depending on flags:

	if (show_actions.isEmpty())
		show_actions << action_showMachineImage << action_showMemHex << action_showMemDisass << action_showMemGraphical
					 << action_showMemAccess;

	if (XSAFE)
		foreach (ToolWindow* toolwindow, tool_windows) { assert(toolwindow->item == nullptr); }
	showInspector(machine.get(), action_showMachineImage, no /*!force*/);
	showInspector(mem[0] = new MemObject(isa_MemHex), action_showMemHex, no /*!force*/);
	showInspector(mem[1] = new MemObject(isa_MemDisass), action_showMemDisass, no /*!force*/);
	showInspector(mem[2] = new MemObject(isa_MemGraphical), action_showMemGraphical, no /*!force*/);
	showInspector(mem[3] = new MemObject(isa_MemAccess), action_showMemAccess, no /*!force*/);

	assert(findShowActionForItem(mem[0]) == action_showMemHex);
	assert(findShowActionForItem(mem[1]) == action_showMemDisass);
	assert(findShowActionForItem(mem[2]) == action_showMemGraphical);
	assert(findShowActionForItem(mem[3]) == action_showMemAccess);


	if (model_info->has_zx80_bus)
	{
		add_actions = QList<QAction*>() << action_addZx3kRam << action_addZx16kRam << action_addTs1016Ram
										<< action_addStonechip16kRam << action_addMemotech16kRam
										<< action_addMemotech64kRam << action_addZonxBox81 << action_addZxPrinter
										<< action_addPrinterTs2040;

		items_menu->addActions(add_actions);

		if (alwaysAddAy && !machine->ay) action_addZonxBox81->setChecked(true);
		if (alwaysAddJoy && !machine->joystick) {} // TODO
		if (alwaysAddRam && machine->ram.count() < 16 kB)
		{
			if (model == zx80) action_addZx3kRam->setChecked(true);
			else if (model == ts1000 || model == ts1500) action_addTs1016Ram->setChecked(true);
			else action_addZx16kRam->setChecked(true);
		}

		action_RzxRecord->setEnabled(no);
		action_RzxRecordAutostart->setEnabled(no);
		action_RzxRecordAppendSna->setEnabled(no);
	}

	else if (model_info->has_zxsp_bus)
	{
		add_actions = QList<QAction*>() << action_addKempstonJoy << action_addDktronicsDualJoy << action_addProtekJoy
										<< action_addZxIf2 << action_addKempstonMouse << action_addZonxBox
										<< action_addDidaktikMelodik
										<< (model_info->has_port_1ffd ? action_addMultiface3 :
											model_info->has_port_7ffd ? action_addMultiface128 :
																		action_addMultiface1)
										// TODO: MF128 worked also on 48k models => Setting?
										<< action_addFullerBox << action_addGrafPad << action_addIcTester
										<< action_addZxIf1 << action_addDivIDE << action_addFdcBeta128
										<< action_addFdcD80 << action_addFdcJLO << action_addFdcPlusD
										<< action_addZxPrinter << action_addPrinterAerco << action_addPrinterLprint3
										<< action_addPrinterTs2040;

		if (model_info->rom_size == 16 * 1024)
			add_actions.append(action_addCurrahMicroSpeech); // only 48k machines work
		if (model_info->ram_size == 16 * 1024) add_actions.append(action_addCheetah32kRam);
		if (machine->ula->isA(isa_UlaZxsp)) add_actions.append(action_addSpectraVideo);

		items_menu->addActions(add_actions);
		items_menu->addAction(action_showLenslok);

		if (alwaysAddAy && !machine->ay) action_addDidaktikMelodik->setChecked(true);
		if (alwaysAddJoy && !machine->joystick) action_addKempstonJoy->setChecked(true);
		if (alwaysAddRam && machine->ram.count() < 48 kB) action_addCheetah32kRam->setChecked(true);
		if (alwaysAddDivide && !machine->model_info->has_floppy_drive && machine->model_info->canAttachDivIDE())
			action_addDivIDE->setChecked(true);

		action_RzxRecord->setEnabled(yes);
		action_RzxRecordAutostart->setEnabled(yes);
		action_RzxRecordAppendSna->setEnabled(yes);
	}

	else if (model == jupiter)
	{
		add_actions = QList<QAction*>() << action_addJupiter16kRam;
		items_menu->addActions(add_actions);
		if (alwaysAddRam) action_addJupiter16kRam->setChecked(true);

		action_RzxRecord->setEnabled(no);
		action_RzxRecordAutostart->setEnabled(no);
		action_RzxRecordAppendSna->setEnabled(no);
	}

	items_menu->removeAction(action_dummy); // falls sie drin ist

	machine->suspend();
	machine->powerOn();

	in_machine_ctor = was_in_machine_ctor;
	return machine.get();
}

void MachineController::openFile()
{
	xlogIn("MachineController:openFile()");

	cstr filter;
	//	if(machine->isA(isa_MachineZxsp))
	filter =
		"ZXSP Snapshots (*.z80 *.sna);;" // <-- default
		"ZX80 Snapshots (*.z80 *.o *.80);;"
		"ZX81 Snapshots (*.z80 *.p *.81 *.p81);;"
		"Jupiter Ace Snapshots (*.z80 *.ace);;"
		"Discs (*.dsk);;"
		"Tapes (*.tzx *.tap);;"
		"Roms (*.rom);;"
		"Recordings (*.rzx);;"
		"Source files (*.asm *.ass *.src *.s);;"
		"All Files (*)";

	if (machine->isA(isa_MachineZx81))
		filter =
			"ZX81 Snapshots (*.z80 *.p *.81 *.p81);;" // <-- default
			"ZX80 Snapshots (*.z80 *.o *.80);;"
			"ZXSP Snapshots (*.z80 *.sna);;"
			"Jupiter Ace Snapshots (*.z80 *.ace);;"
			"Discs (*.dsk);;"
			"Tapes (*.tzx *.tap);;"
			"Roms (*.rom);;"
			"Source files (*.asm *.ass *.src *.s);;"
			"All Files (*)";

	if (machine->isA(isa_MachineZx80))
		filter =
			"ZX80 Snapshots (*.z80 *.o *.80);;" // <-- default
			"ZX81 Snapshots (*.z80 *.p *.81 *.p81);;"
			"ZXSP Snapshots (*.z80 *.sna);;"
			"Jupiter Ace Snapshots (*.z80 *.ace);;"
			"Discs (*.dsk);;"
			"Tapes (*.tzx *.tap);;"
			"Roms (*.rom);;"
			"Source files (*.asm *.ass *.src *.s);;"
			"All Files (*)";

	if (model == jupiter)
		filter =
			"Jupiter Ace Snapshots (*.ace *.z80);;" // <-- default
			"ZXSP Snapshots (*.z80 *.sna);;"
			"ZX80 Snapshots (*.z80 *.o *.80);;"
			"ZX81 Snapshots (*.z80 *.p *.81 *.p81);;"
			"Discs (*.dsk);;"
			"Tapes (*.tzx *.tap);;"
			"Roms (*.rom);;"
			"Source files (*.asm *.ass *.src *.s);;"
			"All Files (*)";

	cstr filepath = selectLoadFile(this, "Open Snapshot", filter);
	if (filepath) loadSnapshot(filepath);
}

void MachineController::reloadFile()
{
	xlogIn("MachineController:ReloadFile");
	cstr path = dupstr(
		filepath ? filepath :
				   getRecentFile(RecentFiles, 0).toUtf8().data()); // note: filepath will be deleted by init_with_file()
	if (path && *path) loadSnapshot(path);
}

void MachineController::saveAs()
{
	xlogIn("MachineController:SaveAs");

	bool f = machine->suspend();

	cstr filter = catstr(
		machine->rzxIsLoaded() ? "RZX Recording (*.rzx);;" : "",
		model == zx80													   ? "ZX80 Snapshots (*.z80 *.o *.80);;" :
		model == zx81													   ? "ZX81 Snapshots (*.z80 *.p *.81);;" :
		model == jupiter												   ? "Jupiter Ace Snapshots (*.z80 *.ace);;" :
		machine->isA(isa_MachineZxsp) && NV(machine->ram).count() <= 48 kB ? "ZXSP Snapshots (*.z80 *.sna);;" :
																			 "ZXSP Snapshots (*.z80);;",
		"Rom (*.rom);;", model_info->isA(isa_MachineZxsp) ? "Screenshot (*.scr);;" : "", "All Files (*)");

	cstr filepath = selectSaveFile(this, "Save snapshot", filter);
	if (filepath)
	{
		try
		{
			nvptr(machine)->saveAs(filepath);
			setFilepath(filepath);
		}
		catch (AnyError& e)
		{
			showAlert("%s", e.what());
		}
	}

	if (f) machine->resume();
}


// =======================================================
//				Qt virtual functions
// =======================================================

void MachineController::resizeEvent(QResizeEvent* e)
{
	// resize Event:
	// reimplemented virtual Qt method
	// update the 'zoom' entries in the windows menu
	// note: interessanterweise funktioniert das genau so, wie ich will:
	//   wenn man mit der Maus das Fenster resized bekommt man das ResizeEvent
	//   wenn man jetzt die zugehörige QAction checked, müsste eigentlich
	//   via signal&slot setZoom() aufgerufen werden, was das Fenster auf die
	//   Zoom-typische Default-Größe resizen sollte. Tut es aber nicht. Maus-draggen geht vor.
	//   => Ich brauche die Signals nicht zu blocken, wodurch auch die QActionGroup funktioniert.
	//   Auch können wir hier schon die richtige Größe aus dem QGLScreen auslesen. Super.

	QMainWindow::resizeEvent(e);
	QAction* a = isFullScreen() ? action_fullscreen : action_zoom[screen->getZoom() - 1];
	a->setChecked(1);
}

void MachineController::changeEvent(QEvent* e)
{
	xlogIn("MachineController[%s]:changeEvent(%i)", machine->name, int(e->type()));

	QWidget::changeEvent(e);

	if (e->type() == QEvent::ActivationChange)
	{
		bool activated = appl->activeWindow() == this;
		if (activated)
		{
			xlogline("window activated");

			if (this != front_machine_controller)
			{
				if (front_machine_controller)
				{
					front_machine_controller->allKeysUp();
					front_machine_controller->hideAllToolwindows();
				}
				front_machine			 = machine.get();
				front_machine_controller = this;
				showAllToolwindows();
			}
		}
		else // deactivated
		{
			xlogline("window deactivated");
		}
		window_menu->checkWindows();
	}
}

void MachineController::contextMenuEvent(QContextMenuEvent* e)
{
	xlogIn("MachineController:contextMenuEvent");

	QPoint gpos = e->globalPos();
	// QPoint lpos = e->pos();
	context_menu->popup(gpos /*,QAction*atAction=0*/);
	e->accept();
}

bool MachineController::event(QEvent* e)
{
	if (XXLOG)
		//	if(XXLOG || (XLOG && e->type()!=129 && e->type()!=173))
		logIn("MachineController:event: %s", QEventTypeStr(e->type()));

	//	switch(int(e->type()))
	//	{
	//	case QEvent::WindowActivate:
	//	//	if(isActiveWindow()){}
	//	//	is_active_window=yes;
	//	//	tool_windows->showAllInspectors();
	//		break;
	//	case QEvent::WindowDeactivate:
	//	//	is_active_window=no;
	//		tool_windows->hideAllInspectors();
	//		break;
	//	default:
	//		break;
	//	}
	return QMainWindow::event(e);
	//	return 0;	// not processed
}

static KbdModifiers zxmodifiers(uint32 qtm) // helper
{
	uint zxm = 0;
	if (qtm & Qt::ShiftModifier) zxm |= KbdModifiers::ShiftKeyMask;
	if (qtm & Qt::MetaModifier) zxm |= KbdModifiers::ControlKeyMask;
	if (qtm & Qt::AltModifier) zxm |= KbdModifiers::AltKeyMask;
	return KbdModifiers(zxm);
}

void MachineController::allKeysUp()
{
	if (machine && machine->keyboard) machine->keyboard->allKeysUp();
	keyboardJoystick->allKeysUp();
}

inline bool cmdkey_involved(QKeyEvent* e)
{
	return (e->modifiers() & Qt::ControlModifier) || (e->key() == Qt::Key_Control);
}

static uint16 get_charcode(QKeyEvent* e)
{
	// e.key()				Großbuchstabe der Taste, wenn kein Modifierkey gedrückt wäre
	// e.modifiers()		Maske aller gedrückter Modifier
	// 							Qt::SHIFT   = Shift-Taste
	// 							Qt::META    = Control-Taste
	// 							Qt::CTRL    = Cmd-Taste
	// 							Qt::ALT     = Alt-Taste
	// e.nativeModifiers()	OSX-Modifiermaske, außer wenn Modifiertaste alleine gedrückt ist, dann 0
	// e.nativeScanCode()	nutzlos: 0 oder 1
	// e.nativeVirtualKey()	OSX-Keycode, außer wenn Modifiertaste alleine gedrückt ist, dann 0
	// 						ACHTUNG: Der Tastencode für 'A' ist auch 0!
	// e.text()				resulting printable char, empty for control codes, cursor and modifier keys

	// -->

	// modifer:	   keycode=0 & text=0 & keycap=0x0100002x --> keycode=0 & charcode=0
	// cursor:     keycode>0 & text=0 & keycap=0x0100001x --> keycode>0 & charcode=keycap&0xff
	// bs,esc,ret: keycode>0 & text>0 & keycap>0          --> keycode>0 & charcode=text
	// plain key:  keycode≥0 & text>0 & keycap≥' '	      --> keycode≥0 & charcode=text
	// key + alt:  keycode≥0 & text>0 & keycap≥' '        --> keycode≥0 & charcode=lc(keycap)
	// key + ctrl: keycode≥0 & text=0 & keycap≥' '		  --> keycode≥0 & charcode=lc(keycap)

	// return charcode = 0 --> modifier key
	//        charcode > 0 --> text or keycap

	// returned charcode may be non-ascii and therefore mapped to nothing by Keyboard
	// characters entered with ALT are not recognized, e.g.: ©£@[]|{}\ on German keyboard,
	// because ALT-keys are mapped to their keycap because ALT serves as a 2nd SHIFT in Keyboard

	int		keycap		= e->key();				 // key without modifiers, uppercase, e.g. "1" for "!"
	QString text		= e->text();			 // resulting character
	uint32	modifiers	= e->modifiers();		 // modifier mask
	uint	keycode		= e->nativeVirtualKey(); // OSX keycode
	bool	is_modifier = keycode == 0 && keycap > 0xFFFF;

	xxlogline("key cap:   0x%08x (%i) '%c'", keycap, keycap, keycap >= 32 && keycap < 0x7f ? keycap : ' ');
	xxlogline("modifiers: 0x%08x", modifiers);
	xxlogline("key code:  0x%08x", keycode);
	xxlogline("text:      %s", quotedstr(text.toUtf8().data()));

	return is_modifier									? 0u :
		   text.count() && ~modifiers & Qt::AltModifier ? text.at(0).unicode() :
														  uint16(tolower(uint16(keycap)));
}

void MachineController::keyPressEvent(QKeyEvent* e)
{
	if (e->isAutoRepeat()) return;

	xlogIn("MachineController:keyPressEvent");

	if (cmdkey_involved(e)) { allKeysUp(); }
	else
	{
		uint32 modifiers = e->modifiers();
		uint8  keycode	 = uint8(e->nativeVirtualKey());
		uint16 charcode	 = get_charcode(e);

		if (machine && machine->rzxIsLoaded() && action_RzxRecordAutostart->isChecked())
		{
			NVPtr<Machine> machine(this->machine.get());
			if (machine->rzxIsPlaying()) machine->rzxStartRecording(); // -> callback rzxStateChanged()
		}

		if (charcode && keyboardJoystick->isActive())
			for (uint i = 0; i < 5; i++)
				if (keyjoy_keys[i] == keycode)
				{
					keyboardJoystick->keyDown(1 << i);
					return;
				}

		if (machine && machine->keyboard) machine->keyboard->realKeyDown(charcode, keycode, zxmodifiers(modifiers));
	}

	emit signal_keymapModified();
}

void MachineController::keyReleaseEvent(QKeyEvent* e)
{
	xlogIn("MachineController:keyReleaseEvent");

	if (cmdkey_involved(e)) { allKeysUp(); }
	else
	{
		uint32 modifiers = e->modifiers();
		uint8  keycode	 = uint8(e->nativeVirtualKey());
		uint16 charcode	 = get_charcode(e);

		if (charcode && keyboardJoystick->isActive())
			for (uint i = 0; i < 5; i++)
				if (keyjoy_keys[i] == keycode)
				{
					keyboardJoystick->keyUp(1 << i);
					return;
				}
		if (machine && machine->keyboard) machine->keyboard->realKeyUp(charcode, keycode, zxmodifiers(modifiers));
	}

	emit signal_keymapModified();
}


// =======================================================
//                  RUN THE MACHINE
// =======================================================

void MachineController::setFilepath(cstr path)
{
	delete[] filepath;
	filepath = nullptr;
	if (path)
	{
		filepath = newcopy(path);
		addRecentFile(RecentFiles, filepath);
	}
	keyjoy_fnmatch_pattern = getKeyJoyFnmatchPattern(keyjoy_keys, filepath);

	//	setTitle();
	//}
	// void MachineController::setTitle()
	//{

	setWindowTitle(filepath ? filename_from_path(filepath) : model_info->name);
	window_menu->setTitle();
	model_actiongroup->actions().at(model)->setChecked(1); // add checkmark to current model in model menu
}

void MachineController::saveScreenshot() // controlMenu
{
	xlogIn("MachineController:save_screenshot");
	cstr dir  = fullpath("~/Desktop/", yes);
	cstr name = filepath ? basename_from_path(filepath) : model_info->name;
	str	 time = datetimestr(time_t(now())); // time[10]='-'; time[13]='.'; time[16]='.';
	cstr path = catstr(dir, name, " [", time, "]", ".gif");
	try
	{
		screen->saveScreenshot(path);
	}
	catch (FileError& e)
	{
		showAlert("File error: \n%s", e.what());
	}
}

void MachineController::recordMovie(bool f) // controlMenu
{
	xlogIn("MachineController:record_movie(%s)", f ? "on" : "off");

	if (f)
	{
		if (!screen->isRecording())
		{
			cstr dir  = fullpath("~/Desktop/", yes);
			cstr name = filepath ? basename_from_path(filepath) : model_info->name;
			str	 time = datetimestr(time_t(now())); // time[10]='-'; time[13]='.'; time[16]='.';
			cstr path = catstr(dir, name, " [", time, "]", ".gif");

			try
			{
				bool f = action_gifAnimateBorder->isChecked();
				screen->startRecording(path, f);
			}
			catch (FileError& e)
			{
				showAlert("File error: \n%s", e.what());
				action_recordMovie->setChecked(off);
			}
		}
	}
	else { screen->stopRecording(); }
}

void MachineController::setKeyboardMode(KbdMode mode)
{
	xlogIn("MachineController:set_keyboard_mode");
	machine->keyboard->setKbdMode(mode);
	action_setKbdBasic->setChecked(mode == kbdbasic);
	action_setKbdGame->setChecked(mode == kbdgame);
	action_setKbdBtZXKbd->setChecked(mode == kbdbtzxkbd);
}

void MachineController::enableAudioIn(bool f)
{
	xlogIn("MachineController:enableAudioIn");
	machine->audio_in_enabled = f;
	action_audioin_enabled->setChecked(f);
}

void MachineController::setWindowZoom(int factor)
{
	xlogIn("MachineController:set_window_zoom()");

	showNormal();
	limit(1, factor, 4);
	QRect box = geometry(); // rect();
	int	  w = (32 + 2 * 4) * 8 * factor, h = (24 + 2 * 3) * 8 * factor, x = int(box.x() + (box.width() - w) / 2),
		y = int(box.y() + (box.height() - h) * 1 / 4);

	setGeometry(x, max(44, y), w, h);
	show();
	//	arrangeOverlays();			nötig? müsste auch ein resizeEvent geben
	// update_all = yes;
}

void MachineController::powerResetMachine()
{
	xlogIn("MachineController:powerOffOn");

	setFilepath(nullptr);
	machine->powerOff(); // must be suspended
	screen->hideOverlayPlay();
	screen->hideOverlayRecord();
	setKeyboardMode(settings.get_KbdMode(key_new_machine_keyboard_mode, kbdbasic));
	machine->powerOn();
}

void MachineController::resetMachine()
{
	xlogIn("MachineController:Reset");

	setFilepath(nullptr);
	nvptr(machine)->reset();
	screen->hideOverlayPlay();
	screen->hideOverlayRecord();
	setKeyboardMode(settings.get_KbdMode(key_new_machine_keyboard_mode, kbdbasic));
}

void MachineController::enableBreakpoints(bool f)
{
	xlogIn("MachineController:enableBreakpoints(%i)", int(f));

	machine->lock();
	if (f) machine->cpu_options |= cpu_break_rwx;
	else machine->cpu_options &= ~cpu_break_rwx;
	machine->unlock();
}

void MachineController::haltMachine(bool f)
{
	xlogIn("MachineController:haltMachine(%i)", int(f));
	if (f) machine->suspend();
	else machine->resume();
	// note: we'll get a callback to machineRunStateChanged()
}

void MachineController::addExternalItem(isa_id item_id, bool add)
{
	xlogIn("MachineController:slot_add_external_item(%i,%s)", item_id, add ? "add" : "remove");

	bool f = machine->suspend();

	if (add) nvptr(machine)->addExternalItem(item_id);
	else nvptr(machine)->removeItem(item_id);

	if (f) machine->resume();
}

void MachineController::addExternalRam(isa_id item_id, bool add, uint options)
{
	// Add or remove Ram
	//  -> remove currently attached Ram, if any
	//  -> add or remove Ram
	//  -> reset machine

	xlogIn("MachineController:slot_add_external_ram(%i,%s)", item_id, add ? "add" : "remove");

	bool f = machine->powerOff();

	nvptr(machine)->remove<ExternalRam>();
	if (add) nvptr(machine)->addExternalRam(item_id, options);

	if (f) machine->powerOn();
}

void MachineController::addMultiface1(bool add)
{
	bool f = machine->suspend();

	if (add)
	{
		bool joystick_enabled = settings.get_bool(key_multiface1_enable_joystick, yes);
		nvptr(machine)->addMultiface1(joystick_enabled);
	}
	else nvptr(machine)->remove<Multiface1>();

	if (f) machine->resume();
}

void MachineController::addMemotech64kRam(bool add)
{
	uint dip_switches = settings.get_uint(key_memotech64k_dip_switches, 0x06);
	addExternalRam(isa_Memotech64kRam, add, dip_switches);
}

void MachineController::addZx3kRam(bool add)
{
	uint ramsize = settings.get_uint(key_zx3k_ramsize, 3 kB);
	addExternalRam(isa_Zx3kRam, add, ramsize);
}

void MachineController::addDivIDE(bool add)
{
	xlogIn("MachineController:addDivIDE(%i)", add);

	bool f = machine->powerOff();

	if (add)
	{
		uint ramsize  = settings.get_uint(key_divide_ram_size, 32 kB);
		cstr romfile  = settings.get_cstr(key_divide_rom_file);
		cstr diskfile = settings.get_cstr(key_divide_disk_file);

		DivIDE* divide = nvptr(machine)->addDivIDE(ramsize, romfile);

		cstr err = nullptr;
		if (divide->getRomFilepath() == nullptr) // failed to load
			err = divide->insertRom(romfile);	 // error should repeat

		if (err && romfile)
		{
			showWarning("Failed to load %s\n%s\nLoading default Rom instead.", romfile, err);
			settings.remove(key_divide_rom_file); // suppress error next time
			err = divide->insertDefaultRom();
		}

		if (err) showAlert("Failed to load internal Rom\n%s\nRemoving jumper_E", err);

		if (diskfile) divide->insertDisk(diskfile); // shows it's own errors
	}
	else nvptr(machine)->remove<DivIDE>();

	if (f) machine->powerOn();

	action_addDivIDE->setChecked(add);
}

void MachineController::addSpectraVideo(bool add)
{
	xlogIn("MachineController::addSpectraVideo(%i)", add);

	bool f = machine->powerOff();

	if (add)
	{
		using Dip = SpectraVideo::DipSwitches;

		uint dip_switches = 0;
		if (settings.get_bool(key_spectra_enable_if1_rom_hooks, false)) dip_switches |= Dip::EnableIf1RomHooks;
		if (settings.get_bool(key_spectra_enable_rs232, false)) dip_switches |= Dip::EnableRs232;
		if (settings.get_bool(key_spectra_enable_joystick, false)) dip_switches |= Dip::EnableJoystick;
		if (settings.get_bool(key_spectra_enable_new_video_modes, true)) dip_switches |= Dip::EnableNewVideoModes;

		nvptr(machine)->addSpectraVideo(dip_switches);
	}
	else nvptr(machine)->removeSpectraVideo();

	if (f) machine->powerOn();

	action_addSpectraVideo->setChecked(add);
}

void MachineController::showLenslok(bool f)
{
	xlogIn("MachineController:show_lenslok(%s)", f ? "on" : "off");

	if (f) // show
	{
		assert(!lenslok);
		cstr name1 = filepath ? basename_from_path(filepath) : nullptr;
		cstr name2 = nullptr;
		{
			NVPtr<Machine> m(machine.get());
			if (m->taperecorder->isLoaded()) name2 = basename_from_path(m->taperecorder->getFilepath());
			else if (m->fdc && m->fdc->getSelectedDrive()->diskLoaded())
				name2 = basename_from_path(m->fdc->getSelectedDrive()->disk->filepath);
		}
		lenslok = new Lenslok(this, name1, name2);
		connect(lenslok, &QObject::destroyed, this, [=] {
			action_showLenslok->blockSignals(1);
			action_showLenslok->setChecked(0);
			action_showLenslok->blockSignals(0);
		});
		lenslok->show();
	}
	else // remove
	{
		delete lenslok;
		lenslok = nullptr;
	}
}

void MachineController::setRzxAutostartRecording(bool f)
{
	// toggle setting for "during rzx playback autostart recording on any key press":

	settings.setValue(key_rzx_autostart_recording, f);
}

void MachineController::setRzxAppendSnapshots(bool /*f*/)
{
	// toggle setting for "append new snapshots to rzx file (do not rewind the rzx file)"
	// note: setting will be picked from the QAction.
}

void MachineController::setRzxRecording(bool f)
{
	// start or stop recording into rzx file

	if (!machine) return;
	if (f) nvptr(machine)->rzxStartRecording();
	else nvptr(machine)->rzxStopRecording();
}


// ###########################################################################################
//								Callbacks
// ###########################################################################################

static QAction* find_action_for_item(QList<QAction*>& array, const volatile IsaObject* item) // helper
{
	foreach (QAction* action, array)
	{
		if (action->data().toInt() == item->id) return action;
	}

#if XXLOG
	logline("*** didn't find item %s (%s) in action list: ***", item->name, isa_names[item->id]);
	foreach (QAction* a, array) { logline("  • %s", isa_names[a->data().toInt()]); }
#endif

	return nullptr;
}

void MachineController::itemAdded(std::shared_ptr<Item> item) volatile
{
	// callback from Item c'tor

	xlogIn("MachineController::itemAdded()");
	assert(isMainThread());

	// we delay the updates because in case of a new machine it is not yet stored in this->machine:
	bool				force = !in_machine_ctor;
	std::weak_ptr<Item> item_wp {item};
	QTimer::singleShot(0, NV(this), [=] { NV(this)->item_added(item_wp, force); });
}

void MachineController::item_added(std::weak_ptr<Item> item_wp, bool force)
{
	// we manage the 'Extensions' menu here.
	// 'add' and 'show' actions are un|checked and dis|enabled
	// an existing ToolWindow is reused
	// Assumptions:
	// 	 external item: the 'add' action exists (and is visible in the menu)

	// if zxsp is started with a file to load then the initial default machine
	// may be unsuitable and is immediately destroyed and the required one is created.
	// then we get itemAdded events for items which no longer exist.
	Item* item = item_wp.lock().get();
	if (!item) return;
	assert(NV(machine)->all_items.contains(item));

	// Ula and Mmu are handled as one item:
	if (dynamic_cast<Mmu*>(item)) return;
	assert(dynamic_cast<Ula*>(item) == nullptr || machine->mmu != nullptr);

	assert(item); //wg. bogus lint warning
	int g = item->grp_id;

	if (item->isExternal() && g != isa_TapeRecorder)
	{
		QAction* addAction = find_action_for_item(add_actions, item);
		if (addAction == nullptr) { showAlert("no addAction found for item %s", item->name); }
		if (addAction != nullptr)
		{
			addAction->blockSignals(true);
			addAction->setChecked(true);
			addAction->setEnabled(true);
			addAction->blockSignals(false);
		}
	}

	QAction* showaction = new QAction(item->name, this /*parent*/);
	showaction->setCheckable(yes);
	showaction->setData(QVariant(item->id));
	connect(showaction, &QAction::toggled, this, [=](bool f) { toggleToolwindow(item, showaction, f); });

	switch (g)
	{
	case isa_TapeRecorder:
		showaction->setShortcut(CTRL | Qt::Key_T);
		showaction->setIcon(QIcon(":/Icons/tape.gif"));
		break;
	case isa_Ula:
		showaction->setShortcut(CTRL | Qt::Key_U);
		showaction->setIcon(QIcon(":/Icons/screen.gif"));
		break;
	case isa_Keyboard:
		showaction->setShortcut(CTRL | Qt::Key_K);
		showaction->setIcon(QIcon(":/Icons/mini_zxsp.gif"));
		break;
	case isa_Ay:
		showaction->setShortcut(CTRL | Qt::Key_Y);
		showaction->setIcon(QIcon(":/Icons/ay.gif"));
		break;
		//	case isa_Mmu:			// TCC
	case isa_MassStorage: // +3 internal Floppy Disc: MassStorage wg. Inspector Window Gruppe
	case isa_Fdc:
		showaction->setShortcut(CTRL | Qt::Key_D);
		showaction->setIcon(QIcon(":/Icons/save.png"));
		break;
	case isa_Printer: showaction->setIcon(QIcon(":/Icons/printer.gif")); break;
	case isa_Mouse: showaction->setIcon(QIcon(":/Icons/mouse.png")); break;
	case isa_Z80: showaction->setShortcut(CTRL | Qt::Key_Z); break;
	case isa_Joy:
		showaction->setShortcut(CTRL | Qt::Key_J);
		showaction->setIcon(
			QIcon(dynamic_cast<Joy&>(*item).getNumPorts() == 1 ? ":/Icons/joystick-1.gif" : ":/Icons/joystick-2.gif"));
		break;
	}

	window_menu->insertAction(separator, showaction);
	show_actions.append(showaction);

	showInspector(item, showaction, force);
}

void MachineController::itemRemoved(Item* item) volatile
{
	// callback from Item d'tor

	xlogIn("MachineController::itemRemoved()");
	assert(isMainThread());

	if (in_dtor) return; // during this.dtor the items_menu is already purged
	bool force = !in_machine_dtor;
	NV(this)->item_removed(item, force);
}

void MachineController::item_removed(Item* item, bool force)
{
	hideInspector(item, force);

	QAction* addAction	= find_action_for_item(add_actions, item);
	QAction* showAction = find_action_for_item(show_actions, item);

	assert(window_menu);
	window_menu->removeAction(showAction);
	show_actions.removeAll(showAction);
	delete showAction;

	if (addAction)
	{
		addAction->blockSignals(true);
		addAction->setChecked(false);
		addAction->setEnabled(true);
		addAction->blockSignals(false);
	}
}

void MachineController::rzxStateChanged() volatile
{
	// callback from machine, any thread

	QTimer::singleShot(0, NV(this), [this] {
		assert(isMainThread());
		action_RzxRecord->blockSignals(true);
		action_RzxRecord->setChecked(nvptr(NV(this)->machine)->rzxIsRecording());
		action_RzxRecord->blockSignals(false);
	});
}

void MachineController::memoryModified(Memory* m, uint how) volatile
{
	// callback from machine

	//assert(isMainThread());
	emit NV(this)->signal_memoryModified(m, how);
}

void MachineController::machineSuspendStateChanged() volatile
{
	// callback from machine, any thread

	QTimer::singleShot(0, NV(this), [this] {
		assert(isMainThread());
		bool f = NV(this)->machine->isSuspended();
		action_suspend->blockSignals(yes);
		action_suspend->setChecked(f);
		action_suspend->blockSignals(no);
		action_stepIn->setEnabled(f);
		action_stepOut->setEnabled(f);
		action_stepOver->setEnabled(f);
	});
}


// ###########################################################################################
//								Queries
// ###########################################################################################

QAction* MachineController::findShowActionForItem(const volatile IsaObject* item)
{
	return find_action_for_item(show_actions, item);
}

bool MachineController::isRzxAutostartRecording() const volatile { return action_RzxRecordAutostart->isChecked(); }

QList<QAction*> MachineController::getKeyboardActions() // for context menus
{
	return QList<QAction*>() << action_setKbdBasic << action_setKbdGame << action_setKbdBtZXKbd;
}


// ###########################################################################################
//								Tool Windows
// ###########################################################################################


void MachineController::showAllToolwindows()
{
	// called from changeEvent()
	// for the new front window

	assert(isMainThread());
	xlogIn("MachineController::showAllToolWindows()");

	foreach (ToolWindow* toolwindow, tool_windows)
	{
		toolwindow->restore_window_position();
		toolwindow->show();
	}
}

void MachineController::hideAllToolwindows()
{
	// called from changeEvent()
	// for the old front window

	assert(isMainThread());
	xlogIn("MachineController::hideAllToolWindows()");

	foreach (ToolWindow* toolwindow, tool_windows)
	{
		toolwindow->save_window_position();
		toolwindow->hide();
	}
}

void MachineController::toggleToolwindow(volatile IsaObject* item, QAction* showaction, bool f)
{
	//	The show_action for an item was toggled
	//	show existing or create and show a new Inspector for this item
	//	or close the exisiting one.

	assert(isMainThread());
	xlogIn("MachineController::toggleToolWindow(%s)", f ? "show" : "hide");

	if (f) // show
	{
		newToolwindow(item, showaction)->show();
	}
	else // hide
	{
		foreach (ToolWindow* toolwindow, tool_windows)
		{
			if (toolwindow->item == item) { toolwindow->deleteLater(); }
		}
	}
}

ToolWindow* MachineController::findToolWindowForItem(const volatile IsaObject* item)
{
	// find toolwindow for this item.
	// if more than one toolwindow shows an inspector for this item, then any is returned.
	// return NULL if no toolwindow found
	// mostly for use in ToolWindow::kill()

	assert(item != nullptr);

	logIn("toolwindows.size = %i", tool_windows.count());
	foreach (ToolWindow* toolwindow, tool_windows)
	{
		logline("next toolwindows");
		if (toolwindow->item == item) return toolwindow;
	}
	return nullptr;
}

ToolWindow* MachineController::newToolwindow(volatile IsaObject* item, QAction* showaction)
{
	//	create new tool window with item inspector
	//	• for item with associated show_action
	//	• or empty inspector if item == NULL

	assert(isMainThread());
	xlogIn("MachineController::newToolWindow()");

	ToolWindow* toolwindow = new ToolWindow(this, item, showaction);
	tool_windows << toolwindow;

	connect(toolwindow, &QObject::destroyed, this, [=] {
		// remove the toolwindow from the tool_windows[] list.
		// note: unchecking of showaction is handled in ToolWindow::kill().
		tool_windows.removeAll(toolwindow);
	});

	return toolwindow;
}

void MachineController::showInspector(IsaObject* item, QAction* showaction, bool force)
{
	//	An Item has been added to the machine
	//	If we have a toolwindow for the item's group
	//	then show this item in this inspector
	//	else if force==true create new toolwindow

	//	from addItem menu action: force = true  => always show inspector
	//	from Machine creator:	  force = false => reuse inspectors from old machine only

	assert(isMainThread());
	assert(item);
	xlogIn("MachineController::show_inspector(%i)", force);

	if (!showaction) showaction = findShowActionForItem(item);
	assert(showaction);

	// reuse existing empty toolwindow for the item's group:
	foreach (ToolWindow* toolwindow, tool_windows)
	{
		if (toolwindow->item == nullptr && toolwindow->grp_id == item->grp_id)
		{
			toolwindow->kill();
			toolwindow->init(item, showaction);
			toolwindow->show(); // should be visible anyway
			return;
		}
	}

	// else open new toolwindow:
	if (force) newToolwindow(item, showaction)->show();
}

void MachineController::hideInspector(IsaObject* item, bool force)
{
	// An Item has been removed.
	// if force==true  close all it's toolwindows
	// if force==false leave toolwindows open and only replace inspectors with the 'Empty Inspector'

	//	from addItem menu action: force = true  => always close inspector
	//	from Machine destructor:  force = false => leave toolwindow open for new machine

	assert(isMainThread());
	xlogIn("MachineController::hide_inspector(%i)", force);

	for (int i = tool_windows.count(); --i >= 0;)
	{
		ToolWindow* toolwindow = tool_windows[i];

		if (toolwindow->item == item)
		{
			if (force) { delete toolwindow; }
			else
			{
				toolwindow->kill();
				toolwindow->init();
			}
		}
	}
}

} // namespace gui


/*






























*/
