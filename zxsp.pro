QT += core gui
QT += network opengl multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# http://qt-project.org/faq/answer/what_does_the_syntax_configdebugdebugrelease_mean_what_does_the_1st_argumen
CONFIG(release,debug|release) { DEFINES += NDEBUG RELEASE } # ATTN: curly brace must start in same line!
CONFIG(debug,debug|release) { DEFINES += DEBUG } # ATTN: curly brace must start in same line!

CONFIG += c++14
DEFINES += QT_NO_SESSIONMANAGER
QMAKE_CXXFLAGS += -Wno-multichar -Wdeprecated-declarations
RESOURCES += Resources/zxsp.qrc

#win32{}
#unix{}
#macx{}
#unix:!macx{}
#linux-g++{}

macx: QMAKE_MAC_SDK = macosx10.9
macx: LIBS += -framework CoreAudio -framework ApplicationServices -framework IOKit -framework AudioToolbox -lz
unix:!macx: LIBS += -pthread -lz


INCLUDEPATH += \
	Source \
	Source/Qt \
	Source/OS \
	Source/Uni \
	Source/Uni/Audio \
	Source/Uni/TapeFile \
	Source/Uni/Video \
	Source/Uni/Machine \
	Source/Uni/Items \
	Source/Uni/Keyboard \
	Source/Uni/ZxInfo \
	Libraries \
	zasm/Source \

macx: INCLUDEPATH += Source/OS/Mac
unix:!macx: INCLUDEPATH += Source/OS/Linux


macx: SOURCES += \
	Source/OS/Mac/UsbJoystick.cpp \
	Source/OS/Mac/Dsp.cpp \
	Source/OS/Mac/UsbDevice.cpp \
	Source/OS/Mac/mac_util.cpp \
	Libraries/audio/macos/AudioDecoder.cpp \
	Libraries/audio/macos/CAStreamBasicDescription.cpp \

macx: HEADERS += \
	Source/OS/Dsp.h \
	Source/OS/Mac/UsbJoystick.h \
	Source/OS/Mac/UsbDevice.h \
	Source/OS/Mac/mac_util.h \
	Libraries/audio/macos/CADebugMacros.h \
	Libraries/audio/macos/CAStreamBasicDescription.h \
	Libraries/audio/macos/CAMath.h \
	Libraries/audio/macos/CADebugPrintf.h \


unix:!macx: SOURCES += \
	Source/OS/Linux/UsbJoystick.cpp \
	Source/OS/Linux/missing_definitions.cpp \
	Libraries/audio/Linux/AudioDecoder.cpp \

unix:!macx: HEADERS += \
	Source/OS/Dsp.h \
	Source/OS/Linux/UsbJoystick.h \


