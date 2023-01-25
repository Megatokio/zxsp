#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QMainWindow>
#include <QActionGroup>
#include "cpp/cppthreads.h"
#include "zxsp_types.h"
#include "IsaObject.h"
#include "ZxInfo/ZxInfo.h"
class Overlay;
class ToolWindow;


extern void showAlert   ( cstr msg, ... )  __printflike(1,2);	// thread-safe  "red" alert:	stop sign
extern void showWarning ( cstr msg, ... )  __printflike(1,2);	// thread-safe  "yellow" alert: attention sign
extern void showInfo	( cstr msg, ... )  __printflike(1,2);	// thread-safe  a friendly information alert


class MachineController : public QMainWindow
{
	Q_OBJECT
	Q_DISABLE_COPY(MachineController)

	friend void runMachinesForSound();
	friend void setFrontMachineController(MachineController*);
	friend void setFrontMachine(Machine*);
	friend class ScreenTc2048;			// --> gif_recorder(); TODO: bereinigen
	friend class ToolWindow;			// maintained by ToolWindow
	friend class ConfigureKeyboardJoystickDialog;

	bool in_ctor, in_dtor, in_machine_ctor, in_machine_dtor;

// general info
	Model			model;
	const ZxInfo*	model_info;			// generic model info
	cstr			filepath;			// Snapshot file path or nil

// controlled objects:
	volatile Machine* machine;
	Screen*			screen;				// ScreenZxsp* or ScreenMono*
	IsaObject*		mem[4];
	Lenslok*		lenslok;

	uint8		keyjoy_keys[5];			// (RLDUF) Qt keycode to use for keyboard joystick up-down-left-right-fire
	cstr		keyjoy_fnmatch_pattern;	// the filename pattern, for which the keys were set

// GUI:
	//ToolWindowController*	tool_windows;
	QList<QAction*> add_actions;		// note: objects get not owned by the array
	QList<QAction*> show_actions;
	QList<ToolWindow*>	tool_windows;

public:
	QAction
		*separator,				*separator2,			*separator3,			*action_dummy,
		*action_zoom[4],		*action_fullscreen,
		*action_showAbout,		*action_showPreferences,
		*action_newMachine,		*action_openFile,		*action_reloadFile,		*action_closeWindow,
		*action_saveAs,			*action_screenshot,		*action_recordMovie,	*action_pwrOnReset,
		*action_recentFiles,
		*action_reset,			*action_nmi,			*action_suspend,		*action_stepIn,
		*action_stepOver,		*action_stepOut,		*action_minimize,		*action_enable_breakpoints,
		*action_setKbdGame,		*action_setKbdBasic,	*action_setKbdBtZXKbd,
		*action_audioin_enabled,*action_setSpeed100_50,	*action_setSpeed100_60,
		*action_setSpeed120,	*action_setSpeed200,	*action_setSpeed400,	*action_setSpeed800,
		*action_RzxRecord,		*action_RzxRecordAutostart,	*action_RzxRecordAppendSna,
		*action_newInspector,	*action_showMachineImage,
		*action_showMemHex,		*action_showMemDisass,	*action_showMemAccess,	*action_showMemGraphical,
		*action_showLenslok,	*action_gifAnimateBorder,

		// add external items:
		*action_addDivIDE,		*action_addSpectraVideo,	*action_addCurrahMicroSpeech,
		*action_addFdcBeta128,	*action_addFdcD80,			*action_addFdcJLO,			*action_addFdcPlusD,
		*action_addKempstonJoy,	*action_addProtekJoy,		*action_addIcTester,		*action_addDktronicsDualJoy,
		*action_addZxPrinter,	*action_addPrinterAerco,	*action_addPrinterTs2040,	*action_addPrinterLprint3,
		*action_addZonxBox,		*action_addZonxBox81,		*action_addDidaktikMelodik,
		*action_addZx16kRam,	*action_addCheetah32kRam,	*action_addJupiter16kRam,	*action_addMemotech64kRam,
		*action_addTs1016Ram,	*action_addMemotech16kRam,	*action_addZx3kRam,			*action_addStonechip16kRam,
		*action_addMultiface1,	*action_addMultiface128,	*action_addMultiface3,		*action_addFullerBox,
		*action_addGrafPad,		*action_addKempstonMouse,	*action_addZxIf1,			*action_addZxIf2;

private:
	QActionGroup*model_actiongroup;		// --> model menu
	QMenu		*context_menu,	*file_menu,
				*model_menu,	*options_menu,
				*items_menu,	*control_menu,
				*memory_menu,	*speed_menu;
	WindowMenu*	window_menu;


//	static Model best_model_for_file(cstr filepath);
	Screen*		new_screen_for_model(Model);
	Machine*	new_machine_for_model(Model);

