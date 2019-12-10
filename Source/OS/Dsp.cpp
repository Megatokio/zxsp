/*	Copyright  (c)	Günter Woigk 2019 - 2019
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
#include "Dsp.h"
#include "Uni/globals.h"
#include <math.h>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include "Qt/qt_util.h"
#include "cpp/cppthreads.h"

Time		system_time			= 0.0;
Frequency	samples_per_second	= 44100;

StereoSample Dsp::audio_out_buffer [DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING];
StereoSample Dsp::audio_in_buffer  [DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING];

// set in Dsp():
static QAudioDeviceInfo audioOut_device;
static QAudioDeviceInfo audioIn_device;
static Dsp* audio_runner_device = nullptr;
QAudioOutput* Dsp::audio_output = nullptr;
QAudioInput*  Dsp::audio_input = nullptr;

bool Dsp::audio_input_device_present  = no; 	// preset to no if error
bool Dsp::audio_input_device_enabled  = no; 	// preset to no if error
bool Dsp::audio_output_device_enabled = yes;	// must be present
Sample Dsp::audio_output_volume = 0.3f;

static Sample  		play_through_damping = 0.0f;	// no playthrough during startup: no 'klick' if audio-in disabled in prefs
static StereoSample	audio_out_center	 = 0.0f;	// for audio out High-pass filter
static StereoSample	audio_in_center		 = 0.0f;	// for audio in High-pass filter
static Sample		audio_in_bias_damping = 0.0f;	// 0.001f
static Sample		audio_out_bias_damping = 0.0f;	// 0.001f

template<typename Z, typename Q>
inline void copy_data (Z* z, Q* q, uint cnt)
{
	while (cnt--) { *z++ = *q++; }
}

template<typename Z, typename Q, typename F>
inline void copy_data_with_volume (Z* z, Q* q, uint cnt, F f)
{
	while (cnt--) { *z++ = *q++ * f; }
}

template<typename T>
inline void clear_data (T* z, T q, uint cnt)
{
	while (cnt--) { *z++ = q; }
}

inline void highPassFilter (StereoSample* p, int count, StereoSample& center, float damping)
{
	// remove signal bias
	for (StereoSample* e = p+count; p < e; p++) { center += (*p-=center) * damping; }
}

inline void Dsp::highpassInputBuffer ()
{
	if (audio_in_bias_damping != 0.0f)
		highPassFilter(audio_in_buffer+DSP_SAMPLES_STITCHING, DSP_SAMPLES_PER_BUFFER, audio_in_center, audio_in_bias_damping);
}

inline void Dsp::highpassOutputBuffer ()
{
	if (audio_out_bias_damping != 0.0f)
		highPassFilter(audio_out_buffer, DSP_SAMPLES_PER_BUFFER, audio_out_center, audio_out_bias_damping);
}

inline void shiftStitching (StereoSample* bu)
{
	// copy the stitching from end to start of buffer:
	copy_data(bu, bu+DSP_SAMPLES_PER_BUFFER, DSP_SAMPLES_STITCHING);
}

inline void Dsp::shiftOutputStitching() { shiftStitching(audio_out_buffer); }

inline void Dsp::shiftInputStitching() { shiftStitching(audio_in_buffer); }

inline void Dsp::copyInputToOutputBuffer()
{
	// copy audio_in_buffer[] to audio_out_buffer[] or just clear audio_out_buffer[]
	// preserve stitching at start

	if (play_through_damping == 0.0f)
		clear_data(audio_out_buffer+DSP_SAMPLES_STITCHING, StereoSample(0), DSP_SAMPLES_PER_BUFFER);
	else
		copy_data_with_volume(audio_out_buffer+DSP_SAMPLES_STITCHING, audio_in_buffer+DSP_SAMPLES_STITCHING,
							  DSP_SAMPLES_PER_BUFFER, play_through_damping);
}

inline void Dsp::clearInputBuffer()
{
	// clear audio_in_buffer[]
	// preserve stitching at start
	clear_data(audio_in_buffer+DSP_SAMPLES_STITCHING, StereoSample(0), DSP_SAMPLES_PER_BUFFER);
}

Dsp::Dsp (const QAudioDeviceInfo& outDevInfo, const QAudioDeviceInfo& inDevInfo)
{
	QAudioFormat output_format{outDevInfo.preferredFormat()};
	output_format.setSampleType(QAudioFormat::Float);
	output_format.setChannelCount(2);
	output_format = outDevInfo.nearestFormat(output_format);
	print_QAudioFormat("Dsp Audio Output Device", output_format);
	assert(outDevInfo.isFormatSupported(output_format));
	assert(output_format.channelCount() == 2);
	assert(output_format.sampleType() == QAudioFormat::Float);
	assert(output_format.sampleSize() == sizeof(float)*_bits_per_byte);
	assert(output_format.bytesPerFrame() == sizeof(StereoSample));

	QAudioFormat input_format{inDevInfo.nearestFormat(output_format)};
	print_QAudioFormat("Dsp Audio Input Device", input_format);
	assert(inDevInfo.isFormatSupported(input_format));
	assert(input_format.channelCount() == 2);
	assert(input_format.sampleType() == QAudioFormat::Float);
	assert(input_format.sampleSize() == sizeof(float)*_bits_per_byte);
	assert(input_format.bytesPerFrame() == sizeof(StereoSample));

	samples_per_second = output_format.sampleRate();
	system_time = 0.0;

	audio_output = new QAudioOutput (outDevInfo, output_format);
	//audio_output->setBufferSize(output_format.bytesForFrames(DSP_SAMPLES_PER_BUFFER));
	connect(audio_output, &QAudioOutput::stateChanged, [](QAudio::State state)
	{
		assert(isMainThread());
		switch (state)
		{
		case QAudio::ActiveState:
			debugstr("audio_output switched to ActiveState\n");
			break;
		case QAudio::SuspendedState:
			debugstr("audio_output switched to SuspendedState\n");
			break;

		case QAudio::IdleState:	// Finished playing (no more data)
			debugstr("audio_output switched to IdleState\n");
			audio_output->stop();
			//sourceFile.close();
			//delete audio_output;
			break;

		case QAudio::StoppedState:	// Stopped for other reasons
			debugstr("audio_output switched to StoppedState\n");
			if (audio_output->error() != QAudio::NoError)
			{
				debugstr("audio_output error = %i\n", audio_output->error());
			}
			break;

		case QAudio::InterruptedState:
			debugstr("audio_output switched to InterruptedState\n");
			break;
		}
	});

	audio_input = new QAudioInput (inDevInfo, input_format);
	audio_input->setBufferSize(input_format.bytesForFrames(DSP_SAMPLES_PER_BUFFER));
	connect(audio_input, &QAudioInput::stateChanged, [](QAudio::State state)
	{
		switch (state)
		{
		case QAudio::ActiveState:
			debugstr("audio_input switched to ActiveState\n");
			break;
		case QAudio::SuspendedState:
			debugstr("audio_input switched to SuspendedState\n");
			break;

		case QAudio::IdleState:	// Finished playing (no more data)
			debugstr("audio_input switched to IdleState\n");
			audio_input->stop();
			//sourceFile.close();
			//delete audio_output;
			break;

		case QAudio::StoppedState:	// Stopped for other reasons
			debugstr("audio_input switched to StoppedState\n");
			if (audio_output->error() != QAudio::NoError)
			{
				debugstr("audio_input error = %i\n", audio_output->error());
			}
			break;

		case QAudio::InterruptedState:
			debugstr("audio_input switched to InterruptedState\n");
			break;
		}
	});

	bool success = this->open(QIODevice::ReadWrite);
	assert(success);
}

Dsp::~Dsp()
{
	if (audio_input)  audio_input->stop();
	if (audio_output) audio_output->stop();
	delete audio_input;
	delete audio_output;
}

void Dsp::start()
{
	logIn("Dsp::start()");

	assert(audio_runner_device->isOpen());

	samples_per_second = audio_output->format().sampleRate();
	system_time = 0.0;

	//audio_output->setVolume(1.0);
	audio_output->start(audio_runner_device);
	logline("audio_output buffer size = %i", audio_output->bufferSize());

	if (audio_input_device_enabled)
		audio_input->start(audio_runner_device);
}

qint64 Dsp::bytesAvailable() const
{
	return DSP_SAMPLES_PER_BUFFER * sizeof(StereoSample);
	//return m_buffer.size() + QIODevice::bytesAvailable();
}

qint64 Dsp::readData (char* data, qint64 size)
{
	// run machines and return generated data from audio_out_buffer[]
	// to be sent to audioOut device
	// ATTN: this function _must_ read size bytes or -1 for QIODevice to work properly.

	assert(isMainThread() == false);
	assert(audio_output->format().bytesPerFrame() == sizeof(StereoSample));
	assert(uint64(size) % sizeof(StereoSample) == 0);

	//assert(uint64(size) <= DSP_SAMPLES_PER_BUFFER * sizeof(StereoSample));
	//assert(size == DSP_SAMPLES_PER_BUFFER * sizeof(StereoSample));
	if (uint64(size) < DSP_SAMPLES_PER_BUFFER * sizeof(StereoSample))
		return 0;
	size = DSP_SAMPLES_PER_BUFFER * sizeof(StereoSample);

	shiftOutputStitching();
	copyInputToOutputBuffer();
	runMachinesForSound();
	system_time += DSP_SAMPLES_PER_BUFFER / samples_per_second;

	if (audio_output_device_enabled && audio_output_volume > 0.0f)
	{
		highpassOutputBuffer();
		copy_data_with_volume(reinterpret_cast<StereoSample*>(data), audio_out_buffer, DSP_SAMPLES_PER_BUFFER, audio_output_volume);
	}
	else clear_data(reinterpret_cast<StereoSample*>(data), StereoSample(), DSP_SAMPLES_PER_BUFFER);

	return size;
}

qint64 Dsp::writeData (const char* data, qint64 size)
{
	// copy data from audioIn device into audio_in_buffer[]
	// ATTN: this function _must_ write size bytes or -1 for QIODevice to work properly.

	shiftInputStitching();
	if (audio_input_device_enabled)
	{
		copy_data(audio_in_buffer+DSP_SAMPLES_STITCHING, reinterpret_cast<cStereoSample*>(data), DSP_SAMPLES_PER_BUFFER);
		highpassInputBuffer();
	}
	else clearInputBuffer();

	return size;
}

void Dsp::enableAudioInputDevice (bool f)
{
	if (f && !audio_input_device_present)
	{
		showWarning( "No audio input device found." );
	}
	audio_input_device_enabled = f && audio_input_device_present;
}

void Dsp::enableAudioOutputDevice (bool f)
{
	audio_output_device_enabled = f;
}

void Dsp::setOutputVolume (Sample volume)
{
	if (volume <= 0.0f)
	{
		audio_output_device_enabled = off;
	}
	else
	{
		audio_output_volume = fminf(volume,1.0f);
		audio_output_device_enabled = on;
	}
}

void Dsp::outputSamples (cStereoSample& sample, Time aa/*start [seconds]*/, Time ee/*end [seconds]*/)
{
	// Output sample value to audio_out_buffer[]:

	aa *= samples_per_second;		// sample-based
	ee *= samples_per_second;		// ""

	int32 a = int32(aa);
	int32 e = int32(ee);

	assert(a>=0);
	assert(e>=a);
	assert(e<DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING);

	StereoSample* pa = audio_out_buffer + a;
	*pa -= sample * Sample(aa-floor(aa));

	StereoSample* pe = audio_out_buffer + e;
	*pe += sample * Sample(ee-floor(ee));

	while (pa<pe) { *pa++ += sample; }
}