SOURCES +=	\
	Libraries/audio/convert_audio.cpp \
	Libraries/kio/exceptions.cpp \
	Libraries/cstrings/cstrings.cpp \
	Libraries/graphics/gif/Colormap.cpp \
	Libraries/graphics/gif/Pixelmap.cpp \
	Libraries/graphics/gif/GifEncoder.cpp \
	Libraries/kio/kio.cpp \
	Libraries/unix/log_to_file.cpp \
	Libraries/unix/os_utilities.cpp \
	Libraries/cstrings/tempmem.cpp \
	Libraries/cpp/cppthreads.cpp \
	Libraries/unix/FD.cpp \
	Libraries/unix/files.cpp \
	Libraries/unix/n-compress.cpp \
	Libraries/kio/TestTimer.cpp \
	Libraries/audio/WavFile.cpp \
	Libraries/Z80/goodies/z80_clock_cycles.cpp \
	Libraries/Z80/goodies/z80_opcode_length.cpp \
	Libraries/Z80/goodies/z80_disass.cpp \
	Libraries/Qt/QEventTypes.cpp \
	Libraries/Devices/File.cpp \
	Libraries/Devices/SerialDevice.cpp \
	Libraries/Devices/StdFile.cpp \
	\
	zasm/Source/Error.cpp \
	zasm/Source/Label.cpp \
	zasm/Source/Segment.cpp \
	zasm/Source/Source.cpp \
	zasm/Source/Z80Assembler.cpp \
	zasm/Source/Z80Header.cpp \
	zasm/Source/CharMap.cpp \
	zasm/Source/helpers.cpp \
	zasm/Source/outputfile.cpp \
	zasm/Source/listfile.cpp \
	zasm/Source/SyntaxError.cpp \
	zasm/Source/zx7.cpp \
	zasm/Source/assemble8080.cpp \
	zasm/Source/assembleZ80.cpp \
	zasm/Source/convert8080.cpp \
	zasm/Source/runTestcode.cpp \
	zasm/Source/Z80Registers.cpp \
	zasm/Source/Z80.cpp \
	zasm/Source/Z180.cpp \
	zasm/Source/Value.cpp \
	\
	Source/Qt/MachineList.cpp \
	Source/Qt/qt_util.cpp \
	Source/Qt/Settings.cpp \
	Source/Qt/SimpleTerminal.cpp \
	Source/Qt/CheckUpdate.cpp \
	Source/Qt/Preferences.cpp \
	Source/Qt/Application.cpp \
	Source/Qt/WindowMenu.cpp \
	Source/Qt/ToolWindow.cpp \
	Source/Qt/MachineController.cpp \
	Source/Qt/MyLineEdit.cpp \
	Source/Qt/MySimpleToggleButton.cpp \
	Source/Qt/RecentFilesMenu.cpp \
	Source/Qt/Lenslok.cpp \
	Source/Qt/Mouse.cpp \
	\
	Source/Qt/Screen/Screen.cpp \
	Source/Qt/Screen/ScreenMono.cpp \
	Source/Qt/Screen/ScreenZxsp.cpp \
	\
	Source/Qt/Inspector/SpectraVideoInspector.cpp \
	Source/Qt/Inspector/WalkmanInspector.cpp \
	Source/Qt/Inspector/Machine50x60Inspector.cpp \
	Source/Qt/Inspector/Inspector.cpp \
	Source/Qt/Inspector/KempstonJoyInsp.cpp \
	Source/Qt/Inspector/ZonxBoxInsp.cpp \
	Source/Qt/Inspector/UlaInsp.cpp \
	Source/Qt/Inspector/Tc2048JoyInsp.cpp \
	Source/Qt/Inspector/Z80Insp.cpp \
	Source/Qt/Inspector/AyInsp.cpp \
	Source/Qt/Inspector/JoyInsp.cpp \
	Source/Qt/Inspector/TapeRecorderInsp.cpp \
	Source/Qt/Inspector/IcTesterInsp.cpp \
	Source/Qt/Inspector/KempstonMouseInsp.cpp \
	Source/Qt/Inspector/CursorJoyInsp.cpp \
	Source/Qt/Inspector/DktronicsDualJoyInsp.cpp \
	Source/Qt/Inspector/ZxIf1Insp.cpp \
	Source/Qt/Inspector/ZxIf2Insp.cpp \
	Source/Qt/Inspector/ZxPrinterInsp.cpp \
	Source/Qt/Inspector/FullerBoxInsp.cpp \
	Source/Qt/Inspector/InvesJoyInsp.cpp \
	Source/Qt/Inspector/DidaktikMelodikInsp.cpp \
	Source/Qt/Inspector/SinclairJoyInsp.cpp \
	Source/Qt/Inspector/Tc2068JoyInsp.cpp \
	Source/Qt/Inspector/FdcPlus3Insp.cpp \
	Source/Qt/Inspector/FdcBeta128Insp.cpp \
	Source/Qt/Inspector/FdcPlusDInsp.cpp \
	Source/Qt/Inspector/FdcD80Insp.cpp \
	Source/Qt/Inspector/FdcJLOInsp.cpp \
	Source/Qt/Inspector/PrinterAercoInsp.cpp \
	Source/Qt/Inspector/PrinterLprint3Insp.cpp \
	Source/Qt/Inspector/PrinterPlus3Insp.cpp \
	Source/Qt/Inspector/PrinterTs2040Insp.cpp \
	Source/Qt/Inspector/GrafPadInsp.cpp \
	Source/Qt/Inspector/Multiface1Insp.cpp \
	Source/Qt/Inspector/Multiface128Insp.cpp \
	Source/Qt/Inspector/Multiface3Insp.cpp \
	Source/Qt/Inspector/Zx3kInsp.cpp \
	Source/Qt/Inspector/Memotech64kRamInsp.cpp \
	Source/Qt/Inspector/MachineInspector.cpp \
	Source/Qt/Inspector/Tk85JoyInsp.cpp \
	Source/Qt/Inspector/MemoryInspector.cpp \
	Source/Qt/Inspector/MemoryAccessInspector.cpp \
	Source/Qt/Inspector/MemoryDisassInspector.cpp \
	Source/Qt/Inspector/MemoryGraphInspector.cpp \
	Source/Qt/Inspector/MemoryHexInspector.cpp \
	Source/Qt/Inspector/TccDockInspector.cpp \
	Source/Qt/Inspector/DivIDEInspector.cpp \
	Source/Qt/Inspector/KeyboardInspector.cpp \
	Source/Qt/Inspector/CurrahMicroSpeechInsp.cpp \
	Source/Qt/Inspector/MultifaceInsp.cpp \
	Source/Qt/Inspector/SmartSDCardInspector.cpp \
	\
	Source/Qt/Dialogs/ConfigDialog.cpp \
	Source/Qt/Dialogs/ConfigureKeyboardJoystickDialog.cpp \
	Source/Qt/Overlays/Overlay.cpp \
	\
	Source/Uni/TapeFile/CswBuffer.cpp \
	Source/Uni/TapeFile/TapeFile.cpp \
	Source/Uni/TapeFile/TapeData.cpp \
	Source/Uni/TapeFile/TapData.cpp \
	Source/Uni/TapeFile/O80Data.cpp \
	Source/Uni/TapeFile/TzxData.cpp \
	Source/Uni/TapeFile/AudioData.cpp \
	Source/Uni/TapeFile/RlesData.cpp \
	Source/Uni/TapeFile/TapeFileDataBlock.cpp \
	\
	Source/Uni/Machine/Machine.cpp \
	Source/Uni/Machine/MachineZx80.cpp \
	Source/Uni/Machine/MachineZx81.cpp \
	Source/Uni/Machine/MachineZxsp.cpp \
	Source/Uni/Machine/MachineJupiter.cpp \
	Source/Uni/Machine/MachineZx128.cpp \
	Source/Uni/Machine/MachineTc2048.cpp \
	Source/Uni/Machine/MachineTc2068.cpp \
	Source/Uni/Machine/MachineZxPlus2a.cpp \
	Source/Uni/Machine/MachineZxPlus3.cpp \
	Source/Uni/Machine/MachineTk85.cpp \
	Source/Uni/Machine/MachineTs1000.cpp \
	Source/Uni/Machine/MachineTs1500.cpp \
	Source/Uni/Machine/MachineInves.cpp \
	Source/Uni/Machine/MachineTk90x.cpp \
	Source/Uni/Machine/MachineTk95.cpp \
	Source/Uni/Machine/MachineZxPlus2.cpp \
	Source/Uni/Machine/MachinePentagon128.cpp \
	\
	Source/Uni/Items/Item.cpp \
	Source/Uni/Items/Joy/Joy.cpp \
	Source/Uni/Items/Joy/SinclairJoy.cpp \
	Source/Uni/Items/Joy/KempstonJoy.cpp \
	Source/Uni/Items/Joy/Tc2048Joy.cpp \
	Source/Uni/Items/Joy/Tc2068Joy.cpp \
	Source/Uni/Items/Joy/InvesJoy.cpp \
	Source/Uni/Items/Joy/CursorJoy.cpp \
	Source/Uni/Items/Joy/Tk85Joy.cpp \
	Source/Uni/Items/Joy/ZxIf2.cpp \
	Source/Uni/Items/Joy/DktronicsDualJoy.cpp \
	Source/Uni/Items/Ula/Ula.cpp \
	Source/Uni/Items/Ula/UlaZxsp.cpp \
	Source/Uni/Items/Ula/UlaInves.cpp \
	Source/Uni/Items/Ula/UlaZx81.cpp \
	Source/Uni/Items/Ula/UlaZx80.cpp \
	Source/Uni/Items/Ula/UlaJupiter.cpp \
	Source/Uni/Items/Ula/UlaTc2048.cpp \
	Source/Uni/Items/Ula/Mmu.cpp \
	Source/Uni/Items/Ula/MmuZxsp.cpp \
	Source/Uni/Items/Ula/Mmu128k.cpp \
	Source/Uni/Items/Ula/MmuPlus3.cpp \
	Source/Uni/Items/Ula/MmuInves.cpp \
	Source/Uni/Items/Ula/MmuZx81.cpp \
	Source/Uni/Items/Ula/MmuZx80.cpp \
	Source/Uni/Items/Ula/MmuJupiter.cpp \
	Source/Uni/Items/Ula/MmuTc2048.cpp \
	Source/Uni/Items/Ula/Ula128k.cpp \
	Source/Uni/Items/Ula/UlaPlus3.cpp \
	Source/Uni/Items/Ula/MmuTk85.cpp \
	Source/Uni/Items/Ula/MmuTs1500.cpp \
	Source/Uni/Items/Ula/MmuTc2068.cpp \
	Source/Uni/Items/Ula/Crtc.cpp \
	Source/Uni/Items/Fdc/Fdc.cpp \
	Source/Uni/Items/Fdc/FdcPlus3.cpp \
	Source/Uni/Items/Fdc/FdcBeta128.cpp \
	Source/Uni/Items/Fdc/FdcPlusD.cpp \
	Source/Uni/Items/Fdc/FdcD80.cpp \
	Source/Uni/Items/Fdc/FdcJLO.cpp \
	Source/Uni/Items/Fdc/MGT.cpp \
	Source/Uni/Items/Fdc/OpusDiscovery.cpp \
	Source/Uni/Items/Fdc/Disciple.cpp \
	Source/Uni/Items/Fdc/SmartSDCard.cpp \
	Source/Uni/Items/Fdc/Fdc765.cpp \
	Source/Uni/Items/Fdc/DivIDE.cpp \
	Source/Uni/Items/Fdc/FloppyDiskDrive.cpp \
	Source/Uni/Items/Fdc/IdeDevice.cpp \
	Source/Uni/Items/Printer/Printer.cpp \
	Source/Uni/Items/Printer/ZxPrinter.cpp \
	Source/Uni/Items/Printer/PrinterPlus3.cpp \
	Source/Uni/Items/Printer/PrinterAerco.cpp \
	Source/Uni/Items/Printer/PrinterTs2040.cpp \
	Source/Uni/Items/Printer/PrinterLprint3.cpp \
	Source/Uni/Items/Ram/Jupiter16kRam.cpp \
	Source/Uni/Items/Ram/Zx16kRam.cpp \
	Source/Uni/Items/Ram/Cheetah32kRam.cpp \
	Source/Uni/Items/Ram/Zx3kRam.cpp \
	Source/Uni/Items/Ram/Memotech64kRam.cpp \
	Source/Uni/Items/Ram/ExternalRam.cpp \
	Source/Uni/Items/Ay/Ay.cpp \
	Source/Uni/Items/Ay/FullerBox.cpp \
	Source/Uni/Items/Ay/AySubclasses.cpp \
	Source/Uni/Items/Multiface/Multiface1.cpp \
	Source/Uni/Items/Multiface/Multiface128.cpp \
	Source/Uni/Items/Multiface/Multiface3.cpp \
	Source/Uni/Items/Multiface/Multiface.cpp \
	Source/Uni/Items/Z80/Z80_Disassembler.cpp \
	Source/Uni/Items/Z80/zxsp_Z80.cpp \
	\
	Source/Uni/Items/IcTester.cpp \
	Source/Uni/Items/KempstonMouse.cpp \
	Source/Uni/Items/ZxIf1.cpp \
	Source/Uni/Items/WafaDrive.cpp \
	Source/Uni/Items/AmxMouse.cpp \
	Source/Uni/Items/Grafpad.cpp \
	Source/Uni/Items/TapeRecorder.cpp \
	Source/Uni/Items/SpectraVideo.cpp \
	Source/Uni/Items/CurrahMicroSpeech.cpp \
	Source/Uni/Items/Keyboard.cpp \
	Source/Uni/Items/SP0256.cpp \
	Source/Uni/Items/MassStorage.cpp \
	\
	Source/Uni/Video/ZxspRenderer.cpp \
	Source/Uni/Video/Tc2048Renderer.cpp \
	Source/Uni/Video/Renderer.cpp \
	Source/Uni/Video/MonoRenderer.cpp \
	Source/Uni/Video/SpectraRenderer.cpp \
	Source/Uni/Video/TVDecoderMono.cpp \
	\
	Source/Uni/Files/file_szx.cpp \
	Source/Uni/Files/FloppyDisk.cpp \
	Source/Uni/Files/TccRom.cpp \
	Source/Uni/Files/file_z80.cpp \
	Source/Uni/Files/Z80Head.cpp \
	Source/Uni/Files/RzxBlock.cpp \
	Source/Uni/Files/RzxFile.cpp \
	\
	Source/Uni/ZxInfo/ZxInfo.cpp \
	\
	Source/Uni/zxsp_helpers.cpp \
	Source/Uni/IoInfo.cpp \
	Source/Uni/Memory.cpp \
	Source/Uni/IsaObject.cpp \


