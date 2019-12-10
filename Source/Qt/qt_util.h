#pragma once
/*	Copyright  (c)	Günter Woigk 2012 - 2019
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

#include <QRgb>
#include <QWidget>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include "kio/kio.h"


extern void setColors   (QWidget* widget, QRgb foregroundcolor, QRgb backgroundcolor = 0/*transparent*/);

extern cstr selectLoadFile(QWidget* parent, cstr headline, cstr filefilterstr);
extern cstr selectSaveFile(QWidget* parent, cstr headline, cstr filefilterstr);

extern cstr tostr (QAudioFormat::Endian endi) noexcept;
extern cstr tostr (QAudioFormat::SampleType styp) noexcept;
extern cstr tostr (const QAudioFormat& afmt) noexcept;
extern void print_QAudioFormat (cstr title, const QAudioFormat& afmt);
extern void print_QAudioDeviceInfo (cstr title, const QAudioDeviceInfo& info);
