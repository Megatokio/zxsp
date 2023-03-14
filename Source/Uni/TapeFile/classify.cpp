#define SAFE 3
#define LOG	 1

#include "AudioData.h"
#include "CswBuffer.h"
#include "DspTime.h"
#include "StereoSample.h"
#include "TapeFileDataBlock.h"
#include "TzxData.h"
#include "audio/AudioDecoder.h"
#include "globals.h"


#if 0

static const double log_1   = log(1);
static const double log_20  = log(20);
static const double log_100 = log(100);
static const double log_300 = log(300);
static const double log_1k  = log(1000);
static const double log_3k  = log(8000);
static const double log_8k  = log(8000);
static const double log_10k = log(10000);
static const double log_40k = log(40000);

static const double log_60db = 3.4894219113732;	// log(0x7fff / 1000.0);	// -60 dB
static const double log_50db = 4.6414349767685;	// log(0x7fff / 316.0);		// -50 dB
static const double log_40db = 5.7920070043673;	// log(0x7fff / 100.0);		// -40 dB
static const double log_30db = 6.9440200697625;	// log(0x7fff / 31.6);		// -30 dB
static const double log_20db = 8.0945920973613;	// log(0x7fff / 10.0);		// -20 dB
static const double log_10db = 9.2466051627566;	// log(0x7fff / 3.16);		// -10 dB
static const double log_0db  = 10.397177190355;	// log(0x7fff / 1.0);		// -0  dB


/*	calculate probability for a pulse to be data, music or silence
	scrutinizes:
		pulse length (-> Frequency f)
		loudness (mid_s)
		squareness of pulse (mid_s/max_s)
	not yet used:
		amount of local minima/maxima inside pulse
		sudden level increase or decrease between adjacent pulses (can't be handled here)
	TODO we could do some table lookup to improve speed
*/
inline void classify_pulse(double log_f, uint mid_s, uint max_s, double& data, double& music, double& silence)
{

// Bewertung der Frequenz:
//	double log_f = log(f);				// we use log(frequency)

	// music:
	// unter 20Hz: 0%
	// dann ansteigend
	// ab 300Hz: 100%

	if	   (log_f<log_20)   { music = 0; }
	else if(log_f<log_300)	{ music = (log_f-log_20) / (log_300-log_20); }
	else			{ music = 1; }

	// data:									// note: Specci: normal: 1kHz + 2kHz
	// bis 300Hz: 0%
	// dann ansteigend
	// 1kHz - 3kHz: 100%
	// dann abfallend
	// ab 8kHz: 0%				// damit sollten die meisten Speedloader zumindest mit 75% noch drin sein

	if	   (log_f<log_1k)	{ data = (log_f-log_300) / (log_1k-log_300); }
	else if(log_f<=log_3k)	{ data = 1; }
	else					{ data = (log_8k-log_f) / (log_8k-log_3k); }

	// silence:
	// unter 100Hz: 100%
	// dann abfallend
	// ab 1kHz: 0%

	if	   (log_f<log_100)	{ silence = 1; }
	else if(log_f<log_1k)	{ silence = (log_1k-log_f) / (log_1k-log_100); }
	else			{ silence = 0; }


// Bewertung der Lautstärke:
	double log_s = log(mid_s);					// we use dB which is log

	// data:
	// bis -30dB: 0%
	// dann ansteigend
	// ab -10dB: 100%

	if	   (log_s>log_10db)	{ data *= 1; }
	else if(log_s>log_30db)	{ data *= (log_30db - log_s) / (log_30db - log_10db); }
	else					{ data *= 0; }

	// silence:
	// bis -60dB: 100%
	// dann abfallend
	// ab -30dB: 0%

	if	   (log_s<log_60db)	{ silence *= 1; }
	else if(log_s<log_30db)	{ silence *= (log_30db - log_s) / (log_30db - log_60db); }
	else					{ silence *= 0; }

	// music:
	//	bis -60dB: 0%
	//	dann ansteigend
	//	-40dB bis -10dB: 100%		-10dB is für mittl. Sample-Wert schon extrem laut und übel verzerrt
	//	dann abfallend
	//	0dB: 0%

	if     (log_s<log_60db)	{ music *= 0; }
	else if(log_s<log_40db)	{ music *= (log_60db - log_s) / (log_60db - log_40db); }
	else if(log_s<log_10db)	{ music *= 1; }
	else					{ music *= (log_0db - log_s) / (log_0db - log_10db); }


// Bewertung der Pulsform: Quadratigkeit:
	double q = (double)mid_s / max_s;

//	Quadratigkeit jeweils ohne Rauschen:
//	Q = 0   -> Nadelimpuls
//	Q = 0.5 -> Dreieck
//	Q = 0.7 -> Sinus
//	Q = 1.0 -> Rechteck
//	Mit Rauschen:
//	Q = 0.1 -> Nadelimpuls
//	Q = 0.5 -> Dreieck
//	Q = 0.6 -> Sinus
//	Q = 0.9 -> Rechteck


	// data:
	// bis 0.5: 0%
	// bis 0.8 steigend
	// ab 0.8: 100%

	if     (q<0.5)	{ data *= 0; }
	else if(q<0.8)	{ data *= (q-0.5) / (0.8-0.5); }
	else			{ data *= 1; }

	// music:
	// bis 0.3: 0%
	// dann steigend
	// 0.5 - 0.7: 100%
	// dann fallend
	// ab 0.9 0%

	if	   (q<0.3)	{ music *= 0; }
	else if(q<0.5)	{ music *= (q-0.3) / (0.5-0.3); }
	else if(q<0.7)	{ music *= 1; }
	else if(q<0.9)	{ music *= (0.9-q) / (0.9-0.7); }
	else			{ music *= 0; }

	// silence/noise:
	// ≤ 0.4: 100%
	// dann fallend
	// ab 0.6: 0%

	if	   (q<0.4)	{ silence *= 1; }
	else if(q<0.6)	{ silence *= (0.4-q) / (0.4-0.2); }
	else			{ silence *= 0; }
}


