/*	Copyright  (c)	Günter Woigk 2016 - 2019
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RzxFile.h"
#include "RzxBlock.h"
#include <zlib.h>

//#ifndef _MSC_VER
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "unix/files.h"
#include "Uni/globals.h"
#include "unix/files.h"
#include "kio/TestTimer.h"



// ============================================================
//					Gameplay Recording Data
// ============================================================


/*	state:

	OutOfSync:

		bi			invalid
		blocks[]	may be empty
					all IRBs are Compressed
		blocks[0]	may be a Snapshot or an IRB

	Playing:

		blocks[] ≥ 2
		blocks[bi] is an IRB, Playing

	Snapshot:		special state while playing

		blocks[] ≥ 1
		blocks[bi] is a SnapshotBlock

	Recording:

		blocks[] ≥ 2
		blocks[bi] is an IRB, Recording

	EndOfFile:		special state while recording

		last frame is complete => caller can append frame with startFrame()
		note: EndOfFile after Playing may have an incomplete last frame => OutOfSync
			  better don't finish the last frame & keep Playing -> switch to Recording possible

		blocks[] ≥ 0
		bi == blocks.count
			or
		blocks[bi] is an IRB at EndOfBlock



Special Cases:

1. Der Benutzer möchte die Datei an der aktuellen Position oder an einer bestimmten Stelle abschneiden

	Wenn der Benutzer schneidet, sollten wir nur ganze Frames schneiden.
	Die Frage ist, warum er überhaupt schneidet.
	Schneidet er vor oder am aktuellen Frame, verliert er die Synchronisation. Wozu?
	Schneidet er hinter dem aktuellen Frame, ist das evtl. ok. Aber, wozu?

	Abspielende festlegen:
		Am Dateiende wird normalerweise auf Recording gewechselt.
			Es ist unnötig, das Abspielende vorab zu bestimmen.
			Man kann auch so jederzeit die Kontrolle übernehmen. Und das ist flexibler.
		Evtl. wg. Auto-Halt-CPU at EndOfFile?
			-> Dazu kann die Maschine die Stop-Position selbst verwalten, ohne die Datei zu kürzen.

	Der Benutzer möchte die rzx-Datei nur bis zu diesem Punkt speichern.
		-> writeFileUpToBlockAndFrame() oder writeFileUpToCurrentPosition() oder so.


2. Playing: Es soll an der aktuellen Position auf Aufnahme umgeschaltet werden

	z.B. wenn der Benutzer während dem Playback eine Eingabe in die Maschine macht. (Keyboard oder Joystick)
	z.B. statt OutOfSync wenn über das Ende eines Frames hinaus getInput() aufgerufen wurde
		 -> es muss auch noch das letzte input-Byte gespeichert werden!
	z.B. statt EndOfFile wenn die Maschine am Ende des letzten Frames angekommen ist

	Playing -->	startRecording() --> Recording


3. Dateiende nach Playback: Es soll weitergespielt und aufgezeichnet werden

	Wenn das File im Status EndOfFile ist, nimmt es an, dass der letzte Frame vollständig abgeschlossen ist:
	Beim Playback wird der neue Frame mit einem Interrupt beginnen. (außer idR. nach getSnapshot())
		• nach readFile()
		• nach purge()
		• nach storeSnapshot()
		• nach endFrame()

	Am Dateiende nach Playback kann aber der letzte Frame noch unvollständig sein.
	Man müsste deshalb zwischen EndOfFile (Recording) und EndOfFile (Playback) unterscheiden.
	Bei EndOfFile (Playback) müsste man zum vorangehenden Frame zurückgehen und dort wie unter Punkt 2 anhängen.

	Einfacher ist es, am Ende des letzten Frames diesen erst gar nicht abzuschließen. Der Status bleibt bei Playing.
	Dann kann man gemäß Punkt 2 auf Aufnahme umschalten.


4. Recording: Den aktuellen, sehr kurze Frame-Schnippel an den vorherigen Frame anhängen

	Nach einem verzögerten Interrupt soll der aktuelle, sehr kurze Frame-Schnippel
	an den vorherigen Frame angehängt werden.
	Danach soll wieder mit Recording (im neuen, leeren Frame) weitergemacht werden.

	Probleme:
		Der vorige Frame wurde als repeated Frame abgespeichert
		Der vorige Frame ist im vorigen Block
			... und ist ein repeated Frame
		Der vorige Block ist ein Snapshot
*/




