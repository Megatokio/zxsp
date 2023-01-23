// Copyright (c) 2014 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IdeDevice.h"
#include <fcntl.h>
#include "globals.h"
#include "unix/files.h"
#include "unix/FD.h"
#include <pthread.h>
#include "kio/peekpoke.h"


/*
	This file tries to implement a Generic IDE Device acc. to ATA-5		class IdeDevice
	This file tries to implement a Compact Flash Card acc. to ATA-5		class IdeCFCard
	this file tries to implement a IDE/ATA Hard Disk  acc. to ATA-5		class IdeHardDisk
	this file tries to implement a IDE/ATAPI CD-Rom   acc. to ATA-5		class IdeCDRom			TODO


note:	Quantum Fireball TM	3.8 GB
		6 Heads
		6810 Cylinders
		122 .. 232 Sectors/Cylinder
*/



/*
IDE registers
IDE has 8 registers with CS0=0, CS1=1  (selectable with address lines A0…2)
	and 2 registers with CS0=1, CS1=0

CS0=0
->	0 = DATA REGISTER (R/W)
		16 bit wide		disk files are almost always read and stored with little endian byte order:
						high byte of data word goes into the lower memory address

	1 = ERROR REGISTER (R) / FEATURES REGISTER (W)
		This is the error information register when read;
		the feature register when written.
		(MFM: the write precompensation register when written.)

		bit 0	AMNF:  Address mark not found.	(probably very old HDDs only)
		bit 1	TK0NF: Track 0 not found.		(probably very old HDDs only)
		bit 2	ABRT:  This bit is set when you have given an indecent command to the disk.
				Mostly wrong parameters (wrong head number etc..) cause the disk to respond with error flag
				in the status bit set and the ABRT bit set.
		bit 3	MCR:  Media Change Requested. probably the 'eject' button of removable drives.
		bit 4	IDNF: Sector ID not found.		(probably very old HDDs only)
		bit 5	MC:   Removable media has been changed.
		bit 6	UNC:  Uncorrectable data error.
		bit 7	BAD:  bad block detected

	2 = SECTOR COUNT (R/W)
		number of sectors to transfer in one read/write command.
		0 == 256

	3 = SECTOR NUMBER		or LBA bits 0..7 (R/W)
		Start sector for reading/writing.
		The sector number starts at 1 and runs up to 255

	4 = CYLINDER LOW		or LBA bits 8..15 (R/W)
	5 = CYLINDER HIGH		or LBA bits 16..23 (R/W)
		cylinders (not LBA) were limited to 1023 (10 bit total) at some point in time
		for default CHS translation they are limited to 16383
		host may reprogram drive for up to 65535 cylinders.

	6 = DRIVE/HEAD			or LBA bits 24..28 (R/W)
		Head and device select register:
		Bits 3..0:	head number (0..15).
		Bit 4:		0 = IDE master device, 1 = IDE slave device.
		Bits 7..5:	101b = CSH addressing
					111b = LBA addressing
					In old (MFM) controllers you could specify if you wanted 128,256,512 or 1024 bytes/sector.
					In the IDE world only 512 bytes/sector is supported.
					Today bits 5 and 7 are ignored, only bit 6 is used to select between CHS and LBA.

	7 = STATUS REGISTER (R) / COMMAND REGISTER (W)
		Write: command. Read: get the status of the IDE device.
		Reading his register also clears any interrupts from the IDE device.

		bit 0	ERROR: If this bit is set then an error has occurred while executing the latest command.
				The error status itself is to be found in the error register.
		bit 1	INDEX PULSE: Each revolution of the disk this bit is pulsed to '1' once. (probably MFM controllers only)
		bit 2	ECC: if this bit is set then an ECC correction on the data was executed.
		bit 3	DRQ: If this bit is set then the disk either wants data (disk write) or has data for you (disk read).
		bit 4	SKC: Indicates that a seek has been executed with success. (probably MFM controllers only)
		bit 5	WFT: indicates a write error has happened. ("I've never seen it go active.")
		bit 6	RDY: indicates that the disk has finished its power-up.
				Wait for this bit to be active before doing anything (execpt reset) with the disk.
		bit 7	BSY: This bit is set when the disk is executing a command.
				Wait for this bit to clear before you can start giving orders to the disk.

CS1=0
->	4 =	2nd status register/interrupt and reset register.
		Both the primary and secondary status register use the same bit coding.
		When read this register gives you the same status byte as the primary (/CS0=0, /CS1=1, A2..A0=111B) status register.
		The only difference is that reading this register does not clear the interrupt from the IDE device when read.
		When written you can enable/disable the interrupts the IDE device generates;
		Also you can give a software reset to the IDE device.

		bit 1	IRQ enable. If this bit is '0' the disk will give and IRQ when it has finished executing a command.
				When it is '1' the disk will not generate interrupts.
		bit 2	RESET bit. If you pulse this bit to '1' the disk will execute a software reset. The bit is normally '0'.
				I do not use it because I have full software control of the hardware /RESET line.

	6 = Device Control register		ATA5 pg.58

	7 = active status of the IDE device.
		In this register (read-only) you can find out if the IDE master or slave is currently active and find the currently selected head number.
		In a PC environment you can also find out if the floppy is currently in the drive. I have not used this register yet.
		bit 0    : master active. If this bit is set then the master IDE device is active.
		bit 1    : slave active. If this bit is set then the slave IDE device is active.
		bits 5..2: complement of the currently active disk head.
		bit 6    : write bit. This bit is set when the device is writing.
		bit 7    : in a PC environment this bit indicates if a floppy is present in the floppy drive. Here it has no meaning.
*/


#define MAX_SECTORS_PER_MULTIPLE	64		// must be 2^N
#define	ATA_VERSION					5		// must be 5.	handles up to 128 GiB for CF => always suitable



// ---------------------------------------------------------
//		creator & similar
// ---------------------------------------------------------


IdeDevice::IdeDevice(cstr fpath, DeviceType dtyp, bool is_master)
:
	devicetype(dtyp),
	is_master(is_master),
	is_cfa(dtyp==CFCard),
	is_packet(dtyp==CDRom),
	is_general(!is_packet),
	can_write(dtyp!=CDRom),
	max_sectors_per_multiple(MAX_SECTORS_PER_MULTIPLE),
	status_register(0),
	filepath(nullptr),
	io_mode(io_none),
	hd_mode(hd_invalid)			// damit wir nicht vorzeitig in hd_mode schreiben
{
	// Timing constants
	reset_delay =  1e-3;		// not-RDY after reset
	read_delay  = 10e-6;		// BSY after read command issued: ~ time to read a flash page
	write_delay1=  0e-6;		// BSY after write command issued: ~ time to start command
	write_delay2=  1e-3;		// BSY after last byte written to buffer[]:  ~ time to flash the page
	write_delay3= 10e-3;		// BSY after last byte written to buffer[] and uncorrectable write error happened

	// der Disk-I/O wird auf einen Worker-Thread verlagert.
	// Grund: im Audio-Thread sollten wir keinen Disk-I/O machen, weil der könnte dauern
	// start worker & test worker running

	int e = pthread_create( &worker_thread, nullptr/*attr*/, worker_proc, this/*args*/ );
	if(e) { showAlert("Creating the HDD service failed:\n%s",errorstr()); return; }

	open_diskfile(fpath);
	reset(0);
}


IdeDevice::~IdeDevice()
{
	hd_wait_busy();
	hd_mode = hd_exit;
	hd_semaphore.release();
	hd_wait_busy();
	delete[] filepath;
}


// ---------------------------------------------------------
//		worker thread
// ---------------------------------------------------------


// helper:
void IdeDevice::reset_hd_data()
{
	delete[] filepath; filepath = nullptr;
	total_sectors = 0;
	disk_writable = no;
	file_writable = no;
	supports_CHS  = no;
	dflt_num_heads     = num_heads     = 0;
	dflt_num_cylinders = num_cylinders = 0;
	dflt_num_sectors   = num_sectors   = 0;
}

