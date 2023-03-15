// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*	AY-3-8912 sound chip emulation
	------------------------------


$FFFD	%1111.----.----.--0-	ZX128 AY reg. select/read i/o
$BFFD	%1011.----.----.--0-	ZX128 AY reg. write -/o


	24.Mar.1995 kio	First revision on MacOS
	05.Jun.1995 kio	Combined ear and mic sound output
	29.Nov.1995 kio	AY-3-8912 emulation
	05.Jan.1999 kio	port to Linux
	09.jan.1999 kio	ay emu fixed, perfect sound sampling
	31.dec.1999 kio	started port back to MacOS
	13.jul.2000 kio implemented signals
*/


/* ==================================================================
		Hardware independent part
================================================================== */

#define LOGLEVEL 1
#include "Ay.h"
#include "Machine.h"
#include "ZxInfo/ZxInfo.h"

#undef BIT
#define BIT(N, B) (((N) >> (B)) & 1) // get bit value at bit position B


// logarithmic volume table:
static Sample logVol[16];


// valid bits masks:
cuint8 ayRegMask[16] = // exising bits mask
	{
		0xff, 0x0f, 0xff, 0x0f, 0xff,
		0x0f, // channel A,B,C period
		0x1f, // noise period
		0xff, // mixer control
		0x1f, 0x1f,
		0x1f, // channel A,B,C volume
		0xff, 0xff,
		0x0f, // envelope period and shape
		0xff,
		0xff // i/o ports
};


// registers after reset:
const uchar AYregReset[] = {
	0xff,		0x0f,		0xff, 0x0f, 0xff,
	0x0f, // channel A,B,C period
	0x1f, // noise period
	0x3f, // mixer control
	0x00,		0x00,
	0x00, // channel A,B,C volume
	0 /*0xff*/, 0 /*0xff*/,
	0x00, // envelope period and shape	note: some sq-tracker demos assume registers=0 after reset
	0xff,
	0xff // i/o ports
};


/* ===============================================================
		amplifier stage of a channel
=============================================================== */


/* ----	de/select envelope for this channel --------------------
 */
inline void Channel::useEnvelope(bool env)
{
	use_envelope = env;
	volume		 = env ? envelopevolume : soundvolume;
	if (amp_in) output = volume;
}


/* ----	set volume and select envelope --------------------------
		in: value written to the channel's volume register
		each channel has it's own amplifier.
		changing the volume takes effect immediately
		bits 0-3 define the channel's volume, which is stepped logarithmically
		and bit 4 overrides bits 0-3 selecting the envelope generator's volume
*/
void Channel::setVolume(uchar n)
{
	soundvolume = logVol[n & 0x0f];
	useEnvelope(n & 0x10);
}


/* ----	set envelope volume -------------------------------------
		in: new sample value of envelope output
		immediately sets new volume if envelope is enabled
*/
inline void Channel::setEnvelopeSample(Sample n)
{
	envelopevolume = n;
	if (use_envelope)
	{
		volume = n;
		if (amp_in) output = n;
	}
}


/* ----	set envelope volume -------------------------------------
		in: new volume of envelope output
		immediately sets new volume if envelope is enabled
*/
inline void Channel::setEnvelopeVolume(uchar n) { setEnvelopeSample(logVol[n]); }


/* ----	change state of binary sound input ----------------------
 */
inline void Channel::ampInput(bool in)
{
	amp_in = in;
	output = in ? volume : 0;
}


/* ----	get output of amplifier stage ---------------------------
 */
inline Sample Channel::getOutput() { return output; }


/* ===============================================================
		mixer stage of a channel
=============================================================== */


/* ----	enable sound generation ----------------------------------
		in: !(bit_from_mixer_control_register)

		note:	in a real ay chip, sound output to the amplifier is
				calculated from the mixer inputs like this:
			=>	output = (disable_sound||sound_input) && (disable_noise||noise_input)
				in this emulation we use the equivalent calculation:
			=>	output = (!enable_sound||sound_input) && (!enable_noise||noise_input)
*/
void Channel::enableSound(bool f)
{
	sound_enable = f;
	sound_out	 = sound_in || !f;
	ampInput(sound_out && noise_out);
}


