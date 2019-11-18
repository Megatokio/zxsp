#pragma once
/*	Copyright  (c)	Günter Woigk 2009 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

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

EXT cstr	appl_name;              // Application.cpp:         "zxsp"
EXT cstr	appl_path;              // Application.cpp
EXT cstr	appl_bundle_path;       // Application.cpp
EXT cstr	appl_rsrc_path;         // Application.cpp
EXT cstr	appl_version_str;		// Application.cpp: 		e.g.: "0.6.6"
EXT uint8	appl_version_h;			// Application.cpp:
EXT uint8	appl_version_m;			// Application.cpp:
EXT uint8	appl_version_l;			// Application.cpp:
EXT cstr	basic_token[];          // ZxInfo/BasicTokens.cpp

EXT void	write_mem		(FD& fd, CoreByte const* q, uint32 cnt) throws;	// MachineZxsp.cpp
EXT void	read_mem		(FD& fd, CoreByte* z, uint32 cnt)       throws;	// MachineZxsp.cpp
EXT Model	modelForSna		(FD& fd) throws;									// MachineZxsp.cpp
EXT Model	bestModelForFile(cstr fpath);		// Uni/Files/bestModelForFile.cpp

EXT void	checkUpdate		(bool verbose);		// CheckUpdate.cpp

EXT void	showAlert		( cstr msg, ... );	// ConfigDialog.cpp: "red" alert:    stop sign
EXT void	showWarning		( cstr msg, ... );	// ConfigDialog.cpp: "yellow" alert: attention sign
EXT void	showInfo		( cstr msg, ... );	// ConfigDialog.cpp: a friendly information alert





