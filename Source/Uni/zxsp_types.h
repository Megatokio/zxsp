/*	Copyright  (c)	Günter Woigk 2002 - 2019
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

#ifndef ZXSP_TYPES_H
#define ZXSP_TYPES_H

#include "kio/kio.h"
#include "Audio/DspTime.h"
#include "Uni/Language.h"
//#include "Uni/IsaObject.h"


typedef double	Time;			// time	[s]
typedef double	Frequency;		// frequency [1/s]

typedef float 	Sample;			typedef Sample const		cSample;
		class	StereoSample;	typedef StereoSample const	cStereoSample;

class FD;

typedef uint8	uchar32[32];
typedef uint8	uchar64[64];
typedef uint8	uchar256[256];

typedef uint32 CoreByte;		// Z80


enum KbdMode {			// preferred keyboard translation:		((Kbd.h))
		kbdgame,		// prefer physical translation (from scan code)
		kbdbasic,		// use logical translation (from character code)
		kbdbtzxkbd,		// game mode on the "Recreated ZX Keyboard"
		num_kbdmodes };


class Application;
class WindowMenu;
class ZxItemsMenu;
class TempMemPool;

// physical joysticks: usb/kbd-emu/none:
class Joystick;				typedef Joystick const cJoystick;
class	KbdJoystick;
class	UsbJoystick;

class GifEncoder;
class Pixelmap;				typedef Pixelmap const cPixelmap;
class ZxPixelmap;
typedef uchar Comp;			typedef Comp const cComp;
class Colormap;				typedef Colormap const cColormap;
struct ZxInfo;				typedef ZxInfo const cZxInfo;

class TapeFile;
class TapeRecorder;
class TapeData;
	class TzxData;
	class O80Data;
	class TapData;
	class AudioData;
	class RlesData;
class CswBuffer;

class MachineController;
class Memory;

class Screen;
class	ScreenMono;
class	ScreenZxsp;

class ToolWindowController;
class ToolWindow;
class Inspector;
class Lenslok;

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
		class   MachineZx80;
		class   MachineZx81;
		class   MachineJupiter;
		class   MachineZxsp;
		class   MachineZx128;
		class   MachineZxPlus3;
		class   MachineTc2048;
		class   MachineTc2068;
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

#endif
