/*	CREATOR
	state = EndOfFile
*/
RzxFile::RzxFile()
{
	xlogIn("new RzxData");
	init();
}


/*	DESTRUCTOR
*/
RzxFile::~RzxFile()
{
	xlogIn("~RzxData");
	kill();
}


//	helper: init RzxFile
//
//	state --> EndOfFile
//
void RzxFile::init()
{
	filename = NULL;
	creator_name = NULL;
	creator_major_version = 0;
	creator_minor_version = 0;
	rzx_file_version = 0;
	blocks.purge();
	bi = 0; // blocks.count();
	state = EndOfFile;
}


//	helper: destroy RzxFile
//
void RzxFile::kill()
{
	delete[] creator_name;
	delete[] filename;
	//for(uint i=0; i<blocks.count(); i++) { blocks[i].kill(); }
	//blocks.purge();
}




// ===========================================================================
//								   _____      _____
//						 /\       |  __ \    |_   _|
//						/  \      | |__) |     | |
//					   / /\ \     |  ___/      | |
//					  / ____ \    | |         _| |_
//					 /_/    \_\   |_|        |_____|
//
// ===========================================================================


/*	ANY TIME: purge RzxFile

	state --> EndOfFile
*/
void RzxFile::purge()
{
	kill();
	init();
}


/*	ANY TIME: rewind RzxFile

	state --> EndOfFile		file is empty:			ready for recording
		  --> Snapshot		start snapshot exists:	ready for playback
		  --> OutOfSync		start snapshot missing:	file is not usable

	if called while "Recording" then the current frame is lost.
	(we cannot finalize the frame because we don't know the icount.)
*/
void RzxFile::rewind()
{
	if(bi < blocks.count() && blocks[bi].isaInputRecordingBlock())
		blocks[bi].compress();

	bi = 0;

	if(blocks.count()==0)
		state = EndOfFile;

	else if(blocks[0].isaSnapshotBlock())
		state = Snapshot;

	else if(blocks[0].isaInputRecordingBlock())
		state = OutOfSync;

	else IERR();		// isaMachineSnapshot: TODO
}


