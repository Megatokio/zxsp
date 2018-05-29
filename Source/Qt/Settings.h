/*	Copyright  (c)	Günter Woigk 2012 - 2018
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


#ifndef QT_SETTINGS_H
#define QT_SETTINGS_H

#include "kio/kio.h"
#include <QSettings>
#include "ZxInfo/ZxInfo.h"
#include "zxsp_types.h"
class QMenu;
class StrArray;
class QAction;


/*	the global instance:
*/
extern class Settings settings;


// Keys:

#define key_save_and_restore_session    "settings/save_and_restore_session"		/* bool */
#define key_saved_session               "settings/saved_session"				/* QString  … filepath */

#define key_startup_model               "settings/startup_model"                /* int  */
#define key_startup_screen_size         "settings/startup_screen_size"          /* int: 1-4; 0=fullscreen */
#define key_startup_open_keyboard       "settings/startup_open_keyboard"        /* bool */
#define key_startup_open_taperecorder   "settings/startup_open_taperecorder"    /* bool */
#define key_startup_open_disk_drive		"settings/startup_open_disk_drive"      /* bool */
#define key_startup_open_machine_image  "settings/startup_open_machine_image"   /* bool */
#define key_warn_if_audio_in_fails      "settings/audio_input_enabled"          /* bool */
#define key_auto_start_stop_tape        "settings/auto_start_stop_tape"         /* bool */
#define key_fast_load_tape              "settings/fast_load_tape"               /* bool */

#define key_new_machine_keyboard_mode   "settings/new_machine_keyboard_mode"    /* int  */
#define key_new_snapshot_keyboard_mode  "settings/new_snapshot_keyboard_mode"   /* int  */
#define key_always_attach_soundchip     "settings/always_attach_soundchip"      /* bool */
#define key_always_attach_joystick      "settings/always_attach_joystick"       /* bool */
#define key_always_attach_rampack       "settings/always_attach_rampack"        /* bool */
#define key_use_individual_settings     "settings/use_individual_settings"      /* bool: for each snapshot */
#define key_new_machine_audioin_enabled	"settings/new_machine_audioin_enabled"	/* bool: note: currently never set */

#define key_check_for_update            "settings/check_for_update"             /* bool */
#define key_check_update_timestamp      "settings/check_update_timestamp"       /* double */

#define key_zx3k_ramsize                "settings/zx3k_ramsize"                 /* uint: speichert Auswahl im Zx3kInsp */
#define key_memotech64k_dip_switches    "settings/memotech64k_dip_switches"     /* uint: speichert Auswahl im Memotech64kInsp */

#define key_framerate_zx80_60hz			"settings/key_framerate_zx80_60hz"		/* bool */
#define key_framerate_jupiter_60hz		"settings/key_framerate_jupiter_60hz"	/* bool */
#define key_framerate_tk85_60hz			"settings/key_framerate_tk85_60hz"		/* bool */
#define key_framerate_tk90x_60hz		"settings/key_framerate_tk90x_60hz"		/* bool */
#define key_framerate_tk95_60hz			"settings/key_framerate_tk95_60hz"		/* bool */

#define key_gif_movies_animate_border	"settings/gif_movies_animate_border"	/* bool */

#define key_mainwindow_position         "gui/mainwindow_position/"              /* QRect: for each zoom */
#define key_toolwindow_position         "gui/toolwindow_position/"              /* QPoint: for each Item grp_id */
#define key_toolwindow_toolbar_height	"gui/toolwindow_toolbar_height"			/* int */

#define	key_memoryview_datasource(N)		usingstr("gui/memoryview/%i/datasource",int(N))		/* int: 1-4; AsSeenByCpu, AllRom, RomPages, AllRam, RamPages */
#define	key_memoryview_ram_page(N)			usingstr("gui/memoryview/%i/ram_page",int(N))		/* int  */
#define	key_memoryview_rom_page(N)			usingstr("gui/memoryview/%i/rom_page",int(N))		/* int  */
#define	key_memoryview_scrollposition(N)	usingstr("gui/memoryview/%i/scrollposition",int(N))	/* int32 */
#define	key_memoryview_bytes_per_row(N)		usingstr("gui/memoryview/%i/bytes_per_row",int(N))	/* int  */
#define	key_memoryview_rows(N)				usingstr("gui/memoryview/%i/rows",int(N))			/* int  */
#define	key_memoryview_size(N)				usingstr("gui/memoryview/%i/size",int(N))			/* QSize */
#define	key_memoryview_access_pixelsize		"gui/memoryview/3/pixelsize"				/* int  */
#define key_memoryview_access_decaymode		"gui/memoryview/3/decaymode"				/* int  */
#define	key_memoryview_hex_is_words			"gui/memoryview/0/is_words"					/* bool */

#define key_spectra_enable_if1_rom_hooks	"settings/spectra_enable_if1_rom_hooks"		// bool
#define key_spectra_enable_rs232			"settings/spectra_enable_rs232"				// bool
#define key_spectra_enable_new_video_modes	"settings/spectra_enable_new_video_modes"	// bool
#define key_spectra_enable_joystick			"settings/spectra_enable_joystick"			// bool

//#define key_divide_link_E					"settings/key_divide_link_E"				// bool
#define	key_divide_rom_file					"settings/key_divide_rom_file"				// string
#define	key_divide_disk_file				"settings/key_divide_disk_file"				// string
#define	key_divide_ram_size					"settings/key_divide_ram_size"				// int
#define key_always_attach_divide			"settings/always_attach_divide"			    // bool

// SMART SD card interface:
#define key_smart_card_joystick_enabled		"settings/smart_card_joystick_enabled"		// bool
#define key_smart_card_memory_enabled		"settings/smart_card_memory_enabled"		// bool
#define key_smart_card_force_bank_B			"settings/smart_card_force_bank_B"			// bool
#define key_smart_card_write_flash_enabled	"settings/smart_card_write_flash_enabled"	// bool

#define key_lenslok							"settings/lenslok"							// String
#define	key_multiface1_enable_joystick		"settings/multiface1_enable_joystick"		// bool
#define key_rzx_autostart_recording			"settings/rzx_autostart_recording_on_key"	// bool

// Keyboard Joystick:
#define key_os_key_code_to_key_caps			"settings/os_key_code_to_key_caps"			// encoded in a String
#define key_kbd_joystick_fnmatch_patterns	"settings/kbd_joystick_fnmatch_patterns"	// encoded in a StringList

// selectFile dialogs:
#define key_selectFile_filter_(f,F)			usingstr("gui/%s_file_filter_%s",f?"save":"load",F)	// String
#define key_selectFile_directory_(f,F)		usingstr("gui/%s_file_dir_%s",f?"save":"load",F)	// String



class Settings : public QSettings
{
public:
	QAction*	action_gifAnimateBorder;

	Settings();
	~Settings();

	cstr		get_cstr(cstr key, cstr dflt=NULL);			// also takes care of putting the result in temp men...
	str			get_str(cstr key, cstr dflt=NULL);			// also takes care of putting the result in temp men...
	int			get_int(cstr key, int dflt);
	Model		get_Model(cstr key, Model dflt);
	KbdMode		get_KbdMode(cstr key, KbdMode dflt);
	uint		get_uint(cstr key, uint dflt);
	bool		get_bool(cstr key, bool dflt);
	double		get_double(cstr key, double dflt);
	void		get_QStringList(cstr key, QStringList& result);

	void		get_StrArray(cstr key, StrArray& result);
	void		set_StrArray(cstr key, StrArray& value);


private:
	void		setGifAnimateBorder(bool);
};


#endif

