void Dsp::outputSamples (Sample sample, Time aa/*start [seconds]*/, Time ee/*end [seconds]*/)
{
	// Output sample value to audio_out_buffer[]:

	aa *= samples_per_second;		// sample-based
	ee *= samples_per_second;		// ""

	int32 a = int32(aa);
	int32 e = int32(ee);

	assert(a>=0);
	assert(e>=a);
	assert(e<DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING);

	StereoSample* pa = audio_out_buffer + a;
	*pa -= sample * Sample(aa-floor(aa));

	StereoSample* pe = audio_out_buffer + e;
	*pe += sample * Sample(ee-floor(ee));

	while (pa<pe) { *pa++ += sample; }
}

static void filter_QAudioDeviceInfoList (QList<QAudioDeviceInfo>& list)
{
	for (int i = 0; i < list.size(); i++ )
	{
		if (list[i].supportedCodecs().size() == 0)
			list.removeAt(i--);
	}
}

void Dsp::startCoreAudio (bool /*input_enabled*/)
{
	debugstr("startCoreAudio\n");

	IFDEBUG( { uint64 i=0; double d; memcpy(&d,&i,8); assert(d==0.0); } )
	IFDEBUG( { uint32 i=0; float d; memcpy(&d,&i,4); assert(d==0.0f); } )

	audioOut_device = QAudioDeviceInfo::defaultOutputDevice();
	audioIn_device = QAudioDeviceInfo::defaultInputDevice();

	if ((0))
	{
		QList<QAudioDeviceInfo> audioOut_devices {QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)};
		filter_QAudioDeviceInfoList(audioOut_devices);

		logline("-----");
		for (int i=0; i<audioOut_devices.size(); i++)
		{
			bool f = audioOut_devices[i] == audioOut_device;
			cstr s = f ? "*** DEFAULT audioOut device ***" : "";
			print_QAudioDeviceInfo(usingstr("audioOut device #%i: %s",i,s),audioOut_devices[i]);
			logline("-----");
		}
	}

	if ((0))
	{
		QList<QAudioDeviceInfo> audioIn_devices {QAudioDeviceInfo::availableDevices(QAudio::AudioInput)};
		filter_QAudioDeviceInfoList(audioIn_devices);

		logline("-----");
		for (int i=0; i<audioIn_devices.size(); i++)
		{
			bool f = audioIn_devices[i] == audioIn_device;
			cstr s = f ? "*** DEFAULT audioIn device ***" : "";
			print_QAudioDeviceInfo(usingstr("audioIn device #%i: %s",i,s),audioIn_devices[i]);
			logline("-----");
		}
	}

	audio_runner_device = new Dsp(audioOut_device,audioIn_device);
	audio_runner_device->start();
}

void Dsp::stopCoreAudio ( )
{
	debugstr("stopCoreAudio\n");
}

