// static
void* IdeDevice::worker_proc(void* self)
{
	return ((IdeDevice*)self)->worker_proc();
}


/*	worker thread executor:
	- waits for hd_semaphore
	- executes command hd_mode
	- sets hd_error and clears hd_mode
*/
void* IdeDevice::worker_proc()
{
	FD diskfile;
	off_t base = 0;				// file offset for LBA=0
	reset_hd_data();

	for(;;)
	{
		hd_mode = hd_idle;		// clear pending command
		hd_semaphore.request();	// wait for next command
		hd_error = noerror;		// preset error

		try
		{
			switch(hd_mode)
			{
			default:			// note: könnte nach CD-Rom command DEVICE_RESET passieren.
			{
				IERR();
			}

			case hd_exit:		// terminate worker thread
			{
				hd_mode = hd_idle;
				return nullptr;
			}

			case hd_open:		// open diskfile
			{					// caller must block until command finishs!

				reset_hd_data();
				diskfile.close_file(0);				// just in case
				base = 0;

				xlogline("IdeDevice: open disk image %s", hd_path);

				try
				{
					diskfile.open_file(hd_path,can_write?O_RDWR:O_RDONLY);
					file_writable = can_write;		// file is open R/W
					disk_writable = can_write;		// true if also can_write set (not for CD-Rom)
				}
				catch(FileError&)
				{
					diskfile.open_file(hd_path,O_RDONLY);
					file_writable = no;
					disk_writable = no;
				}

				if(eq(lowerstr(extension_from_path(hd_path)),".hdf"))
				{
					uint8 bu[0x100]; diskfile.read_bytes(bu,0x100);
					if(memcmp(bu,"RS-IDE\x1A",7)) throw FileError(diskfile,wrongmagic);
					xlogline("  file version:  %02X",int(bu[7]));
					if(bu[8]&1) throw FileError(diskfile,filecontainslowbytesonly); // does not work with DivIDE anyway
					if(bu[8]&2) logline("  ATAPI");		// ATAPI ignored. should work as CF card or HDD as well. TODO
					base = peek2Z(bu+9);
					uint n;
					n = peek2Z(bu+0x16 + 2*1); if(n<16384) dflt_num_cylinders = num_cylinders = n;	// if any of CHS is invalid
					n = peek2Z(bu+0x16 + 2*3); if(n<16)    dflt_num_heads	  = num_heads	  = n;	// then C*H*S=0
					n = peek2Z(bu+0x16 + 2*6); if(n<64)	   dflt_num_sectors	  = num_sectors   = n;	// => C*H*S!=total_sectors
				}

				off_t fsize = diskfile.file_size() - base;
				if(fsize > (off_t)1<<(28+9))
				{
					diskfile.close_file(0);
					hd_error = disksizeexceeds128GB;
					continue;
				}

				if(fsize<720*1024)
				{
					diskfile.close_file(0);
					hd_error = disksizelessthan720K;
					continue;
				}

			// ok: mount disk:
				total_sectors = uint32(fsize/512);
				filepath = newcopy(hd_path);

			// calculate disk geometry for CHS:
			// maximum is 63*16*16383 = 16514064 sectors which is ~ 8.45 GB
				supports_CHS = total_sectors <= 16 * 63 * 16383;
				if(supports_CHS)
				{
					if(uint32(num_cylinders) * num_heads * num_sectors != total_sectors)
					{
						uint best_s = 0;
						uint best_c = 0;
						uint best_h = 0;

						for(uint h=1;h<=16;h++)
						for(uint s=9;s<=63;s++)
						{
							uint32 c = total_sectors/h/s;
							if(c>16383) c=16383;
							if(h*c*s>best_s) { best_s=h*c*s; best_h=h; best_c=c; }
						}

						dflt_num_heads     = num_heads		= best_h;
						dflt_num_cylinders = num_cylinders	= best_c;
						dflt_num_sectors   = num_sectors	= best_s/best_c/best_h;
					}
				}
				else
				{
					dflt_num_heads     = num_cylinders	= 16383;		// ATA5 pg.20
					dflt_num_cylinders = num_heads		= 16;
					dflt_num_sectors   = num_sectors	= 63;
				}

				xlogline("  file size:     %llu",	ullong(base+fsize));
				xlogline("  sector base:   %u",		uint(base));
				xlogline("  is writable:   %s",		disk_writable?"yes":"no");	// here: disk_writable==file_writable
				xlogline("  supports CHS:  %s",		supports_CHS?"yes":"no");
				xlogline("  total sectors: %lu",	ulong(total_sectors));
				xlogline("  heads:         %u",		num_heads);
				xlogline("  cylinders:     %u",		num_cylinders);
				xlogline("  sectors:       %u",		num_sectors);
				continue;
			}

			case hd_close:	// close diskfile
			{
				diskfile.close_file();
				reset_hd_data();
				continue;
			}

			case hd_read:	// read sector
			{				// TODO: delay
				assert(hd_sector<total_sectors);
				diskfile.seek_fpos(base + (off_t)hd_sector*512);
				diskfile.read_bytes(buffer,512);
				continue;
			}

			case hd_write:	// write sector
			{				// TODO: delay
				assert(hd_sector<total_sectors);
				assert(disk_writable);
				diskfile.seek_fpos(base + (off_t)hd_sector*512);
				diskfile.write_bytes(buffer,512);
				continue;
			}
			}
		}
		catch(AnyError& e)
		{
			hd_error = e.error();
			continue;
		}
	}
	return nullptr;
}

// open disk file (blocking)
void IdeDevice::open_diskfile(cstr fpath)
{
	hd_wait_busy();
	hd_path = fpath;
	hd_mode = hd_open;
	hd_semaphore.release();
	hd_wait_busy();
	if(hd_error!=noerror)
	{
		showAlert("Open disc file failed:\n%s",errorstr(hd_error));
	}
	else
	{
		xlogline("IdeDevice: loaded \"%s\"",filepath);
		if(devicetype==CDRom && total_sectors > 800 MB / 512)
						  showInfo("The disc file seems to be too large for a CD-Rom.");
	}
}

// close disk file (blocking)
void IdeDevice::close_diskfile()
{
	hd_wait_busy();
	hd_mode = hd_close;
	hd_semaphore.release();
	hd_wait_busy();
	if(hd_error!=noerror) showAlert("Close disc file failed:\n%s",errorstr(hd_error));
}

// write buffer to disk (non-blocking)
void IdeDevice::hd_write_sector(uint32 sector)
{
	assert(hd_mode==hd_idle);
	hd_sector = sector;
	hd_mode = hd_write;
	hd_semaphore.release();
}

// read buffer from disk (non-blocking)
void IdeDevice::hd_read_sector(uint32 sector)
{
	assert(hd_mode==hd_idle);
	hd_sector = sector;
	hd_mode = hd_read;
	hd_semaphore.release();
}


// ---------------------------------------------------------
//		helpers
// ---------------------------------------------------------


// After power-on reset, hardware reset, cmd DEVICE RESET, and cmd EXECUTE DEVICE DIAGNOSTIC command
// a signature is placed in the registers:
// see ATA5 pg.280
//
void IdeDevice::place_signature_in_registers()
{
	sector_count  = 0x01;
	sector_number = 0x01;
	drive_head	  = 0;
	if(is_packet) { cylinder_low  = 0x14; cylinder_high = 0xeb; }
	else		  { cylinder_low  = 0x00; cylinder_high = 0x00;	}
}

// helper: increment sector number and decrement sector_count
// increments either LBA or sector_number only
//
void IdeDevice::increment_sector_count_and_number()
{
	--sector_count;

	if(drive_head & drvhd_LBA_mask)
	{
		if((++lba_address&0x0fffffff)==0) lba_address -= 0x10000000;	// really security only...
	}
	else
		++sector_number;
}

