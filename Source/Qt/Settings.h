#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZxInfo/ZxInfo.h"
#include "kio/kio.h"
#include "zxsp_globals.h"
#include <QSettings>
class QMenu;
class StrArray;
class QAction;


namespace gui
{

/*	the global instance:
 */
extern class Settings settings;


// Keys:

static constexpr char key_new_version_info[]		   = "settings/new_version_info";
static constexpr char key_save_and_restore_session[]   = "settings/save_and_restore_session";  // bool
static constexpr char key_saved_session[]			   = "settings/saved_session";			   // QString  … filepath
static constexpr char key_startup_model[]			   = "settings/startup_model";			   // int
static constexpr char key_startup_screen_size[]		   = "settings/startup_screen_size";	   // int: 1-4; 0=fullscreen
static constexpr char key_startup_open_keyboard[]	   = "settings/startup_open_keyboard";	   // bool
static constexpr char key_startup_open_taperecorder[]  = "settings/startup_open_taperecorder"; // bool
static constexpr char key_startup_open_disk_drive[]	   = "settings/startup_open_disk_drive";   // bool
static constexpr char key_startup_open_machine_image[] = "settings/startup_open_machine_image"; // bool
static constexpr char key_warn_if_audio_in_fails[]	   = "settings/audio_input_enabled";		// bool
static constexpr char key_auto_start_stop_tape[]	   = "settings/auto_start_stop_tape";		// bool
static constexpr char key_fast_load_tape[]			   = "settings/fast_load_tape";				// bool
static constexpr char key_new_machine_keyboard_mode[]  = "settings/new_machine_keyboard_mode";	// int
static constexpr char key_new_snapshot_keyboard_mode[] = "settings/new_snapshot_keyboard_mode"; // int
static constexpr char key_always_attach_soundchip[]	   = "settings/always_attach_soundchip";	// bool
static constexpr char key_always_attach_joystick[]	   = "settings/always_attach_joystick";		// bool
static constexpr char key_always_attach_rampack[]	   = "settings/always_attach_rampack";		// bool
static constexpr char key_use_individual_settings[]	   = "settings/use_individual_settings"; // bool: for each snapshot
static constexpr char key_new_machine_audioin_enabled[] =
	"settings/new_machine_audioin_enabled"; // bool: TODO: never set

static constexpr char key_check_for_update[]	   = "settings/check_for_update";		// bool
static constexpr char key_check_update_timestamp[] = "settings/check_update_timestamp"; // double

static constexpr char key_zx3k_ramsize[] = "settings/zx3k_ramsize"; // uint: speichert Auswahl im Zx3kInsp
static constexpr char key_memotech64k_dip_switches[] =
	"settings/memotech64k_dip_switches"; // uint: speichert Auswahl im Memotech64kInsp

static constexpr char key_framerate_zx80_60hz[]		  = "settings/key_framerate_zx80_60hz";	   // bool
static constexpr char key_framerate_jupiter_60hz[]	  = "settings/key_framerate_jupiter_60hz"; // bool
static constexpr char key_framerate_tk85_60hz[]		  = "settings/key_framerate_tk85_60hz";	   // bool
static constexpr char key_framerate_tk90x_60hz[]	  = "settings/key_framerate_tk90x_60hz";   // bool
static constexpr char key_framerate_tk95_60hz[]		  = "settings/key_framerate_tk95_60hz";	   // bool
static constexpr char key_gif_movies_animate_border[] = "settings/gif_movies_animate_border";  // bool
static constexpr char key_mainwindow_position[]		  = "gui/mainwindow_position/";			   // QRect: for each zoom
static constexpr char key_toolwindow_position[]		  = "gui/toolwindow_position/";		 // QPoint: for each Item grp_id
static constexpr char key_toolwindow_toolbar_height[] = "gui/toolwindow_toolbar_height"; // int

// datasource 1-4: AsSeenByCpu, AllRom, RomPages, AllRam, RamPages
inline cstr key_memoryview_datasource(int N) { return usingstr("gui/memoryview/%i/datasource", N); }
inline cstr key_memoryview_ram_page(int N) { return usingstr("gui/memoryview/%i/ram_page", N); }			 // int
inline cstr key_memoryview_rom_page(int N) { return usingstr("gui/memoryview/%i/rom_page", N); }			 // int
inline cstr key_memoryview_scrollposition(int N) { return usingstr("gui/memoryview/%i/scrollposition", N); } // int32
inline cstr key_memoryview_bytes_per_row(int N) { return usingstr("gui/memoryview/%i/bytes_per_row", N); }	 // int
inline cstr key_memoryview_rows(int N) { return usingstr("gui/memoryview/%i/rows", N); }					 // int
inline cstr key_memoryview_size(int N) { return usingstr("gui/memoryview/%i/size", N); }					 // QSize

static constexpr char key_memoryview_access_pixelsize[]	   = "gui/memoryview/3/pixelsize";				// int
static constexpr char key_memoryview_access_decaymode[]	   = "gui/memoryview/3/decaymode";				// int
static constexpr char key_memoryview_hex_is_words[]		   = "gui/memoryview/0/is_words";				// bool
static constexpr char key_spectra_enable_if1_rom_hooks[]   = "settings/spectra_enable_if1_rom_hooks";	// bool
static constexpr char key_spectra_enable_rs232[]		   = "settings/spectra_enable_rs232";			// bool
static constexpr char key_spectra_enable_new_video_modes[] = "settings/spectra_enable_new_video_modes"; // bool
static constexpr char key_spectra_enable_joystick[]		   = "settings/spectra_enable_joystick";		// bool
static constexpr char key_divide_link_E[]				   = "settings/key_divide_link_E";				// bool
static constexpr char key_divide_rom_file[]				   = "settings/key_divide_rom_file";			// string
static constexpr char key_divide_disk_file[]			   = "settings/key_divide_disk_file";			// string
static constexpr char key_divide_ram_size[]				   = "settings/key_divide_ram_size";			// int
static constexpr char key_always_attach_divide[]		   = "settings/always_attach_divide";			// bool
static constexpr char key_smart_card_joystick_enabled[]	   = "settings/smart_card_joystick_enabled";	// bool
static constexpr char key_smart_card_memory_enabled[]	   = "settings/smart_card_memory_enabled";		// bool
static constexpr char key_smart_card_force_bank_B[]		   = "settings/smart_card_force_bank_B";		// bool
static constexpr char key_smart_card_write_flash_enabled[] = "settings/smart_card_write_flash_enabled"; // bool
static constexpr char key_lenslok[]						   = "settings/lenslok";						// String
static constexpr char key_multiface1_enable_joystick[]	   = "settings/multiface1_enable_joystick";		// bool
static constexpr char key_rzx_autostart_recording[]		   = "settings/rzx_autostart_recording_on_key"; // bool

// Keyboard Joystick:
static constexpr cstr key_os_key_code_to_key_caps		= "settings/os_key_code_to_key_caps"; // encoded in a String
static constexpr cstr key_kbd_joystick_fnmatch_patterns = "settings/kbd_joystick_fnmatch_patterns"; // as a StringList

// selectFile dialogs:
#define key_selectFile_filter_(f, F)	usingstr("gui/%s_file_filter_%s", f ? "save" : "load", F) // String
#define key_selectFile_directory_(f, F) usingstr("gui/%s_file_dir_%s", f ? "save" : "load", F)	  // String


class Settings : public QSettings
{
public:
	QAction* action_gifAnimateBorder;

	Settings();
	~Settings() override;

	cstr		 get_cstr(cstr key, cstr dflt = nullptr); // also takes care of putting the result in temp men...
	str			 get_str(cstr key, cstr dflt = nullptr);  // also takes care of putting the result in temp men...
	int			 get_int(cstr key, int dflt);
	Model		 get_Model(cstr key, Model dflt);
	KeyboardMode get_KbdMode(cstr key, KeyboardMode dflt);
	uint		 get_uint(cstr key, uint dflt);
	bool		 get_bool(cstr key, bool dflt);
	double		 get_double(cstr key, double dflt);
	void		 get_QStringList(cstr key, QStringList& result);

	void get_StrArray(cstr key, StrArray& result);
	void set_StrArray(cstr key, StrArray& value);


private:
	void setGifAnimateBorder(bool);
};

} // namespace gui
