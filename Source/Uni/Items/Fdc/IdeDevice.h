#pragma once
/*	Copyright  (c)	Günter Woigk 2014 - 2019
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

#include "kio/kio.h"
#include "zxsp_types.h"
#include "unix/FD.h"
#include "cpp/cppthreads.h"
#include <unistd.h>


class IdeDevice			// Base class for IdeCFcard, IdeHadDisk and IdeCDrom
{
public:
	enum	DeviceType	{ HardDisk, CDRom,  CFCard };
	enum	PowerMode	{ Active,   Standby, Idle, Sleep };

protected:
	enum	IOMode		{ io_none, io_write_buffer, io_read_buffer, io_write_disk, io_read_disk };
	enum	HDMode		{ hd_idle, hd_open, hd_close, hd_read, hd_write, hd_exit, hd_invalid };

// Basic settings:
	DeviceType	devicetype;
	bool	is_master;
	bool	is_cfa;				// CFA command set		--> CF card
	bool	is_packet;			// PACKET command set	--> CD-Rom
	bool	is_general;			// General Feature Set	--> HDD, CF card
	bool	can_write;			// device can write		--> no CD-Rom
	uint8	max_sectors_per_multiple;

// Registers:
	uint8	error_register;		// r
	uint8	feature_register;	// w
	uint8	sector_count;		// rw
	uint8	status_register;	// r
	uint8	command_register;	// w

	union
	{
		uint32 lba_address;
		struct
		{
		#ifdef __LITTLE_ENDIAN__
			uint8	sector_number;		// lba_0_7		// rw
			uint8	cylinder_low;		// lba_8_15		// rw
			uint8	cylinder_high;		// lba_16_23	// rw
			uint8	drive_head;			// lba_24_28	// rw
		#endif
		#ifdef __BIG_ENDIAN__
			uint8	drive_head;			// lba_24_28	// rw
			uint8	cylinder_high;		// lba_16_23	// rw
			uint8	cylinder_low;		// lba_8_15		// rw
			uint8	sector_number;		// lba_0_7		// rw
		#endif
		};
	};

// Disk file:
	cstr	filepath;			// as set in c'tor
	uint32	total_sectors;		// from filesize
	uint	dflt_num_sectors;	// from total_sectors
	uint	dflt_num_cylinders;	// from total_sectors
	uint	dflt_num_heads;		// from total_sectors
	uint	num_sectors;		// as set by init() or cmd INITIALIZE DEVICE PARAMETERS
	uint	num_cylinders;		// ""
	uint	num_heads;			// ""
	bool	supports_CHS;		// Drives with less than 8.45 GB must support CHS addressing
	bool	disk_writable;		// disk is writable
	bool	file_writable;		// diskfile is open for r/w => disk may be write enabled

// State:
	uint8	buffer[512];		// i/o buffer
	uint	buffer_ptr;

	uint8	sectors_per_multiple;
	bool	is_selected;
	IOMode	io_mode;
	bool	pio_8bit_data_mode;		// CF card only
	Time	busy_until;
	PowerMode powermode;

	// Timing constants
	Time	reset_delay;	// not-RDY after reset
	Time	read_delay;		// BSY after read command issued: ~ time to read a flash page
	Time	write_delay1;	// BSY after write command issued: ~ time to start command
	Time	write_delay2;	// BSY after last byte written to buffer[]:  ~ time to flash the page
	Time	write_delay3;	// BSY after last byte written to buffer[] and uncorrectable write error happened

// worker thread:
	pthread_t	worker_thread;
	static void* worker_proc(void*);
	void*		worker_proc();
	HDMode		hd_mode;
	cstr		hd_path;
	uint32		hd_sector;
	int			hd_error;
	PSemaphore	hd_semaphore;

public:
	IdeDevice(cstr filepath, DeviceType, bool master=yes);
	virtual ~IdeDevice();

	void	reset			(Time);
	void	writeRegister	(Time, uint reg, uint8 value);		// CS0=0, CS1=1, rrr>0
	uint8	readRegister	(Time, uint reg);					// CS0=0, CS1=1, rrr>0
	void	writeData		(Time, uint16 value);				// CS0=0, CS1=1, rrr=0
	uint16	readData		(Time);								// CS0=0, CS1=1, rrr=0
	cstr	getFilepath		()				{ return filepath; }
	bool	isLoaded		()				{ return total_sectors!=0; }
	bool	isWritable		()				{ return disk_writable; }
	bool	isSelected		()				{ return is_selected; }
	bool	isBusy			(Time t)		{ return getStatusRegister(t) & status_BSY_mask; }
	bool	is_busy			()				{ return status_register & status_BSY_mask; }
	bool	is_ready		()				{ return status_register & status_RDY_mask; }
	bool	is_drq			()				{ return status_register & status_DRQ_mask; }
	void	audioBufferEnd	(Time);
	uint8	getStatusRegister(Time);
	void	setWritable		(bool f)		{ disk_writable = f && can_write && file_writable; }

	static const int
	// register addresses for CS0=0, CS1=1:
	ide_data_register		= 0,	// rw
	ide_error_register		= 1,	// r
	ide_feature_register	= 1,	// w
	ide_sector_count		= 2,	// rw
	ide_sector_number		= 3,	// rw
	ide_cylinder_low		= 4,	// rw
	ide_cylinder_high		= 5,	// rw
	ide_drive_head			= 6,	// rw
	ide_status_register		= 7,	// r
	ide_command_register	= 7,	// w

	// bits in error register:
	error_AMNF_mask		= 0x01,	// Address mark not found.	not supported / never set
	error_TK0NF_mask	= 0x02,	// Track 0 not found.		not supported / never set
	error_ABORT_mask	= 0x04,	// command or parameter error
	error_MCR_mask		= 0x08,	// Media Change Requested. probably the 'eject' button of removable drives.
	error_IDNF_mask		= 0x10,	// Sector ID not found.		not supported / never set
	error_MC_mask		= 0x20,	// Removable media has been changed.
	error_UNC_mask		= 0x40,	// Uncorrectable data error.
	error_BAD_mask		= 0x80,	// bad block detected

	// bits in status register:
	status_ERROR_mask	= 0x01,	// If this bit is set then an error has occurred while executing the latest command.
								// The error status itself is to be found in the error register.
	status_IDX_mask		= 0x02,	// index pulse of the disk = '1'.	not supported / never set
	status_ECC_mask		= 0x04,	// ECC correction was performed.
	status_DRQ_mask		= 0x08,	// Data request: the disk either wants data (disk write) or has data for you (disk read)
	status_SKC_mask		= 0x10,	// Indicates that a seek has been executed with success.
	status_WFT_mask		= 0x20,	// write error has happened.
	status_RDY_mask		= 0x40,	// indicates that the disk has finished its power-up.
								// Wait for this bit to be active before doing anything (execpt reset) with the disk.
	status_BSY_mask		= 0x80,	// This bit is set when the disk is doing something for you.
								// wait for this bit to clear before you start giving orders to the disk.

	// bits in drive_head:
	drvhd_LBA_mask		= 1<<6,	// 0=chs,    1=lba
	drvhd_MASTER_mask	= 1<<4,	// 0=master, 1=slave

	// misc. commands:
	CFA_REQUEST_EXTENDED_ERROR_CODE		= 0x03,
	DEVICE_RESET						= 0x08,
	CFA_WRITE_SECTORS_WITHOUT_ERASE		= 0x38,
	READ_VERIFY_SECTORS					= 0x40,
	EXECUTE_DEVICE_DIAGNOSTIC			= 0x90,
	CFA_ERASE_SECTORS					= 0xC0,
	CFA_WRITE_MULTIPLE_WITHOUT_ERASE	= 0xCD,
	CFA_TRANSLATE_SECTOR				= 0x87,
	WRITE_BUFFER						= 0xE8,
	SET_FEATURES						= 0xEF;

private:
	void	reset_hd_data();
	void	hd_write_sector(uint32 sector);
	void	hd_read_sector(uint32 sector);
	bool	hd_command_finished()				{ return hd_mode==hd_idle; }
	void	hd_wait_busy()						{ while(hd_mode!=hd_idle) usleep(5000);	}

	void	open_diskfile(cstr fpath);
	void	close_diskfile();
	void	read_buffer_from_disk(Time);
	void	read_data_from_buffer();
	void	write_data_to_buffer();
	void	write_buffer_to_disk(Time);

	void	create_identify_drive_page();
	void	create_cfa_translate_sector_page();
	void	place_signature_in_registers();
	void	increment_sector_count_and_number();
	uint32	calc_sector();
	uint32	calc_LBA_from_CHS(uint c, uint h, uint s);
	void	calc_CHS_from_LBA(uint32 lba, uint& c, uint& h, uint& s);
	bool	sector_id_is_valid();
	void	end_command();
	void	end_command(uint error);
	void	handle_command(Time t, uint8 cmd);
};


class IdeCFCard : public IdeDevice
{
public:
	explicit IdeCFCard(cstr filepath, bool master=yes)		:IdeDevice(filepath,CFCard,master){}
};


class IdeHardDisk : public IdeDevice
{
public:
	explicit IdeHardDisk(cstr filepath, bool master=yes)	:IdeDevice(filepath,HardDisk,master){}
};


class IdeCDRom : public IdeDevice
{
public:
	explicit IdeCDRom(cstr filepath, bool master=yes)		:IdeDevice(filepath,CDRom,master){}
};




























