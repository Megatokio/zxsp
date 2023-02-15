// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "file_stx.h"

#if defined(__LITTLE_ENDIAN__)
  #define ID(A, B, C, D) ((A << 0) + (B << 8) + (C << 16) + (D << 24))
#elif defined(__BIG_ENDIAN__)
  #define ID(A, B, C, D) ((A << 24) + (B << 16) + (C << 8) + (D << 0))
#else
  #error "endian macro"
#endif


// zx-state header
// The zx-state header appears right at the start of a zx-state (.szx) file.
// It is used to identify the version of the file and to specify which model of ZX Spectrum (or clone) the file refers
// to. Following the zx-state header is an optional ZXSTCREATOR creator information block. Following that are the
// ZXSTZ80REGS and ZXSTSPECREGS blocks followed by zero or more additional blocks representing the current state of the
// emulated Spectrum.


// Flags (chFlags)
#define ZXSTMF_ALTERNATETIMINGS 1

struct StxHeader
{
	enum {					  // flags:
		ALTERNATETIMINGS = 1, // The emulated Spectrum uses alternate timings (one cycle later than normal timings). If
							  // reset, the emulated Spectrum uses standard timings. This flag is only applicable for
							  // the ZXSTMID_16K, ZXSTMID_48K and ZXSTMID_128K models. machine_id:
		ZX16K		 = 0,  // 16k ZX Spectrum
		ZX48K		 = 1,  // 48k ZX Spectrum or ZX Spectrum+
		ZX128K		 = 2,  // ZX Spectrum 128
		PLUS2		 = 3,  // ZX Spectrum +2
		PLUS2A		 = 4,  // ZX Spectrum +2A/+2B
		PLUS3		 = 5,  // ZX Spectrum +3
		PLUS3E		 = 6,  // ZX Spectrum +3e
		PENTAGON128	 = 7,  // Pentagon 128
		TC2048		 = 8,  // Timex Sinclair TC2048
		TC2068		 = 9,  // Timex Sinclair TC2068
		SCORPION	 = 10, // Scorpion ZS-256
		SE			 = 11, // ZX Spectrum SE
		TS2068		 = 12, // Timex Sinclair TS2068
		PENTAGON512	 = 13, // Pentagon 512
		PENTAGON1024 = 14, // Pentagon 1024
		NTSC48K		 = 15, // 48k ZX Spectrum (NTSC)
		ZX128KE		 = 16  // ZX Spectrum 128Ke
	};

	uint32 magic;		  // Byte sequence of 'Z', 'X', 'S', 'T' to identify the file as a zx-state file.
	uint8  major_version; // Major version number of the file format. Currently 1.
	uint8  minor_version; // Minor version number of the file format. Currently 4.
	uint8  machine_id;	  // The model of ZX Spectrum (or clone) to switch to when loading the file.
	uint8  flags;		  // ALTERNATETIMINGS
};


// const
uint32 StxidZ80 = ID('Z', '8', '0', 'R'), StxidSpecRegs = ID('S', 'P', 'C', 'R'), StxidAtaSp = ID('Z', 'X', 'A', 'T'),
	   StxidAtaSpRam = ID('A', 'T', 'R', 'P'), StxidAY = ID('A', 'Y', 0, 0), StxidCF = ID('Z', 'X', 'C', 'F'),
	   StxidCFRam = ID('C', 'F', 'R', 'P'), StxidCovox = ID('C', 'O', 'V', 'X'), StxidBeta128 = ID('B', '1', '2', '8'),
	   StxidBetaDisk = ID('B', 'D', 'S', 'K'), StxidCreator = ID('C', 'R', 'T', 'R'),
	   StxidDock = ID('D', 'O', 'C', 'K'), StxidGS = ID('G', 'S', 0, 0), StxidGSRamPage = ID('G', 'S', 'R', 'P'),
	   StxidKeyboard = ID('K', 'E', 'Y', 'B'), StxidIf1 = ID('I', 'F', '1', 0), StxidIf2Rom = ID('I', 'F', '2', 'R'),
	   StxidJoystick = ID('J', 'O', 'Y', 0), StxidMdrCart = ID('M', 'D', 'R', 'V'), StxidMouse = ID('A', 'M', 'X', 'M'),
	   StxidMultiface = ID('M', 'F', 'C', 'E'), StxidOpus = ID('O', 'P', 'U', 'S'),
	   StxidOpusDisk = ID('O', 'D', 'S', 'K'), StxidPlus3 = ID('+', '3', 0, 0), StxidPlus3Disk = ID('D', 'S', 'K', 0),
	   StxidPlusD = ID('P', 'L', 'S', 'D'), StxidPlusDDisk = ID('P', 'D', 'S', 'K'),
	   StxidRamPage = ID('R', 'A', 'M', 'P'), StxidRom = ID('R', 'O', 'M', 0), StxidTimex = ID('S', 'C', 'L', 'D'),
	   StxidSIDE = ID('S', 'I', 'D', 'E'), StxidSpecDrum = ID('D', 'R', 'U', 'M'), StxidTape = ID('T', 'A', 'P', 'E'),
	   StxidUSpeech = ID('U', 'S', 'P', 'E'), StxidZxPrinter = ID('Z', 'X', 'P', 'R'),
	   StxidSpectra = ID('S', 'P', 'R', 'A');


// Block Header.
// Each real block starts with this header.
struct StxBlock
{
	uint32 id;	 // Indicates which type of block follows this header. It is a 4 byte sequence stored in a uint32.
	uint32 size; // Specifies the size of the block following this header. It does not include the size of this header.
};


// ZXSTZ80REGS
// Contains the Z80 registers and other internal state values. It does not contain any specific model registers.
struct StxZ80 : StxBlock
{
	enum { EILAST = 1, HALTED = 2 };

