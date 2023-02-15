// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Preferences.h"
#include "Dsp.h"
#include "Qt/Settings.h"
#include "ZxInfo.h"
#include "cpp/cppthreads.h"
#include "zxsp_types.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QRadioButton>


#define key_saved_session			  "settings/saved_session"			   /* QString  … filepath */
#define key_audio_playthrough_damping "settings/audio_playthrough_damping" /* int */

// helper
inline Qt::CheckState is_checked(bool f) { return f ? Qt::Checked : Qt::Unchecked; }


class MyGroupLabel : public QLabel
{
public:
	MyGroupLabel(cstr text) : QLabel(text)
	{
		QFont font = this->font();
		font.setBold(true);
		setFont(font);
		this->setContentsMargins(0, 10, 0, 0);
	}
};


Preferences::Preferences(QWidget* parent) : QWidget(parent)
{
	QGridLayout* gridlayout = new QGridLayout(this);
	gridlayout->setHorizontalSpacing(0);

	QCheckBox* check_for_update = new QCheckBox("Check for update (daily)");
	check_for_update->setCheckState(is_checked(settings.get_bool(key_check_for_update, yes)));
	connect(check_for_update, &QCheckBox::toggled, this, &Preferences::setCheckForUpdate);

	QCheckBox* start_open_keyboard = new QCheckBox("Open keyboard view");
	start_open_keyboard->setCheckState(is_checked(settings.get_bool(key_startup_open_keyboard, no)));
	connect(start_open_keyboard, &QCheckBox::toggled, this, &Preferences::setStartOpenKeyboard);

	QCheckBox* start_open_disk_drive = new QCheckBox("Open floppy disk drive");
	start_open_disk_drive->setCheckState(is_checked(settings.get_bool(key_startup_open_disk_drive, no)));
	connect(start_open_disk_drive, &QCheckBox::toggled, this, &Preferences::setStartOpenDiskDrive);

	QCheckBox* start_open_taperecorder = new QCheckBox("Open tape recorder");
	start_open_taperecorder->setCheckState(is_checked(settings.get_bool(key_startup_open_taperecorder, no)));
	connect(start_open_taperecorder, &QCheckBox::toggled, this, &Preferences::setStartOpenTaperecorder);

	QCheckBox* start_open_image = new QCheckBox("Open machine image");
	start_open_image->setCheckState(is_checked(settings.get_bool(key_startup_open_machine_image, no)));
	connect(start_open_image, &QCheckBox::toggled, this, &Preferences::setStartOpenMachineImage);

	QCheckBox* start_warn_audioin = new QCheckBox("Warn if setup of audio input fails");
	start_warn_audioin->setCheckState(is_checked(settings.get_bool(key_warn_if_audio_in_fails, yes)));
	connect(start_warn_audioin, &QCheckBox::toggled, this, &Preferences::setStartEnableAudioIn);
	start_warn_audioin->setEnabled(on);

	QCheckBox* save_and_restore_sessions = new QCheckBox("Save and restore session");
	save_and_restore_sessions->setCheckState(is_checked(settings.get_bool(key_save_and_restore_session, no)));
	connect(save_and_restore_sessions, &QCheckBox::toggled, this, &Preferences::setSaveAndRestore);
	save_and_restore_sessions->setEnabled(off);

	QCheckBox* new_machine_ay = new QCheckBox("Always attach soundchip");
	new_machine_ay->setCheckState(is_checked(settings.get_bool(key_always_attach_soundchip, no)));
	connect(new_machine_ay, &QCheckBox::toggled, this, &Preferences::setNewMachineAy);

	QCheckBox* new_machine_joystick = new QCheckBox("Always attach joystick");
	new_machine_joystick->setCheckState(is_checked(settings.get_bool(key_always_attach_joystick, no)));
	connect(new_machine_joystick, &QCheckBox::toggled, this, &Preferences::setNewMachineJoystick);

	QCheckBox* new_machine_rampack = new QCheckBox("Always attach Ram extension");
	new_machine_rampack->setCheckState(is_checked(settings.get_bool(key_always_attach_rampack, no)));
	connect(new_machine_rampack, &QCheckBox::toggled, this, &Preferences::setNewMachineRampack);

	QCheckBox* new_machine_divide = new QCheckBox("Always attach DivIDE interface");
	new_machine_divide->setCheckState(is_checked(settings.get_bool(key_always_attach_divide, no)));
	connect(new_machine_divide, &QCheckBox::toggled, this, &Preferences::setNewMachineDivide);

	QCheckBox* use_individual_settings = new QCheckBox("Remember settings for each snapshot");
	use_individual_settings->setCheckState(is_checked(settings.get_bool(key_use_individual_settings, no)));
	connect(use_individual_settings, &QCheckBox::toggled, this, &Preferences::setSnapshotIndividualSettings);
	use_individual_settings->setEnabled(off);

	QCheckBox* auto_start_stop_tape = new QCheckBox("Auto start/stop tape");
	auto_start_stop_tape->setCheckState(is_checked(settings.get_bool(key_auto_start_stop_tape, no)));
	connect(auto_start_stop_tape, &QCheckBox::toggled, this, &Preferences::setStartStopTape);

	QCheckBox* fast_load_tape = new QCheckBox("Instant load/save standard tape blocks");
	fast_load_tape->setCheckState(is_checked(settings.get_bool(key_fast_load_tape, no)));
	connect(fast_load_tape, &QCheckBox::toggled, this, &Preferences::setFastLoadTape);

// there are 2 overloaded signals in QComboBox: use cast to select the proper one:
#define FP(F) static_cast<void (QComboBox::*)(int)>(F)

	QComboBox* new_machine_kbdmode = new QComboBox();
	new_machine_kbdmode->addItem("Keyboard Game Mode");
	new_machine_kbdmode->addItem("Keyboard BASIC Mode");
	new_machine_kbdmode->addItem("Recreated ZX Keyboard Game Mode");
	new_machine_kbdmode->setCurrentIndex(settings.get_KbdMode(key_new_machine_keyboard_mode, kbdbasic));
	new_machine_kbdmode->setFixedWidth(180);
	new_machine_kbdmode->setFocusPolicy(Qt::NoFocus);
	connect(new_machine_kbdmode, FP(&QComboBox::currentIndexChanged), this, &Preferences::setNewMachineKeyboardMode);

	QComboBox* new_snapshot_kbdmode = new QComboBox();
	new_snapshot_kbdmode->addItem("Keyboard Game Mode");
	new_snapshot_kbdmode->addItem("Keyboard BASIC Mode");
	new_snapshot_kbdmode->addItem("Recreated ZX Keyboard Game Mode");
	new_snapshot_kbdmode->setCurrentIndex(settings.get_KbdMode(key_new_snapshot_keyboard_mode, kbdgame));
	new_snapshot_kbdmode->setFixedWidth(180);
	connect(new_snapshot_kbdmode, FP(&QComboBox::currentIndexChanged), this, &Preferences::setSnapshotKeyboardMode);
	new_snapshot_kbdmode->setFocusPolicy(Qt::NoFocus);

	QComboBox* new_machine_model = new QComboBox();
	new_machine_model->setFocusPolicy(Qt::NoFocus);
	for (int i = 0, m = 0; m < num_models; m++)
	{
		if (!zx_info[m].is_supported) continue;
		new_machine_model->addItem(zx_info[m].name);
		if (settings.get_int(key_startup_model, zxsp_i3) == m) new_machine_model->setCurrentIndex(i);
		modelList[i] = m;
		i++;
	}
	new_machine_model->setFixedWidth(180);
	connect(new_machine_model, FP(&QComboBox::currentIndexChanged), this, &Preferences::setNewMachineModel);

	QComboBox* start_screen_size = new QComboBox();
	start_screen_size->addItems(
		QStringList() << "Fullscreen"
					  << "Size x 1"
					  << "Size x 2"
					  << "Size x 3"
					  << "Size x 4");
	start_screen_size->setFocusPolicy(Qt::NoFocus);
	start_screen_size->setCurrentIndex(settings.get_int(key_startup_screen_size, 2));
	start_screen_size->setFixedWidth(180);
	connect(start_screen_size, FP(&QComboBox::currentIndexChanged), this, &Preferences::setDefaultScreenSize);

	// Layout:
	// das Layout ist 2 Spalten breit
	// Normalerweise erstrecken sich Einträge über beide Spalten
	int i = 0;
	gridlayout->addWidget(new MyGroupLabel("Options at startup:"), i++, 0);
	gridlayout->addWidget(check_for_update, i++, 0);
	gridlayout->addWidget(save_and_restore_sessions, i++, 0, 1, 2);
	gridlayout->addWidget(start_screen_size, i, 0);
	gridlayout->addWidget(new QLabel("Screen size at start"), i++, 1);
	gridlayout->addWidget(start_open_keyboard, i++, 0, 1, 2);
	gridlayout->addWidget(start_open_taperecorder, i++, 0, 1, 2);
	gridlayout->addWidget(start_open_disk_drive, i++, 0, 1, 2);
	gridlayout->addWidget(start_open_image, i++, 0, 1, 2);
	gridlayout->addWidget(start_warn_audioin, i++, 0, 1, 2);

	gridlayout->addWidget(new MyGroupLabel("Options for new machines:"), i++, 0);
	gridlayout->addWidget(new_machine_model, i, 0);
	gridlayout->addWidget(new QLabel("Default model"), i++, 1);
	gridlayout->addWidget(new_machine_kbdmode, i, 0);
	gridlayout->addWidget(new QLabel("Default keyboard mode"), i++, 1);
	gridlayout->addWidget(auto_start_stop_tape, i++, 0, 1, 2);
	gridlayout->addWidget(fast_load_tape, i++, 0, 1, 2);

	gridlayout->addWidget(new MyGroupLabel("Options for new snapshots:"), i++, 0);
	gridlayout->addWidget(new_snapshot_kbdmode, i, 0);
	gridlayout->addWidget(new QLabel("Default keyboard mode"), i++, 1);
	gridlayout->addWidget(new_machine_ay, i++, 0, 1, 2);
	gridlayout->addWidget(new_machine_joystick, i++, 0, 1, 2);
	gridlayout->addWidget(new_machine_rampack, i++, 0, 1, 2);
	gridlayout->addWidget(new_machine_divide, i++, 0, 1, 2);
	gridlayout->addWidget(use_individual_settings, i++, 0, 1, 2);

	setLayout(gridlayout);
}


