// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "zxsp_types.h"

extern void	 write_mem(FD& fd, const CoreByte* q, uint32 cnt); // MachineZxsp.cpp
extern void	 read_mem(FD& fd, CoreByte* z, uint32 cnt);		   // MachineZxsp.cpp
extern Model modelForSna(FD& fd);							   // MachineZxsp.cpp
extern Model bestModelForFile(cstr fpath, Model default_model);
