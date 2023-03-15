#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "StereoSample.h"

//
// ay-3-8912 emulation
// hardware independent part
//

extern const uint8 ayRegMask[16];
const bool		   low	= false;
const bool		   high = true;


/* ----	the sound channels of the ay chip ---------------------------------------------
 */
class Channel
{
	// ----	the amplifier stage of a channel ----------------------------------------------
private:
	Sample soundvolume;	   // volume of this channel if not use envelope
	Sample envelopevolume; // volume output of envelope generator
	Sample volume;		   // sound or envelope
	Sample output;		   // volume or 0
	bool   use_envelope;
	bool   amp_in;

	void ampInput(bool);

public:
	void   setEnvelopeVolume(uchar);
	void   setEnvelopeSample(Sample);
	void   useEnvelope(bool);
	void   setVolume(uchar);
	Sample getOutput();
	bool   usesEnvelope() { return use_envelope; }
	Sample getVolume() { return volume; }

	// ----	the mixer stage of a channel --------------------------------------------------
private:
	bool sound_enable; // from mixer control register
	bool sound_in;	   // from sound generator
	bool sound_out;	   // output of gate: (sound_in||!sound_enable)
	bool noise_enable; // from mixer control register
	bool noise_in;	   // from noise generator
	bool noise_out;	   // output of gate: (noise_in||!noise_enable)

	void mixSoundInput(bool);

public:
	void mixNoiseInput(bool);
	void enableSound(bool);
	void enableNoise(bool);

	// ----	the sound generator stage of a channel ---------------------------------------
private:
	Time  time_for_cycle;
	Time  reload;		// reload value
	Time  when;			// next tick to toggle state
	uchar fine, coarse; // real calues for reload register

public:
	Channel(Time time_for_cycle);

	void reset(Time now);
	void setFinePeriod(uchar);
	void setCoarsePeriod(uchar);
	void trigger();

	friend class Ay;
};


/* ----	the noise generator of the ay chip -------------------------------------------
 */
class Noise
{
	Time	 time_for_cycle;
	Time	 reload;	// reload value
	Time	 when;		// next tick to toggle output
	int32	 shiftreg;	// random number shift register; bit 0 is output
	Channel *A, *B, *C; // propagate output state

public:
	Noise(Time time_for_cycle, Channel*, Channel*, Channel*);

	void reset(Time now);
	void setPeriod(uchar);
	void trigger();

	friend class Ay;
};


/* ----	the envelope generator of the ay chip ----------------------------------------
 */
class Envelope
{
	Time	 time_for_cycle;
	Time	 reload;	   // reload value
	Time	 when;		   // next tick to inc/dec volume
	uchar	 fine, coarse; // real values for reload register
	int		 index;		   // current output state: volume 0 ... 15
	bool	 repeat;	   // repeat ramping
	bool	 invert;	   // toggle ramping direction
	int		 direction;	   // ramping up/down/off
	Channel *A, *B, *C;	   // propagate output state

public:
	Envelope(Time time_for_cycle, Channel*, Channel*, Channel*);

	void reset(Time now);
	void setShape(Time now, uchar);
	void setFinePeriod(Time now, uchar);
	void setCoarsePeriod(Time now, uchar);
	void trigger();

	friend class Ay;
};


/* ----	class for ay sound chip ---------------
 */
class Ay : public Item
{
public:
	enum StereoMix { mono, abc_stereo, acb_stereo };

protected:
	uint16	  select_mask;
	uint16	  select_bits;
	uint16	  write_mask;
	uint16	  write_bits;
	Time	  time_for_cycle;
	StereoMix stereo_mix;

	// AY sound chip registers:
	uint  ay_reg_nr;  // selected register
	uint8 ay_reg[16]; // 16 registers/ports of the AY chip
					  // ports: values written to that register

	// internal state:
	Channel	 channel_A, channel_B, channel_C;
	Noise	 noise;
	Envelope envelope;

	// sample output:
	// progress:
	Sample volume;				// per channel: 0.0 .. 0.33
	Time   time_of_last_sample; // bis zu diesem Zeitpunkt wurde schon Sound erzeugt

protected:
	Ay(Machine*, isa_id, Internal, cstr sel, cstr wr, cstr rd, Frequency, StereoMix);
	Ay(Machine* m, cstr s, cstr w, cstr r, Frequency f, StereoMix x) : Ay(m, isa_InternalAy, internal, s, w, r, f, x) {}
	~Ay() override;

	// notification for attached hardware:
	// called on audio thread
	virtual void portAOutputValueChanged(Time, uint8) {} // \ portXOutputValue() still returns the old value.
	virtual void portBOutputValueChanged(Time, uint8) {} // / register 14 or 15 was modified or data direction changed.
														 // callback: get input at port pins:
														 // called on audio thread
	virtual uint8 getInputValueAtPortA(Time, uint16) { return 0xff; }
	virtual uint8 getInputValueAtPortB(Time, uint16) { return 0xff; }

public:
	// set & read register
	void setRegister(uint r, uint8 n) { set_register(time_of_last_sample, r, n); }
	void setRegisters(uint8 n[16])
	{
		for (uint i = 0; i < 16; i++) setRegister(i, n[i]);
	}
	void setRegNr(uint n) { ay_reg_nr = n & 0x0f; }

	const uint8* getRegisters() const { return ay_reg; } // ports: \ value written to this register
	uint8		 getRegister(uint n) const volatile
	{
		return ay_reg[n & 0x0f];
	} // ports: / not neccessarily what the cpu reads
	uint getRegNr() const volatile { return ay_reg_nr; }

	const volatile uint8* getRegisters() const volatile { return ay_reg; }

	// set register at runtime:
	void run_until(Time);
	void set_register(Time, uint r, uint8 n);
	void setRegister(Time, uint r, uint8 n);

	// volume & clock:
	void	  setVolume(Sample v) { volume = v * 0.33f; }
	void	  setClock(Frequency psg_cycles_per_second);
	Frequency getClock() const volatile { return 1.0 / time_for_cycle; }

	// set stereo/mono:
	void	  setStereoMix(StereoMix n) { stereo_mix = n; }
	StereoMix getStereoMix() const volatile { return stereo_mix; }

	// ports:
	// get output at port pins:
	uint8 getOutputValueAtPortA() { return ay_reg[7] & 0x40 ? ay_reg[14] : 0xff; }
	uint8 getOutputValueAtPortB() { return ay_reg[7] & 0x80 ? ay_reg[15] : 0xff; }

protected:
	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time, int32 cc) override;
	void input(Time, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time, int32 cc, uint16 addr, uint8 byte) override;
	void audioBufferEnd(Time) override;
	// void	videoFrameEnd	(int32 cc) override;
};
