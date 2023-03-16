#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include <QRgb>
#include <QWidget>


namespace gui
{

extern void setColors(QWidget* widget, QRgb foregroundcolor, QRgb backgroundcolor = 0 /*transparent*/);

extern cstr selectLoadFile(QWidget* parent, cstr headline, cstr filefilterstr);
extern cstr selectSaveFile(QWidget* parent, cstr headline, cstr filefilterstr);

extern cstr	  MHzStr(double frequency);
extern int32  intValue(cstr);
extern double mhzValue(cstr);
extern uint16 printablechar(uint8 c); // unprintable -> middle-dot

} // namespace gui