// slot
void Preferences::setNewMachineModel(int index)
{
	xlogline("Preferences::setNewMachineModel %i", index);
	settings.setValue(key_startup_model, modelList[index]);
}

// slot
void Preferences::setSaveAndRestore(bool f)
{
	xlogline("Preferences::setSaveAndRestore %s", f ? "on" : "off");
	settings.setValue(key_save_and_restore_session, f);
	// TODO: andere QCheckBoxes enablen/disablen
}

// slot
void Preferences::setDefaultScreenSize(int index)
{
	xlogline("Preferences::setDefaultScreenSize %i", index);
	settings.setValue(key_startup_screen_size, index);
}

// slot
void Preferences::setStartOpenKeyboard(bool f)
{
	xlogline("Preferences::setStartOpenKeyboard %s", f ? "on" : "off");
	settings.setValue(key_startup_open_keyboard, f);
}

// slot
void Preferences::setStartOpenDiskDrive(bool f)
{
	xlogline("Preferences::setStartOpenDiskDrive %s", f ? "on" : "off");
	settings.setValue(key_startup_open_disk_drive, f);
}

// slot
void Preferences::setStartOpenTaperecorder(bool f)
{
	xlogline("Preferences::setStartOpenTaperecorder %s", f ? "on" : "off");
	settings.setValue(key_startup_open_taperecorder, f);
}

