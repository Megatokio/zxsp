// Copyright (c) 2023 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "zxsp_types.h"


// externally provided global functions:

extern void showMessage(MessageStyle, cstr text);

// these macros also work in classes which provide a member function showMessage():
#define showAlert(...)	 showMessage(ALERT, usingstr(__VA_ARGS__))
#define showWarning(...) showMessage(WARN, usingstr(__VA_ARGS__))
#define showInfo(...)	 showMessage(INFO, usingstr(__VA_ARGS__))


// externally provided global data:

extern Frequency samples_per_second; // for audio output channel
extern Time		 system_time;		 // monotonic real time [seconds]
extern cstr		 appl_rsrc_path;	 // where are the roms, audio fx,