	uint16 af, bc, de, hl;
	uint16 af1, bc1, de1, hl1;
	uint16 ix, iy, sp, pc;
	uint8  i, r, iff1, iff2; // iff's must be 0 or 1
	uint8  im;				 // 0, 1 or 2
	uint32 cpu_cycle; // The t-states value at the time the snapshot was made. This counts up from zero to the maximum
					  // number of t-states per frame for the specific Spectrum model.
	uint8
		int_time_cc; // The number of t-states left on restart when an interrupt can occur. This is used to support
					 // interrupt re-triggering properly. On the Spectrum, the ULA holds the INTREQ line of the Z80 low
					 // for up to 48 t-states (depending on the model). If interrupts are enabled during this time, the
					 // Z80 will accept the request and invoke the appropriate interrupt service routine. The AMX mouse
					 // also asserts the INTREQ line when it needs attention. It is therefore possible for this member
					 // to be non-zero even if the t-state counter suggests we are not at the beginning of a frame.
	uint8
		flags; // EILAST	The last instruction executed was an EI instruction or an invalid $DD or $FD prefix.
			   // HALTED	The last instruction executed was a HALT instruction. The CPU is currently executing NOPs and
			   // will continue to do so until the next interrupt occurs. This flag is mutually exclusive with EILAST.
	uint16 mem_ptr; // Internal Z80 register used to generate bits 5 and 3 of the F register after executing a BIT
					// x,(HL) instruction. Set to 0 (zero) if not supported.
};


// ZXSTSPECREGS
// The Spectrum's ULA state specifying the current border colour, memory paging status etc.
struct StxSpecRegs : StxBlock
{
	uint8 border;	 // The current border colour. This can be 0 (black) through to 7 (white).
	uint8 port_7ffd; // The current value of port $7ffd which is used to control memory paging on 128k Spectrums.
					 // For 16k and 48k Spectrums, this should be set to 0 (zero).
	union
	{
		uint8 port_1ffd; // The current value of port $1ffd which controls additional memory paging features on the
						 // Spectrum +2A/+3 and the ZS Scorpion. Should be set to 0 (zero) for other models.
		uint8 port_eff7; // The current value of port $eff7 on the Pentagon 1024 which controls access to the additional
						 // memory. Not used on other models.
	};
	uint8 port_fe; // The last value of written to port $fe. Only bits 3 and 4 (the MIC and EAR bits) are guaranteed to
				   // be valid. Use 'border' to set the current border colour.

	uint8 reserved[4]; // set to 0
};


// ZXATASP IDE interface
// Sami Vehmaa's ZXATASP IDE hard disk and CompactFlash card interface for the 16K, 48K, 128, +2, +2A and +3 ZX
// Spectrums. This block will be followed by zero or more ZXSTATARAM blocks which store the contents of the interfaces
// onboard memory.
struct StxAtaSp : StxBlock
{
	enum {
		UPLOADJUMPER = 1, // The upload jumper on the interface is enabled.
		WRITEPROTECT = 2  // All on-board memory is write protected.
	};

	uint16 flags;		 // UPLOADJUMPER | WRITEPROTECT
	uint8  portA;		 // The current value of the ZXATASP's port A ($009f).
	uint8  portB;		 // The current value of the ZXATASP's port B ($019f).
	uint8  portC;		 // The current value of the ZXATASP's port C ($029f).
	uint8  control;		 // The current value of the ZXATASP's control port ($039f).
	uint8  num_rampages; // The number of 16K RAM pages installed in this ZXATSP interface.
	uint8  active_page;	 // Specifies the RAM page currently paged into $0000 - $3fff, or 255 if no paging is currently
						 // active.
};


// ZXSTATARAM
// This block is used to save and restore the contents of the ZXATASP's built-in RAM.
// A zx-state file will contain a number of these blocks, depending on the model of the ZXATASP being emulated (128KB or
// 512KB). A ZXATASP block will precede the first of these blocks. ZXATASP IDE Interface RAM page
struct StxAtaSpRam : StxBlock
{
	enum {
		COMPRESSED = 1
	}; // The RAM page data (beginning from chData) will be compressed using the Zlib compression library.

	uint16 flags;	// COMPRESSED
	uint8  page_no; // Page number of this 16KB RAM page. For the 128KB version of the ZXATASP interface, this is 0 - 7.
					// For the 512KB version, this is 0 - 31.
	uint8 data[1];	// The actual compressed or uncompressed memory page data. When uncompressed, this member is exactly
					// 16KB (16,384) bytes in size.
};
//	Note: when loading Zlib compressed pages, the compressed size can be obtained by: compressedSize = blk.dwSize - (
//sizeof( ZXSTATARAM ) - sizeof( ZXSTBLOCK ) - 1 );


// ZXSTAYBLOCK
// The state of the AY chip found in all 128k Spectrums, Pentagons, Scorpions and Timex machines. This block may also be
// present for 16k/48k Spectrums if Fuller Box or Melodik emulation is enabled. AY Block. Contains the AY register
// values
struct StxAY : StxBlock
{
	enum {
		BUILT_IN  = 0, // 128k
		FULLERBOX = 1, // Fuller Box emulation.
		ZX128AY	  = 2  // Melodik Soundbox emulation. (external 128K compatible sound box)
	};

	uint8 flags;	   // BUILT_IN, FULLERBOX or ZX128AY
	uint8 current_reg; // The currently selected AY register (0-15).
	uint8 regs[16];	   // The current values of the AY registers.
					// Note: The AY chip does not use all 8 bits for all registers. Where this is the case, the unused
					// bits should be set to 0 (zero).
};


// ZXSTCF
// Sami Vehmaa's ZXCF CompactFlash interface for the 16K, 48K, 128, +2, +2A and +3 ZX Spectrums.
// This block will be followed by zero or more ZXSTCFRAM blocks which store the contents of the interface's onboard
// memory. ZXCF CompactFlash interface
struct StxCF : StxBlock
{
	enum { UPLOADJUMPER = 1 }; // The upload jumper on the interface is enabled.

	uint16 flags;		 // UPLOADJUMPER
	uint8  mem_ctrl;	 // The current value of the ZXCF's memory control register.
	uint8  num_rampages; // The amount of RAM installed in this ZXCF interface, in units of 16KB. Normally either 32
						 // (512KB version) or 64 (1024KB version).
};


// ZXSTCFRAM
// This block is used to save and restore the contents of the ZXCF's built-in RAM.
// A zx-state file will contain a number of these blocks, depending on the model of the ZXCF being emulated (512KB or
// 1024KB). A ZXSTCF block will proceed the first of these blocks. ZXCF CompactFlash Interface RAM page
struct StxCFRam : StxBlock
{
	enum { COMPRESSED = 1 }; // the RAM page data[] will be compressed using the Zlib compression library.

