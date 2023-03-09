#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "OS/DspTime.h"
#include "Uni/Language.h"
#include "ZxInfo/ZxInfo.h"
#include "isa_id.h"
#include "kio/kio.h"


extern void showAlert(cstr msg, ...);	// ConfigDialog.cpp
extern void showWarning(cstr msg, ...); // ConfigDialog.cpp
extern void showInfo(cstr msg, ...);	// ConfigDialog.cpp


extern cstr appl_rsrc_path; // Application.cpp
extern cstr basic_token[];	// ZxInfo/BasicTokens.cpp


using CoreByte	= uint32; // Z80
using Time		= double; // time [s]
using Frequency = double; // frequency [1/s]
using Sample	= float;
class StereoSample;


enum KeyboardModifiers // modifier key masks:
{
	ShiftKeyMask   = 1, // caps shift key
	ControlKeyMask = 2, // control key
	AltKeyMask	   = 4	// alt/option key: used as an auxilliary caps shift key
};

enum KeyboardMode { // preferred keyboard translation:		((Kbd.h))
	kbdgame,		// prefer physical translation (from scan code)
	kbdbasic,		// use logical translation (from character code)
	kbdbtzxkbd,		// game mode on the "Recreated ZX Keyboard"
	num_kbdmodes
};

enum JoystickID // physical joysticks
{
	usb_joystick0	 = 0,
	usb_joystick1	 = 1,
	usb_yoystick2	 = 2,
	kbd_joystick	 = 3,
	no_joystick		 = 4,
	num_joystick_ids = 5
};

enum JoystickButtons //	as for Kempston joystick interface: %000FUDLR
{
	button_fire1_mask = 0x10,
	button_up_mask	  = 0x08,
	button_down_mask  = 0x04,
	button_left_mask  = 0x02,
	button_right_mask = 0x01
};

enum MouseButtons // same as Qt
{
	left_button	  = 1,
	right_button  = 2,
	middle_button = 4,
};

class GifEncoder;
class Pixelmap;
class ZxPixelmap;
using Comp = uchar;
class Colormap;
struct ZxInfo;

class TapeFile;
class TapeRecorder;
class TapeData;
class TzxData;
class O80Data;
class TapData;
class AudioData;
class RlesData;
class CswBuffer;

class Memory;

class IsaObject;
class Renderer;
class ZxspRenderer;
class Tc2048Renderer;
class SpectraRenderer;
class MonoRenderer;
class GifWriter;
class ZxspGifWriter;
class Tc2048GifWriter;
class SpectraGifWriter;
class MonoGifWriter;
class Machine;
class MachineZx80;
class MachineZx81;
class MachineJupiter;
class MachineZxsp;
class MachineZx128;
class MachineZxPlus3;
class MachineTc2048;
class MachineTc2068;
class Item;
class MemoryItem;
class Z80;
class Crtc;
class Ula;
class UlaMono;
class UlaJupiter;
class UlaZx80;
class UlaZx81;
class UlaZxsp;
class UlaInves;
class Ula128k;
class UlaPlus2;
class UlaPlus3;
class UlaPlus2A;
class UlaTc2048;
class Mmu;
class MmuJupiter;
class MmuZx80;
class MmuZx81;
class MmuZxsp;
class MmuInves;
class Mmu128k;
class MmuPlus3;
class MmuTc2048;
class MmuTc2068;
class TapeRecorder;
class Keyboard;
class Joy;
class KempstonJoy;
class InvesJoy;
class Tc2048Joy;
class DktronicsDualJoy;
class CursorJoy;
class ProtekJoy;
class SinclairJoy;
class ZxPlus2Joy;
class ZxPlus2AJoy;
class ZxPlus3Joy;
class ZxIf2;
class Tk90xJoy;
class Tk95Joy;
class Tc2068Joy;
class Tk85Joy;
class Ay;
class InternalAy;
class ZonxBox;
class ZonxBox81;
class DidaktikMelodik;
class FullerBox;
class ZxIf1;
class Printer;
class ZxPrinter;
class PrinterAerco;
class PrinterLprint3;
class PrinterPlus3;
class PrinterTs2040;
class Fdc;
class FdcBeta128;
class FdcD80;
class FdcJLO;
class FdcPlus3;
class FdcPlusD;
class FdcInsp;
class ExternalRam;
class Cheetah32kRam;
class Jupiter16kRam;
class Zx16kRam;
class Ts1016Ram;
class Stonechip16kRam;
class Memotech16kRam;
class Memotech64kRam;
class Zx3kRam;
class GrafPad;
class Multiface;
class Multiface1;
class Multiface128;
class Multiface3;
class IcTester;
class KempstonMouse;
class SpectraVideo;
class DivIDE;
class CurrahMicroSpeech;
