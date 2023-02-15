// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Application.h"
#include "Dsp.h"
#include "Qt/Settings.h"
#include "StereoSample.h"
#include "cpp/cppthreads.h"
#include "cstrings/tempmem.h"
#include "kio/TestTimer.h"
#include <CoreAudio/CoreAudio.h>


// Time		system_time			= 0.0;
// Frequency	samples_per_second	= 44100;


namespace Dsp
{

// ---- core audio interrupt ----

static PLock audio_callback_lock; // 2006-11-16 kio:	needed if in and out devices are different

// bool			audio_input_device_present	= no;	// preset to no if error
// bool			audio_input_device_enabled	= no;	// preset to no if error
// bool			audio_output_device_enabled = yes;	// must be present
// Sample 		audio_output_volume			= 0.3f;
// Sample  		play_through_damping		= 0.0;	// no playthrough during startup: no 'klick' if audio-in disabled in
// prefs
static StereoSample audio_out_center = 0.0f; // for audio out High-pass filter
static StereoSample audio_in_center	 = 0.0f; // for audio in High-pass filter

// StereoSample	audio_out_buffer [DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING];
// StereoSample	audio_in_buffer	 [DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING];

static AudioDeviceID	   input_device_id	  = 0; // actually used audio devices
static AudioDeviceID	   output_device_id	  = 0;
static AudioDeviceIOProcID audio_out_ioProcID = nullptr; // kio 2012-08-08   procIDs for my audioDeviceIOProc()
static AudioDeviceIOProcID audio_in_ioProcID  = nullptr; // kio 2012-08-08


// ###################################################################################
// High-pass filter: (remove signal bias)

inline void HighPassFilter(StereoSample* p, int count, StereoSample& center, float damping)
{
	for (StereoSample* e = p + count; p < e; p++) { center += (*p -= center) * damping; }
}

inline void HighpassInputBuffer()
{
	HighPassFilter(audio_in_buffer + DSP_SAMPLES_STITCHING, DSP_SAMPLES_PER_BUFFER, audio_in_center, 0.001);
}

inline void HighpassOutputBuffer()
{
	HighPassFilter(audio_out_buffer, DSP_SAMPLES_PER_BUFFER, audio_out_center, 0.001);
}


// ###################################################################################
// audio input handling:

inline void ShiftInputStitching() { shiftBuffer(audio_in_buffer); }

inline void ClearInputBuffer()
{
	StereoSample* z = audio_in_buffer + DSP_SAMPLES_STITCHING;
	StereoSample* e = z + DSP_SAMPLES_PER_BUFFER;
	while (z < e) { *z++ = 0.0; }
}

inline void ReadInputData(const AudioBufferList* inInputData)
{
	uint buffers  = inInputData->mNumberBuffers;
	uint channels = inInputData->mBuffers[0].mNumberChannels;


	static bool f = 1;
	if (f)
	{
		f = 0;
		logline("Dsp:audioDeviceIOProc: %u audio-in buffers.", buffers);
		logline("Dsp:audioDeviceIOProc: %u channels per buffer.", channels);
	}

	IFDEBUG(for (uint i = 1; i < buffers; i++) {
		assert(inInputData->mBuffers[i].mNumberChannels == channels);
		assert(inInputData->mBuffers[i].mDataByteSize == (DSP_SAMPLES_PER_BUFFER * channels * sizeof(Sample)));
		assert(buffers == 1 || channels == 1);
	})

	StereoSample* z = audio_in_buffer + DSP_SAMPLES_STITCHING;
	StereoSample* e = z + DSP_SAMPLES_PER_BUFFER;

	switch (buffers)
	{
	default: // multiple buffers => assume 1 channel per buffer
	{		 //  2 buffers:  stereo?		not yet seen		2007-08-13
			 // >2 buffers:  dolby 5.1?	not yet seen		2007-08-13

		// ignore other channels:
		//	for( uint i=2; i<inInputData->mNumberBuffers; i++ ) {}

		// copy audio-in buffers to audio_in_buffer to  (stereo -> stereo):
		cSample* q0 = (cSample*)inInputData->mBuffers[0].mData;
		cSample* q1 = (cSample*)inInputData->mBuffers[1].mData;
		while (z < e) { *z++ = StereoSample(*q0++, *q1++); }
	}
	break;

	case 1: // 1 buffer => assume interleaved channels
	{		//   1 channel: mono
			//	 2 channels: stereo  (99% of all cases)
			//  >2 channels: dolby5.1?

		switch (channels)
		{
		case 1: // mono
		{
			cSample* q = (cSample*)inInputData->mBuffers[0].mData;
			while (z < e) { *z++ = *q++; }
		}
		break;

		case 2: // interleaved stereo ((99% of cases))
		{
			cStereoSample* q = (cStereoSample*)inInputData->mBuffers[0].mData;
			while (z < e) { *z++ = *q++; }
		}
		break;

		default: // e.g. 6 channels: dolby 5.1?  l/r channel seem to be buffer[0/1]
		{
			cSample* q = (cSample*)inInputData->mBuffers[0].mData;
			while (z < e)
			{
				*z++ = *(StereoSample*)(q);
				q += channels;
			}
		}
		break;
		}
		break;
	}
	}
}


// ###################################################################################
// audio output handling:

inline void ShiftOutputStitching() { shiftBuffer(audio_out_buffer); }

inline void CopyInputToOutputBuffer()
{
	IFDEBUG({
		static union
		{
			uint64 i;
			double d;
		};
		i = 0;
		assert(d == 0.0);
	})

	memset(audio_out_buffer + DSP_SAMPLES_STITCHING, 0, DSP_SAMPLES_PER_BUFFER * sizeof(*audio_out_buffer));

	//	if( play_through_damping==0.0 )
	//		for( uint i=DSP_SAMPLES_STITCHING; i < DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING; i++)
	//		{ audio_out_buffer[i] = 0.0; }
	//	else
	//		for( uint i=DSP_SAMPLES_STITCHING; i < DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING; i++ )
	//		{ audio_out_buffer[i] = audio_in_buffer[i] * play_through_damping; }
}

inline void WriteOutputData(AudioBufferList* outOutputData)
{
	uint buffers  = outOutputData->mNumberBuffers;
	uint channels = outOutputData->mBuffers[0].mNumberChannels;

	static bool f = 1;
	if (f)
	{
		f = 0;
		logline("Dsp:audioDeviceIOProc: %u audio-out buffers.", buffers);
		logline("Dsp:audioDeviceIOProc: %u channels per buffer.", channels);
	}
	IFDEBUG(for (uint i = 1; i < buffers; i++) {
		assert(outOutputData->mBuffers[i].mNumberChannels == channels);
		assert(outOutputData->mBuffers[i].mDataByteSize == (DSP_SAMPLES_PER_BUFFER * channels * sizeof(Sample)));
		assert(buffers == 1 || channels == 1);
	})

	cStereoSample* q	  = audio_out_buffer;
	cStereoSample* e	  = q + DSP_SAMPLES_PER_BUFFER;
	Sample		   volume = audio_output_volume;

	switch (buffers)
	{
	default: // multiple buffers => assume 1 channel per buffer
	{		 //  2 buffers:  stereo?		not yet seen		2006-10-23
			 // >2 buffers:  dolby 5.1?	MOTU 828 kmII		2007-05-18

		// copy audio_out_buffer to audio-out buffers (mono -> stereo):
		Sample* z0 = (Sample*)outOutputData->mBuffers[0].mData;
		Sample* z1 = (Sample*)outOutputData->mBuffers[1].mData;
		while (q < e)
		{
			*z0++ = q->left * volume;
			*z1++ = q->right * volume;
			q++;
		}

		// clear other channels (if any):
		for (uint i = 2; i < buffers; i++)
		{
			Sample* z = (Sample*)outOutputData->mBuffers[i].mData;
			Sample* e = z + outOutputData->mBuffers[i].mDataByteSize / sizeof(Sample);
			while (z < e) { *z++ = 0.0; }
		}
	}
	break;

	case 1: // 1 buffer => assume interleaved channels
	{		//   1 channel: mono
			//	 2 channels: stereo  (99% of all cases)
			//  >2 channels: dolby5.1?

		Sample* z = (Sample*)outOutputData->mBuffers[0].mData;

		switch (channels)
		{
		case 1: // mono
			while (q < e) { *z++ = (q++)->mono() * volume; }
			break;

		case 2: // interleaved stereo ((99% of cases))
			while (q < e)
			{
				z[0] = q->left * volume;
				z[1] = q->right * volume;
				q++;
				z += 2;
			}
			break;

		default: // e.g. 6 channels: dolby 5.1?  l/r channel seem to be buffer[0/1]
			while (q < e)
			{
				z[0] = q->left * volume;
				z[1] = q->right * volume;
				q++;
				z += 2;
				for (uint i = 2; i < channels; i++) { *z++ = 0.0; }
			}
			break;
		}
		break;
	}
	}
}


// ###################################################################################
//	Core Audio Callback Routine:
//	-that's what it's all about-
//
static OSStatus audioDeviceIOProc(
	AudioDeviceID /*inDevice*/,
	const AudioTimeStamp* /*inNow*/,
	const AudioBufferList* inInputData,
	const AudioTimeStamp*  inInputTime,
	AudioBufferList*	   outOutputData,
	const AudioTimeStamp*  inOutputTime,
	void* /*inClientData*/
)
{
	TempMemPool tmp; // better create an explicite temp mem pool for alien threads

	TT; // Test Timer

	PLocker lock(audio_callback_lock); // 2006-11-16 kio

	try
	{
		if ((inInputData && inInputData->mNumberBuffers != 0) || !audio_input_device_present)
		{
			static float64 lasttime = 0;
			if (inInputTime->mSampleTime - lasttime != DSP_SAMPLES_PER_BUFFER && lasttime != 0.0)
				logline(
					"WARNING audio-in at sample %lu: Δ samples = %lu",
					(ulong)inInputTime->mSampleTime,
					(ulong)(inInputTime->mSampleTime - lasttime));
			lasttime = inInputTime->mSampleTime;

			ShiftInputStitching();
			if (audio_input_device_enabled /*implies audio_input_device_present*/)
			{
				ReadInputData(inInputData);
				if (0) HighpassInputBuffer();
			}
			else
				ClearInputBuffer();
		}
		else // audio-out-only callback for systems with different audio-in and -out device IDs
		{
			static bool f = 1;
			if (f)
			{
				f = 0;
				logline("Dsp:audioDeviceIOProc: separate audio-in and -out callbacks.");
			}
		}

		if (outOutputData && outOutputData->mNumberBuffers != 0)
		{
			static float64 lasttime = 0;
			if (inOutputTime->mSampleTime - lasttime != DSP_SAMPLES_PER_BUFFER && lasttime != 0.0)
				logline(
					"WARNING audio-out at sample %lu: Δ samples = %lu",
					(ulong)inOutputTime->mSampleTime,
					(ulong)(inOutputTime->mSampleTime - lasttime));
			lasttime = inOutputTime->mSampleTime;

			ShiftOutputStitching();
			CopyInputToOutputBuffer();
			runMachinesForSound(); // DOIT!
			if (audio_output_device_enabled && audio_output_volume > 0.0)
			{
				if (0) HighpassOutputBuffer();
				WriteOutputData(outOutputData);
			}

			system_time += DSP_SAMPLES_PER_BUFFER / samples_per_second;

#if XLOG
			{
				Time time = now() - tt._start;

				static uint n = 0;
				static Time times[1000];
				times[n++] = time;
				if (n == NELEM(times))
				{
					Time min = 1;
					Time max = 0;
					time	 = 0;
					while (n)
					{
						Time t = times[--n];
						time += t;
						if (t < min) min = t;
						if (t > max) max = t;
					}
					xlogline(
						"audio-out interrupt: min %.3f msec, max %.3f msec, average %.3f msec",
						min * 1000,
						max * 1000,
						time);
				}
			}
#endif

			TTest(0.8 * seconds_per_dsp_buffer(), "WARNING: audio-out interrupt took %.3f msec");
		}
		else
		{
#if XLOG
			{
				Time time = now() - tt._start;

				static uint n = 0;
				static Time times[1000];
				times[n++] = time;
				if (n == NELEM(times))
				{
					Time min = 1;
					Time max = 0;
					time	 = 0;
					while (n)
					{
						Time t = times[--n];
						time += t;
						if (t < min) min = t;
						if (t > max) max = t;
					}
					xlogline(
						"audio-in  interrupt: min %.3f msec, max %.3f msec, average %.3f msec",
						min * 1000,
						max * 1000,
						time);
				}
			}
#endif

			TTest(0.8 * seconds_per_dsp_buffer(), "WARNING: audio-in interrupt took %.3f msec");
		}

		return 0; // success
	}
	catch (std::exception& e)
	{
		logline("audio irpt: exception: %s", e.what());
		return -1; // error
	}
}


// ###################################################################################
//	stop core audio interrupts
//
void stopCoreAudio()
{
	xlogIn("Dsp:StopCoreAudio");

	// remove callbacks:
	(void)AudioDeviceStop(input_device_id, audio_in_ioProcID);	 // kio 2012-08-08
	(void)AudioDeviceStop(output_device_id, audio_out_ioProcID); // kio 2012-08-08

	// wait for any current sound callback to finish:
	PLocker lock(audio_callback_lock);
	//    waitDelay(0.01);
}


// ###################################################################################
//	start core audio interrupt
//		abort on any problem with the output device
//		disable audio input on any problem with the input device
//
void startCoreAudio(bool input_enabled) //, int playthrough_mode)
{
	xlogIn("Dsp:StartCoreAudio");
	xlogline("DSP_SAMPLES_PER_BUFFER = %u", uint(DSP_SAMPLES_PER_BUFFER));


	OSStatus				   status = 0 /*ok*/;
	AudioObjectPropertyAddress address; // kio 2012-04-29
	UInt32					   propertySize;

#if 0
	status = UpdateDeviceList();
	if(status) throw AnyError("Dsp:UpdateDeviceList");
	status = AudioHardwareAddPropertyListener(kAudioHardwarePropertyDevices, AHPropertyListenerProc, nullptr );
	if(status) throw AnyError("AudioHardwareAddPropertyListener");
#endif

	// block audio interrupt until setup completed:
	PLocker lock(audio_callback_lock);

	// clear stitching samples: (required?)
	for (int i = 0; i < DSP_SAMPLES_STITCHING; i++)
	{
		audio_out_buffer[DSP_SAMPLES_PER_BUFFER + i] = 0.0;
		audio_in_buffer[DSP_SAMPLES_PER_BUFFER + i]	 = 0.0;
	}


	// Setup audio-out:
	// abort on any problem with the output device

	try
	{
		// Get output device ID:

		propertySize	  = sizeof(output_device_id);
		address.mSelector = kAudioHardwarePropertyDefaultOutputDevice; // kio 2012-04-29
		address.mScope	  = kAudioObjectPropertyScopeGlobal;		   // kio 2012-04-29
		address.mElement  = kAudioObjectPropertyElementMaster;		   // kio 2012-04-29
		status			  = AudioObjectGetPropertyData(
			   kAudioObjectSystemObject,
			   &address, // kio 2012-04-29
			   0,
			   nullptr,
			   &propertySize,
			   &output_device_id); // kio 2012-04-29

		if (status) throw AnyError("GetDefaultOutputDevice");
		if (output_device_id == kAudioDeviceUnknown) throw AnyError("GetDefaultOutputDevice: kAudioDeviceUnknown");

		// Check & print output device status

		AudioStreamBasicDescription outputStreamBasicDescription;
		propertySize	  = sizeof(outputStreamBasicDescription);
		address.mSelector = kAudioDevicePropertyStreamFormat; // kio 2012-04-29
		address.mScope	  = kAudioDevicePropertyScopeOutput;  // kio 2012-04-29
		address.mElement  = 0 /*Channel*/;					  // kio 2012-04-29 TODO: Channel
		status			  = AudioObjectGetPropertyData(		  // kio 2012-04-29
			   output_device_id,					  // AudioObjectID inObjectID						// kio 2012-04-29
			   &address,					   // const AudioObjectPropertyAddress* inAddress	// kio 2012-04-29
			   0,							   // UInt32 inQualifierDataSize 					// kio 2012-04-29
			   nullptr,						   // const void* inQualifierData 					// kio 2012-04-29
			   &propertySize,				   // UInt32* ioDataSize 							// kio 2012-04-29
			   &outputStreamBasicDescription); // void* outData   								// kio 2012-04-29

		if (status) throw AnyError("GetOutputStreamFormat");

		{
			logIn("Dsp: CoreAudio default output device:");
			logline("%.2f mSampleRate", outputStreamBasicDescription.mSampleRate);
			logline("%4.4s mFormatID", (char* /*str*/) & outputStreamBasicDescription.mFormatID);
			logline("%4u mBytesPerPacket", uint(outputStreamBasicDescription.mBytesPerPacket));
			logline("%4u mFramesPerPacket", uint(outputStreamBasicDescription.mFramesPerPacket));
			logline("%4u mBytesPerFrame", uint(outputStreamBasicDescription.mBytesPerFrame));
			logline("%4u mChannelsPerFrame", uint(outputStreamBasicDescription.mChannelsPerFrame));
			logline("%4u mBitsPerChannel", uint(outputStreamBasicDescription.mBitsPerChannel));
		}
		samples_per_second = (Frequency)outputStreamBasicDescription.mSampleRate;

		UInt32 ofmt = outputStreamBasicDescription.mFormatID;
		UInt32 ocpf = outputStreamBasicDescription.mChannelsPerFrame;
		UInt32 obpf = outputStreamBasicDescription.mBytesPerFrame;

		if (ofmt != kAudioFormatLinearPCM) throw AnyError("Default Audio Output Device does not use Linear PCM");
		if (obpf != ocpf * sizeof(Sample)) throw AnyError("Default Audio Output Device has not 4 bytes per sample");

		// Configure output device

		UInt32 bufferByteCount;
		propertySize	  = sizeof(bufferByteCount);
		bufferByteCount	  = DSP_SAMPLES_PER_BUFFER * obpf /*output bytes per frame*/;
		address.mSelector = kAudioDevicePropertyBufferSize;	 // kio 2012-04-29
		address.mScope	  = kAudioDevicePropertyScopeOutput; // kio 2012-04-29
		address.mElement  = 0 /*Channel*/;					 // kio 2012-04-29 TODO: Channel
		status			  = AudioObjectSetPropertyData(		 // kio 2012-04-29
			   output_device_id,					 // AudioObjectID inObjectID, 					// kio 2012-04-29
			   &address,		  // const AudioObjectPropertyAddress*inAddress, 	// kio 2012-04-29
			   0,				  // UInt32 inQualifierDataSize, 					// kio 2012-04-29
			   nullptr,			  // const void* inQualifierData, 				// kio 2012-04-29
			   propertySize,	  // UInt32 inDataSize, 							// kio 2012-04-29
			   &bufferByteCount); // const void* inData							// kio 2012-04-29

		if (status) throw AnyError("SetOutputBufferSize");

		// Start audio output interrupt

		status = AudioDeviceCreateIOProcID(
			output_device_id, audioDeviceIOProc, nullptr, &audio_out_ioProcID);						  // kio 2012-04-29
		if (status) throw AnyError("AudioDeviceCreateIOProcID");									  // kio 2012-04-29
		if (audio_out_ioProcID == nullptr) throw AnyError("AudioDeviceCreateIOProcID returned NULL"); // kio 2012-04-29

		status = AudioDeviceStart(output_device_id, audio_out_ioProcID); // kio 2012-04-29
		if (status) throw AnyError("AudioDeviceStart(output)");
	}
	catch (std::exception& e)
	{
		showAlert(
			"Audio output setup failed:\n%s.\n"
			"Select another default audio output device in the System Settings and start again.",
			status ? usingstr("%s: error %i", e.what(), int(status)) : e.what());
		return;
	}


	// Setup audio-in:
	// disable audio-in on any problem with the input device

	try
	{
		// Get input_device_id

		propertySize	  = sizeof(input_device_id);
		address.mSelector = kAudioHardwarePropertyDefaultInputDevice; // kio 2012-04-29
		address.mScope	  = kAudioObjectPropertyScopeGlobal;		  // kio 2012-04-29
		address.mElement  = kAudioObjectPropertyElementMaster;		  // kio 2012-04-29

		status = AudioObjectGetPropertyData(
			kAudioObjectSystemObject,
			&address,
			0,
			nullptr, // kio 2012-04-29
			&propertySize,
			&input_device_id); // kio 2012-04-29

		if (status) throw AnyError("GetDefaultInputDevice");
		if (input_device_id == kAudioDeviceUnknown) throw AnyError("GetDefaultInputDevice: kAudioDeviceUnknown");

		xlogline("Dsp: Input and output device are %s.", output_device_id == input_device_id ? "same" : "different");

		// Check & print the input device status

		AudioStreamBasicDescription inputStreamBasicDescription;
		propertySize	  = sizeof(inputStreamBasicDescription);
		address.mSelector = kAudioDevicePropertyStreamFormat; // kio 2012-04-29
		address.mScope	  = kAudioDevicePropertyScopeInput;	  // kio 2012-04-29
		address.mElement  = 0 /*Channel*/;					  // kio 2012-04-29
		status			  = AudioObjectGetPropertyData(
			   input_device_id,				  // AudioObjectID inObjectID			// kio 2012-04-29
			   &address,					  // const AudioObjectPropertyAddress* inAddress	// kio 2012-04-29
			   0,							  // UInt32 inQualifierDataSize 		// kio 2012-04-29
			   nullptr,						  // const void* inQualifierData 		// kio 2012-04-29
			   &propertySize,				  // UInt32* ioDataSize 				// kio 2012-04-29
			   &inputStreamBasicDescription); // void* outData   					// kio 2012-04-29

		if (status) throw AnyError("GetInputStreamFormat");

		{
			logIn("Dsp: CoreAudio default input device:");
			logline("%.2f mSampleRate", inputStreamBasicDescription.mSampleRate);				   // 44100
			logline("%4.4s mFormatID", (char*)&inputStreamBasicDescription.mFormatID);			   // lpcm
			logline("%4u mBytesPerPacket", uint(inputStreamBasicDescription.mBytesPerPacket));	   // 8
			logline("%4u mFramesPerPacket", uint(inputStreamBasicDescription.mFramesPerPacket));   // 1
			logline("%4u mBytesPerFrame", uint(inputStreamBasicDescription.mBytesPerFrame));	   // 8
			logline("%4u mChannelsPerFrame", uint(inputStreamBasicDescription.mChannelsPerFrame)); // 2
			logline("%4u mBitsPerChannel", uint(inputStreamBasicDescription.mBitsPerChannel));	   // 32
		}
		UInt32 ifmt = inputStreamBasicDescription.mFormatID;
		UInt32 icpf = inputStreamBasicDescription.mChannelsPerFrame;
		UInt32 ibpf = inputStreamBasicDescription.mBytesPerFrame;

		if (ifmt != kAudioFormatLinearPCM) throw AnyError("Default Audio Input Device does not use Linear PCM");
		if (ibpf != icpf * sizeof(Sample)) throw AnyError("Default Audio Input Device has not 4 bytes per sample");
		if (samples_per_second != Frequency(inputStreamBasicDescription.mSampleRate))
			throw AnyError("Audio-in and -out sampling frequencies differ");

		// Configure input device

		if (input_device_id != output_device_id)
		{
			UInt32 bufferByteCount;
			propertySize	  = sizeof(bufferByteCount);
			bufferByteCount	  = DSP_SAMPLES_PER_BUFFER * ibpf /*input bytes per frame*/;
			address.mSelector = kAudioDevicePropertyBufferSize; // kio 2012-04-29
			address.mScope	  = kAudioDevicePropertyScopeInput; // kio 2012-04-29
			address.mElement  = 0 /*Channel*/;					// kio 2012-04-29	TODO Channel

			status = AudioObjectSetPropertyData( // kio 2012-04-29
				input_device_id,				 // AudioObjectID inObjectID, 					// kio 2012-04-29
				&address,						 // const AudioObjectPropertyAddress* inAddress, // kio 2012-04-29
				0,								 // UInt32 inQualifierDataSize, 					// kio 2012-04-29
				nullptr,						 // const void* inQualifierData, 				// kio 2012-04-29
				propertySize,					 // UInt32 inDataSize, 							// kio 2012-04-29
				&bufferByteCount);				 // const void* inData							// kio 2012-04-29

			if (status) throw AnyError("SetBufferSize");

			// Start audio input interrupt

			status = AudioDeviceCreateIOProcID(
				input_device_id, audioDeviceIOProc, nullptr, &audio_in_ioProcID); // kio 2012-04-29
			if (status) throw AnyError("AudioDeviceCreateIOProcID");			  // kio 2012-04-29
			if (audio_in_ioProcID == nullptr)
				throw AnyError("AudioDeviceCreateIOProcID returned NULL"); // kio 2012-04-29
			status = AudioDeviceStart(input_device_id, audio_in_ioProcID); // kio 2012-04-29
			if (status) throw AnyError("AudioDeviceStart");
		}

		audio_input_device_present = yes;
		audio_input_device_enabled = input_enabled;
		//		setPlaythrough( playthrough_mode );
	}
	catch (AnyError& e)
	{
		if (settings.get_bool(key_warn_if_audio_in_fails, yes))
		{
			xlogline("Dsp: Audio input setup failed:");
			xlogline(status ? "Dsp: %s: error = %ld." : "Dsp: %s.", e.what(), status);

			showWarning(
				"Audio input setup failed:\n%s.\n"
				"Select another default audio input device in the OSX System Settings if you want to load programmes from an external tape recorder.\n"
				"Note: this message can be disabled in the preferences.",
				status ? usingstr("%s: error %i", e.what(), int(status)) : e.what());
		}
	}
}


// ------------------------------------------------------------------------

/*	Utility:
	apply high pass filter to sample buffer

	center  = running center value of signal
	damping = damping per sample; e.g. 0.001

	returns updated center value.
*/
// Sample HighPassFilter ( Sample* z, Sample const* q, int count, Sample center, float damping )
//{
//	for( int n=count; n>0; n-- )
//	{
//		Sample qv = *q++;
//		*z++ = qv - center;
//		center += (qv-center)*damping;
//	}
//	return center;
// }


///*	Set audio-in to audio-out playthrough damping:
//*/
// void setPlaythrough ( uint mode/*as in prefs*/ )
//{
//	switch(mode)
//	{
//	case playthrough_minus10dB:		xlogline("Dsp::setPlaythrough: -10dB"); play_through_damping = 1.0/2.0; break;
//	case playthrough_minus30dB:		xlogline("Dsp::setPlaythrough: -30dB"); play_through_damping = 1.0/8.0; break;
//	case playthrough_off:			xlogline("Dsp::setPlaythrough: off");   play_through_damping = 0.0;     break;
//	default:                        IERR();
//	}
//}

} // namespace Dsp