	uint16 flags;  // COMPRESSED
	uint8 page_no; // Page number of this 16K RAM page. For the 512KB version of the ZXCF interface, this is 0 - 31. For
				   // the 1024KB version, this is 0 - 63.
	uint8 data[1]; // The actual compressed or uncompressed memory page data. When uncompressed, this member is exactly
				   // 16KB (16,384) bytes in size.
};
// When loading Zlib compressed pages, the compressed size can be obtained by: compressedSize = blk.dwSize - ( sizeof(
// ZXSTCFRAM ) - sizeof( ZXSTBLOCK ) - 1 );


// ZXSTCOVOX
// The state of the Covox sound system.
// Covox state
struct StxCovox : StxBlock
{
	uint8 current_volume; // The current amplitude level of the Covox's DAC.
	uint8 reserved[3];	  // set to 0
};


// ZXSTBETA128
// Beta 128 disk interface from Technology Research UK Ltd. A clone of this interface is also used in the Russian
// Pentagon and Scorpion machines. Any blocks specifying which disk files are in which drive will follow this one. Beta
// 128 disk interface used by the Pentagon and Scorpion
struct StxBeta128 : StxBlock
{
	enum {
		CONNECTED = 1, // The interface is connected and enabled. This is always set for Pentagon and Scorpion machines.
		CUSTOMROM = 2, // A custom TR-DOS ROM is installed. The ROM image begins at chRomData.
					   // The default ROM is the last version by Technology Research, version 5.03.
		PAGED	  = 4, // The TR-DOS ROM is currently paged in.
		AUTOBOOT  = 8, // The Beta 128's Auto boot feature is enabled (48k ZX Spectum only).
		SEEKLOWER = 16, // The WD179x FDC's current seek direction is towards lower cylinder numbers. Otherwise, it is
						// towards higher ones.
		COMPRESSED = 32 // If a custom TR-DOS ROM is embedded in this block, it has been compressed with the Zlib
						// compression library.
	};

	uint32 flags;		// CONNECTED | CUSTOMROM | COMPRESSED | PAGED | AUTOBOOT | SEEKLOWER
	uint8  num_drives;	// The number of disk drives connected (1-4).
	uint8  sys_reg;		// The last value written to the Beta 128's system register (port $ff).
	uint8  track_reg;	// The current value of the WD179x FDC's track register (port $3f).
	uint8  sector_reg;	// The current value of the WD179x FDC's sector register (port $5f).
	uint8  data_reg;	// The current value of the WD179x FDC's data register (port $7f).
	uint8  status_reg;	// The current value of the WD179x FDC's status register (port $1f).
	uint8  rom_data[1]; // A Zlib compressed or uncompressed custom TR-DOS ROM (if one was installed). The uncompressed
						// ROM size is always 16,384 bytes.
};
// When loading Zlib compressed ROMs, the compressed size can be obtained by: compressedSize = blk.dwSize - ( sizeof(
// ZXSTBETA128 ) - 1 - sizeof( ZXSTBLOCK )); Any ZXSTBETADISK blocks specifying which disk files are in which drives
// will follow this one.


// ZXSTBETADISK
// Each disk drive connected to the Beta 128 disk interface will have one of these blocks, if a disk is inserted. They
// follow the ZXSTBETA128 block which identifies the number of drives. Beta 128 disk image
struct StxBetaDisk : StxBlock
{
	enum {				  // Flags
		EMBEDDED   = 1,	  // The actual disk image is embedded in this block.
		COMPRESSED = 2,	  // If ZXSTBDF_EMBEDDED is also set, the disk image in this block is compressed using the Zlib
						  // compression library.
		WRITEPROTECT = 4, // Specifies whether or not the disk image is write-protected.
						  // Disk image types
		TRD = 0, // TRD	The disk image is in the .trd format.
		SCL = 1, // SCL	The disk image is in the .scl format.
		FDI = 2, // FDI	The disk image is in the .fdi format.
		UDI = 3	 // UDI	The disk image is in the Ultra disk image (.udi) format.
	};

	uint32 flags;	  // EMBEDDED | COMPRESSED | WRITEPROTECT
	uint8  drive_num; // Specifies the drive to insert this disk image into (0-3).
	uint8  cylinder;  // The cylinder the drive heads are currently over (0-86).
	uint8  disk_type; // Specifies the type of disk image. This can be one of: TRD, SCL, FDI or UDI
	union
	{
		char filename[1]; // The file name of the disk image which should be opened and inserted into this drive, if the
						  // disk image is not embedded.
		uint8 disk_image[1]; // If the disk image is embedded, this member is the (possibly compressed) disk image to be
							 // inserted into this drive.
	};
};
// These blocks follow the ZXSTBETA128 block which identifies the number of drives.
// When loading this block, you should use the chDiskType member to determine the disk image format, rather than the
// file extension of szFileName.


// ZXSTCREATOR
// This block identifies the program that created this zx-state file.
struct StxCreator : StxBlock
{
	char   creator[32];	  // The name of the emulator or utility program that created this file
	uint16 major_version; // Creator program's major version number
	uint16 minor_version; // Creator program's minor version number
	uint8  data[1];		  // Variable length data specific to the creator program
};


// ZXSTDOCK
// An expansion cartridge for the Timex TS2068, TC2068 and Unipolbrit UK2068 computers. There will be one entry for each
// 8KB page enabled in the machines EXROM and DOCK memory banks by the expansion cartridge in use. This block is also
// used to store 128KB of the 272KB RAM in the Spectrum SE. Timex Sinclair DOCK memory
struct StxDock : StxBlock
{
	enum {
		COMPRESSED = 1, // The RAM page data[] will be compressed using the Zlib compression library.
		RAM		   = 2, // 1: The page is read-write, 0: the page is read-only.
		EXROMDOCK  = 4	// 1: The page is from the DOCK bank, 0: the page is from the EXROM bank.			NOTE: was 3 on
						// the SZX webpage!
	};

	uint16 flags;	// COMPRESSED | RAM | EXROMDOCK
	uint8  page_no; // Page number of this 8K RAM page in the applicable bank (DOCK or EXROM). This can range from 0-7,
				   // and pages are only written if they are provided by an expansion cartridge (i.e. no page is written
				   // for the Timex ROM 1 in the EXROM bank at page 0). If the machine being emulated is a Spectrum SE,
				   // there will be 8 DOCK pages and 8 EXROM pages of RAM.
	uint8 data[1]; // The actual compressed or uncompressed memory page data. When uncompressed, this member is exactly
				   // 8KB (8,192) bytes in size.
};
// When loading Zlib compressed pages, the compressed size can be obtained by: compressedSize = blk.dwSize - ( sizeof(
// ZXSTDOCK ) - sizeof( ZXSTBLOCK ) - 1 );