// helper: test whether sector id is valid
// tests either the LBA or CSH
//
bool IdeDevice::sector_id_is_valid()
{
	if(drive_head&drvhd_LBA_mask)
	{
		return (lba_address&0x0fffffff) < total_sectors;
	}
	else
	{
		return	sector_number && sector_number <= num_sectors &&		// note: sector ids start at 1
				(cylinder_low+256*cylinder_high) < num_cylinders &&
				(drive_head&0x0f) < num_heads;
	}
}


// ATA5 pg.20: LBA = (((cylinder_number * heads_per_cylinder) + head_number) * sectors_per_track) + sector_number - 1
#define calc_LBA_from_CHS(C,H,S)	((uint32(((C)*num_heads)+(H))*num_sectors)+(S)-1)
#define calc_CHS_from_LBA(A,C,H,S)	S=(A)%num_sectors+1, H=((A)/num_sectors)%num_heads, C=(A)/(num_sectors*num_heads)



// helper: calc file position for current LBA or CSH
//
uint32 IdeDevice::calc_sector()
{
	if(drive_head&drvhd_LBA_mask)
	{
		xlogline("sector: LBA=%u",lba_address&0x0fffffff);

		return lba_address&0x0fffffff;
	}
	else
	{
		xlogline("sector: C=%i, H=%i, S=%i", cylinder_low+cylinder_high*256, drive_head&0x0f, int(sector_number));

		return calc_LBA_from_CHS(cylinder_low + cylinder_high*256, drive_head&0x0f, sector_number);
	}
}

// helper: end command with error
// note: do not call while disk io still pending!
//
void IdeDevice::end_command(uint error)
{
	io_mode = io_none;
	status_register = status_RDY_mask | status_ERROR_mask | status_BSY_mask;	// BSY wird autom. entfernt
	error_register = error;
}

// helper: end command with success
//
void IdeDevice::end_command()
{
	io_mode = io_none;
	status_register = status_RDY_mask | status_BSY_mask;	// BSY wird autom. entfernt
	error_register = 0;
}

// helper: start reading buffer from disk
//
void IdeDevice::read_buffer_from_disk(Time t)
{
	if(!sector_id_is_valid()) { end_command(error_ABORT_mask); return; }

	if(powermode!=Active)
	{
		if(powermode==Standby)
		{
			if(devicetype==HardDisk) t += 1.5;
			if(devicetype==CDRom)    t += 3;
		}
		powermode = Active;
	}

	hd_read_sector(calc_sector());
	io_mode = io_read_disk;

	busy_until = t + read_delay;
	error_register  = 0;
	status_register = status_RDY_mask | status_BSY_mask;
}

// helper: start writing buffer to disk
//
void IdeDevice::write_buffer_to_disk(Time t)
{
	if(!sector_id_is_valid()) { end_command(error_ABORT_mask); return; }
	if(!disk_writable) { end_command(error_ABORT_mask); return; }

	if(powermode!=Active)
	{
		if(powermode==Standby)
		{
			if(devicetype==HardDisk) t += 1.5;
			if(devicetype==CDRom)    t += 3;
		}
		powermode = Active;
	}

	hd_write_sector(calc_sector());
	io_mode = io_write_disk;

	busy_until = t + write_delay2;
	error_register  = 0;
	status_register = status_RDY_mask | status_BSY_mask;
}

// helper: start host reading from sector buffer
//
void IdeDevice::read_data_from_buffer()
{
	io_mode = io_read_buffer;
	buffer_ptr = 0;
	status_register = status_RDY_mask | status_DRQ_mask;
}

// helper: start host writing to sector buffer
//
void IdeDevice::write_data_to_buffer()
{
	io_mode = io_write_buffer;
	buffer_ptr = 0;
	status_register = status_RDY_mask | status_DRQ_mask;
}

//	helper: create buffer[] for command "Identify Drive"
//	bekannte Felder werden eingesetzt.
//	unbekannte Felder werden auf 0 gesetzt.
//	Als Modellname wird der Dateiname (evtl. mit Pfad, bei echten Devices wird das reichen) eingesetzt.
//	ATA5 pg. 87
//
void IdeDevice::create_identify_drive_page()
{
	// This command prepares a buffer (256 words) with information about the drive.
	//			Description												Example
	//	0		bit field: bit 6: fixed disk, bit 7: removable medium 	0x0040
	//	1		Default number of cylinders								16383
	//	3		Default number of heads									16
	//	6		Default number of sectors per track						63
	//	7-8		reserved for the CFA
	//	10-19 	Serial number (in ASCII)								G8067TME
	//	23-26 	Firmware revision (in ASCII)							GAK&1B0
	//	27-46 	Model name (in ASCII)									Maxtor 4G160J8
	//	47		0x8000 + 01h…FFh = Maximum number of sectors that shall be transferred per interrupt on READ/WRITE MULTIPLE commands
	//	49		bit field: bit 9: LBA supported							0x2f00
	//	53		bit field: bit 0: words 54-58 are valid					0x0007
	//					   bit 1: words 64-70 are valid.
	//							  A device that supports PIO mode 3 or above or Multiword DMA mode 1 or above must set bit 1.
	//					   bit 2: the device supports Ultra DMA and the values reported in word 88 are valid.
	//	54		Current number of cylinders								16383
	//	55		Current number of heads									16
	//	56		Current number of sectors per track						63
	//	57-58 	Current CHS capacity									16514064		ATA5 pg.110
	//	60-61 	Default LBA capacity									268435455
	//	64		Advanced PIO modes supported		ATA5 pg.104
	//	80		ATA version							ATA5 pg.105
	//	82-83 	Command sets supported									7c69 4f09
	//	85-86 	Command sets enabled									7c68 0e01
	//	100-103 Maximum user LBA for 48-bit addressing					320173056
	//	160		CFA power mode						ATA5 pg.108
	//	161-165	reserved for CFA
	//	255 	Checksum and signature (0xa5)							0x44a5

	memset(buffer,0,512);

	// Brain Teaser:

	// input() geht davon aus, dass der buffer[] LITTLE_ENDIAN ist,
	// weil das diskfile vom realen IDE controller wie alle Files seit Ur-Tagen so herum ausgelesen wird.
	// input() wird also den buffer[] mit Peek2Z() auslesen,
	// deshalb müssen wir alle Zahlen mit Poke2Z() reinschreiben.
	// Dummerweise sagt der IDE-Standard, dass die String-Ablage hier BIG ENDIAN ist. (first char in upper byte)
	// Deshalb müssen wir alle Strings swappen.

	// TODO: CDRom-Bits

	uint16* words = (uint16*)buffer;
	poke2Z(words+0,(1<<6));					// fixed disk, ATA device
	//if(is_cfa) words[0] = 0x848A;			// ATA5 pg.45: supports CFA feature set: alt. method, wir setzen aber word83.bit2
	poke2Z(words+1,dflt_num_cylinders);
	poke2Z(words+2,0xC837);					// ATA5 pg.89: Device does not require SET FEATURES subcommand to spin-up after power-up and IDENTIFY DEVICE response is complete
	poke2Z(words+3,dflt_num_heads);
	poke2Z(words+6,dflt_num_sectors);

	memcpy(words+10,"Clondyke",8);			// serial#
	memcpy(words+23,"Cyclists",8);			// firmware revision
	cstr diskname = filepath==nullptr ? "Cemetery" : startswith(filepath,"/dev/") ? filepath : basename_from_path(filepath);
	strncpy((ptr)(words+27),diskname,40);	// model name
	for(uint i=10;i<=46;i++) poke2X(words+i,peek2Z(words+i));					// first char in upper byte

	poke2Z(words+47,0x8000+max_sectors_per_multiple);
	poke2Z(words+49, (1<<9)+(1<<8));		// ATA5 pg.89: LBA supported
	poke2Z(words+53, (1<<0));				// ATA5 pg.89: words 54..58 are valid
	poke2Z(words+54, num_cylinders);
	poke2Z(words+55, num_heads);
	poke2Z(words+56, num_sectors);
	poke2Z(words+57, ((off_t)num_heads*num_sectors*num_cylinders) % 0x10000);	// erst das untere Wort
	poke2Z(words+58, ((off_t)num_heads*num_sectors*num_cylinders) / 0x10000);	// dann das obere Wort
	poke2Z(words+60, total_sectors % 0x10000);									// erst das untere Wort
	poke2Z(words+61, total_sectors / 0x10000);									// dann das obere Wort
	poke2Z(words+80,(1<<ATA_VERSION));

	poke2Z(words+82,(1<<14)+				// ATA5 pg.91:	NOP supported
					(is_packet?(1<<9)		//				DEVICE RESET supported
							  :(3<<12)));	//				READ BUFFER & WRITE BUFFER supported
	poke2Z(words+83,(1<<14)+				// ATA5 pg.91:	shall be set to 1
					(is_cfa?(1<<2):0));		//				supports CFA feature set
	poke2Z(words+84,(1<<14));				// ATA5 pg.92:	shall be set to 1

	poke2 (words+85, peek2(words+82));		// command sets/features enabled
	poke2Z(words+86, 0);

	poke2Z(words+160,is_cfa?(1<<15)+50:0);	// ATA5 pg.94: CFA: 50mA => no need to handle power mode 1

	poke2Z(words+255,0x00a5);				// checksum & signature
	int8 sum = 0;
	uptr p, p0 = buffer;
	for(p=p0; p<p0+511; p++) sum += *p;
	*p = 0-sum;
}