	QAction*	new_action		(cstr icon, cstr title, const QKeySequence& key, std::function<void(bool)> fu, isa_id id=isa_none);
	QAction*	new_action		(cstr icon, cstr title, const QKeySequence& key, std::function<void()> fu, isa_id id=isa_none);
	void		create_actions();
	void		create_menus();
	void		create_mainmenubar();
	void		kill_machine();
	Machine*	init_machine(Model, uint32 ramsize, bool ay, bool joy, bool alwaysAddRam, bool alwaysAddDivide);
	void		set_window_zoom(int);
	void		set_model(QAction*);	// from model_actiongroup
	void		open_file();
	void		reload_file();
	void		save_as();
	void		save_screenshot();
	void		record_movie(bool);
	void		power_reset_machine();
	void		reset_machine();
	void		halt_machine(bool);
	void		enable_breakpoints(bool);
	void		show_lenslok(bool);
	void		add_external_item(isa_id,bool);
	void		add_external_ram(isa_id,bool);
	void		add_spectra_video(bool);
	void		set_rzx_recording(bool);
	void		set_rzx_autostart_recording(bool);
	void		set_rzx_append_snapshots(bool);
	void		startup_open_toolwindows();
	void		set_filepath(cstr);
	void		set_keyboard_mode(KbdMode);
	void		enable_audio_in(bool);
	void		all_keys_up();

	ToolWindow*	new_toolwindow(volatile IsaObject* item = nullptr, QAction* showaction = nullptr);
	void		show_inspector(IsaObject*, QAction* showaction, bool force);	// from Item c'tor
	void		hide_inspector(IsaObject*, bool force);							// from Item d'tor
	void		toggle_toolwindow(volatile IsaObject*, QAction* actionshow, bool showhide);	// from action
	void		show_all_toolwindows();											// changeEvent()
	void		hide_all_toolwindows();											// changeEvent()

protected:
	void	contextMenuEvent(QContextMenuEvent*) override;
	void	changeEvent(QEvent*) override;
	bool	event(QEvent*) override;
	void	resizeEvent(QResizeEvent*) override;
public:
	void	keyPressEvent(QKeyEvent*) override;
	void	keyReleaseEvent(QKeyEvent*) override;


// ---- PUBLIC -------------------------------------------------------------

public:			MachineController(QString filepath);
				~MachineController();

	Machine volatile* getMachine()			{return machine;}
	Screen*		getScreen		() volatile	{ return screen; }	// callback from running machine
	void		setScreen		(Screen*);

	Model		getModel		()			{return model;}
	cZxInfo*	getModelInfo	()			{return model_info;}	// generic model info

	void		loadSnapshot	(cstr);
	bool		isRzxAutostartRecording() volatile const;
	QAction*	findShowActionForItem(const volatile IsaObject*);
	QList<QAction*> getKeyboardActions();
	ToolWindow*	findToolWindowForItem(const volatile IsaObject* item);

	void		memoryModified(Memory* m, uint how);	// callback from machine
	void		machineRunStateChanged() volatile;		// callback from machine
	void		rzxStateChanged() volatile;				// callback from machine
	void		itemAdded(Item*);						// callback from Item c'tor
	void		itemRemoved(Item*);						// callback from Item d'tor

signals:
	void		signal_keymapModified();
	void		signal_memoryModified(Memory*, uint how);
};