/*	ANY TIME: read rzx file

	return:	state --> Snapshot:  file can be played
	throws:	state --> OutOfSync: file empty, snapshot missing, data corrupted or truncated:
								 after rewind() the file may become playable up to the error position.
*/
void RzxFile::readFile(cstr filename, bool snapshotOnly) throws // DataError,file_error
{
	xlogIn("RzxFile.readFile(%s)",filename);

	TT;

	purge();
	state = OutOfSync;
	this->filename = newcopy(filename);

	//	RZX Headr:
	//	0x00 	"RZX!" 	ASCII[4] 	RZX signature (hex 0x21585A52)
	//	0x04 	0x00 	BYTE		RZX major version
	//	0x05 	0x0C 	BYTE		RZX minor version
	//	0x06 	-		DWORD		Flags (reserved)
	//	0x0A 	- 	- 	RZX			blocks sequence

	FD fd(filename);			// throws
	uint8 bu[10];
	fd.read_bytes(bu,10);
	if(memcmp(bu,"RZX!",4)) throw DataError(wrongfiletype);

	xlogline("rzx file version = %u.%u",bu[4],bu[5]);
	rzx_file_version = peek2X(bu+4);				// MSB first
	if(rzx_file_version > MaxRzxLibraryVersion)
		throw DataError(wrongfiletype, usingstr("unsupported rzx file version %u.%u", bu[4],bu[5]));

	uint32 flags = peek4Z(bu+6);
	if(flags) xlogline("flags = 0x%08X",flags);

	off_t filesize = fd.file_size();
	if(filesize > 99 MB) throw DataError("file too long");


	// read the blocks:

	for( off_t fileposition = 10; fileposition < filesize; )
	{
		uint   btyp = fd.read_uint8();
		uint32 blen = fd.read_uint32_z();
		fileposition += blen;
		if(fileposition > filesize) throw DataError("Block 0x%02X exceeds file size", btyp);

		switch(btyp)
		{
		case 0x10:
			//	0x00 	0x10 	BYTE		Block ID
			//	0x01 	29+N 	DWORD		Block length
			//	0x05 	-		ASCIIZ[20] 	Creator name
			//	0x19 	VMAJ 	WORD		Creator major version number
			//	0x1B 	VMIN 	WORD		Creator minor version number
			//	0x1D 	-		BYTE[N] 	Creator custom data (optional)

			xlogline("Creator Information Block");

			if(blen < 0x1D) throw DataError("block too short");
			{
				str s = newstr(20); fd.read_bytes(s,20);
				for(int i=20;i && (s[--i]==' '||s[i]==0);) { s[i]=0; }	// SpecEmu pads with spaces up to slen=19
				delete[] creator_name; creator_name = s;
			}
			creator_major_version = fd.read_uint16_z();
			creator_minor_version = fd.read_uint16_z();
			xlogline("creator = %s %u.%u", creator_name, creator_major_version, creator_minor_version);
			if(blen>0x1d) xlogline("custom data = %u bytes", blen-0x1d);
			break;

		case 0x30:
		{
			xlogline("Snapshot Block");

			RzxBlock& block = blocks.grow();
			try
			{
				block.readSnapshotBlock(fd,blen,filename);
				if(snapshotOnly) return;
			}
			catch(AnyError&)
			{
				block.kill();
				blocks.drop();
				throw;
			}
			break;
		}

		case 0x80:
		{
			xlogline("Input Recording Block");

			RzxBlock& block = blocks.grow();
			try
			{
				block.readInputRecordingBlock(fd,blen);
				if(block.num_frames==0)
				{
					xlogline("warning: empty block");
					block.kill(); blocks.drop();
				}
				else block.compress();
			}
			catch(AnyError&)
			{
				// if readInputRecordingBlock() throws, then the block is usable but truncated, maybe empty.
				if(block.num_frames==0) { block.kill(); blocks.drop(); }
				else block.compress();
				throw;
			}

			#ifdef XLOG
				// the playback machine activates the timer interrupt at the start of each rzx frame.
				// It is believed that timer interrupts end after 48 cc, which limits the value for start_cc.
				//		(interrupts generated by extensions are not supported by rzx.)
				// cc can only be arbitrarily high right after loading a snapshot:
				//		only then the machine will not activate INT unless cc ≤ 48.
				//		the exact duration of the timer interrupt varies between emulators!
				//		the "debated" range is approx. from cc=32 to cc=64.
				//			  depending on model and emulator even longer.
				//		if interrupts are enabled and recorder and player disagree on whether the timer interrupt
				//			  is still active, then the machine is immediately OutOfSync right from the start.
				//		however it is unlikely that a snapshot starts in this range,
				//			  and if, it may be a .sna file (which always starts with interrupts disabled)
				//			  or the machine already started interrupt handling and disabled interrupts as well.
				//		NOTE: zxsp will enable the timer interrupt up to cc ≤ 48.
				//		DENK: eventually even if future research finds different values for any model.

				uint32& cc = block.cc_at_start;
				if(blocks.count()>2 && blocks[blocks.count()-2].isaInputRecordingBlock())
				{ if(cc > 48) logline("cc at start too high: %u (reset)", cc); cc = 0; }
				else
				{ if(cc>=32 && cc<=64) logline("unclear interrupt state after snapshot. cc = %u", cc); }
			#endif
			break;
		}

		default:
			xlogline("Unhandled Block 0x%02X, blen = %u (ignored)",bu[0],blen);
			fd.seek_fpos(fileposition);		// note: does not throw eof
			continue;
		}

		if(fd.file_position() == fileposition) continue;
		if(fd.file_position() > fileposition) throw DataError("block data exceeds block length");
		xlogline("%u unused bytes at end of block", fileposition - fd.file_position());
		fd.seek_fpos(fileposition);
	}

	if(blocks.count()==0) throw DataError("file contains no data");	// no Block 0x30 or 0x80
	if(blocks[0].isaInputRecordingBlock()) throw DataError("file has no initial snapshot");

	//bi = 0;
	state = Snapshot;	// position = Snapshotfile Block

	TTest(1e-3,"RzxFile:readFile()");
}