# Library Headers:

HEADERS += \
	Libraries/audio/convert_audio.h \
	Libraries/kio/kio.h \
	Libraries/kio/cdefs.h \
	Libraries/kio/auto_config.h \
	Libraries/kio/detect_configuration.h \
	Libraries/kio/standard_types.h \
	Libraries/kio/exceptions.h \
	Libraries/kio/errors.h \
	Libraries/kio/peekpoke.h \
	Libraries/kio/util/swap.h \
	Libraries/kio/util/msbit.h \
	Libraries/kio/util/count1bits.h \
	Libraries/kio/TestTimer.h \
	Libraries/Qt/QEventTypes.h \
	\
	Libraries/unix/log.h \
	Libraries/unix/os_utilities.h \
	Libraries/unix/FD.h \
	Libraries/unix/files.h \
	Libraries/unix/n-compress.h \
	\
	Libraries/cpp/cppthreads.h \
	\
	Libraries/graphics/gif/Colormap.h \
	Libraries/graphics/gif/Pixelmap.h \
	Libraries/graphics/gif/GifEncoder.h \
	Libraries/graphics/gif/GifDecoder.h \
	Libraries/graphics/gif/GifArray.h \
	Libraries/graphics/geometry.h \
	\
	Libraries/Templates/Array.h \
	Libraries/Templates/HashMap.h \
	Libraries/Templates/sort.h \
	Libraries/Templates/RCPtr.h \
	Libraries/Templates/RCObject.h \
	Libraries/Templates/NVPtr.h \
	Libraries/Templates/StrArray.h \
	\
	Libraries/cstrings/cstrings.h \
	Libraries/cstrings/tempmem.h \
	Libraries/hash/sdbm_hash.h \
	\
	Libraries/audio/AudioDecoder.h \
	Libraries/audio/WavFile.h \
	\
	Libraries/Z80/goodies/z80_opcodes.h \
	Libraries/Z80/goodies/z80_goodies.h \
	Libraries/Z80/goodies/CpuID.h \
	\
	Libraries/Devices/devices_types.h \
	Libraries/Devices/File.h \
	Libraries/Devices/SerialDevice.h \
	Libraries/Devices/StdFile.h \


