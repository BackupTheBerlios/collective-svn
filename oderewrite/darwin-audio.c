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

#include "collective.h"
#include "input.h"
#include "logger.h"

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/DefaultAudioOutput.h>
#include <AudioToolbox/AudioConverter.h>

double channel[16];
input_device_t inputdevice;

/*
 * INTERNAL functions from here on
 */
uint16 *buffer;
static AudioDeviceID inputDeviceID;
static AudioStreamBasicDescription inputStreamBasicDescription;
static OSStatus audioIOProc(AudioDeviceID inDevice,const AudioTimeStamp *inNow,const AudioBufferList *inInputData,const AudioTimeStamp *inInputTime,AudioBufferList *outOutputData,const AudioTimeStamp *inOutputTime, void *inClientData);

static int Darwin_Audio_Initialize(void) {
	Boolean writeable;
	OSStatus error = noErr;
	UInt32 propertySize;

	buffer = (uint16 *)malloc(512*1024); // !! Should be _PLENTY_ (See if this size can be calculated somehow)
	if(buffer == NULL) {
		logger(SYSLOG_FATAL,"SND","Unable to allocate buffer\n");
		exit(0);
	} else {
		//printf("Allocated buffer at %p\n",(void *)buffer);
	}

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
	
	logger(SYSLOG_INFO,"SND","Successfully opened DEFAULT_INPUT at %.0fHz samplingrate",inputStreamBasicDescription.mSampleRate);

	/* Start audio processing */
	error = AudioDeviceStart(inputDeviceID,audioIOProc);
	if(error) logger(SYSLOG_FATAL,"SND","Unable to start audio processing\n");

	return(0);
}

static void Darwin_Audio_Release(void) {
	/* Unregister the AudioDeviceIOProc */
	AudioDeviceRemoveIOProc(inputDeviceID,audioIOProc);
}

/*
 * PPM Decoder
 */
#define LEVEL_LOW  0
#define LEVEL_HIGH 1

static int32 prevState = 1,chIndex = 0;

static void PulseWidthDecoder(unsigned int width,unsigned char level) {
	static float length,pos;
	
	length = ((float)width / inputStreamBasicDescription.mSampleRate);

	// 0.695..1.09..1.5
	
	pos = length * 1000;
	
	pos = pos - 0.695;
	pos = pos / (1.5 - 0.695);
	pos = pos * 2.0;
	pos = pos - 1.0;
	
	if( length > (5.0 / 1000) ) { // The PPM sync-pulse would be the only signal we would find thats longer than 5ms
		chIndex = 0;
	} else {
		if(chIndex > 0 && chIndex < 17 && chIndex & 1) channel[chIndex>>1] = pos;

		chIndex++;
	}
	
}

void PPMDecode(uint16 *buffer,uint32 framesamples) {
	uint16 sample;
	uint32 i;

	for(i=0;i<framesamples;i++) {

		sample = buffer[i];

		if(        (prevState < 0) && sample == 1.0 ) { // Rising edge
			PulseWidthDecoder(-prevState,LEVEL_LOW);
			prevState = 1;
		} else if( (prevState > 0) && sample == 0.0 ) { // Falling edge
			PulseWidthDecoder(prevState,LEVEL_HIGH);
			prevState = -1;
		} else { // No edge
			if( sample == 0.0 ) prevState += -1;
			else                prevState +=  1; 
		}

	}
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

	PPMDecode(buffer,frames);

	return(noErr);
}

/*
 * PUBLIC functions
 */
int Darwin_Audio_GetNumInputs(void) {
	return(16);
}
double *Darwin_Audio_GetInputs(void) {
	return(channel);
}

void Architecture_Initialize(void) {
	inputdevice.name         = (char *)malloc(80);
	strcpy(inputdevice.name,"Audio / PPM");

	inputdevice.Initialize   = Darwin_Audio_Initialize;
	inputdevice.GetNumInputs = Darwin_Audio_GetNumInputs;
	inputdevice.GetInputs    = Darwin_Audio_GetInputs;
	inputdevice.Release      = Darwin_Audio_Release;

	Input_RegisterDevice(&inputdevice);
	
	Darwin_Audio_Initialize();
}

double Architecture_GetHighresTime(void) {
	struct timeval tp;
	double time;
	
	gettimeofday(&tp,NULL);
	time  = tp.tv_sec;
	time += (double)tp.tv_usec / 1000000.0;

	return(time);
}