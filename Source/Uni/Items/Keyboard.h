#pragma once
// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "Keymap.h"
#include "ZxInfo/ZxInfo.h"


//	preferred keyboard translation:	// defined in zxsp_types.h"
//		kbdphys						// prefer physical translation (from scan code)
//		kbdlog						// use logical translation (from character code)


typedef uint8 AsciiToZxkeyMap[0x80]; // Unicode     -> enum ZxspKey	(mapping 'by meaning')
typedef uint8 OskeyToZxkeyMap[0x40]; // OSX keycode -> enum ZxspKey	(mapping 'by key position')


enum KbdModifiers // modifier key masks:
{
	ShiftKeyMask   = 1, // realworld caps shift key
	ControlKeyMask = 2, // realworld control key
	AltKeyMask	   = 4	// realworld alt/option key: used as an auxilliary caps shift key
};


/* ========================================================================
 */
class Keyboard : public Item
{
protected:
	Model model; // emulated specci model
	uint8 csh;	 // scancode of caps shift key
	uint8 ssh;	 // scancode of symbol shift key
	uint8 csh2;
	uint8 ssh2;

	AsciiToZxkeyMap ascii2zxkey_map;
	OskeyToZxkeyMap oskey2zxkey_map;

	// state:
	KbdMode mode;	   // prefer physical or logical key translation ?
	uint	modifiers; // modifiers down on realworld keyboard
	uint	numkeys;   // keys pressed on real and virtual keyboard
	uint8	zxkeys[16];
	uint8	oskeys[16];
	uint	stickykeys;

public:
	Keymap keymap; // map of depressed keys as seen by user
				   // Keymap	ula->keymap;		// keyboard matrix as seen by ula


protected:
	Keyboard(Machine*, isa_id, const AsciiToZxkeyMap, const OskeyToZxkeyMap);

public:
	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	// void	reset			(Time t, int32 cc) override;
	// void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	// void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;
	void saveToFile(FD&) const throws override;
	void loadFromFile(FD&) throws override;

	Keymap& getKeymap() { return keymap; }
	void	specciKeyDown(uint8 zxkey); // called from KeyboardInspector
	void	specciKeyUp(uint8 zxkey);	// called from KeyboardInspector

	void realKeyDown(uint16 unicode, uint8 oskeycode, KbdModifiers); // called from MachineController
	void realKeyUp(uint16 unicode, uint8 oskeycode, KbdModifiers);	 // called from MachineController
	void keyBtZxKbd(uint16 unicode, uint8 oskeycode, KbdModifiers);	 // handle Recreated ZX Keyboard

	void setKbdGame();
	void setKbdBasic();
	void setKbdBtZxKbd();
	void setKbdMode(KbdMode);
	void allKeysUp();

private:
	VIR void convert_to_matrix(Keymap&) {}
	void	 update_keymap();
};


// ==========================================================
//						sub classes
// ==========================================================


class KeyboardZx81 : public Keyboard
{
public:
	// ZX80, ZX81, TS1000, TS1500, TK85
	explicit KeyboardZx81(Machine*, isa_id = isa_KbdZx81);
};


class KeyboardZx80 : public KeyboardZx81
{
public:
	explicit KeyboardZx80(Machine*, isa_id = isa_KbdZx80);
};


class KeyboardZxsp : public Keyboard
{
public:
	// ZXSP, TK90X, TK95
	explicit KeyboardZxsp(Machine*, isa_id = isa_KbdZxsp);
};


class KeyboardZxPlus : public Keyboard
{
public:
	// ZXPLUS, Inves, ZX128*, ZXPlus2*, ZXPlus2A*, ZXPlus3*
	explicit KeyboardZxPlus(Machine*, isa_id = isa_KbdZxPlus);

private:
	void convert_to_matrix(Keymap&) override;
};


class KeyboardTimex : public Keyboard
{
public:
	// TS2068, TC2068, u2086, TC2048
	explicit KeyboardTimex(Machine*, isa_id = isa_KbdTimex);

private:
	void convert_to_matrix(Keymap&) override;
};


class KeyboardJupiter : public Keyboard
{
public:
	// Jupiter Ace
	explicit KeyboardJupiter(Machine*, isa_id = isa_KbdJupiter);

private:
	void convert_to_matrix(Keymap&) override;
};