# zasm Z80 Assembler Headers:

HEADERS += \
	zasm/Source/Error.h \
	zasm/Source/Label.h \
	zasm/Source/Segment.h \
	zasm/Source/settings.h \
	zasm/Source/Source.h \
	zasm/Source/SyntaxError.h \
	zasm/Source/Z80Assembler.h \
	zasm/Source/settings.h \
	zasm/Source/Z80Header.h \
	zasm/Source/CharMap.h \
	zasm/Source/helpers.h \
	zasm/Source/zx7.h \
	zasm/Source/Z80Registers.h \
	zasm/Source/Z80.h \
	zasm/Source/Value.cpp \


# zxsp Headers - OS stuff (AudioIO, Joysticks):

HEADERS += \
	Source/settings.h \
	Source/version.h \


# zxsp Headers - Qt GUI stuff:

HEADERS += \
	Source/Qt/Inspector/Inspector.h \
	Source/Qt/Inspector/AyInsp.h \
	Source/Qt/Inspector/CursorJoyInsp.h \
	Source/Qt/Inspector/DktronicsDualJoyInsp.h \
	Source/Qt/Inspector/FullerBoxInsp.h \
	Source/Qt/Inspector/IcTesterInsp.h \
	Source/Qt/Inspector/InvesJoyInsp.h \
	Source/Qt/Inspector/JoyInsp.h \
	Source/Qt/Inspector/KempstonJoyInsp.h \
	Source/Qt/Inspector/KempstonMouseInsp.h \
	Source/Qt/Inspector/TapeRecorderInsp.h \
	Source/Qt/Inspector/Tc2048JoyInsp.h \
	Source/Qt/Inspector/UlaInsp.h \
	Source/Qt/Inspector/Z80Insp.h \
	Source/Qt/Inspector/ZonxBoxInsp.h \
	Source/Qt/Inspector/ZxIf1Insp.h \
	Source/Qt/Inspector/ZxIf2Insp.h \
	Source/Qt/Inspector/ZxPrinterInsp.h \
	Source/Qt/Inspector/DidaktikMelodikInsp.h \
	Source/Qt/Inspector/SinclairJoyInsp.h \
	Source/Qt/Inspector/Tc2068JoyInsp.h \
	Source/Qt/Inspector/FdcPlus3Insp.h \
	Source/Qt/Inspector/FdcBeta128Insp.h \
	Source/Qt/Inspector/FdcPlusDInsp.h \
	Source/Qt/Inspector/FdcD80Insp.h \
	Source/Qt/Inspector/FdcJLOInsp.h \
	Source/Qt/Inspector/PrinterAercoInsp.h \
	Source/Qt/Inspector/PrinterLprint3Insp.h \
	Source/Qt/Inspector/PrinterPlus3Insp.h \
	Source/Qt/Inspector/PrinterTs2040Insp.h \
	Source/Qt/Inspector/GrafPadInsp.h \
	Source/Qt/Inspector/Multiface1Insp.h \
	Source/Qt/Inspector/Multiface128Insp.h \
	Source/Qt/Inspector/Multiface3Insp.h \
	Source/Qt/Inspector/Zx3kInsp.h \
	Source/Qt/Inspector/Memotech64kRamInsp.h \
	Source/Qt/Inspector/MachineInspector.h \
	Source/Qt/Inspector/Tk85JoyInsp.h \
	Source/Qt/Inspector/MemoryInspector.h \
	Source/Qt/Inspector/MemoryDisassInspector.h \
	Source/Qt/Inspector/MemoryGraphInspector.h \
	Source/Qt/Inspector/MemoryAccessInspector.h \
	Source/Qt/Inspector/MemoryHexInspector.h \
	Source/Qt/Inspector/SpectraVideoInspector.h \
	Source/Qt/Inspector/WalkmanInspector.h \
	Source/Qt/Inspector/Machine50x60Inspector.h \
	Source/Qt/Inspector/TccDockInspector.h \
	Source/Qt/Inspector/DivIDEInspector.h \
	Source/Qt/Inspector/KeyboardInspector.h \
	Source/Qt/Inspector/CurrahMicroSpeechInsp.h \
	Source/Qt/Inspector/MultifaceInsp.h \
	Source/Qt/Inspector/SmartSDCardInspector.h \
	\
	Source/Qt/Screen/ScreenMono.h \
	Source/Qt/Screen/ScreenZxsp.h \
	Source/Qt/Screen/Screen.h \
	\
	Source/Qt/Dialogs/ConfigDialog.h \
	Source/Qt/Dialogs/ConfigureKeyboardJoystickDialog.h \
	Source/Qt/Overlays/Overlay.h \
	\
	Source/Qt/gui_types.h \
	Source/Qt/MachineList.h \
	Source/Qt/Mouse.h \
	Source/Qt/CheckUpdate.h \
	Source/Qt/qt_util.h \
	Source/Qt/Settings.h \
	Source/Qt/Preferences.h \
	Source/Qt/Application.h \
	Source/Qt/ToolWindow.h \
	Source/Qt/WindowMenu.h \
	Source/Qt/MachineController.h \
	Source/Qt/MyLineEdit.h \
	Source/Qt/SimpleTerminal.h \
	Source/Qt/MySimpleToggleButton.h \
	Source/Qt/Lenslok.h \
	Source/Qt/RecentFilesMenu.h \