/*	ANY TIME: write compressed rzx file
	Creator = APPL_NAME
	Creator version = appl_version_h*256 + version_m, version_l
					  note: appl_version_h = 0

	state = preserved.

	note: if called while recording, then the current frame is not included.
*/
void RzxFile::writeFile(cstr filename) throws
{
	FD fd(filename,'w');

	//	RZX Headr:
	fd.write_bytes("RZX!",4);					// "RZX!"
	fd.write_uint16_x(OurRzxLibraryVersion);	// major+minor file version (major byte first!)
	fd.write_uint32_z(0);						// flags

	// Creator Information Block:
	fd.write_char(0x10);						// block ID
	fd.write_uint32_z(29);						// block length: 29 = min. length => no custom data
	char crea[20]; strncpy(crea,APPL_NAME,20);
	fd.write_bytes(crea,20);					// char[20] creator name
	fd.write_uint16_z(APPL_VERSION_H*256+APPL_VERSION_M);
	fd.write_uint16_z(APPL_VERSION_L);			// uint16[2] creator version
	//fd.write_bytes(NULL,0);					// no custom data

	// the blocks:
	for(uint i=0; i<blocks.count(); i++)
	{
		blocks[i].write(fd);
	}
}


/*	PLAYBACK: get Icount (R register count) of current frame

	Playing --> Playing
*/
int RzxFile::getIcount()
{
	assert(isPlaying());
	assert(bi<blocks.count());
	assert(blocks[bi].isaInputRecordingBlock());
	assert(blocks[bi].state != RzxBlock::Compressed);

	return blocks[bi].iCount();
}


/*	PLAYBACK: get CPU cycle at start of current frame

	Playing --> Playing
*/
int32 RzxFile::getStartCC()
{
	assert(isPlaying());
	assert(bi<blocks.count());
	assert(blocks[bi].isaInputRecordingBlock());

	return blocks[bi].startCC();
}


/*	PLAYBACK: get Snapshot filename

	Snapshot -> Playing | EndOfFile
*/
cstr RzxFile::getSnapshot()
{
	assert(isSnapshot());
	assert(bi<blocks.count());
	assert(blocks[bi].isaSnapshotBlock());

a:	cstr fn = blocks[bi++].snapshot_filename;

b:	if(bi==blocks.count())
		state = EndOfFile;

	else if(blocks[bi].isaInputRecordingBlock())
	{
		int icount = blocks[bi].uncompress();
		if(icount<0) { blocks[bi].compress(); goto b; }		// EndOfBlock -> empty Block!
		else state = Playing;
	}

	else if(blocks[bi].isaSnapshotBlock())
		goto a;		// hm hm..

	else TODO();	// isaMachineSnapshot

	return fn;
}


/*	RECORDING: store Snapshot in File
	next call should be startFrame() or startBlock()

	EndOfFile -> EndOfFile
*/
void RzxFile::storeSnapshot(cstr fname)
{
	assert(isEndOfFile());
	assert(bi == blocks.count() || (bi == blocks.count()-1 && blocks[bi].isaIRB() && blocks[bi].isEndOfBlock()));

	if(bi < blocks.count())
	{
		if(blocks[bi].num_frames) blocks[bi++].compress();
		else { blocks[bi].kill(); blocks.drop(); }
	}

	RzxBlock& block = blocks.grow();
	block.init(RzxBlock::IsaSnapshotBlock);
	block.snapshot_filename = newcopy(fname);
	bi++;
}


/*	RECORDING: start new Block
	note: first block must be a Snapshot block. You can't start recording without first storing a snapshot.

	EndOfFile -> Recording
*/
void RzxFile::startBlock(int32 cc)
{
	assert(isEndOfFile());
	assert(bi>0);
	assert(bi == blocks.count() || (bi == blocks.count()-1 && blocks[bi].isaIRB() && blocks[bi].isEndOfBlock()));

	if(bi < blocks.count())
	{
		if(blocks[bi].num_frames) blocks[bi++].compress();
		else { blocks[bi].kill(); blocks.drop(); }
	}

	RzxBlock& block = blocks.grow();
	block.init(RzxBlock::IsaInputRecordingBlock);
	block.startFrame(cc);
	state = Recording;
}