// ZXSTDSKFILE
// +3 Disk image
// Each +3 disk drive that has a disk inserted in it will have one of these blocks.
// They follow the ZXSTPLUS3 block which identifies the number of drives.
struct StxPlus3Disk : StxBlock
{
	enum {
		COMPRESSED = 1, // Not implemented. All disk images are currently links to external .dsk or .ipf files
		EMBEDDED   = 2, // Not implemented. All disk images are currently links to external .dsk or .ipf files
		SIDEB	   = 4	// When a double-sided disk is inserted into a single-sided drive, specifies the side being read
						// from/written to. If set, Side B is the active side, otherwise it is Side A.
	};

	uint16 flags;	  // COMPRESSED | EMBEDDED | SIDEB
	uint8  drive_num; // Specifies which drive to insert this disk image into. It will either be 0 (A:) or 1 (B:)
	uint32 uncompressed_size; // The length of the filename at chData
	uint8  data[1]; // The file name of the disk file (.dsk or .ipf) which should be opened and inserted into this drive
};
// These blocks follow the ZXSTPLUS3 block which identifies the number of drives.


// ZXSTGS
// General Sound
// Contains the Z80 registers and other internal state information for the General Sound Interface.
// Memory state is stored separately in ZXSTGSRAMPAGE blocks which will follow this block.
struct StxGS : StxBlock
{
	enum {		   // GS model
		GS128 = 0, // General Sound with 128KB of RAM.)
		GS512 = 1, // General Sound with 512KB of RAM.
				   // Flags
		EILAST = 1, // The last instruction executed was an EI instruction.
		HALTED =
			2, // The last instruction executed was a HALT instruction. The CPU is currently executing NOPs and will
			   // continue to do so until the next interrupt occurs. This flag is mutually exclusive with EILAST.
		CUSTOMROM = 64, // A custom GS ROM is installed. The ROM image is stored in rom_data[]. Note: The default ROM is
						// version 1.04.
		COMPRESSED = 128 // If a custom GS ROM is embedded in this block, it has been compressed with the Zlib
						 // compression library.
	};

	uint8 model;		   // The GS model being emulated. This can be one of: GS128 or GS512
	uint8 upper_page;	   // The current 32KB RAM (or ROM if 0) page at $8000 - $ffff.
	uint8 channel_vol[4];  // The 6-bit volume level of each of the four sound channels. Bits 6 and 7 should be set to
						   // zero.
	uint8  channel_out[4]; // The output level to each of the four sound channels.
	uint8  flags;		   // EILAST | HALTED | CUSTOMROM | COMPRESSED
	uint16 af, bc, de, hl;
	uint16 af1, bc1, de1, hl1;
	uint16 ix, iy, sp, pc;
	uint8  i, r, iff1, iff2; // iff's must be 0 or 1
	uint8  im;				 // 0, 1 or 2
	uint32 cpu_cycle;  // The t-states value at the time the snapshot was made. This counts up from zero to the maximum
					   // number of t-states per 50Hz frame.
	uint8 int_time_cc; // The number of t-states left on restart when an interrupt can occur. This is used to support
					   // interrupt re-triggering properly.)
	uint8 f53bit_reg;  // Internal Z80 register used to generate bits 5 and 3 of the F register after executing a BIT
					   // x,(HL) instruction.
	uint8 rom_data[1]; // A Zlib compressed or uncompressed custom GS ROM (if one was installed). The uncompressed ROM
					   // size is always 32,768 bytes.
};
// When loading Zlib compressed ROMs, the compressed size can be obtained by: compressedSize = blk.dwSize - ( sizeof(
// ZXSTGS ) - 1 - sizeof( ZXSTBLOCK )); Remarks: Memory state is stored separately in ZXSTGSRAMPAGE blocks which will
// follow this block.


// ZXSTGSRAMPAGE
// A zx-state file will contain a number of these blocks, depending on the model of GS being emulated (128KB or 512KB).
// A ZXSTGS block will proceed the first of these blocks. General Sound 32KB RAM page 32KB GS Ram page
struct StxGSRamPage : StxBlock
{
	enum { COMPRESSED = 1 };

	uint16 flags;  // COMPRESSED:	The RAM page data (beginning from chData) will be compressed using the Zlib
				   // compression library.
	uint8 page_no; // The 32KB RAM page number (0-14 for the GS512 or 0-3 for the GS128).
	uint8 data[1]; // The actual compressed or uncompressed memory page data.
};
// When loading Zlib compressed pages, the compressed size can be obtained by: compressedSize = blk.dwSize - ( sizeof(
// ZXSTGSRAMPAGE ) - sizeof( ZXSTBLOCK ) - 1 ); Remarks: A ZXSTGS block will proceed the first of these blocks.


// ZXSTKEYBOARD
// The state of the Spectrum keyboard and any keyboard joystick emulation.
struct StxKeyboard : StxBlock
{
	enum {			// flags:
		ISSUE2 = 1, // Indicates Issue 2 keyboard emulation is enabled.
					// Note: This is only applicable for the 16k or 48k ZX Spectrum. For other models, set this member
					// to 0 (zero). joystick:
		KEMPSTON	 = 0, // Kempston joystick emulation
		FULLER		 = 1, // Fuller joystick emulation
		CURSOR		 = 2, // Cursor (AGF or Protek) emulation
		SINCLAIR1	 = 3, // Sinclair Interface II port 1 (or Spectrum +2A/+3 joystick 1)
		SINCLAIR2	 = 4, // Sinclair Interface II port 2 (or Spectrum +2A/+3 joystick 2)
		SPECTRUMPLUS = 5, // Spectrum+/128/+2/+2A/+3 cursor keys
		TIMEX1		 = 6, // Timex TC2048, TC2068, TS2068 and Spectrum SE built-in joystick, port 1.
		TIMEX2		 = 7, // Timex TC2048, TC2068, TS2068 and Spectrum SE built-in joystick, port 2.
		NONE		 = 8  // None
	};