// helper: create buffer[] for command CFA TRANSLATE SECTOR
// cmd in: CSH or LBA
// cmd out: 512 bytes of info
//
// proc in: buffer as read from the sector for test for all-FF
//			sector address valid
//
void IdeDevice::create_cfa_translate_sector_page()		// ATA4++		ATA5 pg. 71
{
	// test whether sector is erased:
	bool erased = yes;
	for(uptr p = buffer+512; erased && p>buffer;) { if(~*--(uint64*&)p) erased=no; }

	// get write count:
	uint32 writecount = 1;								// TODO

	// erase buffer[]:
	memset(buffer,0,512);

	// calc LBA/CHS:
	uint32 lba      = lba_address & 0x0fffffff;
	uint32 cylinder = cylinder_high*256+cylinder_low;	// 0 .. 16383
	uint32 sector   = sector_number;					// 1 .. 63
	uint32 head     = drive_head&0x0f;					// 0 .. 15

	if(drive_head&0x40) { calc_CHS_from_LBA(lba,cylinder,head,sector); }
	else				{ lba = calc_LBA_from_CHS(cylinder,head,sector); }

	// fill in data:
	buffer[0] = cylinder>>8;		//	00h		Cylinder number MSB
	buffer[1] = cylinder;			//	01h		Cylinder number LSB
	buffer[2] = head;				//	02h		Head number
	buffer[3] = sector;				//	03h		Sector number
	buffer[4] = lba>>16;			//	04h		LBA bits (23:16)
	buffer[5] = lba>>8;				//	05h		LBA bits (15:8)
	buffer[6] = lba;				//	06h		LBA bits (7:0)
									//	07-12h	Reserved
	buffer[0x13] = erased ? -1 : 0;	//	13h		Sector erased flag (FFh = erased; 00h = not erased)
									//	14-17h	Reserved
	buffer[0x18] = writecount>>16;	//	18h		Sector write cycles count bits (23:16)
	buffer[0x19] = writecount>>8;	//	19h		Sector write cycles count bits (15:8)
	buffer[0x1A] = writecount;		//	1Ah		Sector write cycles count bits (7:0)
									//	1B-1FFh	Reserved  -  note: ATA5 says 1B-FFh, but i think this is an error
}



// ---------------------------------------------------------
//		interface
// ---------------------------------------------------------


void IdeDevice::reset(Time t)
{
	error_register = 0;			// no error
	feature_register = 0;		// no features written. whatever.
	status_register = 0;		// RDY = 0
	command_register = 0;		// pending command = nop
	place_signature_in_registers();

	sectors_per_multiple = 0;
	pio_8bit_data_mode = no;
	is_selected = is_master;
	io_mode = io_none;
	powermode = Active;

	busy_until = t + reset_delay;

	hd_wait_busy();
	hd_mode = hd_idle;
}


// write data to data register
// device must be selected
//
void IdeDevice::writeData(Time t, uint16 value)		// CS0=0, CS1=1, rrr=0
{
	assert(is_selected);
	if(io_mode!=io_write_buffer) return;
	assert(!is_busy());
	assert(is_drq());

//	!SEL				=> ignored					ATA5 pg.50++
//	 SEL,  BSY			=> indeterminate
//	 SEL, !BSY, !DRQ	=> ignored
//	 SEL, !BSY,  DRQ	=> transfer 16-bit data word
//					=====> SEL, !BSY, DRQ => I/O; else ignored

	if(pio_8bit_data_mode) { poke1Z(buffer+buffer_ptr, value); buffer_ptr+=1; }
	else				   { poke2Z(buffer+buffer_ptr, value); buffer_ptr+=2; }

	if(buffer_ptr>=512)
	{
		if(command_register==WRITE_BUFFER) end_command();
		else write_buffer_to_disk(t);
	}
}


// read data from data register
// data transfer "buffer -> host" must be in progress
//
//	if drive is not selected:
//	  if 2 drives are connected: do not call!
//	  if only 1 drive (master) is connected:				ATA5 pg.288
//		return own register as if selected
//		not shure about setting error condition TODO
//
uint16 IdeDevice::readData(Time t)						// CS0=0, CS1=1, rrr=0
{
	if(io_mode!=io_read_buffer) return 0xffff;
	assert(!is_busy());
	assert(is_drq());

//	!SEL				=> ignored	(if 2 drives)			ATA5 pg.50++
//	 SEL,  BSY			=> indeterminate
//	 SEL, !BSY, !DRQ	=> ignored
//	 SEL, !BSY,  DRQ	=> transfer 16-bit data word
//					=====> SEL, !BSY, DRQ => I/O; else ignored

	uint16 rval;
	if(pio_8bit_data_mode) { rval = 0xff00 + peek1Z(buffer+buffer_ptr); buffer_ptr+=1; }	// CFA only
	else				   { rval = peek2Z(buffer+buffer_ptr); buffer_ptr+=2; }

	if(buffer_ptr>=512)							// this input fetches the last word
	{
		increment_sector_count_and_number();
		if(sector_count==0) end_command();
		else if(sector_id_is_valid()) read_buffer_from_disk(t);
		else end_command(error_ABORT_mask);
	}
	return rval;
}


//	read from register reg with CS0=0, CS1=1
//	do NOT use for the data register!
//
//	if drive is not selected:
//	  if 2 drives are connected: do not call!
//	  if only 1 drive (master) is connected:				ATA5 pg.288
//		return own registers 0 .. 6 as if selected
//		return 0x00 for control register
//
uint8 IdeDevice::readRegister(Time t, uint reg)
{
	assert(reg&7);			// not the data register
	xlogline("IdeDevice::readRegister %i",reg&7);

//										ATA5 pg.50++
//	!SEL,		=> ignored.		(if 2 drives)
//	 SEL,  BSY	=> Place the contents of the Status register on the data bus.	CmdReg: exit the interrupt pending state.
//	 SEL, !BSY	=> Place the contents of the register on the data bus.			CmdReg: exit the interrupt pending state.

	if(!is_busy()) switch(reg&7)
	{
	case ide_sector_count:	 return sector_count;
	case ide_sector_number:	 return sector_number;		// == ide_lba_0_7
	case ide_cylinder_low:	 return cylinder_low;		// == ide_lba_8_15
	case ide_cylinder_high:	 return cylinder_high;		// == ide_lba_16_23
	case ide_drive_head:	 return drive_head;			// == ide_lba_24_28
	case ide_error_register: return error_register;		// TODO: cleared on read?
	}

	return is_selected ? getStatusRegister(t) : 0x00;
}