/*	RECORDING: start next Frame
	may start a new block if the current block is getting too large
	note: first block must be a Snapshot block. You can't start recording without first storing a snapshot.

	EndOfFile -> Recording
*/
void RzxFile::startFrame(int32 cc)
{
	assert(isEndOfFile());
	assert(bi>0);
	assert(bi == blocks.count() || (bi == blocks.count()-1 && blocks[bi].isaIRB() && blocks[bi].isEndOfBlock()));

	if( bi == blocks.count()			// z.Zt. kein Block in Arbeit
		|| blocks[bi].num_frames > 1000	// time to split
		|| blocks[bi].ucsize > 50000 )	// time to split
		return startBlock(cc);

	blocks[bi].startFrame(cc);
	state = Recording;
}


/*	RECORDING: finalize Frame

	Recording -> EndOfFile
*/
void RzxFile::endFrame(uint icount)
{
	assert(isRecording());
	assert(bi == blocks.count()-1);

	blocks[bi].endFrame(icount);
	state = EndOfFile;
}


/*	PLAYBACK: goto next Frame
	advance to next frame except if at end of file

	Playing -> Playing | Snapshot

	return:	≥0:	icount
			-1:	state = Snapshot -> call getSnapshot() or
				state = Playing  -> end of file: startRecording() or halt CPU or setOutOfSync()
*/
int RzxFile::nextFrame()
{
	assert(isPlaying());
	assert(bi < blocks.count());

	int icount = blocks[bi].nextFrame();	// -> icount or -1 = EndOfBlock

	while(icount == -1)						// EndOfBlock
	{
		if(bi+1 == blocks.count())			// EndOfFile? => don't move!
		{
			state = Playing;				// EndOfFile
			return -1;
		}

		blocks[bi++].compress();

		if(blocks[bi].isaInputRecordingBlock())
		{
			icount = blocks[bi].uncompress();	// -> icount or -1 = EndOfBlock
		}

		else if(blocks[bi].isaSnapshotBlock())
		{
			state = Snapshot;
			return -1;
		}

		else // if(blocks[bi].isaMachineSnapshot())
		{
			TODO();
		}
	}

	return icount;
}


/*	PLAYBACK: get next input byte

	Playing --> Playing

	return ≥ 0:	input byte from file.
		   -1:	EndOfFrame = OutOfSync.
				if the user wishes to resume playing at this position,
				the caller must startRecording() in the current frame and then
				record the actual input byte with storeInput(byte).
*/
int RzxFile::getInput()
{
	assert(isPlaying());
	assert(bi < blocks.count());

	return blocks[bi].getByte();	// byte or -1 = EndOfFrame = OutOfSync
}


/*	RECORDING: input byte speichern

	Recording --> Recording
*/
void RzxFile::storeInput(uint8 byte)
{
	assert(isRecording());
	assert(bi < blocks.count());

	blocks[bi].storeByte(byte);
}


/*	PLAYBACK: truncate & start recording
	truncate current frame and start recording
	needed to resume playing (==Recording) in potentially incomplete last frame

	Playing -> Recording

	Umschalten auf "Recording":

	1) Benutzer: Der GUI-Thread sieht bei der Wiedergabe immer nur "Playing".
				 "Snapshot" wird in runForSound() sofort abgearbeitet
				 und danach ist der Status wieder "Playing" oder "EndOfFile".
				 Bei "EndOfFile" muss startFrame() benutzt werden.
	2) Automatisch statt OutOfSync oder EndOfFile:
				 Der Status ist "Playing".
	3) Automatisch wenn der letzte Block ein SnapshotBlock war:
				 Der Status ist "EndOfFile" und startFrame() muss benutzt werden.
*/
void RzxFile::startRecording()
{
	assert(isPlaying());
	assert(bi < blocks.count());

	// remove all blocks behind the current block:
	while(blocks.count() > bi+1)
	{
		blocks.last().kill();
		blocks.drop();
	}

	// truncate current block and start recording:
	blocks[bi].startRecording();
	state = Recording;
}


/*	PLAYBACK: is the current frame the last frame?
*/
bool RzxFile::isLastFrame() const
{
	assert(isPlaying());

	return bi == blocks.count()-1 && blocks[bi].isLastFrame();
}


/*	ANY TIME: declare the current state invalid
	invalid mostly means that the machine is OutOfSync.

	state --> OutOfSync
*/
void RzxFile::setOutOfSync()
{
	if(bi<blocks.count() && blocks[bi].isaIRB())
		blocks[bi].compress();

	state = OutOfSync;
}


