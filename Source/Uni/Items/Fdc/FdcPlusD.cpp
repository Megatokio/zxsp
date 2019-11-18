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


#include "FdcPlusD.h"

/*	Plus D Interface
	up to 2 Disk drives
	1 Centronics port
*/

//    Wos:
//    #define P_PLUSD_CMD                     0xe3    /* Command */
//    #define P_PLUSD_STATE                   0xe3    /* State */
//    #define P_PLUSD_PAGE                    0xe7    /* Memory paging */
//    #define P_PLUSD_TRACK                   0xeb    /* Track */
//    #define P_PLUSD_SYSTEM                  0xef    /* System register */
//    #define P_PLUSD_SECTOR                  0xf3    /* Sector */
//    #define P_PLUSD_PRINTER                 0xf7    /* Printer data/ready */
//    #define P_PLUSD_DATA                    0xfb    /* Data */



static cstr o_addr = NULL;//TODO
static cstr i_addr = NULL;//TODO

FdcPlusD::FdcPlusD(Machine *m)
:    Fdc(m,isa_FdcPlusD,external,o_addr,i_addr)
{
}