// write to register reg with CS0=0, CS1=1
// note: writing to registers goes to both, slave & master!
// do NOT use for the data register!
//
void IdeDevice::writeRegister(Time t, uint reg, uint8 value)
{
	reg &= 7;

	assert(reg);				// not the data register
	xlogline("IdeDevice::writeRegister %i, 0x%02X", reg&7, uint(value));

//														ATA5 pg.50++
//	!SEL,  BSY		 => ignored.
//	 SEL,  BSY		 => indeterminate.					CmdReg: if the device supports the DEVICE RESET command, exit the interrupt pending state.
//
//	 SEL, !BSY,  DRQ => ignored.						CmdReg: if the device supports the DEVICE RESET command, exit the interrupt pending state.
//
//	!SEL, !BSY		 => Place data into the register.	CmdReg: Do not execute except EXECUTE DEVICE DIAGNOSTICS.
//	 SEL, !BSY, !DRQ => Place data into the register.	CmdReg: execute command (exit the interrupt pending State).


	if(is_busy())	// => ignore except if SEL + BSY + PACKET + CMD=DEVICE_RESET
	{
		if(is_selected && reg==7 && value==DEVICE_RESET && is_packet) handle_command(t,value);
		return;
	}

// now:
//	 SEL, !BSY,  DRQ => ignored.						CmdReg: if the device supports the DEVICE RESET command, exit the interrupt pending state.
//	!SEL, !BSY		 => Place data into the register.	CmdReg: Do not execute except EXECUTE DEVICE DIAGNOSTICS.
//	 SEL, !BSY, !DRQ => Place data into the register.	CmdReg: execute command (exit the interrupt pending State).

	if(is_selected && is_drq()) // => ignore except if PACKET + CMD=DEVICE_RESET
	{
		if(reg==7 && value==DEVICE_RESET && is_packet) handle_command(t,value);
		return;
	}

// now:
//	!SEL, !BSY		 => Place data into the register.	CmdReg: Do not execute except EXECUTE DEVICE DIAGNOSTICS.
//	 SEL, !BSY, !DRQ => Place data into the register.	CmdReg: execute command (exit the interrupt pending State).

	switch(reg)
	{
	case ide_command_register:
		if((is_ready() && is_selected) ||					// exec cmd if selected, !BSY, RDY, !DRQ
		   (value==EXECUTE_DEVICE_DIAGNOSTIC&&!is_drq()) ||	// EXECUTE_DEVICE_DIAGNOSTIC: egal: SEL, RDY
		   (value==DEVICE_RESET&&is_packet&&is_selected))	// DEVICE_RESET: egal: RDY, DRQ, BSY
			handle_command(t,value);
		return;
	case ide_feature_register:
		feature_register = value;
		return;
	case ide_sector_count:		// 0 => 256
		sector_count  = value;
		return;
	case ide_sector_number:		// == ide_lba_0_7
		sector_number = value;
		return;
	case ide_cylinder_low:		// == ide_lba_8_15
		cylinder_low  = value;
		return;
	case ide_cylinder_high:		// == ide_lba_16_23
		cylinder_high = value;
		return;
	case ide_drive_head:		// == ide_lba_24_28
		drive_head = value;
		is_selected = is_master == !(value&drvhd_MASTER_mask);
		return;
	}
}


uint8 IdeDevice::getStatusRegister(Time t)
{
	if(t<busy_until || hd_mode!=hd_idle)
	{
		assert( (is_busy() || !is_ready()) );
		assert( (io_mode==io_read_buffer || io_mode==io_write_buffer) == is_drq() );
	}

	else switch(io_mode)			// t≥busy_until && hd_mode==hd_idle
	{
		default:					// io_none: no data transfer in progress
		{
			assert(!is_drq());
			status_register |= status_RDY_mask;		// ready (after reset)
			status_register &= ~status_BSY_mask;	// falls cmd aborted
			break;
		}
		case io_write_buffer:		// data transfer in progress: host -> buffer
		case io_read_buffer:		// data transfer in progress: buffer -> host
		{
			assert(!is_busy() && is_drq());
			break;
		}
		case io_read_disk:			// read transfer disk -> buffer finished:
		{
			if(hd_error)			// i/o finished with error:
			{
				end_command(error_UNC_mask);
				busy_until += write_delay3;				// TODO: failed_read_delay
			}

			else if(command_register==READ_VERIFY_SECTORS)	// VERIFY finished => kein Datentransfer zum Host
			{
				increment_sector_count_and_number();
				if(sector_count==0) end_command();
				else read_buffer_from_disk(t);
			}

			else if(command_register==CFA_TRANSLATE_SECTOR)	// read sector for test for erased => now transfer the sector info page
			{
				if(sector_id_is_valid()) { create_cfa_translate_sector_page(); read_data_from_buffer(); }
				else end_command(error_ABORT_mask);
			}

			else					// Starte Datenübertragung zum Host
			{
				read_data_from_buffer();
			}

			break;
		}
		case io_write_disk:			// write transfer buffer -> disk finished
		{
			if(hd_error)			// i/o finished with error:
			{
				end_command(error_BAD_mask);
				busy_until += write_delay3;
			}

			else
			{
				increment_sector_count_and_number();
				if(sector_count==0) end_command();		// letzter Sektor geschrieben => fertig
				else									// weitere Sektoren schreiben:
					 if(command_register==CFA_ERASE_SECTORS) write_buffer_to_disk(t); // nächsten Sektor löschen
				else write_data_to_buffer();			// Datentransfer Host -> Buffer restarten
			}

			break;
		}
	}

	return status_register;
}


void IdeDevice::audioBufferEnd(Time t)
{
	(void) getStatusRegister(t);
	busy_until -= t;
}