/*	RECORDING: append the very short current frame to the previous frame

	Recording -> EndOfFile

	Nach einem verzögerten Interrupt soll der aktuelle, sehr kurze Frame-Schnippel
	an den vorherigen Frame angehängt werden.

	At frame start the timer interrupt was not immediately accepted due to DI or similar.
	=> The previous frame was finished and the recording and later the playback machine will not interrupt.
	Few cc later the CPU executed an EI instruction and interrupted on the recording machine.
	The playback machine accepts an interrupt only at the very first instruction of a frame,
	because it does not know exactly how long the recording machine accepted interrupts.
	=> The recording machine must finish the current frame and start a new one,
	so that the playback machine accepts the interrupt at the start of this new frame.

	The few instructions executed so far should be appended to the previous frame,
	so that "frames" in the rzx file and "frames" in the machine match, as far as possible.
	Note: Appending this short frame snippet to the previous frame is actually NOT REQUIRED.
	The file will playback perfectly even if these few instructions are stored in their own "frame".


	Problems:

	• The current frame is the first in a new block and the previous frame is in the previous block.
	Or maybe the previous block is a snapshot block.

	Appending means: uncompress, amend and compress the previous block.
	This is especially unpleasant as we probably just compressed it in the very same sound interrupt.
	So this means a lot of additional work in this sound interrupt and sound may 'click'.
	=> The first frame in a block is never amended to the previous frame.

	• The previous frame is a "repeated frame" and our frame snippet contains input records.

	We must append the inputs from this frame to the previous frame. But we don't know how many
	inputs actually were done in the previous frame, because the count only contains a flag
	to reuse the data of the frame before. So we don't know where to append our inputs.
	=> A frame is never amended to the previous frame if that's a repeated frame.

	• Our frame snippet contains input records (general case)

	If we amend a frame then we append the additional inputs right after the previous frame's data.
	For that we must assume that the number of stored input values in that frame is exactly
	the number of input bytes actually read.
	We might do some optimizations somewhere which break this assumption, e.g. around repeated frames.
	This must not be done, or we cannot combine frame snippets which have inputs at all.
	=> for simplicity frame snippets which contain inputs are not appended too.


	Eventually it's better to avoid frame snippets:

	After a ffb where interrupts are disabled in the CPU,
	runForSound() could inspect the next instruction
		to see whether it's an EI,
			followed by an instruction which ends within INT duration
			or followed by an instruction which ends after INT goes inactive,
		or whether it's another instruction which ends within INT duration
		or whether it's another instruction which ends after INT goes inactive.
	=>	Problem: with some extensions memory paging may occur unexpectedly.
			The Z80 macro can test whether we are on a potentially magic address.
			Extensions which switch pages *before* the opcode is read are rare.
			Extensions in general don't work with RZX recordings.
			This problem could be ignored.
	=>	Then we'd never need to amend a frame.

	Disadvantage:
		During DI recorded frames always start late. (~ 40cc)


	Eventually it's better to record all frame snippets "as is".

		We'll see…
*/
void RzxFile::amendFrame(uint icount)
{
	assert(isRecording());
	assert(icount && icount <= 12);	// 48cc = 12*4

	// is the previous frame in the previous block?
	if(blocks[bi].fpos == 0)
	{
		// Da das Rzx-File auch abspielbar ist, wenn wir den aktuellen Frame-Schnippel NICHT an den vorigen
		// Frame anhängen, und nur num_frames etwas an Aussagekraft verliert, hängen wir den Frame-Schnippel
		// hier eben NICHT an.

		xlogline("RzxFile.amendFrame: could not amend previous frame because it's in the previous block");
		endFrame(icount);
		return;
	}

	// the previous frame is in this block:

	blocks[bi].amendFrame(icount);
}


//static
cstr RzxFile::getFirstSnapshot(cstr filename)
{
	RzxFile rzx;
	try { rzx.readFile(filename,yes); }
	catch(AnyError&) {}

	if(rzx.blocks.count() && rzx.blocks.last().isaSnapshotBlock())
		return rzx.blocks.last().snapshot_filename;
	else
		return NULL;
}
























