/*
 * Copyright (c) 2005 James Jacobsson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $RCFS: Audio/Darwin/Darwinaudio.c,v 1.1.1.1 2005/03/17 17:41:54 jaja Exp $
 *
 */

#include "rcfs.h"
#include "logger.h"

#include <stdio.h>
#include <assert.h>

#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/DefaultAudioOutput.h>
#include <AudioToolbox/AudioConverter.h>

static int    audio_mode = 0;
uint16 *buffer;
static AudioDeviceID inputDeviceID;
static AudioStreamBasicDescription inputStreamBasicDescription;
static OSStatus audioIOProc(AudioDeviceID inDevice,const AudioTimeStamp *inNow,const AudioBufferList *inInputData,const AudioTimeStamp *inInputTime,AudioBufferList *outOutputData,const AudioTimeStamp *inOutputTime, void *inClientData);

int Audio_Initialize(int mod) {
	Boolean writeable;
	OSStatus error = noErr;
	UInt32 propertySize;

	audio_mode = mod;
	buffer = (uint16 *)malloc(512*1024); // !! Should be _PLENTY_ (See if this size can be calculated somehow)
	if(buffer == NULL) {
		logger(SYSLOG_FATAL,"SND","Unable to allocate buffer\n");
		exit(0);
	} else {
		//printf("Allocated buffer at %p\n",(void *)buffer);
	}

	if(audio_mode == AUDIO_INPUT) {
		/* Get the default input device ID */
		error = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDefaultInputDevice,&propertySize,&writeable);
		if(error) logger(SYSLOG_FATAL,"SND","Unable to get input deviceID\n");
		error = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,&propertySize,&inputDeviceID);
		if(error) logger(SYSLOG_FATAL,"SND","Unable to get input device properties\n");

		if(inputDeviceID == kAudioDeviceUnknown) logger(SYSLOG_FATAL,"SND","Input device is an unknown device\n");

		/* Get tbe input stream description */
		error = AudioDeviceGetPropertyInfo(inputDeviceID,0,true,kAudioDevicePropertyStreamFormat,&propertySize,&writeable);
		if(error) logger(SYSLOG_FATAL,"SND","Unable to get size of input stream properties\n");
		error = AudioDeviceGetProperty(inputDeviceID,0,true,kAudioDevicePropertyStreamFormat,&propertySize,&inputStreamBasicDescription);
		if(error) logger(SYSLOG_FATAL,"SND","Unable to get input stream properties\n");

		/* Register the AudioDeviceIOProc */
		error = AudioDeviceAddIOProc(inputDeviceID, audioIOProc, NULL);
		if(error) logger(SYSLOG_FATAL,"SND","Unable to register callback routine for input device\n");
	}

	logger(SYSLOG_INFO,"SND","Successfully opened DEFAULT_INPUT at %.0fHz samplingrate",inputStreamBasicDescription.mSampleRate);

	/* Start audio processing */
	error = AudioDeviceStart(inputDeviceID,audioIOProc);
	if(error) logger(SYSLOG_FATAL,"SND","Unable to start audio processing\n");

	return(0);
}

float Audio_GetSamplingRate(void) {
	return(inputStreamBasicDescription.mSampleRate);
}

void Audio_SendBuffer(void) {
}

void Audio_Release(void) {
	/* Unregister the AudioDeviceIOProc */
	AudioDeviceRemoveIOProc(inputDeviceID,audioIOProc);
}

/*
 * Audio callback routine
 */
static OSStatus audioIOProc(AudioDeviceID inDevice,const AudioTimeStamp *inNow,const AudioBufferList *inInputData,const AudioTimeStamp *inInputTime,AudioBufferList *outOutputData,const AudioTimeStamp *inOutputTime, void *inClientData) {
	Float32 *inBuffer,prevSample = 0.0,sample;
	int frames,frame;
	
	inBuffer = (Float32 *)inInputData->mBuffers[0].mData;
	frames = inInputData->mBuffers[0].mDataByteSize / inputStreamBasicDescription.mBytesPerFrame;
	
	for(frame = 0;frame < frames;frame++) {
		sample = (inBuffer[0] + inBuffer[1]) / 2;
		if(sample < 0.3) sample = 0.0;
		if(sample > 0.7) sample = 1.0;
		if( (sample != 0.0) && (sample != 1.0) ) sample = prevSample;
		prevSample = sample;
		
		buffer[frame] = sample;

		inBuffer += inInputData->mBuffers[0].mNumberChannels;
	}
	
	inputdev.AudioDecode(buffer,frames);

	return(noErr);
}
