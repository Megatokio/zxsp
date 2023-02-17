// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*	class AudioData
	----------------

	mono or stereo audio data block
	int16 or float precision
*/

#include "AudioData.h"
#include "CswBuffer.h"
#include "DspTime.h"
#include "StereoSample.h"
#include "TapData.h"
#include "TapeFileDataBlock.h"
#include "TzxData.h"
#include "audio/AudioDecoder.h"
#include "globals.h"
#include <cmath>


#define dB05 18426 // 32767 / sqrt(10^0.5)
#define dB10 10362 // 32767 / sqrt(10^1)
#define dB15 5827  // 32767 / sqrt(10^1.5)
#define dB20 3277  // 32767 / sqrt(10^2)

#define dB25 1843 // 32767 / sqrt(10^2.5)
#define dB30 1036 // 32767 / sqrt(10^3)
#define dB35 583  // 32767 / sqrt(10^3.5)
#define dB40 328  // 32767 / sqrt(10^4)

#define dB45 184 // 32767 / sqrt(10^4.5)
#define dB50 104 // 32767 / sqrt(10^5)
#define dB55 58	 // 32767 / sqrt(10^5.5)
#define dB60 33	 // 32767 / sqrt(10^6)


// audio classification:
//
enum { silence, music, data };


// define range of audio with classification:
//
struct AudioRange
{
	uint32 start;
	uint32 end;
	uint32 pulses;
	uint   art;

	AudioRange(uint32 a, uint32 e, uint32 n, uint art) : start(a), end(e), pulses(n), art(art) {}

	bool   is_silence() { return art == silence; }
	bool   is_music() { return art == music; }
	bool   is_data() { return art == data; }
	bool   is_empty() { return start == end; }
	uint32 count() { return end - start; }
};


