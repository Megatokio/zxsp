// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZxIf1.h"


/*	ZX Interface 1
	up to 8 Microdrives
	1 RS-232 port
	2 ZX Network ports: http://scratchpad.wikia.com/wiki/ZX_Net
*/

//    WoS:
//    Peripheral: ZX Interface I (RS232/Network).
//    Port: ---- ---- ---1 0--- (More information).

//    Peripheral: ZX Interface I (Control).
//    Port: ---- ---- ---0 1--- (More information).

//    Peripheral: ZX Interface I (Microdrive).
//    Port: ---- ---- ---0 0--- (More information).



/* System Variables:
	Notes   Address Name    Contents
	X1      23734   FLAGS3  Flags
	X2      23735   VECTOR  Address used to extend the BASIC interpreter
	X10     23737   SBRT    ROM paging subroutine
	2       23747   BAUD    Two byte number determining the baud rate
							calculated as follows:
							BAUD=(3500000 / (26 * baud rate)) - 2
	1       23749   NTSTAT  Own network station number
	1       23750   IOBORD  Border colour used during I/O. You can poke
							any colour you want.
	N2      23751   SER_FL  2 byte workspace used by RS232
	N2      23753   SECTOR  2 byte workspace used by Microdrive
	N2      23755   CHADD_  Temporary store for CH_ADD
	1       23757   NTRESP  Store for network response code
	1       23758   NTDEST  Beginning of network buffer contains
							destination station number 0-64
	1       23759   NTSRCE  Source station number
	X2      23760   NTNUMB  Network block number 0-65535
	N1      23762   NTTYPE  Header type code
	X1      23763   NTLEN   Data block length 0-255
	N1      23764   NTDCS   Data block checksum
	N1      23765   NTHCS   Header block checksum
	N2      23766   D_STR1  Start of 8 byte file specifier
							2 byte drive number 1-8
	N1      23768   S_STR1  Stream number 0-15   *See note.
	N1      23769   L_STR1  Device type... "M", "N", "T" or "B"
	N2      23770   N_STR1  Length of file name
	N2      23772   D_STR2  Second 8 byte file specifier
							used by MOVE and LOAD commands
	N1      23782   HD_00   Start of workspace for SAVE, LOAD, VERIFY and
							MERGE data type code
	N2      23783   HD_0B   Length of data 0-66535
	N2      23785   HD_0D   Start of data 0-65535
	N2      23787   HD_0F   Program length 0-66535
	N2      23789   HD_11   Line number
	1       23791   COPIES  Number of copies made by SAVE
			23792   Start of Microdrive MAPs or CHANS
*/


ZxIf1::ZxIf1(Machine*m)
: Item(m,isa_ZxIf1,isa_Item,external,nullptr,nullptr)
{
}