/* ----	enable noise generation ----------------------------------
		in: !(bit_from_mixer_control_register)
*/
void Channel::enableNoise(bool f)
{
	noise_enable = f;
	noise_out	 = noise_in || !f;
	ampInput(sound_out && noise_out);
}


/* ----	change input from noise generator -------------------------
 */
inline void Channel::mixNoiseInput(bool in)
{
	noise_in = in;

	if (noise_enable)
	{
		noise_out = in;
		ampInput(sound_out && noise_out);
	}
}


/* ----	change input from sound generator -------------------------
 */
inline void Channel::mixSoundInput(bool in)
{
	sound_in = in;

	if (sound_enable)
	{
		sound_out = in;
		ampInput(sound_out && noise_out);
	}
}


/* ===============================================================
		sound generator stage of a channel
=============================================================== */


/* ----	reset the whole channel ----------------------------------
		reset sound generator, mixer and amplifier
*/
void Channel::reset(Time now)
{
	// reset counter stage
	sound_in = low;
	fine	 = 0xff;
	coarse	 = 0x0f;
	reload	 = 0x0fff * time_for_cycle;
	when	 = now + reload;

	// reset mixer stage
	enableSound(false);
	enableNoise(false);

	// reset amplifier stage
	setVolume(0); // volume=0 & envelope off
	setEnvelopeSample(0);
}


/* ----	creator ---------------------------------------------------
		disable channel
		reset all stages
*/
Channel::Channel(Time time_for_cycle)
{
	this->time_for_cycle = time_for_cycle * 8; // incl. predivider 1/16, but flip every 1/2 cycle on/off !!
	reset(0.0);
}


/* ----	change a channel's pitch ----------------------------------
		the ay input clock is first divided by 16. then this clock is fed into a counter
		which's output is the mixer's "Soundin" state for this channel.
		programming a pitch of 0 seems to have the same effect as a pitch of 1
		each counter has 12 significant bits.

		it is not known when a new value takes effect:
		repeatedly programming the same value has no effect on the counter
		it may be a count down which restores the reload value when it reaches/underflows 0
		it may be a count up which is reset to 0 when it reaches/overlows the reload value

		the current implementation assumes that it is a count down and that the
		new value takes effect when the counter is reload
*/
void Channel::setFinePeriod(uchar n)
{
	fine   = n;
	reload = max(1, (coarse * 256 + n)) * time_for_cycle;
}


void Channel::setCoarsePeriod(uchar n)
{
	n &= 0x0f;
	coarse = n;
	reload = max(1, (n * 256 + fine)) * time_for_cycle;
}


void Channel::trigger()
{
	mixSoundInput(!sound_in);
	when += reload;
}


/* ===============================================================
			noise generator
=============================================================== */


/* ----	reset the noise generator --------------------------------
 */
void Noise::reset(Time now)
{
	const bool s = high;

	reload	 = 0x1f * time_for_cycle;
	when	 = now + reload;
	shiftreg = 0x0001FFFF;
	A->mixNoiseInput(s);
	B->mixNoiseInput(s);
	B->mixNoiseInput(s);
}


/* ----	creator ---------------------------------------------------
		since the noise generator actively propagates it's output state
		to the sound channels, it needs to know where they are
*/
Noise::Noise(Time time_for_cycle, Channel* a, Channel* b, Channel* c)
{
	this->time_for_cycle = time_for_cycle * 16; // incl. predivider 1/16
	A					 = a;
	B					 = b;
	C					 = c;
	reset(0.0);
}


