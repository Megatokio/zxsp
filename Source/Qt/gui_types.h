// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once

#include "OS/DspTime.h"
#include "Uni/Language.h"
#include "isa_id.h"
#include "kio/kio.h"


// physical joysticks: usb/kbd-emu/none:
class Joystick;
class KbdJoystick;
class UsbJoystick;


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
class MachineList;

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