enum Classification { is_nothing, is_silence, is_music, is_data };

/*	Klassifiziere im übergebenen Sample-Buffer den ersten Block
	und liefere dessen Klassifizierung und Länge (in count) zurück
*/
Classification classify(int16* samples, uint32 size, uint32 sps, uint32& count )
{
	Classification	sliding_class   = is_nothing;
	int				sliding_music   = 0;
	int				sliding_silence = 0;
	int				sliding_data    = 0;
	Classification	recent_class[16];
	uint32			recent_sa[16];
	memset(recent_class,0,sizeof(recent_class));

	uint32 si = 0;			// sample index
	uint32 pi = 0;			// pulse index


a:	if(si==size) { count = si; return sliding_class; }

	int    s;				// current sample
	int    max_s = 0;		// maximum sample in pulse
	uint32 sum_s = 0;		// sum of all samples in pulse
	uint32 sa = si;			// index of first sample in pulse

	if(samples[si] >= 0)
	{
		while(si<size && (s=samples[si]) >= 0)
		{
			sum_s += s;
			if(s>max_s) max_s = s;
			si++;
		}
	}
	else
	{
		while(si<size && (s=samples[si]) < 0)
		{
			sum_s -= s;
			if(s<max_s) max_s = s;
			si++;
		}
		max_s = -max_s;					// max. sample in pulse (abs)
	}

	uint32	se = si;					// index of last sample of pulse +1
	uint32	sn = se-sa;					// length of pulse [samples]
	uint    mid_s = sum_s / sn;			// average sample value of pulse (abs)
	Frequency f = (double)sps / (2*sn);	// frequency of pulse [1/s]
	double  log_f = log(f);				// log(frequency)

// Bewertung:
	double silence, data, music;		// values for current pulse
	classify_pulse(log_f, mid_s, max_s, data, music, silence);
	recent_class[pi&15] = data>music && data>silence ? is_data : music>silence ? is_music : is_silence;
	recent_sa[pi++&15]  = sa;

	double fraction;
	// Anteil des aktuellen Pulses am Sliding Value
	// bis 1Hz: 100%
	// dann abfallend
	// ab 1kHz: 10%
	if(log_f>=log_1k)	 fraction = 0.1;
	else if(log_f>log_1) fraction = 0.1 + 0.9 * (log_1k-log_f) / (log_1k-log_1);
	else fraction = 1;

	sliding_silence *= 1-fraction; sliding_silence += fraction * silence;
	sliding_music   *= 1-fraction; sliding_music   += fraction * music;
	sliding_data    *= 1-fraction; sliding_data    += fraction * data;
	Classification old_sliding_class = sliding_class;
	sliding_class   = sliding_data>sliding_music && sliding_data>sliding_silence ? is_data
					: sliding_music>sliding_silence ? is_music : is_silence;
	if(old_sliding_class == sliding_class || old_sliding_class==is_nothing) goto a;

	// Einordnung hat sich geändert!
	// Zurückschauen, seit wann:

	pi--;
	while(pi>0 && recent_class[--pi&15]==sliding_class) { }

	count = recent_sa[++pi&15];
	return old_sliding_class;
}

#endif