/*	classify ranges of samples as "data" or "silence"
	used in preparation for cutting a tape into blocks

	• Am Anfang der Datei wird "Stille" erwartet. Fast immer wird die zurückgegebene Liste mit Stille anfangen.
	  Sie kann aber, wenn die Audiodatei tatsächlich mitten in einem Data-Block startet, auch mit Data anfangen.
	• Danach alternieren die Blocks zwischen Data und Stille.
	• Der letzte Block ist normalerweise wieder Stille, kann aber auch Data sein.

	Ein Data-Block wird beendet, wenn ein Puls zu lang ist. Der Level ist irrelevant.
	Ein Stille-Block wird beendet, wenn ein Sample zu laut ist. Die Pulslänge ist irrelevant.

	=> Ein Data-Block wird nicht durch ein Drop-Out, das nicht völlig untergeht, vorzeitig beendet.
	=> Ein Stille-Block wird nicht durch leise Geräusche oder Rauschen vorzeitig beendet.

	Zusätzlich wird im Stille-Block nur getestet, ob die Samples in Gegenrichtung zu laut sind.
	Nach dem letzten Puls klingt der Level typischerweise langsam ab und ist zunächst "zu laut".
	Das wird so implizit ignoriert. Danach _kann_ ein Puls nur in der anderen Richtung erscheinen,
	da ein Puls nur sichtbar wird, wenn der Ausgang getoggelt wird. Deshalb ist es nicht nötig,
	auf ein Überschreiten des Stille-Levels in der ursprünglichen Richtung zu testen. (echte Musik ist anders…)

	Der Wert 0 wird als "unter der Schwelle" betrachtet, weil einige fehlerhaft künstlich gerenderte Dateien
	für den ZX80/81 das so brauchen. Die "Bit-Gap" wird sonst nicht erkannt, weil sie mit 0-Samples gefüllt wird.
*/
static void classify_audio(const int16* samples, uint32 count, uint32 samples_per_second, Array<AudioRange>& ranges)
{
	xlogIn("AudioData::classify_audio()");

	ranges.purge();
	if (count == 0) return;

	// Define maximum length for a data pulse
	// If a pulse longer than this is encountered while in "data" mode
	// then the classification switches to "silence"
	// note: bit gap length in ZX80 data is 4689cc / 3250000ccps ~ 1.5ms ~ 350Hz
	//
	const uint32 max_samples_for_pulse = samples_per_second / 400; // 2.5ms  ~  200Hz

	// Define maximum silence level
	// If a pulse louder than this is encountered while in "silence" mode
	// then the classification switches to "data"
	//
	// note: real silence is < -40dB. a data signal is expected to be at least -20dB. (that's already very faint.)
	// between silence and data signal there may be any kind of noise, e.g. music.
	//
	const int silence_level = dB25; // suggested: -25dB

	// Define minimum slope rate:
	// If the first sample is below silence level,
	// then the buffer may still start with "pulses" if the samples raise fast above silence level.
	// Estimation: raise at least 50% of silence level within one sample at 44.1kHz:
	//
	const int mindelta = silence_level / 2 * 44100 / samples_per_second;

	// sample indexes:
	const int16* p = samples;		  // next sample
	const int16* e = samples + count; // end of samples[]

	// test whether the buffer starts with a pulse:
	// the buffer may still start with silence, if the first pulse is too long
	if (*p > +silence_level) goto a;
	if (*p < -silence_level) goto a;

	// the first sample is below "silence" level:
	// the buffer may still start with "data" if the samples for the first pulse raise fast above silence level.
	// i don't know whether this test ever succeeds.
	for (p = samples + 1; p < e && *p >= *(p - 1) + mindelta; p++)
	{
		if (*p > +silence_level)
		{
			p = samples;
			goto a;
		}
	}
	for (p = samples + 1; p < e && *p <= *(p - 1) - mindelta; p++)
	{
		if (*p < -silence_level)
		{
			p = samples;
			goto a;
		}
	}

	// buffer starts with silence:
	// search for first loud sample:
	// we cannot use the silence loop in the loop, because we don't know the polarity
	for (p = samples + 1; p < e && abs(*p) <= silence_level; p++) {}
	ranges.append(AudioRange(0, p - samples, 1, silence));

	while (p < e)
	{
	a:
		const int16* start = p; // start of "data" block
		const int16* p0	   = p; // p0 = start of pulse
		uint32		 n	   = 0; // num. pulses

	b:
		if (*p > 0)
			while (p<e&& * p> 0) { p++; } // find zero crossing
		else
			while (p < e && *p <= 0) { p++; } // find zero crossing

		if (p <= p0 + max_samples_for_pulse)
		{
			n++;
			p0 = p;
			if (p < e) goto b;
		} // a pulse

		if (n) ranges.append(AudioRange(start - samples, p0 - samples, n, data));
		if (p0 == e) break;

		// overlong sample detected => start of "silence" block
		// until we detect a pulse of opposite polarity which exceeds the silence level
		// note: pulses of same polarity may have any level!

		if (*p0 > 0)
			while (p < e && *p >= -silence_level) { p++; }
		else
			while (p < e && *p <= +silence_level) { p++; }

		ranges.append(AudioRange(p0 - samples, p - samples, 1, silence));
	}

// Ausgangsbedingung prüfen:
// • Blöcke dürfen nicht leer sein.
// • Es dürfen keine 2 Datenblöcke aufeinander folgen.
//   Mehrere Stille-Blöcke hintereinander sind aber möglich.
// • Nach dem letzten Sample eines Datenblocks muss ein Polaritätswechsel stattfinden.
// • Stille-Blöcke müssen "wie ein Puls" wirken, d.h. der letzte Puls vor und
//   der erste Puls nach einem Stille-Block müssen die gleiche Polarität haben.
#ifndef NDEBUG
	{
		assert(ranges[0].count());
		int last_i = -1;

		for (uint i = 1; i < ranges.count(); i++)
		{
			// Block darf nicht leer sein:
			assert(ranges[i].count());

			// keine 2 Datenblöcke hintereinander:
			assert(ranges[i - 1].is_silence() || ranges[i].is_silence());

			// nach dem letzten Sample eines Datenblocks muss ein Polaritätswechsel stattfinden
			if (ranges[i - 1].is_data())
			{
				assert((samples[ranges[i - 1].end - 1] > 0) != (samples[ranges[i].start] > 0));
			}

			// Polaritätswechsel über Stille-Blöcke hinweg prüfen:
			if (ranges[i].is_data())
			{
				if (last_i != -1)
				{
					if ((i - last_i) & 1)
						assert((samples[ranges[last_i].end - 1] > 0) != (samples[ranges[i].start] > 0));
					else
						assert((samples[ranges[last_i].end - 1] > 0) == (samples[ranges[i].start] > 0));
				}
				last_i = i;
			}
		}
	}
#endif
}


// ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
//							c'tor & d'tor
// ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––


/*	Default Creator
	create empty audio data
*/
AudioData::AudioData() :
	TapeData(isa_AudioData), stereo(no), samples_per_second(0), audio_decoder(), adc_start_pos(), adc_end_pos()
{}


/*	Destructor
 */
AudioData::~AudioData() {}


/*	Creator from audio file
	This does not actually read any samples into float_samples[] or int16_samples[]!
*/
AudioData::AudioData(AudioDecoder* ad, uint32 a, uint32 e) :
	TapeData(isa_AudioData), stereo(no), samples_per_second(ad->samplesPerSecond()), audio_decoder(ad),
	adc_start_pos(a), adc_end_pos(e)
{}


/*	Copy Creator
 */
AudioData::AudioData(const AudioData& q) :
	TapeData(isa_AudioData), float_samples(q.float_samples), int16_samples(q.int16_samples), stereo(q.stereo),
	samples_per_second(q.samples_per_second), audio_decoder(q.audio_decoder), adc_start_pos(q.adc_start_pos),
	adc_end_pos(q.adc_end_pos)
{}


/*	Create from other TapeData subclass
	Converter
	converts sample rate to std audio output sample rate
	  except if q==AudioData then the sample rate is preserved
	TODO: specify sample rate -> possible use for sample rate conversion
*/
AudioData::AudioData(const TapeData& q) :
	TapeData(isa_AudioData), stereo(no), samples_per_second(0), audio_decoder(), adc_start_pos(), adc_end_pos()
{
	xlogIn("AudioData(TapeData)");

	switch (q.isaId())
	{
	case isa_AudioData: new (this) AudioData(reinterpret_cast<const AudioData&>(q)); break;
	default: new (this) AudioData(CswBuffer(q, 3500000), ::samples_per_second); break;
	}
}


// ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
//				convert CswBuffer -> AudioData
// ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––


