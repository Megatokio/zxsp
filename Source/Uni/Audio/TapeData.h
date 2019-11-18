/*	Copyright  (c)	Günter Woigk 2000 - 2019
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


#ifndef TAPEDATA_H
#define	TAPEDATA_H

#include "zxsp_types.h"
#include "DspTime.h"
#include "Templates/Array.h"
#include "IsaObject.h"


/*	class TapeData
	--------------

	"TapeData" is the base class for
	"TapData", "TzxData", "AudioData", "O80Data", etc.

	These classes contain the original data from a tape file
	or converted data from one of the other TapeData classes, or may be empty.

	One "TapeData" instance stores one data block from the tape.
	Some "TapeData" instances and a resulting "CswBuffer" are wrapped into a "TapeFileDataBlock".
	An "Array<>" of "TapeFileDataBlocks" makes a "TapeFile".
	A "TapeFile" is inserted into a "TapeRecorder", which is an "Item" attached to the "Machine".
	The visual representation of a "TapeRecorder" is the "TapeRecorderInspector".
*/


class TapeData : public IsaObject
{
public:
	enum TrustLevel
	{
		no_data,				// data looked like pause or noise only
		conversion_failed,		// decoding failed due to wrong pulse pattern
		truncated_data_error,	// decoding from csw failed at some point
		checksum_error,			// decoded from csw succeeded, but checksum error
		decoded_data,			// decoded from csw without errors, checksum ok
		original_data,			// data from genuine tape file
		conversion_success = truncated_data_error	// minimum for what may be deemed a "success"
	};
	TrustLevel	trust_level;

protected:
					TapeData			(TapeData const&);
	TapeData&		operator=			(TapeData const&) = delete;
	explicit		TapeData			(isa_id, TrustLevel=no_data);

public:
	virtual			~TapeData			();
};


inline TzxData*     TzxDataPtr(TapeData const*p)    { assert(p->isA(isa_TzxData));    return (TzxData*)p; }
inline TapData*     TapDataPtr(TapeData const*p)    { assert(p->isA(isa_TapData));    return (TapData*)p; }
inline O80Data*     O80DataPtr(TapeData const*p)    { assert(p->isA(isa_O80Data));    return (O80Data*)p; }
inline RlesData*    RlesDataPtr(TapeData const*p)   { assert(p->isA(isa_RlesData));   return (RlesData*)p; }
inline AudioData*	AudioDataPtr(TapeData const*p)	{ assert(p->isA(isa_AudioData));	 return (AudioData*)p; }

inline TzxData&     TzxDataRef(TapeData const&p)    { assert(p.isA(isa_TzxData));    return (TzxData&)p; }
inline TapData&     TapDataRef(TapeData const&p)    { assert(p.isA(isa_TapData));    return (TapData&)p; }
inline O80Data&     O80DataRef(TapeData const&p)    { assert(p.isA(isa_O80Data));    return (O80Data&)p; }
inline RlesData&    RlesDataRef(TapeData const&p)   { assert(p.isA(isa_RlesData));   return (RlesData&)p; }
inline AudioData&	AudioDataRef(TapeData const&p)	{ assert(p.isA(isa_AudioData));  return (AudioData&)p; }



#endif

