	uint32 flags;	 // ISSUE2
	uint8  joystick; // Specfies which joystick the PC keyboard should emulate (the actual keys are emulator dependant).
					// This can be one of: KEMPSTON, FULLER, CURSOR, SINCLAIR1, SINCLAIR2, SPECTRUMPLUS, TIMEX1, TIMEX2
					// or NONE
};


// ZXSTIF1
// The current state of the Interface 1.
// This block will appear before any Microdrive cartridge blocks (ZXSTMCART) in the file.
struct StxIf1 : StxBlock
{
	enum {
		ENABLED	   = 1, // Indicates Interface 1 emulation is enabled.
		COMPRESSED = 2, // Specifies the custom Interface 1 ROM image at chRomData has been compressed by the Zlib
						// compression library.
		PAGED = 4		// Indicates that the Interface 1 ROM is currently paged in.
	};

	uint16 flags;			// ENABLED | COMPRESSED | PAGED
	uint8  num_microdrives; // Number of Microdrives attached to the Interface 1 (1-8). Note: this cannot be 0 (zero).
	uint8  reserved[4 * 8 + 3]; // Set to 0.
	uint16
		rom_size; // The uncompressed size of the custom Interface 1 ROM at chData. This is either 8192 or 16384.
				  // It will be 0 if there is no custom ROM installed. In this case the standard v2 ROM should be used.
	uint8 rom_data[1]; // A Zlib compressed or uncompressed custom Interface 1 ROM (if one was installed).
};
// When loading Zlib compressed ROMs, the compressed size can be obtained by: compressedSize = blk.dwSize - ( sizeof(
// ZXSTIF1 ) - 1 - sizeof( ZXSTBLOCK )); Remarks: This block will appear before any Microdrive cartridge blocks
// (ZXSTMCART) in the file.


// ZXSTIF2ROM
// A loaded Interface II ROM cartridge.
struct StxIf2Rom : StxBlock
{
	uint32 romsize; // The size of the compressed ROM cartridge image at chData. The uncompressed size is always 16 KB.
	uint8  data[1]; // The Zlib compressed ROM cartridge image.
};
// This block will not be present in a file if the Spectrum did not have an Interface II ROM cartridge loaded.


// ZXSTJOYSTICK
// Joystick setup for both players.
struct StxJoystick : StxBlock
{
	enum {
		ALWAYSPORT31 = 1, // Joystick option (Deprecated)

		KEMPSTON  = 0, // Kempston joystick emulation
		FULLER	  = 1, // Fuller joystick emulation
		CURSOR	  = 2, // Cursor (AGF or Protek) emulation
		SINCLAIR1 = 3, // Sinclair Interface II port 1 (or Spectrum +2A/+3 joystick 1)
		SINCLAIR2 = 4, // Sinclair Interface II port 2 (or Spectrum +2A/+3 joystick 2)
		COMCOM	  = 5, // Comcom programmable joystick interface
		TIMEX1	  = 6, // Timex TC2048, TC2068, TS2068 and Spectrum SE built-in joystick, port 1.
		TIMEX2	  = 7, // Timex TC2048, TC2068, TS2068 and Spectrum SE built-in joystick, port 2.
		DISABLED  = 8  // Disables joystick emulation for player 1.
	};

	uint32 flags;		// ZXSTJOYF_ALWAYSPORT31:	deprecated as it is an emulator feature rather than hardware state
						// information.
	uint8 type_player1; // Which joystick to emulate for Player 1. This can be one of:
						// KEMPSTON, FULLER, CURSOR, SINCLAIR1, SINCLAIR2, COMCOM, TIMEX1, TIMEX2 or DISABLED
	uint8 type_player2; // Which joystick to emulate for Player 2. This can be one of:
						// KEMPSTON, FULLER, CURSOR, SINCLAIR1, SINCLAIR2, COMCOM, TIMEX1, TIMEX2 or DISABLED
};


// ZXSTMCART
// Microdrive cartridge.
// Each drive that has a cartridge inserted has one of these blocks. These will follow the Interface 1 (ZXSTIF1) block.
struct StxMdrCart : StxBlock
{
	enum {
		COMPRESSED = 1, // Not implemented. All Microdrive cartridges are currently links to external .mdr files
		EMBEDDED   = 2	// Not implemented. All Microdrive cartridges are currently links to external .mdr files
	};

	uint16 flags;		  // COMPRESSED | EMBEDDED
	uint8  drive_num;	  // The drive number this cartridge should be inserted into (1-8)
	uint8  drive_running; // Indicates whether or not the Microdrive motor is running. This will either be 0 or 1 (on)
	uint16 drive_pos;	  // Position of the virtual Microdrive head within the file
	uint16 preamble;	  // Number of preamble bytes left to skip
	uint32 uncompressed_size; // The length of the filename at data[]
	uint8  data[1]; // The file name of the Microdrive cartridge file (.mdr) which should be opened and inserted into
					// this drive
};


// ZXSTMOUSE
// Current mouse emulation state.
struct StxMouse : StxBlock
{
	enum {
		NONE	 = 0, // Mouse emulation is disabled
		AMX		 = 1, // Emulate the AMX mouse
		KEMPSTON = 2  // Emulate the Kempston mouse
	};

	uint8 type;			// The type of mouse being emulated. This can be one of:
						// NONE, AMX or KEMPSTON
	uint8 control_A[3]; // Z80 PIO CTRLA registers for AMX mouse
	uint8 ctonrol_B[3]; // Z80 PIO CTRLB registers for AMX mouse
};


// ZXSTMULTIFACE
// The state of the Multiface 1, Multiface 128 or Multiface 3.
struct StxMultiface : StxBlock
{
	enum {		   // model_48k:
		MF1	  = 0, // Multiface 1
		MF128 = 1, // Multiface 128
				   // flags:
		PAGEDIN	   = 0x01, // Specifies whether the Multiface ROM and RAM are currently paged in.
		COMPRESSED = 0x02, // Specifies whether the Multiface RAM data (beginning at chData) is Zlib compressed or not.
		SOFTWARELOCKOUT =
			0x04, // Indicates that the software lockout feature of Multiface 128s and Multiface 3s has been enabled.
				  // This flag is not valid and should be set to 0 (zero) when emulating the Multiface 1.
		REDBUTTONDISABLED = 0x08, // Indicates that the red (magic) button is disabled.
		DISABLED =
			0x10, // Indicates that the user has disabled Multiface 1 emulation with the physical disable switch. This
				  // flag is not valid and should be set to 0 (zero) when emulating the Multiface 128 or Multiface 3.
		RAM16KMODE = 0x20 // Specifies Multiface hardware which has had the 8KB of ROM and 8KB of RAM replaced with 16KB
						  // of RAM. This is required by advanced Multiface programs such as SoftCrack. If RAM16KMODE is
						  // set, the RAM image stored at data[] will expand to 16,384 bytes.
	};

