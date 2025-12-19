// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Item.h"
#include "VideoData.h"
#include "zxsp_types.h"

namespace zxsp
{

class Crtc : public Item
{
public:
	Crtc(Machine*, isa_id, isa_id grp, Internal, cstr o_addr, cstr i_addr);

	virtual int32 updateScreenUpToCycle(int32 cc)  = 0; // Z80
	virtual void  drawVideoBeamIndicator(int32 cc) = 0; // Machine::runForSound()
	virtual int32 doFrameFlyback(int32 cc)		   = 0; // Machine::runForSound()

	virtual void setBorderColor(uint8 b) { border_color = b; }			  // load .scr
	CoreByte*	 getVideoRam() { return video_ram; }					  // load/save .scr
	uint8		 getBorderColor() const volatile { return border_color; } // save .scr

	ZxspVideoData* getZxspVideoData(VideoData::What, bool aux = no); // Ula
	Zx80VideoData* getZx80VideoData(VideoData::What, bool aux = no); // Ula
	void		   sendVideoData(VideoData*);						 // Ula

	void		 setScreen(IScreen*); // ctor
	virtual void markVideoRam() = 0;  // ctor

	IScreen*  screen	   = nullptr;
	CoreByte* video_ram	   = nullptr; // current video ram
	uint8	  border_color = 0;		  // current border color
};


//
// ----------------------------------
//		Inline Implementations
// ----------------------------------
//

inline void Crtc::setScreen(IScreen* newscreen)
{
	screen = newscreen;
	if (screen) markVideoRam();
}

inline ZxspVideoData* Crtc::getZxspVideoData(VideoData::What what, bool aux)
{
	assert(screen);
	VideoData* z = screen->getVideoData(what, aux);
	assert(dynamic_cast<ZxspVideoData*>(z));
	return static_cast<ZxspVideoData*>(z);
}

inline Zx80VideoData* Crtc::getZx80VideoData(VideoData::What what, bool aux)
{
	assert(screen);
	VideoData* z = screen->getVideoData(what, aux);
	assert(dynamic_cast<Zx80VideoData*>(z));
	return static_cast<Zx80VideoData*>(z);
}

inline void Crtc::sendVideoData(VideoData* data)
{
	assert(screen);
	screen->sendVideoData(data);
}


} // namespace zxsp


/*



















*/