# zxsp Headers - ZX Spectrum Machine & Item Models:

HEADERS += \
	Source/Uni/Interfaces/IMachineController.h \
	Source/Uni/Interfaces/IScreen.h \
	\
	Source/Uni/Audio/StereoSample.h \
	\
	Source/Uni/Machine/Machine.h \
	Source/Uni/Machine/MachineZx80.h \
	Source/Uni/Machine/MachineZx81.h \
	Source/Uni/Machine/MachineZxsp.h \
	Source/Uni/Machine/MachineJupiter.h \
	Source/Uni/Machine/MachineZx128.h \
	Source/Uni/Machine/MachineTc2048.h \
	Source/Uni/Machine/MachineTc2068.h \
	Source/Uni/Machine/MachineZxPlus2a.h \
	Source/Uni/Machine/MachineZxPlus3.h \
	Source/Uni/Machine/MachineTk85.h \
	Source/Uni/Machine/MachineTs1000.h \
	Source/Uni/Machine/MachineTs1500.h \
	Source/Uni/Machine/MachineInves.h \
	Source/Uni/Machine/MachineTk90x.h \
	Source/Uni/Machine/MachineTk95.h \
	Source/Uni/Machine/MachineZxPlus2.h \
	Source/Uni/Machine/MachinePentagon128.h \
	\
	Source/Uni/TapeFile/TapeFile.h \
	Source/Uni/TapeFile/TapeData.h \
	Source/Uni/TapeFile/TapData.h \
	Source/Uni/TapeFile/O80Data.h \
	Source/Uni/TapeFile/TzxData.h \
	Source/Uni/TapeFile/AudioData.h \
	Source/Uni/TapeFile/CswBuffer.h \
	Source/Uni/TapeFile/RlesData.h \
	Source/Uni/TapeFile/TapeFileDataBlock.h \
	\
	Source/Uni/Items/Ula/Ula.h \
	Source/Uni/Items/Ula/Ula.h \
	Source/Uni/Items/Ula/UlaZxsp.h \
	Source/Uni/Items/Ula/UlaInves.h \
	Source/Uni/Items/Ula/UlaZx81.h \
	Source/Uni/Items/Ula/UlaZx80.h \
	Source/Uni/Items/Ula/UlaJupiter.h \
	Source/Uni/Items/Ula/UlaTc2048.h \
	Source/Uni/Items/Ula/Mmu.h \
	Source/Uni/Items/Ula/MmuZxsp.h \
	Source/Uni/Items/Ula/Mmu128k.h \
	Source/Uni/Items/Ula/MmuPlus3.h \
	Source/Uni/Items/Ula/MmuInves.h \
	Source/Uni/Items/Ula/MmuZx81.h \
	Source/Uni/Items/Ula/MmuZx80.h \
	Source/Uni/Items/Ula/MmuJupiter.h \
	Source/Uni/Items/Ula/MmuTc2048.h \
	Source/Uni/Items/Ula/Ula128k.h \
	Source/Uni/Items/Ula/UlaPlus3.h \
	Source/Uni/Items/Ula/UlaTc2048.h \
	Source/Uni/Items/Ula/MmuTk85.h \
	Source/Uni/Items/Ula/MmuTs1500.h \
	Source/Uni/Items/Ula/MmuTc2068.h \
	Source/Uni/Items/Ula/Crtc.h \
	\
	Source/Uni/Items/Joy/Joy.h \
	Source/Uni/Items/Joy/KempstonJoy.h \
	Source/Uni/Items/Joy/Tc2048Joy.h \
	Source/Uni/Items/Joy/InvesJoy.h \
	Source/Uni/Items/Joy/CursorJoy.h \
	Source/Uni/Items/Joy/Tc2068Joy.h \
	Source/Uni/Items/Joy/DktronicsDualJoy.h \
	Source/Uni/Items/Joy/SinclairJoy.h \
	Source/Uni/Items/Joy/ZxIf2.h \
	Source/Uni/Items/Joy/Tk85Joy.h \
	Source/Uni/Items/Fdc/FdcPlus3.h \
	Source/Uni/Items/Fdc/FdcBeta128.h \
	Source/Uni/Items/Fdc/FdcPlusD.h \
	Source/Uni/Items/Fdc/FdcD80.h \
	Source/Uni/Items/Fdc/FdcJLO.h \
	Source/Uni/Items/Fdc/MGT.h \
	Source/Uni/Items/Fdc/Fdc.h \
	Source/Uni/Items/Fdc/DivIDE.h \
	Source/Uni/Items/Fdc/FloppyDiskDrive.h \
	Source/Uni/Items/Fdc/IdeDevice.h \
	Source/Uni/Items/Fdc/OpusDiscovery.h \
	Source/Uni/Items/Fdc/Disciple.h \
	Source/Uni/Items/Fdc/Fdc765.h \
	Source/Uni/Items/Fdc/SmartSDCard.h \
	Source/Uni/Items/Printer/Printer.h \
	Source/Uni/Items/Printer/ZxPrinter.h \
	Source/Uni/Items/Printer/PrinterPlus3.h \
	Source/Uni/Items/Printer/PrinterAerco.h \
	Source/Uni/Items/Printer/PrinterTs2040.h \
	Source/Uni/Items/Printer/PrinterLprint3.h \
	Source/Uni/Items/Ram/ExternalRam.h \
	Source/Uni/Items/Ram/Jupiter16kRam.h \
	Source/Uni/Items/Ram/Zx16kRam.h \
	Source/Uni/Items/Ram/Cheetah32kRam.h \
	Source/Uni/Items/Ram/Zx3kRam.h \
	Source/Uni/Items/Ram/Memotech64kRam.h \
	Source/Uni/Items/Ay/Ay.h \
	Source/Uni/Items/Ay/AySubclasses.h \
	Source/Uni/Items/Ay/FullerBox.h \
	Source/Uni/Items/Multiface/Multiface.h \
	Source/Uni/Items/Multiface/Multiface128.h \
	Source/Uni/Items/Multiface/Multiface3.h \
	Source/Uni/Items/Multiface/Multiface1.h \
	\
	Source/Uni/Items/Z80/Z80macros.h \
	Source/Uni/Items/Z80/Z80codesXY.h \
	Source/Uni/Items/Z80/Z80codesED.h \
	Source/Uni/Items/Z80/Z80codesCB.h \
	Source/Uni/Items/Z80/Z80codes.h \
	Source/Uni/Items/Z80/Z80.h \
	Source/Uni/Items/Z80/Z80_Disassembler.h \
	Source/Uni/Items/Z80/Z80opcodes.h \
	Source/Uni/Items/Z80/Z80options.h \
	\
	Source/Uni/Items/IcTester.h \
	Source/Uni/Items/KempstonMouse.h \
	Source/Uni/Items/TapeRecorder.h \
	Source/Uni/Items/ZxIf1.h \
	Source/Uni/Items/Item.h \
	Source/Uni/Items/Grafpad.h \
	Source/Uni/Items/WafaDrive.h \
	Source/Uni/Items/AmxMouse.h \
	Source/Uni/Items/Keyboard.h \
	Source/Uni/Items/CurrahMicroSpeech.h \
	Source/Uni/Items/SpectraVideo.h \
	Source/Uni/Items/MemObject.h \
	Source/Uni/Items/SP0256.h \
	Source/Uni/Items/MassStorage.h \
	\
	Source/Uni/Video/ZxspRenderer.h \
	Source/Uni/Video/Tc2048Renderer.h \
	Source/Uni/Video/Renderer.h \
	Source/Uni/Video/MonoRenderer.h \
	Source/Uni/Video/SpectraRenderer.h \
	Source/Uni/Video/TVDecoderMono.h \
	\
	Source/Uni/ZxInfo/ZxInfo.h \
	Source/Uni/ZxInfo/info.h \
	Source/Uni/ZxInfo/zxsp_basic_tokens.h \
	\
	Source/Uni/Files/Z80Head.h \
	Source/Uni/Files/file_szx.h \
	Source/Uni/Files/FloppyDisk.h \
	Source/Uni/Files/TccRom.h \
	Source/Uni/Files/RzxFile.h \
	Source/Uni/Files/RzxBlock.h \
	\
	Source/Uni/zxsp_globals.h \
	Source/Uni/zxsp_helpers.h \
	Source/Uni/custom_errors.h \
	Source/Uni/zxsp_types.h \
	Source/Uni/IsaObject.h \
	Source/Uni/isa_id.h \
	Source/Uni/IoInfo.h \
	Source/Uni/Memory.h \
	Source/Uni/precompiled_header.h \
	Source/Uni/about_text.h \
	Source/Uni/Keymap.h \


