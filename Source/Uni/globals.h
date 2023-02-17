#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZxInfo/ZxInfo.h"
#include "zxsp_types.h"
#include <QEvent>


extern Application*		  appl; // Application.cpp
extern void				  runMachinesForSound();
extern MachineController* front_machine_controller; // only for comparison with this etc.
extern volatile void*	  front_machine;			// only for comparison with this etc.

extern cstr QEventTypeStr(int n); // Util/QEventTypes.cpp
extern bool cmdKeyDown();		  // Application.cpp

extern cstr appl_path;		// Application.cpp
extern cstr appl_rsrc_path; // Application.cpp
extern cstr basic_token[];	// ZxInfo/BasicTokens.cpp

extern void	 write_mem(FD& fd, const CoreByte* q, uint32 cnt); // MachineZxsp.cpp
extern void	 read_mem(FD& fd, CoreByte* z, uint32 cnt);		   // MachineZxsp.cpp
extern Model modelForSna(FD& fd);							   // MachineZxsp.cpp
extern Model bestModelForFile(cstr fpath);					   // Uni/Files/bestModelForFile.cpp

extern void checkUpdate(bool verbose); // CheckUpdate.cpp

extern void showAlert(cstr msg, ...);	// ConfigDialog.cpp: "red" alert:    stop sign
extern void showWarning(cstr msg, ...); // ConfigDialog.cpp: "yellow" alert: attention sign
extern void showInfo(cstr msg, ...);	// ConfigDialog.cpp: a friendly information alert