/* ----	set noise counter period -----------------------------------
		in: Value written to noise period register

		there is 1 noise generator. it's output may be mixed to any channel.
		the ay input clock is first divided by 16.
		then this clock is fed into a programmable counter.
		this counter has 5 significant bits.
		programming a pitch of 0 seems to have the same effect as a pitch of 1.
		the counter's output is fed into the clock input of a random generator,
		which's output is the mixer's "Noisein" state for every channel.

		The random generator of the ay chip is a 17-bit shift register.
		The input to the shift register is bit0 xor bit2 (where bit0 is the output).

		the implementation of counters seems to be very similar for channels and noise.
		so the following is the same as for channels:

		it is not known when a new value takes effect:
		repeatedly programming the same value has probably no effect on the counter
		it may be a count down which restores the reload value when it reaches/underflows 0
		it may be a count up which is reset to 0 when it reaches/overlows the reload value

		the current implementation assumes that it is a count down and that the
		new value takes effect when the counter is reload

		note: the MAME sources say that it is a count up
*/
inline void Noise::setPeriod(uchar n)
{
	n &= 0x1f;
	n += !n;
	reload = n * time_for_cycle;
}


/* ----	Trigger noise counter --------------------------------------
		Trigger() must be called when according to When() the counter expired
*/
void Noise::trigger()
{
	// flip the gates
	int32 oldshiftreg = shiftreg;
	shiftreg		  = (oldshiftreg >> 1) + (((oldshiftreg << 16) ^ (oldshiftreg << 14)) & 0x10000);
	when += reload;

	// change in output state ?
	if ((shiftreg ^ oldshiftreg) & 1)
	{
		bool s = shiftreg & 1;
		A->mixNoiseInput(s);
		B->mixNoiseInput(s);
		C->mixNoiseInput(s);
	}
}


/* ===============================================================
			envelope generator
=============================================================== */


/* ----	internal representation of the sound chip's state -------------------------

		envelope generator:
		there is 1 envelope generator.
		envelope shape is controlled by attack, invert and hold.
		the 'repeat' bit (D3 of reg. 13) is not used and always treated as set:
		therefore %00xx is converted to %1001 and %01xx is converted to %1111
		which give the same results.

		D3 = repeat
		D2 = attack
		D1 = toggle
		D0 = hold

		1111, 01xx:	/_____	 repeat &  attack &  toggle &  hold	||  !repeat & attack
		1110:		/\/\/\	 repeat &  attack &  toggle & !hold
		1101:		/~~~~~	 repeat &  attack & !toggle &  hold
		1100:		//////	 repeat &  attack & !toggle & !hold
		1011:		\~~~~~	 repeat & !attack &  toggle &  hold
		1010:		\/\/\/	 repeat & !attack &  toggle & !hold
		1001, 00xx:	\_____	 repeat & !attack & !toggle &  hold	||  !repeat & !attack
		1000:		\\\\\\	 repeat & !attack & !toggle & !hold

		the envelope generator is running free and only retriggered
		when a new envelope is selected.
*/


/* ----	reset the envelope generator -------------------------------------------
 */
void Envelope::reset(Time now)
{
	reload = 0x0000ffff * time_for_cycle;
	when   = now + reload;
	fine = coarse = 0xff;
	index		  = 0;
	repeat = invert = off;
	direction		= 0;

	A->setEnvelopeSample(0);
	B->setEnvelopeSample(0);
	C->setEnvelopeSample(0);
}


/* ----	creator ---------------------------------------------------------------
		since the envelope generator actively propagates it's output state
		to the sound channels, it needs to know where they are
*/
Envelope::Envelope(Time time_for_cycle, Channel* a, Channel* b, Channel* c)
{
	this->time_for_cycle = time_for_cycle * 16; // incl. predivider 1/16
	A					 = a;
	B					 = b;
	C					 = c;
	reset(0.0);
}


/* ----	set counter low byte -------------------------------------------------
		in: byte written to envelope fine period register
*/
inline void Envelope::setFinePeriod(Time now, uchar n)
{
	fine = n;
	if (direction)
	{
		reload = max(1, (int32(coarse) * 256 + n)) * time_for_cycle;
		if (when > now + reload) when = now + reload;
	}
}


