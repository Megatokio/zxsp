#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "DspTime.h"
#include "Uni/Language.h"
#include "isa_id.h"
#include "kio/kio.h"


using Time		= double; // time	[s]
using Frequency = double; // frequency [1/s]

using Sample = float;
class StereoSample;

class FD;

using CoreByte = uint32; // Z80


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


class TempMemPool;

// physical joysticks: usb/kbd-emu/none:
class Joystick;
class KbdJoystick;
class UsbJoystick;

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

namespace gui
{
class Application;
class WindowMenu;
class ZxItemsMenu;
class Overlay;
class OverlayJoystick;
class OverlayRecord;
class OverlayPlay;

class MachineController;

class Screen;
class ScreenMono;
class ScreenZxsp;

class ToolWindowController;
class ToolWindow;
class Lenslok;

class Inspector;
class UlaInsp;
class JoyInsp;
class TapeRecorderInsp;
class Plus2TapeRecorderInsp;
class WalkmanInspector;
class KeyboardInspector;
class Z80Insp;
class Tk85JoyInsp;
class Tk95JoyInsp;
class Tk90xJoyInsp;
class KempstonJoyInsp;
class Tc2048JoyInsp;
class DktronicsDualJoyInsp;
class CursorJoyInsp;
class ProtekJoyInsp;
class SinclairJoyInsp;
class Tc2068JoyInsp;
class IcTesterInsp;
class KempstonMouseInsp;
class FullerBoxInsp;
class ZxIf2Insp;
class ZxIf1Insp;
class ZxPrinterInsp;
class PrinterAercoInsp;
class PrinterLprint3Insp;
class PrinterPlus3Insp;
class PrinterTs2040Insp;
class ZonxBoxInsp;
class AyInsp;
class DidaktikMelodikInsp;
class FdcBeta128Insp;
class FdcD80Insp;
class FdcJLOInsp;
class FdcPlus3Insp;
class FdcPlusDInsp;
class GrafPadInsp;
class MultifaceInsp;
class Multiface1Insp;
class Multiface128Insp;
class Multiface3Insp;
class Jupiter16kRamInsp;
class Zx16kInsp;
class Zx3kInsp;
class Memotech64kRamInsp;
class MemoryInspector;
class SpectraVideoInspector;
class DivIDEInspector;
} // namespace gui
