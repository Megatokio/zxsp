// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once

using cstr		= const char*;
using Time		= double;
using Frequency = double;


// externally provided global functions:

extern void showAlert(cstr msg, ...);
extern void showWarning(cstr msg, ...);
extern void showInfo(cstr msg, ...);


// externally provided global data:

extern Frequency samples_per_second; // for audio output channel
extern Time		 system_time;		 // monotonic real time [seconds]
extern cstr		 appl_rsrc_path;	 // where are the roms, audio fx,