	uint8 model_48k; // The Multiface model to use when emulating 16k or 48k Spectrums. This can be one of:
					 // MF1 | MF128
	uint8 flags;	 // Various flags specifying the current state of the Multiface. This can be a combination of:
					 // PAGEDIN | COMPRESSED | SOFTWARELOCKOUT | REDBUTTONDISABLED | DISABLED | RAM16KMODE
	uint8 data[1];	 // The Zlib compressed or uncompressed Multiface RAM image, depending on whether COMPRESSED is set.
};
// When loading the Zlib compressed RAM image, the compressed size can be obtained by: compressedSize = blk.dwSize - (
// sizeof( ZXSTMULTIFACE ) - 1 - sizeof( ZXSTBLOCK )); Remarks: The last bytes written to ports $1ffd and $7ffd (which
// are used by the Multiface) can be read from the ZXSTBID_SPECREGS block which appears earlier in the file.


// ZXSTOPUS
// Discovery disk interface by Opus Supplies.
// Any blocks specifying which disk files are in which drive will follow this one.
struct StxOpus : StxBlock
{
	enum {
		PAGED = 1, // The Opus Discovery's ROM and RAM are currently paged in.
		COMPRESSED =
			2,		   // Specifies the RAM (and any custom ROM) have been compressed with the Zlib compression library.
		SEEKLOWER = 4, // If set, the WD1770 FDC's current seek direction is towards lower cylinder numbers. Otherwise,
					   // it is towards higher ones.
		CUSTOMROM = 8  // A custom ROM is installed. The Zlib compressed or uncompressed ROM image begins after the RAM
					   // image at chRam. dwcbRom contains the size of the compressed (or uncompressed) data.
	};

	uint32 flags;				// PAGED | COMPRESSED | SEEKLOWER | CUSTOMROM
	uint32 compressed_ram_size; // Size in bytes of the Zlib compressed or uncompressed Opus Discovery RAM. The
								// uncompressed RAM size is always 2,048 bytes.
	uint32 compressed_rom_size; // Size in bytes of the Zlib compressed or uncompressed custom Opus Discovery ROM (if
								// one was installed). The uncompressed ROM size is always 8,192 bytes.
	uint8 control_reg_A;		// The last value written to the 6821 PIA's control register A.
	uint8 peripheral_reg_A;		// The last value written to the 6821 PIA's peripheral register A.
	uint8 data_dir_reg_A;		// The last value written to the 6821 PIA's data direction register A.
	uint8 control_reg_B;		// The last value written to the 6821 PIA's control register B.
	uint8 peripheral_reg_B;		// The last value written to the 6821 PIA's peripheral register B.
	uint8 data_dir_reg_B;		// The last value written to the 6821 PIA's data direction register B.
	uint8 num_drives;			// The number of disk drives connected (1 or 2).
	uint8 track_reg;			// The current value of the WD1770 FDC's track register (address $3001).
	uint8 sector_reg;			// The current value of the WD1770 FDC's sector register (address $3002).
	uint8 data_reg;				// The current value of the WD1770 FDC's data register (address $3003).
	uint8 status_reg;			// The current value of the WD1770 FDC's status register (address $3000).
	uint8 ram[1]; // A Zlib compressed or uncompressed image of the Opus Discovery's RAM. The uncompressed RAM size is
				  // always 2048 bytes. If a custom ROM is installed. The image follows immediately after the RAM data.
};
// Remarks: Any ZXSTOPUSDISK blocks specifying which disk files are in which drives will follow this one.


// ZXSTOPUSDISK
// Associated Opus Discovery disk images
// Each disk drive connected to the Opus Discovery disk interface will have one of these blocks, if a disk is inserted.
// They follow the ZXSTOPUS block which identifies the number of drives.
struct StxOpusDisk : StxBlock
{
	enum {				  // flags:
		EMBEDDED   = 1,	  // // The actual disk image is embedded in this block.
		COMPRESSED = 2,	  // If ZXSTOPDF_EMBEDDED is also set, the disk image in this block is compressed using the Zlib
						  // compression library.
		WRITEPROTECT = 4, // Specifies whether or not the disk image is write-protected.
						  // disk_type:
		OPD		= 0, // The disk image is in the .opd format.
		OPU		= 1, // The disk image is in the .opu format.
		FLOPPY0 = 2, // Real disk mode. drive 'drive_num' should use the first real 3½" floppy disk drive in a system.
					 // In Windows, this is the A: drive. (filename and disk_image void)
		FLOPPY1 = 3	 // Real disk mode. drive 'drive_num' should use the second real 3½" floppy disk drive in a system.
					 // In Windows, this is the B: drive. (filename and disk_image void)
	};

	uint32 flags;	  // EMBEDDED | COMPRESSED | WRITEPROTECT
	uint8  drive_num; // Specifies the drive to insert this disk image into (0-1).
	uint8  cylinder;  // The cylinder the drive heads are currently over (0-86).
	uint8  disk_type; // Specifies the type of disk image. This can be one of: OPD, OPU, FLOPPY0 or FLOPPY1
	union
	{
		char filename[1]; // The file name of the disk image which should be opened and inserted into this drive, if the
						  // disk image is not embedded.
		uint8 disk_image[1]; // If the disk image is embedded, this member is the (possibly compressed) disk image to be
							 // inserted into this drive.
	};
};
// Remarks: These blocks follow the ZXOPUS block which identifies the number of drives.
// When loading this block, you should use the chDiskType member to determine the disk image format, rather than the
// file extension of szFileName. This is especially important since these members are not valid when chDiskType is
// ZXSTOPDT_FLOPPYx.


// ZXSTPLUS3
// +3 disk drives
// The number of drives connected to the Spectrum +3 and whether their motors are turned on. Any blocks specifying which
// disk files are in which drive will follow this one.
struct StxPlus3 : StxBlock
{
	uint8 num_drives; // The number of drives connected. Either 1 or 2.
	uint8 motor_on;	  // Specifies whether or not the drive motors are running. Will be either 0 or 1 (on).
};
// Remarks: Any ZXSTDSKFILE blocks specifying which disk files are in which drive will follow this one.


