// Copyright (c) 2004 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*
   this is a zombie
*/

#include "FullerBox.h"



// Joystick: #7F Fuller Box (FxxxRLDU, active low)

// k1.spdns.de: :-)
//    $xx7F   %0111.1111      Fuller joystick i/-
//    $xx3F   %0011.1111      Fuller AY reg. select/read i/o
//    $xx5F   %0101.1111      Fuller AY write -/o


// Wos FAQ:
//#define P_FULLER_CONTROL                0x3f    /* AY control */
//#define P_FULLER_DATA                   0x5f    /* AY data */
//#define P_FULLER_JOY                    0x7f    /* Joystick */
//
//    Standard Atari-style joysticks could be connected to the interface,
//    which is similar to the more popular Kempston design. The sound board
//    works on port numbers 0x3f and 0x5f. Port 0x3f is used to select the
//    active AY register and to receive data from the AY-3-8912, while port 0x5f
//    is used for sending data. The joystick is connected to port 0x7f.




// Fuller PDF:
//  To select the register – OUT 63, REG
//  To enter data into REG – OUT 95, DATA


// k1.spdns.de: :-)
//    AY Sound Chip
//            OUT (0x3F) - Select a register 0-14
//            IN  (0x5F) - Read the value of the selected register
//            OUT (0x5F) - Write to the selected register

// k1.spdns.de: :-)
//    Joystick interface.
//        Results were obtained by reading from port 0x7f.
//        Result is: F---RLDU, with active bits low.
//        Promised to work with every Imagine software



// to be verified:
static cstr s = "----.----.001-.----";		// ? select: üblicher Port: 0xF5
static cstr r = "----.----.001-.----";		// ? read:   üblicher Port: 0xF6
static cstr w = "----.----.010-.----";		// ? write:  üblicher Port: 0xF6

static const Frequency freq = 1700000;		// guessed
static const Ay::StereoMix mix = Ay::mono;	// there's no indication that it could have had stereo



// 	Ay			( Machine*, cstr sel, cstr wr, cstr rd, Frequency psg_clocks_per_second, cstr name="AY-3-8912", isa_id=isa_Ay );


FullerBox::FullerBox(Machine*m)
:Ay(m,isa_FullerBox,external,s,w,r,freq,mix)
{}