/* ----	set counter high byte -------------------------------------------------
		in: byte written to envelope coarse period register
*/
inline void Envelope::setCoarsePeriod(Time now, uchar n)
{
	coarse = n;
	if (direction)
	{
		reload = max(1, (int32(n) * 256 + fine)) * time_for_cycle;
		if (when > now + reload) when = now + reload;
	}
}


/* ----	select new envelope shape -------------------------------------------
		in: byte written to the envelope control register
		this automatically restarts the envelope generator
*/
void Envelope::setShape(Time now, uchar c)
{
	if (!(c & 8)) c = c & 4 ? 0x0f : 0x09;

	index	  = BIT(c, 2) ? 0 : 15;
	direction = BIT(c, 2) ? 1 : -1;
	invert	  = BIT(c, 1);
	repeat	  = !BIT(c, 0);

	reload = max(1, (int32(coarse) * 256 + fine)) * time_for_cycle;
	when   = now + reload;

	Sample v = logVol[index];
	A->setEnvelopeSample(v);
	B->setEnvelopeSample(v);
	C->setEnvelopeSample(v);
}


/* ---- trigger the envelope counter ---------------------------------------
		Trigger() must be called when according to When() the counter expired
*/
void Envelope::trigger()
{
	index += direction;

	if (index & 0xF0) // ramp finished
	{
		if (repeat)
		{
			if (invert)
			{
				index	  = ~index;
				direction = -direction;
			}
		}
		else
		{
			direction = 0;
			if (!invert) index = ~index;
			reload = 0xffff * time_for_cycle;
		}
		index &= 0x0F;
	}

	when += reload;

	Sample v = logVol[index];
	A->setEnvelopeSample(v);
	B->setEnvelopeSample(v);
	C->setEnvelopeSample(v);
}


/* ===============================================================
			ay sound chip
=============================================================== */

/* ----	standard constructor -----------------------------------
 */
Ay::Ay(Machine* m, isa_id id, Internal internal, cstr s, cstr w, cstr r, Frequency freq, StereoMix mix) :
	Item(m, id, isa_Ay, internal, And(w, s), r),
	select_mask(maskForSpec(s)),
	select_bits(bitsForSpec(s)),
	write_mask(maskForSpec(w)),
	write_bits(bitsForSpec(w)),
	time_for_cycle(1 / freq),
	stereo_mix(mix),
	channel_A(1 / freq),
	channel_B(1 / freq),
	channel_C(1 / freq),
	noise(1 / freq, &channel_A, &channel_B, &channel_C),
	envelope(1 / freq, &channel_A, &channel_B, &channel_C)
{
	xlogIn("new Ay");

	logVol[0]  = 0.0;
	logVol[15] = 1.0;
	for (uint i = 14; i >= 1; i--) logVol[i] = logVol[i + 1] * 0.8f; // org: ~ 3.5 dB
#if XXLOG
	{
		logIn("AY volume table:");
		for (uint i = 0; i < 15; i++) logline("%f", double(logVol[i]));
	}
#endif

	// security setup:
	ay_reg_nr			= 0;
	time_of_last_sample = 0;
	setVolume(1.0);
}

/* ----	destructor ----------------------------------------
 */
Ay::~Ay() {}


/* ----	power-up reset ------------------------------------
		caveat: other items may not yet be initialized!
*/
void Ay::powerOn(/*t=0*/ int32 cc)
{
	Item::powerOn(cc);
	channel_A.reset(0.0);
	channel_B.reset(0.0);
	channel_C.reset(0.0);
	noise.reset(0.0);
	envelope.reset(0.0);
	memcpy(ay_reg, AYregReset, 16);
	ay_reg_nr			= 0;
	time_of_last_sample = 0.0;

	setVolume(1.0);
}

void Ay::reset(Time t, int32 cc)
{
	Item::reset(t, cc);

	run_until(t);
	channel_A.reset(time_of_last_sample);
	channel_B.reset(time_of_last_sample);
	channel_C.reset(time_of_last_sample);
	noise.reset(time_of_last_sample);
	envelope.reset(time_of_last_sample);
	memcpy(ay_reg, AYregReset, 16);
	ay_reg_nr = 0;
}