/*	handle command:
	device must be selected, except:
	EXECUTE_DEVICE_DIAGNOSTIC is executed for both devices (SEL bit ignored)		ATA5 pg.83
	EXECUTE_DEVICE_DIAGNOSTIC is executed even if drive !RDY
	DEVICE_RESET is executed even if drive !RDY, BSY or DRQ		(is_packet only)
*/
void IdeDevice::handle_command(Time t, uint8 cmd)
{
	xlogline("IdeDevice::handle_command 0x%02X", uint(cmd));

	assert(is_selected || cmd==EXECUTE_DEVICE_DIAGNOSTIC);
	assert(io_mode==io_none || (is_packet&&cmd==DEVICE_RESET));
	assert(!is_busy() || (is_packet&&cmd==DEVICE_RESET));
	assert(is_ready() || cmd==EXECUTE_DEVICE_DIAGNOSTIC || (is_packet&&cmd==DEVICE_RESET));

	error_register  = 0x00;
	status_register &= status_RDY_mask;
	command_register = cmd;


	switch(cmd)	// Info:
	{			// NAME: Protocol, GeneralFeatureSet, PacketFeatureSet

	case 0x00:	// NOP: no-data, optional, MANDATORY		General: wir sagen in der identify_drive_page, dass wir's unterstützen
				//	feature_register:
				//	00h NOP				Return command aborted and abort any outstanding queued commands.
				//	01h	NOP	Auto Poll	Return command aborted and do not abort any outstanding queued commands.
				//	02h-FFh	Reserved	Return command aborted and do not abort any outstanding queued commands.
		error_register |= error_ABORT_mask;
		status_register |= status_ERROR_mask;
		return;

	case 0x03:	// CFA REQUEST EXTENDED ERROR: no-data, CFA, prohibited		ATA4++		ATA5 pg.68
				// puts an extended error code in the error_register
				// status.ERR cleared
				//		00h		No error detected / no additional information
				//		01h		Self-test passed
				//		03h		Write / Erase failed
				//		05h		Self-test or diagnostic failed
				//		09h		Miscellaneous error
				//		0Ch		Corrupted media format
				//		10h		ID Not Found / ID Error
				//		11h		Uncorrectable ECC error
				//		14h		ID Not Found
				//		18h		Corrected ECC error
				//		1Fh		Data transfer error / command aborted
				//		20h		Invalid command
				//		21h		Invalid address
				//		27h		Write protect violation
				//		2Fh		Address overflow (address too large)
				//		30-34h	Self-test or diagnostic failed
				//		35h,36h	Supply or generated voltage out of tolerance
				//		37h,3Eh	Self-test or diagnostic failed
				//		38h		Corrupted media format
				//		3Ah		Spare sectors exhausted
				//		3Bh,3Ch,3Fh Corrupted media format
		if(!is_cfa) break;
		error_register = 0;					// TODO
		return;

	case 0x08:	// DEVICE RESET: no-data, prohibited, MANDATORY	ATA3++		ATA5 pg.78
				// reset the selected device
		if(!is_packet) break;
		io_mode = io_none;
		//hd_mode = hd_idle;					// => Overrun bei Worker-Thread-Kommandos wäre möglich
		error_register = 0x01;					// 0x01 = OK		see cmd 0x90: EXECUTE DEVICE DIAGNOSTIC  ATA5 pg.83
		status_register |= status_BSY_mask;
		busy_until = t + reset_delay;			// => BSY			TODO: see ATA5 pg.279
		place_signature_in_registers();
		if(!is_master) drive_head = 0x10;
		is_selected = true;
		return;

//	case 0x10	// RECALIBRATE:												up to ATA3; obsolete since ATA4
//	case 0x1_:	// RECALIBRATE:												up to ATA2; obsolete in ATA3; retired since ATA4

	case 0x20:	// READ SECTOR(S): PIO-in, MANDATORY, MANDATORY				ATA1++
		if(is_packet) break;	// TODO: In response to this command, devices that implement the PACKET Command feature set shall post command aborted and place the PACKET Command feature set signature in the Cylinder High and the Cylinder Low register (see 9.12).
		read_buffer_from_disk(t);
		return;

//	case 0x21:	// READ SECTOR(S) WITHOUT RETRY:							up to ATA4; obsolete since ATA5
//	case 0x22:	// READ LONG WITH RETRY:									up to ATA3; obsolete since ATA4
//	case 0x23:	// READ LONG WITHOUT RETRY:									up to ATA3; obsolete since ATA4
//	case 0x24:	// READ SECTOR(S) EXT: PIO-in, optional, prohibited			ATA6++
//	case 0x25:	// READ DMA EXT: DMA, optional, prohibited					ATA6++
//	case 0x26:	// READ DMA QUEUED EXT: DMA-queued, optional, prohibited	ATA6++
//	case 0x27:	// READ NATIVE MAX ADDRESS EXT: no-data, optional, prohibited	ATA6++
//	case 0x29:	// READ MULTIPLE EXT: PIO-in, optional, prohibited			ATA6++
//	case 0x2A:	// READ STREAM DMA EXT: DMA, optional, prohibited			ATA7++
//	case 0x2B:	// READ STREAM EXT: PIO-in, optional, prohibited			ATA7++
//	case 0x2F:	// READ LOG EXT: PIO-in, optional, optional					ATA6++

	case 0x30:	// WRITE SECTOR(S): PIO-out, MANDATORY, prohibited			ATA1++
		if(is_packet) break;
		write_data_to_buffer();
		return;

//	case 0x31:	// WRITE SECTOR(S) WITHOUT RETRY:							up to ATA4; obsolete since ATA5
//	case 0x32:	// WRITE LONG WITH RETRY:									up to ATA3; obsolete since ATA4
//	case 0x33:	// WRITE LONG WITHOUT RETRY:								up to ATA3; obsolete since ATA4
//	case 0x34:	// WRITE SECTOR(S) EXT: PIO-out, optional, prohibited		obsolete(?) in ATA5; defined in ATA6++
//	case 0x35:	// WRITE DMA EXT: DMA, optional, prohibited					ATA6++
//	case 0x36:	// WRITE DMA QUEUED EXT: DMA-queued, optional, prohibited	ATA6++
//	case 0x37:	// SET MAX ADDRESS EXT: no-data, optional, prohibited		ATA6++

	case 0x38:	// CFA WRITE SECTORS WITHOUT ERASE, PIO-out, CFA, prohibited ATA4++		ATA5 pg. 75
				// TODO: wir könnten die Daten jetzt rein-UNDen...
		if(!is_cfa) break;
		write_data_to_buffer();
		return;

//	case 0x39:	// WRITE MULTIPLE EXT: PIO-out, optional, prohibited		ATA6++
//	case 0x3A:	// WRITE STREAM DMA EXT: DMA, optional, prohibited			ATA7++
//	case 0x3B:	// WRITE STREAM EXT: PIO-out, optional, prohibited			ATA7++
//	case 0x3c:	// WRITE VERIFY:											up to ATA3; obsolete since ATA4
//	case 0x3D:	// WRITE DMA FUA EXT: DMA, optional, prohibited				ATA7++
//	case 0x3E:	// WRITE DMA QUEUED FUA EXT: DMA-queued, optional, prohibited	ATA7++
//	case 0x3F:	// WRITE LOG EXT: PIO-out, optional, optional				ATA6++

	case 0x40:	// READ VERIFY SECTOR(S): no-data, MANDATORY, prohibited	ATA1++
				// This command is identical to the READ SECTOR(S) command,
				// except that the DRQ bit is never set to one, and no data is transferred to the host.
		if(is_packet) break;
		read_buffer_from_disk(t);
		return;

//	case 0x41:	// READ VERIFY SECTOR(S) WITHOUT RETRY:						up to ATA4; obsolete since ATA5
//	case 0x42:	// READ VERIFY SECTOR(S) EXT: no-data, optional, prohibited	ATA6++
//	case 0x45:	// WRITE UNCORRECTABLE EXT: no-data, optional, prohibited	ATA8 (draft)
//	case 0x47:	// READ LOG DMA EXT: DMA, optional, optional				ATA8 (draft)
//	case 0x50:	// FORMAT TRACK:											up to ATA3; obsolete since ATA4
//	case 0x51:	// CONFIGURE STREAM: no-data, optional, optional			ATA7++
//	case 0x57:	// WRITE LOG DMA EXT: DMA, optional, optional				ATA8 (draft)
//	case 0x5C:	// TRUSTED RECEIVE: Packet, optional, MANDATORY				ATA8 (draft)
//	case 0x5D:	// TRUSTED RECEIVE DMA: Packet, optional, MANDATORY			ATA8 (draft)
//	case 0x5E:	// TRUSTED SEND: Packet, optional, MANDATORY				ATA8 (draft)
//	case 0x5F:	// TRUSTED SEND DMA: Packet, optional, MANDATORY			ATA8 (draft)

	case 0x70:	//	SEEK TRACK:	no-data, MANDATORY, prohibited				up to ATA6, obsolete since ATA7		ATA5 pg.162
				//	in: HCS or LBA set
		if(is_packet) break;
		return;	//	TODO: evtl. könnten wir bei einem HDD berechnete Wartezeiten einbauen.

//	case 0x7_:	// SEEK TRACK:												up to ATA2; obsolete in ATA3; retired since ATA4

	case 0x87:	// CFA TRANSLATE SECTOR: PIO-in, CFA, prohibited			ATA4++		ATA5 pg. 71
		if(!is_cfa) break;
		sector_count = 1;
		read_buffer_from_disk(t);			// read sector for empty-test. getStatusRegister() will do the rest
		return;


	case 0x90:	// EXECUTE DEVICE DIAGNOSTIC: no-data, MANDATORY, MANDATORY	ATA1++		ATA5 pg.81
				// This command shall perform the internal diagnostic tests implemented by the device.
				// The DEV bit in the Device/Head register is ignored.
				// Both devices, if present, shall execute this command regardless of which device is selected.
		error_register = 0x01;				// 0x01 = OK
		busy_until = t+reset_delay;			// => BSY			TODO: see ATA5 pg.279
		status_register |= status_BSY_mask;
		place_signature_in_registers();
		return;

	case 0x91:	//	INITIALISE DEVICE PARAMETERS:	no-data, MANDATORY, prohibited		up to ATA5, obsolete since ATA6		ATA5 pg.120
		if(is_packet) break;
		if(!supports_CHS) break;
		if(sector_count<8) break;			// abort wg. parameter error
		num_sectors = sector_count;
		num_heads = (drive_head&0x0f)+1;
		num_cylinders = min(0xffffu,total_sectors/num_heads/num_sectors);
		return;

//	case 0x92:	// DOWNLOAD MICROCODE: PIO-out, optional, prohibited		ATA2++
//	case 0x94:	// STANDBY IMMEDIATE:		up to ATA3; retired since ATA4  -->  ATA8 pg.394		same function: 0xE0
//	case 0x95:	// IDLE IMMEDIATE:			up to ATA3; retired since ATA4  -->  ATA8 pg.394		same function: 0xE1
//	case 0x96:	// STANDBY:					up to ATA3; retired since ATA4  -->  ATA8 pg.394		same function: 0xE2
//	case 0x97:	// IDLE:					up to ATA3; retired since ATA4  -->  ATA8 pg.394		same function: 0xE3
//	case 0x98:	// CHECK POWER MODE:		up to ATA3; retired since ATA4  -->  ATA8 pg.394		same function: 0xE5
//	case 0x99:	// SET SLEEP MODE:			up to ATA3; retired since ATA4  -->  ATA8 pg.394		same function: 0xE6
//	case 0xA0:	// PACKET: Packet, prohibited, MANDATORY					ATA3++	TODO ATAPI ESXDOS
//	case 0xA1:	// IDENTIFY PACKET DEVICE: PIO-in, prohibited, MANDATORY	ATA3++	TODO ATAPI ESXDOS
//	case 0xA2:	// SERVICE: Packet/DMA-queued, optional, optional			ATA3++
//	case 0xB0:	// SMART: no-data, optional, prohibited						ATA3++
//	case 0xB1:	// CFA DEVICE CONFIGURATION: no-data, CFA, optional			ATA6++
//	case 0xB6:	// NV CACHE: optional, prohibited, optional					ATA8++ (draft)

	case 0xC0:	// CFA ERASE SECTORS: no-data, CFA, prohibited				ATA4++		ATA5 pg. 66
				// pre-erase 1..256 sectors starting at LBA / CSH
		if(!is_cfa) break;
		memset(buffer,0xff,512);
		write_buffer_to_disk(t);
		return;

	case 0xC4:	// READ MULTIPLE: PIO-in, MANDATORY, prohibited				ATA1++			ATA5 pg.142
				// unterscheidet sich von READ SECTORS im Wesentlichen nur in den Interrupts, und die behandeln wir eh (noch) nicht
		if(is_packet) break;
		if(sectors_per_multiple==0) break;
		read_buffer_from_disk(t);
		return;

	case 0xC5:	// WRITE MULTIPLE: PIO-out, MANDATORY, prohibited			ATA1++
				// unterscheidet sich von WRITE SECTORS im Wesentlichen nur in den Interrupts, und die behandeln wir eh (noch) nicht
		if(is_packet) break;
		if(sectors_per_multiple==0) break;
		write_data_to_buffer();
		return;

	case 0xC6:	// SET MULTIPLE MODE: no-data, MANDATORY, prohibited		ATA1++
		if(is_packet) break;
		if(sector_count>max_sectors_per_multiple) break;	// abort cmd and keep old setting
		sectors_per_multiple = sector_count;
		return;

//	case 0xC7:	// READ DMA QUEUED: DMA-queued, optional, prohibited		ATA4++
//		break;

	case 0xC8:	// READ DMA: DMA, MANDATORY, prohibited						ATA1++	TODO MANDATORY (?DMA?)
		if(is_packet) break;
		busy_until = t+10;				// DMA wird nicht emuliert => DMA funktioniert nicht und hängt (vermutlich)
		status_register |= status_BSY_mask;
		return;

//	case 0xC9:	// READ DMA WITHOUT RETRY:									up to ATA4; obsolete since ATA5
//		break;

	case 0xCA:	// WRITE DMA: DMA, MANDATORY, prohibited					ATA1++		TODO MANDATORY (?DMA?)
		if(is_packet) break;
		busy_until = t+10;				// DMA wird nicht emuliert => DMA funktioniert nicht und hängt (vermutlich)
		status_register |= status_BSY_mask;
		return;

//	case 0xCB:	// WRITE DMA WITHOUT RETRY:									up to ATA4; obsolete since ATA5
//	case 0xCC:	// WRITE DMA QUEUED: DMA-queued, optional, prohibited		ATA4++

	case 0xCD:	// CFA WRITE MULTIPLE WITHOUT ERASE: PIO-out, CFA, prohibited	ATA5++		ATA5 pg. 73
				// TODO: assert SET MULTIPLE MODE was called...
				// TODO: wir könnten die Daten jetzt rein-UNDen...
		if(!is_cfa) break;
		write_data_to_buffer();
		return;

//	case 0xCE:	// WRITE MULTIPLE FUA EXT: PIO-out, optional, prohibited	ATA7++
//	case 0xD1:	// CHECK MEDIA CARD TYPE: no-data, optional, prohibited		ATA6++

	case 0xE0:	// STANDBY IMMEDIATE: no-data, MANDATORY, MANDATORY			ATA1++
				// Power Management feature set.
				// Power Management feature set is mandatory when power management is not implemented by a PACKET power management feature set.
				// This command is mandatory when the Power Management feature set is implemented.
				// This command causes the device to immediately enter the Standby mode: e.g. spin down the drive at once.
		powermode = Standby;
		return;

	case 0xE1:	// IDLE IMMEDIATE: no-data, MANDATORY, MANDATORY			ATA1++
				// Power Management feature set.
				// Power Management feature set is mandatory when power management is not implemented by a PACKET power management feature set.
				// This command is mandatory when the Power Management feature set is implemented.
				// The IDLE IMMEDIATE command allows the host to immediately place the device in the Idle mode.
		powermode = Idle;
		return;

	case 0xE2:	// STANDBY: no-data, MANDATORY, optional					ATA1++
				// Power Management feature set.
				// Power Management feature set is mandatory when power management is not implemented by a PACKET power management feature set.
				// This command is mandatory when the Power Management feature set is implemented when the PACKET Command feature set is not implemented.
				//
				// The value in the Sector Count register shall determine the time period programmed into the Standby timer. see table below
				//
				// This command causes the device to enter the Standby mode.
				// If the Sector Count register is non-zero then the Standby timer shall be enabled.
				// The value in the Sector Count register shall be used to determine the time programmed into the Standby timer
				// If the Sector Count register is zero then the Standby timer is disabled.
				//
				// sector count register = time of non-activity after which the disk will spin-down.
				// The disk will automatically spin-up again when you issue read/write commands.
				//
				//	0			Timeout disabled
				//	1-240		(value ∗ 5) s
				//	241-251		((value - 240) ∗30) min
				//	252			21 min
				//	253			Period between 8 and 12 hrs
				//	254			Reserved
				//	255			21 min 15 s
		powermode = Standby;
		return;

	case 0xE3:	// IDLE: no-data, MANDATORY, optional						ATA1++
				// Power Management feature set.
				// Power Management feature set is mandatory when power management is not implemented by a PACKET power management feature set.
				// This command is mandatory when the Power Management feature set is implemented and the PACKET Command feature set is not implemented.
				//
				// The IDLE command allows the host to place the device in the Idle mode and also set the Standby timer.
				// sector count register = time of non-activity after which the disk will spin-down. see table above.
		powermode = Idle;
		return;

	case 0xE4:	// READ BUFFER: PIO-in, optional, prohibited				ATA1++
		if(is_packet) break;
		sector_count = 1;
		read_data_from_buffer();
		return;

	case 0xE5:	// CHECK POWER MODE: no-data, MANDATORY, MANDATORY			ATA1++
				// Power Management feature set.
				// Power Management feature set is mandatory when power management is not implemented by a PACKET power management feature set.
				// This command is mandatory when the Power Management feature set is implemented.
				//
				// The CHECK POWER MODE command allows the host to determine the current power mode of the device.
				// The CHECK POWER MODE command shall not cause the device to change power or affect the operation of the Standby timer.
				//
				// return value in Sector Count result value –
				//	00h – device is in Standby mode.
				//	80h – device is in Idle mode.
				//	FFh – device is in Active mode or Idle mode.
		sector_count = powermode==Standby ? 0x00 : powermode==Idle ? 0x80 : 0xFF;
		return;

	case 0xE6:	// SLEEP: no-data, MANDATORY, MANDATORY						ATA1++
				// Power Management feature set.
				// Power Management feature set is mandatory when power management is not implemented by a PACKET power management feature set.
				// This command is mandatory when the Power Management feature set is implemented.
				//
				// This command is the only way to cause the device to enter Sleep mode.
				// This command causes the device to set the BSY bit to one, prepare to enter Sleep mode, clear the BSY bit to zero and assert INTRQ.
				// The host shall read the Status register in order to clear the interrupt pending and allow the device to enter Sleep mode.
				// In Sleep mode, the device only responds to the assertion of the RESET signal and the writing of the SRST bit in the Device Control register
				// and releases the device driven signal lines. The host shall not attempt to access the Command Block registers while the device is in Sleep mode.
				// Because some host systems may not read the Status register and clear the interrupt pending, a device may automatically release INTRQ
				// and enter Sleep mode after a vendor specific time period of not less than 2 s.
				// The only way to recover from Sleep mode is with a software reset, a hardware reset, or a DEVICE RESET command.
				// A device shall not power-on in Sleep mode nor remain in Sleep mode following a reset sequence.
		busy_until = t + 24*60*60;
		status_register = 0;					// !RDY.  das müsste funktionieren...
		return;

	case 0xE7:	// FLUSH CACHE: no-data, MANDATORY, optional				ATA4++
		return;	// we have no caches							TODO: we could wait for write to file on helper thread to complete

	case 0xE8:	// WRITE BUFFER: PIO-out, optional, prohibited				ATA1++
		if(is_packet) break;
		sector_count = 1;
		write_data_to_buffer();					// end_write_sector() catches command 0xE8 and does not write to disk
		return;

//	case 0xE9:	// WRITE SAME:		up to ATA2, obsolete in ATA 3, retired since ATA4
//	case 0xEA:	// FLUSH CACHE EXT: no-data, optional, prohibited			ATA6++

	case 0xEC:	// IDENTIFY DEVICE: PIO-in, MANDATORY, MANDATORY			ATA1++
		create_identify_drive_page();			// virtual
		sector_count = 1;
		read_data_from_buffer();
		return;

	case 0xEF:	// SET FEATURES: no-data, MANDATORY, MANDATORY				ATA1++
				// General feature set
				// Mandatory for all devices.
				// Set transfer mode subcommand is mandatory.
				// Enable/disable write cache subcommands are mandatory when a write cache is implemented.
				// Enable/Disable Media Status Notification sub commands are mandatory if the Removable Media feature set is implemented.
				// All other subcommands are optional.
				//
				// feature_register = subcommand code
				// sector count, sector, cylinder low & high : may contain arguments for subcommand
				//
				//	01h		Enable 8-bit PIO transfer mode (CFA feature set only)
				//	02h		Enable write cache											--> we have no write cache
				//	03h		Set transfer mode based on value in Sector Count register.
				//				PIO default mode				%00000000
				//				PIO default mode, disable IORDY	%00000001
				//				PIO flow control transfer mode	%00001mod
				//				Multiword DMA mode				%00100mod
				//				Ultra DMA mode					%01000mod		mod = transfer mode number
				//	05h		Enable advanced power management
				//	06h		Enable Power-Up In Standby feature set.
				//	07h		Power-Up In Standby feature set device spin-up.
				//	0Ah		Enable CFA power mode 1
				//	31h		Disable Media Status Notification
				//	55h		Disable read look-ahead feature
				//	5Dh		Enable release interrupt
				//	5Eh		Enable SERVICE interrupt
				//	66h		Disable reverting to power-on defaults
				//	81h		Disable 8-bit PIO transfer mode (CFA feature set only)
				//	82h		Disable write cache
				//	85h		Disable advanced power management
				//	86h		Disable Power-Up In Standby feature set.
				//	8Ah		Disable CFA power mode 1
				//	95h		Enable Media Status Notification
				//	AAh		Enable read look-ahead feature
				//	CCh		Enable reverting to power-on defaults
				//	DDh		Disable release interrupt
				//	DEh		Disable SERVICE interrupt
		switch(feature_register)
		{
		case 0x01:	// Enable 8-bit PIO transfer mode (CFA feature set only)
			if(!is_cfa) break;
			pio_8bit_data_mode = true;
			return;

		case 0x81:	// Disable 8-bit PIO transfer mode (CFA feature set only)
			if(!is_cfa) break;
			pio_8bit_data_mode = false;
			return;

		case 0x0a:	// Enable CFA power mode 1
			if(!is_cfa) break;
			return;

		case 0x8A:	// Disable CFA power mode 1
			if(!is_cfa) break;
			return;	//	CFA devices may consume up to 500 mA maximum average RMS current for either 3.3V or 5V operation in Power Mode 1.
					//	CFA devices revert to Power Mode 1 on hardware or power-on reset.
					//	CFA devices revert to Power Mode 1 on software reset except when Set Features disable reverting to power-on defaults is set.
					//	Enabling CFA Power Mode 1 does not spin up rotating media devices.
					//	CFA devices may consume up to 75 mA maximum average RMS current for 3.3V or 100 mA maximum average RMS current for 5V operation in Power Mode 0.
					//	A device in Power Mode 0 shall accept the following commands:
					//	− IDENTIFY DEVICE
					//	− SET FEATURES (function codes 0Ah and 8Ah)
					//	− STANDBY
					//	− STANDBY IMMEDIATE
					//	− SLEEP
					//	− CHECK POWER MODE
					//	− EXECUTE DEVICE DIAGNOSTICS
					//	− CFA REQUEST EXTENDED ERROR
					//	A device in Power Mode 0 may accept any command that the device is capable of executing within the Power Mode 0 current restrictions.
					//	Commands that require more current than specified for Power Mode 0 shall be rejected with an abort error.

		case 0x03:	// Set transfer mode based on value in Sector Count register.
			if(sector_count<=0x0F) return;		// PIO mode. whatever
			else break;
		}
		break;	// unsupported sub command or unsupported transfer mode

//	case 0xF1:	// SECURITY SET PASSWORD: PIO-out, optional, optional		ATA3++
//	case 0xF2:	// SECURITY UNLOCK: PIO-out, optional, optional				ATA3++	vendor specific up to ATA2; e.g. similar to 0xE2 (STANDBY)
//	case 0xF3:	// SECURITY ERASE PREPARE: no-data, optional, optional		ATA3++	vendor specific up to ATA2; e.g. similar to 0xE3 (IDLE)
//	case 0xF4:	// SECURITY ERASE UNIT: PIO-out, optional, optional			ATA3++
//	case 0xF5:	// SECURITY FREEZE LOCK: no-data, optional, optional		ATA3++
//	case 0xF6:	// SECURITY DISABLE PASSWORD: PIO-out, optional, optional	ATA3++
//	case 0xF7:	// vendor specific in all ATA versions						e.g. FORMAT UNIT
//	case 0xF8:	// READ NATIVE MAX ADDRESS: no-data, optional, optional		ATA4++
//	case 0xF9:	// SET MAX ADDRESS: no-data, optional, optional				ATA4++

	default:
		break;	// command error / TODO
	}

// command error:
	logline("IdeDiskDrive: command 0x%02x: not supported", uint(cmd));

	error_register = error_ABORT_mask;
	status_register = status_RDY_mask | status_ERROR_mask;
}



