# Other Files - Info & Helpers

OTHER_FILES += \
	.gitignore \
	Source/OS/Mac/makemacstuff.vs \
	Source/OS/Mac/Dsp.txt \
	Source/OS/DspTime.txt \
	Libraries/kio/linux_errors.txt \
	Source/Uni/ZxInfo/info_video.txt \
	Source/Uni/ZxInfo/makezxinfo.vs \
	Source/Uni/ZxInfo/ZxInfo.csv \
	Source/Uni/Items/Fdc/DivIDE.txt \
	Source/Uni/Files/file_szx.info \
	Info/Disk/FDC765_info.txt \


# sdcc C Compiler Sources and Header Files:

OTHER_FILES += \
	sdcc/lib/___setjmp.s \
	sdcc/lib/__divsint.s \
	sdcc/lib/__divsuchar.s \
	sdcc/lib/__divuint.s \
	sdcc/lib/__divuschar.s \
	sdcc/lib/__modsint.s \
	sdcc/lib/__modsuchar.s \
	sdcc/lib/__moduint.s \
	sdcc/lib/__mulint.s \
	sdcc/lib/__mulschar.s \
	sdcc/lib/__sdcc_call_hl.s \
	sdcc/lib/_localtime.s \
	sdcc/lib/_memmove.s \
	sdcc/lib/_putchar.s \
	sdcc/lib/_strcpy.s \
	sdcc/lib/_strlen.s \
	sdcc/lib/crt0 .s \
	sdcc/lib/heap .s \
	sdcc/lib/___fs2schar.c \
	sdcc/lib/___fs2sint.c \
	sdcc/lib/___fs2slong.c \
	sdcc/lib/___fs2uchar.c \
	sdcc/lib/___fs2uint.c \
	sdcc/lib/___fs2ulong.c \
	sdcc/lib/___fsadd.c \
	sdcc/lib/___fsdiv.c \
	sdcc/lib/___fseq.c \
	sdcc/lib/___fsgt.c \
	sdcc/lib/___fslt.c \
	sdcc/lib/___fsmul.c \
	sdcc/lib/___fsneq.c \
	sdcc/lib/___fssub.c \
	sdcc/lib/___schar2fs.c \
	sdcc/lib/___sint2fs.c \
	sdcc/lib/___slong2fs.c \
	sdcc/lib/___uchar2fs.c \
	sdcc/lib/___uint2fs.c \
	sdcc/lib/___ulong2fs.c \
	sdcc/lib/__assert.c \
	sdcc/lib/__divslong.c \
	sdcc/lib/__divslonglong.c \
	sdcc/lib/__divulong.c \
	sdcc/lib/__divulonglong.c \
	sdcc/lib/__itoa.c \
	sdcc/lib/__modslong.c \
	sdcc/lib/__modulong.c \
	sdcc/lib/__mullong.c \
	sdcc/lib/__mullonglong.c \
	sdcc/lib/__print_format.c \
	sdcc/lib/__rlslonglong.c \
	sdcc/lib/__rlulonglong.c \
	sdcc/lib/__rrslonglong.c \
	sdcc/lib/__rrulonglong.c \
	sdcc/lib/__uitoa.c \
	sdcc/lib/_abs.c \
	sdcc/lib/_acosf.c \
	sdcc/lib/_asctime.c \
	sdcc/lib/_asincosf.c \
	sdcc/lib/_asinf.c \
	sdcc/lib/_atan2f.c \
	sdcc/lib/_atanf.c \
	sdcc/lib/_atof.c \
	sdcc/lib/_atoi.c \
	sdcc/lib/_atol.c \
	sdcc/lib/_calloc.c \
	sdcc/lib/_ceilf.c \
	sdcc/lib/_check_struct_tm.c \
	sdcc/lib/_cosf.c \
	sdcc/lib/_coshf.c \
	sdcc/lib/_cotf.c \
	sdcc/lib/_ctime.c \
	sdcc/lib/_days_per_month.c \
	sdcc/lib/_errno.c \
	sdcc/lib/_expf.c \
	sdcc/lib/_fabsf.c \
	sdcc/lib/_floorf.c \
	sdcc/lib/_free.c \
	sdcc/lib/_frexpf.c \
	sdcc/lib/_gets.c \
	sdcc/lib/_gmtime.c \
	sdcc/lib/_heap.c \
	sdcc/lib/_isalnum.c \
	sdcc/lib/_isalpha.c \
	sdcc/lib/_isblank.c \
	sdcc/lib/_iscntrl.c \
	sdcc/lib/_isdigit.c \
	sdcc/lib/_isgraph.c \
	sdcc/lib/_islower.c \
	sdcc/lib/_isprint.c \
	sdcc/lib/_ispunct.c \
	sdcc/lib/_isspace.c \
	sdcc/lib/_isupper.c \
	sdcc/lib/_isxdigit.c \
	sdcc/lib/_labs.c \
	sdcc/lib/_ldexpf.c \
	sdcc/lib/_log10f.c \
	sdcc/lib/_logf.c \
	sdcc/lib/_ltoa.c \
	sdcc/lib/_malloc.c \
	sdcc/lib/_memchr.c \
	sdcc/lib/_memcmp.c \
	sdcc/lib/_memcpy.c \
	sdcc/lib/_memset.c \
	sdcc/lib/_mktime.c \
	sdcc/lib/_modff.c \
	sdcc/lib/_powf.c \
	sdcc/lib/_printf_small.c \
	sdcc/lib/_printf.c \
	sdcc/lib/_put_char_to_stdout.c \
	sdcc/lib/_put_char_to_string.c \
	sdcc/lib/_puts.c \
	sdcc/lib/_rand.c \
	sdcc/lib/_realloc.c \
	sdcc/lib/_sincosf.c \
	sdcc/lib/_sincoshf.c \
	sdcc/lib/_sinf.c \
	sdcc/lib/_sinhf.c \
	sdcc/lib/_sprintf.c \
	sdcc/lib/_sqrtf.c \
	sdcc/lib/_strcat.c \
	sdcc/lib/_strchr.c \
	sdcc/lib/_strcmp.c \
	sdcc/lib/_strcspn.c \
	sdcc/lib/_strncat.c \
	sdcc/lib/_strncmp.c \
	sdcc/lib/_strncpy.c \
	sdcc/lib/_strpbrk.c \
	sdcc/lib/_strrchr.c \
	sdcc/lib/_strspn.c \
	sdcc/lib/_strstr.c \
	sdcc/lib/_strtok.c \
	sdcc/lib/_strxfrm.c \
	sdcc/lib/_tancotf.c \
	sdcc/lib/_tanf.c \
	sdcc/lib/_tanhf.c \
	sdcc/lib/_time.c \
	sdcc/lib/_tolower.c \
	sdcc/lib/_toupper.c \
	sdcc/lib/_vprintf.c \
	sdcc/lib/_vsprintf.c \
	sdcc/lib/_log_table.h \
	\
	sdcc/include/asm/default/features.h \
	sdcc/include/asm/z80/features.h \
	sdcc/include/assert.h \
	sdcc/include/ctype.h \
	sdcc/include/errno.h \
	sdcc/include/float.h \
	sdcc/include/iso646.h \
	sdcc/include/limits.h \
	sdcc/include/malloc.h \
	sdcc/include/math.h \
	sdcc/include/sdcc-lib.h \
	sdcc/include/setjmp.h \
	sdcc/include/stdalign.h \
	sdcc/include/stdarg.h \
	sdcc/include/stdbool.h \
	sdcc/include/stddef.h \
	sdcc/include/stdint.h \
	sdcc/include/stdio.h \
	sdcc/include/stdlib.h \
	sdcc/include/stdnoreturn.h \
	sdcc/include/string.h \
	sdcc/include/time.h \
	sdcc/include/tinibios.h \
	sdcc/include/typeof.h \
	\
	sdcc/sdcc_info.txt \







