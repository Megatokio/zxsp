#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "cstrings/cstrings.h"
#include "zxsp_types.h"
#include <QString>


extern cstr MHzStr(Frequency);

extern int32 intValue(cstr);
extern int32 intValue(QString);

extern double mhzValue(cstr);
extern double mhzValue(QString);


extern uint16 printablechar(uint8 c); // unprintable -> middle-dot
