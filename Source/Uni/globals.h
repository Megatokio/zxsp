#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "zxsp_types.h"
#include <QEvent>
#include "ZxInfo/ZxInfo.h"

#define EXT extern
#define throws	noexcept(false)			// file_error


EXT Application*		appl;					    // Application.cpp
EXT void				runMachinesForSound();
EXT MachineController*	front_machine_controller;	// only for comparison with this etc.
EXT volatile void*		front_machine;				// only for comparison with this etc.

EXT cstr	QEventTypeStr(int n);   // Util/QEventTypes.cpp
EXT bool	cmdKeyDown();           // Application.cpp

EXT cstr	appl_path;              // Application.cpp
EXT cstr	appl_rsrc_path;         // Application.cpp
EXT cstr	basic_token[];          // ZxInfo/BasicTokens.cpp

EXT void	write_mem		(FD& fd, CoreByte const* q, uint32 cnt) throws;	// MachineZxsp.cpp
EXT void	read_mem		(FD& fd, CoreByte* z, uint32 cnt)       throws;	// MachineZxsp.cpp
EXT Model	modelForSna		(FD& fd) throws;									// MachineZxsp.cpp
EXT Model	bestModelForFile(cstr fpath);		// Uni/Files/bestModelForFile.cpp

EXT void	checkUpdate		(bool verbose);		// CheckUpdate.cpp

EXT void	showAlert		( cstr msg, ... );	// ConfigDialog.cpp: "red" alert:    stop sign
EXT void	showWarning		( cstr msg, ... );	// ConfigDialog.cpp: "yellow" alert: attention sign
EXT void	showInfo		( cstr msg, ... );	// ConfigDialog.cpp: a friendly information alert





