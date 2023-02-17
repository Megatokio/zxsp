#pragma once
// Copyright (c) 2016 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Libraries/unix/FD.h"
#include "RzxBlock.h"
#include "Templates/Array.h"
#include "ZxInfo/ZxInfo.h"
#include "kio/kio.h"
#include <zlib.h>

#define OurRzxLibraryVersion 0x000C // rzx file version we create
#define MaxRzxLibraryVersion 0x000D // some changes in encryption (we don't use)


// Gameplay recording data
class RzxFile
{
public:
	// file and file creator info:
	cstr   filename;
	cstr   creator_name;
	uint16 creator_major_version;
	uint16 creator_minor_version;
	uint16 rzx_file_version;

	// input recording blocks:
	Array<RzxBlock> blocks; // input recording blocks
	uint			bi;		// current index in blocks[]

	enum State {
		// state:
		EndOfFile = 0,	 // == RzxBlock::EndOfBlock
		Playing,		 // == RzxBlock::Playing
		Recording,		 // == RzxBlock::Recording
		Snapshot,		 // current block is a SnapshotBlock
		MachineSnapshot, // current block is a Zxsp Machine Snapshot
		OutOfSync,		 // machine no longer in sync with file; broken file after loadFile()
	};

	State state;

public:
	RzxFile();
	~RzxFile();
	RzxFile(const RzxFile&)			   = delete;
	RzxFile& operator=(const RzxFile&) = delete;

	/*	state:

		OutOfSync:	Das File wurde mit setOutOfSync() als OutOfSync gekennzeichnet.
					Nach readFile(): Das File enthielt Fehler und ist nicht abspielbar.
					-> purge(), readFile()
					-> rewind() macht das File evtl. bis zur Fehlerposition spielbar.

		Playing:	Die Abspielposition ist in einem Block und da in einem Frame.
					Die Maschine liest Inputs bis zum Erreichen des Icount.
					Dann ruft sie nextFrame() auf, was den nächsten Icount zurück liefert.

		Snapshot:	Spezieller Zustand beim Abspielen:
					Der aktuelle Block ist ein Snapshot-Block und der entsprechende Snapshot muss geladen werden.
					Nach dem Lesen des Dateinamens mit getSnapshot() schaltet der Status wieder auf "Playing".

		Recording:	Mit startFrame() schaltet man am "EndOfFile" auf "Recording".
					Danach ruft man N * writeByte() auf und am Ende des Frames endFrame().
					Mit startRecording() schaltet man jederzeit von "Playing" auf "Recording".

		EndOfFile:	Spezieller Zustand bei der Aufnahme:
					z.B. nach purge() oder endFrame().
					Mit startFrame() kann der nächste Frame aufgezeichnet werden.
					-> rewind(), startFrame() oder storeSnapshot().


		new RzxData					--> EndOfFile
		purge			any			--> EndOfFile
		rewind			any			--> EndOfFile | Snapshot | OutOfSync
		readFile		any			--> Snapshot | OutOfSync
		writeFile		any

		storeSnapshot	EndOfFile	--> EndOfFile
		startBlock		EndOfFile	--> Recording
		startFrame		EndOfFile	--> Recording
		endFrame		Recording	--> EndOfFile
		storeInput		Recording	--> Recording

		getSnapshot		Snapshot	--> Playing | EndOfFile
		nextFrame		Playing		--> Playing | Snapshot
		getInput		Playing		--> Playing
		getCC			Playing		--> Playing
		getIcount		Playing		--> Playing
		startRecording	Playing		--> Recording
	*/

	bool isPlaying() const { return state == Playing; }
	bool isSnapshot() const { return state == Snapshot; }
	bool isRecording() const { return state == Recording; }
	bool isOutOfSync() const { return state == OutOfSync; }
	bool isEndOfFile() const { return state == EndOfFile; }
	bool isLastFrame() const;

	void setOutOfSync(); // OutOfSync.					any time
	void purge();		 // alle Daten löschen.			any time
	void rewind();		 // Datei zurückspulen.			any time

	void readFile(cstr filename, bool snapshotOnly = no);			// data_error,file_error		// any time
	void writeFile(cstr filename);									// any time
	void writeFileUpToBlockAndFrame(cstr filename, uint32, uint32); // TODO
	void writeFileUpToCurrentPosition(cstr filename);				// TODO

	cstr  getSnapshot(); // Snapshot Dateipfad lesen		Snapshot -> Playing | EndOfFile
	int	  nextFrame();	 // nächsten Frame starten		Playing -> Playing
	int	  getInput();	 // input lesen					Playing
	int	  getIcount();	 // instr count für Frame lesen	Playing
	int32 getStartCC();	 // CC am Frame-Start lesen		Playing

	void storeSnapshot(cstr);	  // Snapshot speichern			EndOfFile
	void startBlock(int32 cc);	  // neuen Block starten			EndOfFile -> Recording
	void startFrame(int32 cc);	  // neuen Frame starten			EndOfFile -> Recording
	void endFrame(uint icount);	  // Frame abschließen			Recording -> EndOfFile
	void storeInput(uint8 byte);  // input speichern				Recording
	void startRecording();		  // truncate & start recording	Playing -> Recording
	void amendFrame(uint icount); // add current to prev. frame	Recording -> EndOfFile

	static cstr getFirstSnapshot(cstr filename);

private:
	void init();
	void kill();
};