void Ay::run_until(Time when)
{
	Time now;
	int	 who;

	for (;;)
	{
		// who is next ?
		{
			who = 6;
			now = when;
		} // new command is next

		if (noise.when - now < 0)
		{
			if (~ay_reg[7] & 0x38) // for speed...
			{
				who = 1;
				now = noise.when;
			} // noise is next
			else noise.when = now + 1000 * time_for_cycle;
		}
		if (channel_A.when - now < 0)
		{
			if (ay_reg[8] && ~ay_reg[7] & 0x09)
			{
				who = 3;
				now = channel_A.when;
			} // channel A is next
			else channel_A.when = now + 1000 * time_for_cycle;
		}
		if (channel_B.when - now < 0)
		{
			if (ay_reg[9] && ~ay_reg[7] & 0x12)
			{
				who = 4;
				now = channel_B.when;
			} // channel B is next
			else channel_B.when = now + 1000 * time_for_cycle;
		}
		if (channel_C.when - now < 0)
		{
			if (ay_reg[10] && ~ay_reg[7] & 0x24)
			{
				who = 5;
				now = channel_C.when;
			} // channel C is next
			else channel_C.when = now + 1000 * time_for_cycle;
		}
		if (envelope.when - now < 0)
		{
			if (channel_A.usesEnvelope() || channel_B.usesEnvelope() || channel_C.usesEnvelope()) // for speed...
			{
				who = 2;
				now = envelope.when;
			} // envelope is next
			else envelope.when = now + 1000 * time_for_cycle;
		}

		// write samples up to 'now' to DSP:
		if (now > time_of_last_sample)
		{
			StereoSample current_value;

			switch (stereo_mix)
			{
			case mono: // mono: ZX 128k, +2, +3, +3A, TS2068, TC2068
				current_value = volume * (channel_A.getOutput() + channel_B.getOutput() + channel_C.getOutput());
				break;
			case abc_stereo: // western Europe
				current_value.left	= volume * (channel_A.getOutput() * 2 + channel_B.getOutput());
				current_value.right = volume * (channel_C.getOutput() * 2 + channel_B.getOutput());
				break;
			case acb_stereo: // eastern Europe, Didaktik Melodik
				current_value.left	= volume * (channel_A.getOutput() * 2 + channel_C.getOutput());
				current_value.right = volume * (channel_B.getOutput() * 2 + channel_C.getOutput());
				break;
			}

			machine->outputSamples(current_value, time_of_last_sample, now);
			time_of_last_sample = now;
		}

		// handle next trigger event:
		switch (who)
		{
		case 1: noise.trigger(); break;
		case 2: envelope.trigger(); break;
		case 3: channel_A.trigger(); break;
		case 4: channel_B.trigger(); break;
		case 5: channel_C.trigger(); break;
		default: return; // requested timestamp reached
		}
	}
}


