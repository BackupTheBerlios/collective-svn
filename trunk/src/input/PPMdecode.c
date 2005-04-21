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
 *
 */
 
 #include <stdio.h>

#include "collective.h"
#include "logger.h"
#include "config.h"

/*
 * Reads sampled input-data in PPM format, and translates into
 * channel-values
 *
 * !!! Hardcoded to use the audio-interface
 */

/*
 * Architecture
 *
 * The audio callback calls Input_Audio_Decode
 * Input_Audio_Decode keeps the decoded channels in an array
 * Input_GetChannels reads the array and sends it to the calle
 *
 * Why so complicated?  Basically to be compatible with both
 * callback-based and polled audio systems.
 */

#define LEVEL_LOW  0
#define LEVEL_HIGH 1

#define MAX_CHANNELS 16

static float samplingrate;
static int32 prevState,chIndex = 0;
static float channel[MAX_CHANNELS];
static float calibration[MAX_CHANNELS][2];

int Input_Initialize(void) {

	if( audiodev.Initialize == NULL ) {
		logger(SYSLOG_FATAL,"INPUT","Audiodevice is not initialized..");
		return(1);
	}
	
	samplingrate = audiodev.GetSamplingRate();
	
	prevState = 0;
	chIndex = 0;

	return(0);
}

int Input_GetNumChannels(void) {
	return(MAX_CHANNELS);
}

float *Input_GetChannels(void) {
	return(channel);
}

void PulseWidthDecoder(unsigned int width,unsigned char level) {
	static float length,pos;
	
	length = ((float)width / samplingrate);
	
	// 0.695..1.09..1.5
	
	pos = length * 1000;
	
	pos = pos - 0.695;
	pos = pos / (1.5 - 0.695);
	pos = pos * 2.0;
	pos = pos - 1.0;
	
	
//	if(chIndex > MAX_CHANNELS) { printf("Couldnt find a 5ms sync-pulse in a while..\n"); exit(0); }
	if( length > (5.0 / 1000) ) { // The PPM sync-pulse would be the only signal we would find thats longer than 5ms
		chIndex = 0;
	} else {
		if(chIndex > 0 && chIndex < 17 && chIndex & 1) channel[chIndex>>1] = pos;

		chIndex++;
	}
	
}

void Input_AudioDecode(uint16 *buffer,uint32 framesamples) {
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


void Input_AudioDecodeX(uint16 *buffer,uint32 framesamples) {
	uint32 i;
	float  startSamples;
	
	printf(".");
	
	//framesamples = (int)((float)samplingrate * ((float)22.5f / (float)1000.0f));
	startSamples = samplingrate * (10.0f / 1000.0f);
	
	//buffer = audiodev.GetBuffer(framesamples);
	for(i=1;i<framesamples;i++) {
		// Edge-detection
		if( (prevState >= 0) && (buffer[i] == 0) ) {  // From high to low
			printf("!");
			if( prevState <= -(int)startSamples ) {
				// It was a start-pulse, start channel-count from 0
				printf("Sync @ %u\n",chIndex);
				chIndex = 0;
			} else {
				channel[chIndex] = ((float)(-prevState) - calibration[chIndex][0]) / calibration[chIndex][1] - 1.0f;
				chIndex++;
			}
			prevState = 1;
		} else if( (prevState >= 0) && (buffer[i] == 0) ) { // High to low
			prevState = -1;
		}
		else if( buffer[i] >= 1 ) prevState++;
		else if( buffer[i] == 0 ) prevState--;
	}
}