// ZXSTPLUSD
// Plus D disk interface by Miles Gordon Technology Ltd - later sold by Datel Electronics.
// Any blocks specifying which disk files are in which drive will follow this one.
struct StxPlusD : StxBlock
{
	enum {
		// flags
		PAGED = 1, // The Plus D's ROM and RAM are currently paged in.
		COMPRESSED =
			2,		   // Specifies the RAM (and any custom ROM) have been compressed with the Zlib compression library.
		SEEKLOWER = 4, // If set, the WD1772 FDC's current seek direction is towards lower cylinder numbers. Otherwise,
					   // it is towards higher ones. rom_type:
		GDOS   = 0, // The standard G+DOS ROM (Version 1.A).
		UNIDOS = 1, // Uni-DOS ROM.
		CUSTOM = 2	// A custom ROM is installed. The Zlib compressed or uncompressed ROM image is stored starting at
					// ram[compressed_ram_size].
	};

	uint32 flags;				// PAGED | COMPRESSED | SEEKLOWER
	uint32 compressed_ram_size; // Size in bytes of the Zlib compressed or uncompressed custom Plus D RAM. The
								// uncompressed RAM size is always 8,192 bytes.
	uint32 compressed_rom_size; // Size in bytes of the Zlib compressed or uncompressed custom Plus D ROM (if one was
								// installed). The uncompressed ROM size is always 8,192 bytes.
	uint8 rom_type;	   // The type of ROM installed in the Plus D. This can be one of: GDOS, UNIDOS or ZXSTPDRT_CUSTOM
	uint8 control_reg; // The last value written to the Plus D's control register, port $ef.
	uint8 num_drives;  // The number of disk drives connected (1 or 2).
	uint8 track_reg;   // The current value of the WD1772 FDC's track register (port $eb).
	uint8 sector_reg;  // The current value of the WD1772 FDC's sector register (port $f3).
	uint8 data_reg;	   // The current value of the WD1772 FDC's data register (port $fb).
	uint8 status_reg;  // The current value of the WD1772 FDC's status register (port $e3).
	uint8 ram[1]; // A Zlib compressed or uncompressed image of the Plus D's RAM. The uncompressed RAM size is always
				  // 8192 bytes. If a custom ROM is installed. The image follows immediately after the RAM data.
};
// Remarks: Any ZXSTPLUSDDISK blocks specifying which disk files are in which drives will follow this one.


// ZXSTPLUSDDISK
// Associated Plus D disk images
// Each disk drive connected to the Plus D disk interface will have one of these blocks, if a disk is inserted. They
// follow the ZXSTPLUSD block which identifies the number of drives.
struct StxPlusDDisk : StxBlock
{
	enum {
		// flags:
		EMBEDDED   = 1,	  // The actual disk image is embedded in this block.
		COMPRESSED = 2,	  // If EMBEDDED is also set, the disk image in this block is compressed using the Zlib
						  // compression library.
		WRITEPROTECT = 4, // Specifies whether or not the disk image is write-protected.
						  // disk_type:
		MGT		= 0, // The disk image is in the .mgt format.
		IMG		= 1, // The disk image is in the .img format.
		FLOPPY0 = 2, // Real disk mode. Drive 'drive_num' should use the first real 3½" floppy disk drive in a system,
					 // e.g. Drive A. 'filename' and 'disk_image' are unused.
		FLOPPY1 = 3	 // Real disk mode. Drive 'drive_num' should use the second real 3½" floppy disk drive in a system,
					 // e.g. Drive B. 'filename' and 'disk_image' are unused.
	};

	uint32 flags;	  // EMBEDDED | COMPRESSED | WRITEPROTECT
	uint8  drive_num; // Specifies the drive to insert this disk image into (0-1).
	uint8  cylinder;  // The cylinder the drive heads are currently over (0-86).
	uint8  disk_type; // Specifies the type of disk image. This can be one of: MGT, IMG, FLOPPY0 or FLOPPY1
	union
	{
		char filename[1]; // The file name of the disk image which should be opened and inserted into this drive, if the
						  // disk image is not embedded.
		uint8 disk_image[1]; // If the disk image is embedded, this member is the (possibly compressed) disk image to be
							 // inserted into this drive.
	};
};
// Remarks: These blocks follow the ZXSTPLUSD block which identifies the number of drives.
// When loading this block, you should use the 'disk_type' member to determine the disk image format, rather than the
// file extension of 'filename'. This is especially important since these members are not valid when chDiskType is
// ZXSTPDDT_FLOPPYx.


// ZXSTRAMPAGE
// Standard 16kb Spectrum RAM page
// zx-state files will contain a number of 16KB RAM page blocks, depending on the specific Spectrum model.
struct StxRamPage : StxBlock
{
	enum { ZXSTRF_COMPRESSED = 1 }; // The RAM page data[] will be compressed using the Zlib compression library.

	uint16 flags;	// ZXSTRF_COMPRESSED
	uint8  page_no; // Memory page number (usually 0-7, but see below as the Pentagon 512/1024 and ZS Scorpion machines
				   // have additional memory). For 16k Spectrums, only page 5 (0x4000 - 0x7fff) is saved. For 48k
				   // Spectrums and Timex TS/TC models, pages 5, 2 (0x8000 - 0xbfff) and 0 (0xc000 - 0xffff) are saved.
				   // For 128k Spectrums and the Pentagon 128, all pages (0-7) are saved.
				   // For the Pentagon 512, all 32 pages (0-31) are saved.
				   // For the Pentagon 1024, all 64 pages (0-63) are saved.
				   // For the ZS Scorpion 256, all 16 pages (0-15) are saved.
	uint8 data[1]; // The actual compressed or uncompressed memory page data.
};
// When loading Zlib compressed pages, the compressed size can be obtained by: compressedSize = blk.dwSize - ( sizeof(
// ZXSTRAMPAGE ) - sizeof( ZXSTBLOCK ) - 1 ); Remarks: The RAM page blocks are not guaranteed to be in any specific
// order.


// ZXSTROM
// Custom ROM for the current model
// A custom ROM has been installed for the current Spectrum model.
struct StxRom : StxBlock
{
	enum { COMPRESSED = 1 }; // The ROM image at data[] has been compressed with the Zlib compression library.