/*	Create from CswBuffer
	fall-back method for any conversion
	stores mono audio data in int16_samples[]
*/
AudioData::AudioData(const CswBuffer& q, uint32 sps) :
	TapeData(isa_AudioData), stereo(no), samples_per_second(sps), audio_decoder(), adc_start_pos(), adc_end_pos()
{
	xlogIn("AudioData(CswBuffer)");

	// duration of CswBuffer measured in cpu cycles:
	uint32 total_cc = q.getTotalCc();

	// conversion factor cc -> samples:
	double samples_per_cc = (double)samples_per_second / (double)q.ccPerSecond();

	// calculate resulting number of samples: (rounded)
	uint32 total_samples = uint32(0.1 + total_cc * samples_per_cc);
	if (total_samples == 0) return;

	// adjust conversion factor so that decoding the CswBuffer results in a non-fractional amount of samples:
	samples_per_cc = (double)total_samples / (double)total_cc;

	// allocate sample buffer:
	// over-allocate one sample so that we can blindly access samples[e] later, even for the last pulse
	int16_samples.grow(total_samples + 1);
	int16* samples = int16_samples.getData();

	// rewind CswBuffer and get phase0:
	q.seekStart();
	const int volume = 0x7fff / 3; // resulting signal amplitude ~ -10dB
	int16	  phase	 = q.getCurrentPhase() ? volume : ~volume;

	double sa, se = 0; // fractional indexes in samples[]

	for (uint32 cc = 0; cc < total_cc;)
	{
		cc += q.readPulse();

		sa = se;				  // fractional index in samples[]
		se = cc * samples_per_cc; // fractional index in samples[]

		uint32 a = uint32(sa); // integer index in samples[]
		uint32 e = uint32(se); // integer index in samples[]

		if (a == e) // very short pulse, entirely in a single sample
		{
			samples[a] += int16(phase * (se - sa));
		}
		else
		{
			samples[a] += int16(phase * (a + 1 - sa));
			while (++a < e) samples[a] = phase;
			samples[e] = int16(phase * (se - e));
		}
		phase ^= -1; // 0x7fff <-> 0x8000
	}

	int16_samples.drop(); // drop the over-allocated sample
}


// ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
//				convert AudioData -> CswBuffer
// ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––


/*	Create CswBuffer from AudioData
	for converting AudioData into other TapeData subclasses
	for play TapeFile
	a temporary sample buffer may be created but is not cached,
	because we probably don't need the sample data again.
	If this is not true then read the audiofile in qa first!
*/
CswBuffer::CswBuffer(const AudioData& qa, uint32 ccps) :
	data(nullptr), max(0), end(0), cc_end(0), ccps(ccps), recording(no), pos(0), cc_pos(0), phase(0), cc_offset(0)
{
	xlogIn("CswBuffer(AudioData)");

	// temporary samples buffer
	int16* samples;
	uint32 count;

	// convert available data to mono int16[]:

	if ((count = qa.int16_samples.count())) // int16_samples[] exists:
	{
		if (qa.stereo)
		{
			count		   = count / 2; // frame_count = sample_count/2
			samples		   = new int16[count];
			const int16* q = qa.int16_samples.getData();
			for (int16 *z = samples, *ze = z + count; z < ze;)
			{
				int16 v = *q++;
				*z++	= (v >> 1) + (*q++ >> 1);
			}
		}
		else // mono, int16 => source data can be used "as is":
		{
			new (this) CswBuffer(qa.int16_samples.getData(), count, qa.samples_per_second, ccps);
			return;
		}
	}
	else if ((count = qa.float_samples.count())) // float_samples[] exists:
	{
		if (qa.stereo)
		{
			count			= count / 2; // frame_count = sample_count/2
			samples			= new int16[count];
			const Sample* q = qa.float_samples.getData();
			for (int16 *z = samples, *ze = z + count; z < ze;)
			{
				Sample v = *q++;
				v += *q++;
				*z++ = minmax(-32767, int(16384 * v), +32767);
			}
		}
		else
		{
			samples			= new int16[count];
			const Sample* q = qa.float_samples.getData();
			for (int16 *z = samples, *ze = z + count; z < ze;)
			{
				Sample v = *q++;
				*z++	 = minmax(-32767, int(32768 * v), +32767);
			}
		}
	}
	else if (qa.audio_decoder.ptr() && (count = qa.adc_num_samples())) // audiofile exists:
	{
		try
		{
			samples = new int16[count];
			qa.audio_decoder->seekSamplePosition(qa.adc_start_pos);
			qa.audio_decoder->read(samples, count, 1 /*num.channels*/);
		}
		catch (AnyError& e)
		{
			delete[] samples;
			showAlert("Reading \"%s\" failed:\n%s", qa.audio_decoder->getFilename(), e.what());
			return; // return empty block
		}
	}
	else // empty buffer:
	{
		return;
	}

	// convert int16[] -> csw:
	new (this) CswBuffer(samples, count, qa.samples_per_second, ccps);
	delete[] samples;
}