// slot
void Preferences::setStartOpenMachineImage(bool f)
{
	xlogline("Preferences::setStartOpenMachineImage %s", f ? "on" : "off");
	settings.setValue(key_startup_open_machine_image, f);
}

// slot
void Preferences::setStartEnableAudioIn(bool f)
{
	xlogline("Preferences::setStartEnableAudioIn %s", f ? "on" : "off");
	settings.setValue(key_warn_if_audio_in_fails, f);
}

// slot
void Preferences::setStartStopTape(bool f)
{
	xlogline("Preferences::setStartStopTape %s", f ? "on" : "off");
	settings.setValue(key_auto_start_stop_tape, f);
}

// slot
void Preferences::setFastLoadTape(bool f)
{
	xlogline("Preferences::setFastLoadTape %s", f ? "on" : "off");
	settings.setValue(key_fast_load_tape, f);
}

// slot
void Preferences::setNewMachineKeyboardMode(int index)
{
	xlogline("Preferences::setNewMachineKeyboardMode %i", index);
	settings.setValue(key_new_machine_keyboard_mode, index);
}

// slot
void Preferences::setNewMachineAy(bool f)
{
	xlogline("Preferences::setNewMachineAy %s", f ? "on" : "off");
	settings.setValue(key_always_attach_soundchip, f);
}

// slot
void Preferences::setNewMachineJoystick(bool f)
{
	xlogline("Preferences::setNewMachineJoystick %s", f ? "on" : "off");
	settings.setValue(key_always_attach_joystick, f);
}

// slot
void Preferences::setNewMachineRampack(bool f)
{
	xlogline("Preferences::setNewMachineRampack %s", f ? "on" : "off");
	settings.setValue(key_always_attach_rampack, f);
}

// slot
void Preferences::setNewMachineDivide(bool f)
{
	xlogline("Preferences::setNewMachineDivide%s", f ? "on" : "off");
	settings.setValue(key_always_attach_divide, f);
}

// slot
void Preferences::setSnapshotIndividualSettings(bool f)
{
	xlogline("Preferences::setSnapshotIndividualSettings %s", f ? "on" : "off");
	settings.setValue(key_use_individual_settings, f);
}

// slot
void Preferences::setSnapshotKeyboardMode(int index)
{
	xlogline("Preferences::setSnapshotKeyboardMode %i", index);
	settings.setValue(key_new_snapshot_keyboard_mode, index);
}

// slot
void Preferences::setCheckForUpdate(bool f)
{
	xlogline("Preferences::setCheckForUpdate %s", f ? "on" : "off");
	settings.setValue(key_check_for_update, f);
	settings.setValue(key_check_update_timestamp, now()); // when check again
}
