#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZxInfo/ZxInfo.h"
#include "zxsp_types.h"
#include <QEvent>

namespace gui
{
extern class Application*		appl;					  // Application.cpp
extern class MachineController* front_machine_controller; // only for comparison with this etc.

extern void checkUpdate(bool verbose); // CheckUpdate.cpp


} // namespace gui


extern void showAlert(cstr msg, ...);	// ConfigDialog.cpp: "red" alert:    stop sign
extern void showWarning(cstr msg, ...); // ConfigDialog.cpp: "yellow" alert: attention sign
extern void showInfo(cstr msg, ...);	// ConfigDialog.cpp: a friendly information alert


extern cstr QEventTypeStr(int n); // Util/QEventTypes.cpp

extern cstr appl_path;		// Application.cpp
extern cstr appl_rsrc_path; // Application.cpp
extern cstr basic_token[];	// ZxInfo/BasicTokens.cpp

extern Model modelForSna(FD& fd);								// MachineZxsp.cpp
extern Model bestModelForFile(cstr fpath, Model default_model); // Uni/Files/bestModelForFile.cpp