void Ay::set_register(Time when, uint regnr, uint8 newvalue)
{
	// set register to new value
	// the AY is *not* run, 'when' is only used for port and envelope setters.
	// if you want to set the register 'now', use 'time_of_last_sample' for 'when'.

	regnr &= 0x0F;
	newvalue &= ayRegMask[regnr];

	//	if (AYreg[regnr]==newvalue && regnr!=13) break;		---> eel_demo !!!
	switch (regnr)
	{
	case 0: channel_A.setFinePeriod(newvalue); break;
	case 1: channel_A.setCoarsePeriod(newvalue); break;
	case 2: channel_B.setFinePeriod(newvalue); break;
	case 3: channel_B.setCoarsePeriod(newvalue); break;
	case 4: channel_C.setFinePeriod(newvalue); break;
	case 5: channel_C.setCoarsePeriod(newvalue); break;
	case 6: noise.setPeriod(newvalue); break;
	case 7:
	{
		uint8 c = ~newvalue;
		uint8 t = newvalue ^ ay_reg[7];
		if (t & 1) channel_A.enableSound(c & 1);
		if (t & 8) channel_A.enableNoise(c & 8);
		if (t & 2) channel_B.enableSound(c & 2);
		if (t & 16) channel_B.enableNoise(c & 16);
		if (t & 4) channel_C.enableSound(c & 4);
		if (t & 32) channel_C.enableNoise(c & 32);
		if (t & 0xc0)
		{
			if (t & 0x40 && ay_reg[14] != 0xff) portAOutputValueChanged(when, newvalue & 0x40 ? ay_reg[14] : 0xff);
			if (t & 0x80 && ay_reg[15] != 0xff) portBOutputValueChanged(when, newvalue & 0x80 ? ay_reg[15] : 0xff);
		}
	}
	break;
	case 8: channel_A.setVolume(newvalue); break;
	case 9: channel_B.setVolume(newvalue); break;
	case 10: channel_C.setVolume(newvalue); break;
	case 11: envelope.setFinePeriod(when, newvalue); break;
	case 12: envelope.setCoarsePeriod(when, newvalue); break;
	case 13: envelope.setShape(when, newvalue); break;
	case 14:
		if (ay_reg[7] & 0x40 && ay_reg[14] != newvalue) portAOutputValueChanged(when, newvalue);
		break;
	case 15:
		if (ay_reg[7] & 0x80 && ay_reg[15] != newvalue) portBOutputValueChanged(when, newvalue);
		break;
	}
	ay_reg[regnr] = newvalue;
}


void Ay::setRegister(Time when, uint regnr, uint8 newvalue)
{
	run_until(when);
	set_register(when, regnr, newvalue);
}


void Ay::output(Time t, int32 /*cc*/, uint16 addr, uint8 byte)
{
	if ((addr & select_mask) == select_bits) ay_reg_nr = byte & 0x0f;
	else if ((addr & write_mask) == write_bits)
	{
		xlogline("Ay:Output: register %i = $%02x", int(ay_reg_nr), uint(byte));
		setRegister(t, ay_reg_nr, byte);
	}
}


void Ay::input(Time t, int32 /*cc*/, uint16 addr, uint8& byte, uint8& mask)
{
	xlogline("Ay:input: register %i = $%02x", int(ay_reg_nr), uint(ay_reg[ay_reg_nr]));

	if (ay_reg_nr < 14) byte &= ay_reg[ay_reg_nr];
	else if (ay_reg_nr == 14)
	{
		byte &= getInputValueAtPortA(t, addr); // note: addr is req. for TS2068
		if (ay_reg[7] & 0x40) byte &= ay_reg[14];
	}
	else
	{
		byte &= getInputValueAtPortB(t, addr);
		if (ay_reg[7] & 0x80) byte &= ay_reg[15];
	}

	mask = 0xff;
}


void Ay::audioBufferEnd(Time t)
{
	run_until(t);
	time_of_last_sample -= t;
	channel_A.when -= t;
	channel_B.when -= t;
	channel_C.when -= t;
	noise.when -= t;
	envelope.when -= t;
}


/*	Eingangsfrequenz ändern.
 */
void Ay::setClock(Frequency psg_cycles_per_second)
{
	double freq_factor = psg_cycles_per_second * time_for_cycle;
	time_for_cycle	   = 1 / psg_cycles_per_second;
	Time now		   = time_of_last_sample;

	channel_A.when = now + (channel_A.when - now) / freq_factor;
	channel_A.time_for_cycle /= freq_factor;
	channel_A.reload /= freq_factor;
	channel_B.when = now + (channel_B.when - now) / freq_factor;
	channel_B.time_for_cycle /= freq_factor;
	channel_B.reload /= freq_factor;
	channel_C.when = now + (channel_C.when - now) / freq_factor;
	channel_C.time_for_cycle /= freq_factor;
	channel_C.reload /= freq_factor;
	noise.when = now + (noise.when - now) / freq_factor;
	noise.time_for_cycle /= freq_factor;
	noise.reload /= freq_factor;
	envelope.when = now + (envelope.when - now) / freq_factor;
	envelope.time_for_cycle /= freq_factor;
	envelope.reload /= freq_factor;
}