	uint16 flags;			  // COMPRESSED
	uint32 uncompressed_size; // The size in bytes of the custom ROM. This will be one of:
							  // 16,384	16k/48k Spectrum
							  // 32,768	Spectrum 128/+2
							  // 65,536	Spectrum +2A/+3
							  // 32,768	Pentagon 128
							  // 65,536	ZS Scorpion
							  // 16,384	Timex Sinclair TS/TC2048
							  // 24,576	Timex Sinclair TS2068
							  // 32,768	Spectrum SE
							  // NOTE: For models that have more than one ROM, the ROMs will be concatenated together in
							  // ascending order.
	uint8 data[1]; // Either a Zlib compressed or uncompressed ROM image depending on whether the COMPRESSED bit of
				   // 'flags' is set.
};
// Remarks: When a custom ROM is not installed, the standard UK ROMs should be used for all official ZX Spectrum models.
// For the Spectrum +2A/+3, this is the v4.0 ROM. For unofficial models (clones), the appropriate standard ROM should be
// used if a custom ROM is not installed.


// ZXSTSCLDREGS
// The current screen mode and memory paging status of Timex Sinclair machines.
// Timex Sinclair memory paging and screen modes
struct StxTimex : StxBlock
{
	uint8 port_f4; // port $f4 byte which controls memory paging.
	uint8 port_ff; // port $ff byte which controls the screen mode, interrupt state, high resolution colours, and the
				   // remainder of the memory paging system.
};


// ZXSTSIDE
// Simple 8-bit IDE
// The Simple 8-bit IDE interface most commonly used as a component of the original +3e by Garry Lancaster.
struct StxSIDE : StxBlock
{};


// ZXSTSPECDRUM
// SpecDrum state
// The state of the Cheetah SpecDrum.
struct StxSpecDrum : StxBlock
{
	int8 current_volume; // The current amplitude level of the SpecDrum's DAC. This is between +127 and -128.
};
// Remarks: If this block is not present in a zx-state file, SpecDrum emulation should be disabled.


// ZXSTTAPE
// Cassette Recorder state
// The state of the virtual cassette recorder and its contents.
struct StxTape : StxBlock
{
	enum {
		EMBEDDED   = 1, // There is a tape file embedded in this block. The actual data begins at chData.
		COMPRESSED = 2	// The embedded tape file in this block has been compressed by the Zlib compression library
	};

	uint16 current_block_no; // The current block number (ie. the position of the virtual tape head) starting from 0.
							 // For .wav and .voc files, Spectaculator splits these up into 5 second chunks.
							 // 'current_block_no' will refer to one of these chunks.
	uint16 flags; // Various flags indicating whether a tape file is embedded in this block or linked to on disk. This
				  // can be a combination of: EMBEDDED | COMPRESSED
	uint32 uncompressed_size;  // For Zlib compressed tape files, this is the size in bytes they uncompress to. This
							   // value is undefined if the tape file is not embedded.
	uint32 compressed_size;	   // The size of the data at data[].
	char   file_extension[16]; // File extension (case insensitive) of an embedded tape file. This value is undefined if
							 // the tape file is not embedded. Special case: Warajevo .tap files are represented by
							 // "tapw".
	uint8
		data[1]; // The data stored here is one of:
				 // Type					Description
				 // File name			chData is the file name of a linked tape file.
				 //						dwCompressedSize is the length of the file name.
				 // Embedded tape file	If the EMBEDDED bit in 'flags' is set, data[] is an embedded tape file.
				 //						If the COMPRESSED bit in 'flags' is also set, the embedded file will be compressed using the
				 //Zlib compression libarary. 						compressed_size contains the size of the compressed data at data[].
				 //						uncompressed_size contains the size of the expanded data.
				 //						These two values will be the same if the embedded file isn't compressed.
};
// Remarks: If there is no tape file in the cassette recorder, this block is not written to the file.


// ZXSTUSPEECH
// uSpeech state
// The state of the Currah µSpeech.
struct StxUSpeech : StxBlock
{
	uint8 paged_in; // Indicates the the µSpeech ROM is currently paged in. This can be 1 (paged in) or 0.
};
// Remarks: If this block is not present in a zx-state file, µSpeech emulation should be disabled.


// ZXSTZXPRINTER
// Status of the ZX Printer.
struct StxZxPrinter : StxBlock
{
	enum { ENABLED = 1 }; // 1: ZX Printer emulation is enabled. Otherwise it should be disabled.

	uint16 flags; // ENABLED
};


// ZXSTSPECTRA
// SPECTRA Video Interface
// ID: 'S', 'P', 'R', 'A'
struct StxSpectra : StxBlock
{
	enum {								  // flags:
		new_colour_modes_enabled = 1,	  // dip switch: the new colour modes are enabled
		rs232_enabled			 = 2,	  // dip switch: the rs232 port is enabled
		joystick_enabled		 = 4,	  // dip switch: the Kempston joystick port is enabled
		if1_rom_hooks_enabled	 = 8,	  // dip switch: Interface1 Rom paging hooks are enabled
		rom_paged_in			 = 16,	  // the rom is currently paged in (if a rom is loaded)
		port_239_comms_out_bit	 = 32,	  // last value written to the COMMS output
		port_239_cts_out_bit	 = 64,	  // last value written to the CTS output
		port_247_data_out_bit	 = 128,	  // last value written to the TxD output
		ram_page_0_embedded		 = 0x100, // if not included, use data from contended ram
		ram_page_1_embedded		 = 0x200, // if not included, use data from contended ram
		compressed				 = 0x400  // data of ram page 0 and 1 is zlib compressed (individually)
	};

	uint16 flags;
	uint8  port_7fdf;			 // last out to port 0x7FDF (colour mode register)
	uint8  port_fe;				 // last out to port 0xFE in extended colours border mode
	uint16 compressed_size_ram0; // 0 if not included
	uint16 compressed_size_ram1; // 0 if not included
	uint8  data[1];				 // data ram[0] and/or ram[1] depending on bits set in 'flags'
};
// if the joystick is enabled, then a block ZXSTKEYBOARD or ZXSTJOYSTICK for KEMPSTON emulation refer to this joystick
// port. if the joystick is enabled and no such block is present, then the joystick port uses any joystick it finds. if
// a block ZXSTIF2ROM is present, then this rom is plugged into the SPECTRA interface
