#pragma once
/*	Copyright  (c)	Günter Woigk 2002 - 2019
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
#include "DspTime.h"
#include "StereoSample.h"
#include <QIODevice>

typedef float Sample;
typedef Sample const cSample;
class StereoSample;
typedef StereoSample const cStereoSample;
class QAudioOutput;
class QAudioInput;
class QAudioDeviceInfo;

class Dsp : private QIODevice
{
	Q_OBJECT

	qint64 readData (char* data, qint64 max) override;
	qint64 writeData (const char* data, qint64 max) override;
	qint64 bytesAvailable() const override;

	static inline void highpassInputBuffer();
	static inline void highpassOutputBuffer();
	static inline void shiftOutputStitching();
	static inline void shiftInputStitching();
	static inline void copyInputToOutputBuffer();
	static inline void clearInputBuffer();

	static QAudioOutput* audio_output;
	static QAudioInput*  audio_input;

public:
	static StereoSample	audio_out_buffer[];
	static StereoSample	audio_in_buffer[];

	static bool	audio_input_device_present;
	static bool	audio_input_device_enabled;
	static bool	audio_output_device_enabled;
	static Sample audio_output_volume;

	Dsp (const QAudioDeviceInfo& outDevInfo, const QAudioDeviceInfo& inDevInfo);
	~Dsp () override;

	static void start ();

	static void startCoreAudio (bool input_enabled);
	static void stopCoreAudio  ();

	static inline bool isAudioInputDevicePresent () noexcept { return audio_input_device_present; }
	static inline bool isAudioInputDeviceEnabled () noexcept { return audio_input_device_enabled; }
	static void enableAudioInputDevice (bool);

	static inline bool isAudioOutputDevicePresent () noexcept { return 1/*yes*/; }
	static inline bool isAudioOutputDeviceEnabled () noexcept { return audio_output_device_enabled; }
	static void enableAudioOutputDevice (bool);

	static void setOutputVolume (Sample volume);
	static inline Sample getOutputVolume () noexcept { return audio_output_volume; }

	//void setPlaythrough (uint mode);
	//		enum
	//		{		playthrough_minus10dB,
	//				playthrough_minus30dB,
	//			 	playthrough_off
	//		};

	static void outputSamples (cStereoSample&, Time start, Time end);
	static void outputSamples (Sample, Time start, Time end);
};



