/*	Create CswBuffer from samples[] buffer
	Convert int16[] to csw[]

	CC are rounded per pulse
		this may lead to small deviation of overall signal speed
		but it will be at most 0.5 cc/pulse and pulses are typically 250µs max. (2kHz)
		and ccps is typically 3.5MHz so the error is 1/2800 max. => neglectable.

	Die Null-Linie liegt bei +0.5:
	Einige künstlich gerenderte Audiodateien für den ZX80 und ZX81 benutzen 3 Level:
		pos, neg und 0 und laden nur, wenn man 0 als negativ ansieht.

	Der Encoder unterscheidet zwischen "Pause" und "Daten":

	Pausen werden als 1 Puls gespeichert --> leises Rauschen wird nicht als CSW-Signal aufgezeichnet.
	Daten können im Level beliebig abfallen --> Dropouts werden (solange wie möglich) als Daten kodiert.

	Es wird eine Maximal-Länge für einen Puls definiert: 2.5µs ~ 1/400sec ~ 200Hz
		Überschreitet ein Puls diesen Wert, wird in den "Pause"-Modus gewechselt.

	Es wird ein Stille-Level definiert: -25dB
		Überschreitet ein Sample diesen Wert in Gegenrichtung, wird in den "Daten"-Modus geschaltet.
		e.g.: War der letzte Puls positiv, muss ein Sample die neg. Stilleschwelle unterschreiten.
		In der ursprünglichen Richtung kann das Signal beliebig weit auslenken. Es wird angenommen,
		dass der Ausgang des Computers nur 2 Level hat (pos. und neg.) und nach einem pos. Puls nicht ein
		noch positiverer kommen kann, sondern nur ein negativer. Alles andere sind Störgeräusche.

	Am Anfang der Datei startet der Encoder im "Pause"-Modus, allerdings kennt er die Polarität
	des ersten Pulses nicht und evtl. beginnt das Signal ja sogar mit einem Puls. => Tests.

	TODO: "verlorenen Puls" erkennen und restaurieren
	TODO: Signal in Dropouts verbessern
*/
CswBuffer::CswBuffer(const int16* samples, uint32 count, uint32 samples_per_second, uint32 ccps) :
	CswBuffer(ccps, 0, 666)
{
	xlogIn("new CswBuffer(int16[])");

	data = new uint16[count >> 4];

	// Definition of silence level
	// real silence is < -40dB. a data signal is expected to be at least -20dB. (already very faint.)
	// between silence and data signal there may be any kind of noise, e.g. music.
	// we have to define a level above which a pulse is never "silence".
	const int silence_level = dB25; // suggested: -25dB

	// conversion factor samples/second -> cpu_cycles/second:		(samples[] index -> CswBuffer index)
	const double cc_per_sample = (Time)ccps / samples_per_second;

	// Maximum pulse length:
	// note: the bit gap length in a ZX80/ZX81 data signal is 4689/3250000 = 1.44ms
	const uint32 max_samples_for_pulse = samples_per_second / 400; // 2.5ms  ~  200Hz

	// center value for audio signal:
	// some artificial audio files for ZX80/ZX81 use 0 for the bit gap which must be seen as negative to load.
	// => 0 is negative!
	// => exact center value is 0.5 (between: 0=neg, 1=pos)
	//    but we can probably use 0 without risk of neg. run lengths as well (to be tested)
	const double null = 0.5;

	// minimum slope rate:
	// even if 1st sample is below silence level we may start in "Data" mode if the signal rises "fast" enough:
	// Estimation: raise at least 50% of silence level within one sample at 44.1kHz:
	const int mindelta = silence_level / 2 * 44100 / samples_per_second;

	// local data:
	uint32 i   = 0; // sample index
	uint32 i0  = 0; // start of pulse index: puls = [i0 .. [i
	double a0  = 0; // sub-sample adjustment at start of a pulse
	double e0  = 0; // sub-sample adjustment at end of a pulse
	bool   neg = 0; // polarity of current pulse or pause


	/*	Detect initial mode "Data" or "Pause":

		Ist das erste Sample über der Stilleschwelle, sind wir im entsprechenden pos. / neg. Puls.
		Andernfalls prüfen, ob das Signal schnell ansteigt, dann sind wir auch vom Start weg in "Daten".
		Andernfalls sind wir in "Pause":
			Anders als bei der Stille nach einem langen Puls kennen wir aber die Polarität nicht.
			Die Stille gilt also als beendet, wenn ein Sample die Stilleschwelle in irgendeiner Richtung
			überschreitet und die Polarität des Stillepulses wird daraus zurückgefolgert.
	*/

	// test whether the buffer starts with a pulse:
	// note: the buffer may still start with silence, if the pulse is too long
	if (abs(samples[0]) > silence_level) goto a;

	// the first sample is within "silence" level:
	// the buffer may still start in "Data" mode if the samples raise "fast" above silence level:
	// (i don't know whether this test ever succeeds.)
	for (i = 1; i < count && samples[i] >= samples[i - 1] + mindelta; i++)
	{
		if (samples[i] > +silence_level) goto a;
	}
	for (i = 1; i < count && samples[i] <= samples[i - 1] - mindelta; i++)
	{
		if (samples[i] < -silence_level) goto a;
	}

	// buffer starts with silence but we don't know the polarity:
	// => search for first loud sample:
	for (i = 1; i < count && abs(samples[i]) <= silence_level; i++) {}
	// => i is index of first loud sample

	if (i == count)		 // whole buffer is silence!
	{					 // => polarity unknown!
		this->phase = 0; // => set polarity as after TzxPause
		writePulseCc(count * cc_per_sample);
		return;
	}


	//	start in "Pause" mode:

	// set silence polarity to opposite of first pulse polarity:
	// if we see a pulse after some time of silence it _must_ have toggled the output pin.
	// if it didn't toggle the output pin it does not generate an edge in the signal.
	// actually, the save routine may have started with a same-polarity pulse which we could not see!

	neg			= samples[i] > 0; // silence polarity is opposite of first data pulse
	this->phase = !neg;			  // start polarity of CswBuffer
	goto s;


	//	start in "Data" mode:

a:
	i			= 0;
	neg			= samples[0] <= 0; // 1=neg
	this->phase = !neg;			   // 1=pos

	while (i < count)
	{
		i0 = i; // start of pulse

		assert(neg == (samples[i] <= 0));

		if (neg)
			while (i < count && samples[i] <= 0) { i++; } // find zero crossing
		else
			while (i < count && samples[i] > 0) { i++; } // find zero crossing

		// zero crossing:
		// i0 -> first sample of this pulse
		// i  -> first sample of next pulse


		if ((i - i0) <= max_samples_for_pulse && i < count) // a pulse
		{
			assert((samples[i - 1] > 0) != (samples[i] > 0));

			// sample[i] gilt für den Zeitraum  [(i)*ccps/sps ... (i+1)*ccps/sps[
			// der Puls gilt für den Zeitraum   [(i0+a)*ccps/sps ... (i+e)*ccps/sps[
			// wobei a und e die Feinjustierung der Nulldurchgangsposition ist

			// Verschiebung des Endes des letzten Samples: -0.5 ... +0.5
			// ein pos. Wert verlängert das letzte Sample des letzten Pulses
			// und verkürzt das 1. Sample des nächsten Pulses

			// Es wird angenommen, dass ein Sample in der Mitte seines Zeitraums gemessen wird.
			// Der tatsächliche 0-Durchgang fand aber irgendwann zw. den Messpunkten statt:

			double last_sample = samples[i - 1] - null; // last sample of this pulse
			double next_sample = samples[i + 0] - null; // first sample of next pulse

			e0 = std::fabs(last_sample / (last_sample - next_sample)) - 0.5;
		}
		else // silence or i==count
		{
			if (neg)
				while (i < count && samples[i] <= +silence_level) { i++; }
			else
				while (i < count && samples[i] >= -silence_level) { i++; }

			// => i = 1. sample of next pulse

		s:
			e0 = 0;

			if (i + 1 < count)
			{
				double s0 = samples[i + 0] - null;			   // 1. sample des folgenden Pulses
				double s1 = (samples[i + 1] - null) * 0.66667; // 2. sample des folgenden Pulses * 2/3

				// wenn s0 < 2/3 s1 dann wird angenommen, dass der Grund dafür ist, dass
				// der Puls irgendwann mitten im Sample startete.
				//
				// tx = t0 + 1 - s0 / (2/3 s1)

				if (neg ? (s0 < s1) : (s0 > s1)) // Verschiebung des Endes des letzten Samples:
				{								 // um 0 .. +0.99
					e0 = 1.0 - s0 / s1;
				}
				//				else	// s0 ≥ 2/3 s1 => für s0 wird die volle Breite angenommen
				//				{		// das erfasst auch den Fall, dass s1<s0 oder gar s1<0, also ein Puls mit nur 1
				// Sample 					e0 = 0;
				//				}
			}
			//			else if(i<count) // nach der Pause folgt ein Puls mit nur noch 1 Sample, dann ist Buffer-Ende
			//			{				// => der Puls wird in voller Breite gespeichert
			//				e0 = 0;
			//			}
			//			else			// i==count: Die Pause oder der Puls geht bis zum Buffer-Ende
			//			{
			//				e0 = 0;
			//			}
		}

		// store pulse:
		uint32 cc = uint32((i - i0 + e0 - a0) * cc_per_sample + 0.5);
		assert(cc < 0x80000000);
		if (cc < 80) cc = 80; // in case that a pulse (mostly the 1st sync pulse) is about to vanish
		writePulseCc(cc);

		// loop:
		a0	= e0;	// Verschiebung des Startzeitpuntes des 1. Samples des nächsten Pulses
		neg = !neg; // toggle polarity
	}
}


// ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
//						read audio file
// ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––


/*static*/
void AudioData::readFile(cstr fpath, TapeFile& tapeblocks) noexcept(false) // file_error,data_error,bad_alloc
{
	xlogIn("AudioData::readFile(%s)", fpath);
	assert(tapeblocks.count() == 0);

	// create and open AudioFile:
	AudioDecoder* decoder = new AudioDecoder();
	decoder->open(fpath); // throws

	// read audio data in one giant chunk: (10MB/minute)
	uint32 num_samples = uint32(decoder->numSamples());
	if (num_samples == 0) return;
	int16* samples = new int16[num_samples + num_samples / 50]; // over-allocate because size may be inaccurate
	num_samples	   = decoder->read(samples, num_samples + num_samples / 50, 1);
	uint32 sps	   = uint32(decoder->samplesPerSecond());
	uint32 ccps	   = tapeblocks.ccps();

	Array<AudioRange> ranges;
	classify_audio(samples, num_samples, sps, ranges);
	assert(ranges.count() > 0);

	// Datenblöcke und Pausen zusammenfassen:
	// normalerweise immer 1x Daten + 1x Stille
	// mehrere Pausen können aufeinanderfolgen: alle an den letzten Datenblock anhängen!
	// sehr kurze Datenblöcke ebenfalls an den letzten Datenblock anhängen:
	// diese sind entweder Musik, Brummen, Rauschen, Keyklicks oder irgendein Kopierschutz.

	xlogline("ranges: %u (pre merge)", uint(ranges.count()));
	for (uint i = ranges.count(); --i;)
	{
		// AudioRange& range = ranges[i];
		if (ranges[i].is_data() && ranges[i].pulses > 15) continue;

		ranges[i - 1].end = ranges[i].end;
		ranges.remove(i);
	}
	xlogline("ranges: %u (post merge)", uint(ranges.count()));


	for (uint i = 0; i < ranges.count(); i++)
	{
		xlogline("range[%u]: %u pulses", i, uint(ranges[i].pulses));

		uint32			   ai		 = ranges[i].start;
		uint32			   ei		 = ranges[i].end;
		CswBuffer*		   csw		 = new CswBuffer(samples + ai, ei - ai, sps, ccps);
		AudioData*		   audiodata = new AudioData(decoder, ai, ei);
		TapeFileDataBlock* tfd		 = new TapeFileDataBlock(audiodata, csw);

		if (tfd->tapdata)
		{
			xxlogline("pilot start at csw pulse %u", tfd->tapdata->csw_pilot);
			xxlogline("data  start at csw pulse %u", tfd->tapdata->csw_data);
			xxlogline("pause start at csw pulse %u", tfd->tapdata->csw_pause);
		}

		// Manchmal sind zwei Blöcke nur durch eine extrem kurze oder gar keine Pause getrennt.
		// Das wurde dann von der Schneideroutine nicht erkannt, z.B.: Jupiter Ace zwischen Header und Data.
		// Dann müssen wir die hier noch splitten.
		// Das ist nur wichtig, um das Band als .tap-Datei speichern zu können, damit der 2. Block nicht verloren geht.
		// Auf das Laden _jetzt_ hat es keinen Einfluss, außer bei Instant-Laden und für die Anzeige des TapeRecorders.

		if (tfd->tapdata && tfd->tapdata->trust_level >= conversion_success && // ergab dieser Range einen .tap Block?
			csw->getTotalPulses() >= tfd->tapdata->csw_pause + 666)			   // kommen danach noch sehr viele Pulse?
		{
			uint32	csw_splitposition = tfd->tapdata->csw_pause + 2; // putative Schnittposition
			TapData tapdata(
				csw->getData() + csw_splitposition, // Versuche, den Remainder auch in einen
				csw->getTotalPulses() - csw_splitposition,
				ccps); // TapData-Block zu konvertieren

			if (tapdata.trust_level >= conversion_success) // Wenn das gelang:
			{
				csw->seekPos(csw_splitposition); // Puls-Index in Sample-Index umrechnen
				uint32 zi = ai + uint32(double(csw->getCurrentCc()) / ccps * sps + 0.5);

				// Range splitten und nochmal beide Ranges konvertieren:
				ranges.insertat(i + 1, AudioRange(zi, ei, ranges[i].pulses - csw_splitposition, data));
				ranges[i].pulses = csw_splitposition;
				ranges[i--].end	 = zi;
				delete tfd;
				continue;
			}
		}

		if (ranges[i].is_silence() && tfd->getMajorBlockInfo() == nullptr) tfd->setMajorBlockInfo("Silence");
		tapeblocks.append(tfd);
	}


	// Polarität des ersten Blocks fixen:
	// kann nur falsch sein, wenn der range[0]==stille && range[1]==data
	if (tapeblocks.count() > 1)
	{
		if (tapeblocks[0]->cswdata->getCurrentPhase() != tapeblocks[1]->cswdata->getPhase0())
		{
			assert(ranges[0].is_silence());
			assert(ranges[1].is_data());
			tapeblocks[0]->cswdata->invertPolarity();
		}
	}


	delete[] samples;
}


/*static*/
void AudioData::writeFile(cstr fpath, TapeFile& tapeblocks) noexcept(false) // file_error,data_error,bad_alloc
{
	xlogIn("AudioData::writeFile(%s)", fpath);
	showAlert("AudioData::writeFile(): TODO");
	(void)tapeblocks;
	(void)fpath;
}
