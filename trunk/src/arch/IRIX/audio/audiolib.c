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
 * $RCFS: main.c,v 1.1.1.1 2005/03/17 17:41:54 jaja Exp $
 *
 */
 
#include <stdio.h>
#include <signal.h>
#include <dmedia/audio.h>
#include <dmedia/audiofile.h>

#include "collective.h"
#include "logger.h"

static double        samprate;
static ALport        port;
static int16        *buf;

int Audio_Initialize(int mode) {
  int  device;
  ALpv pv;
  ALconfig      portconfig;

#warning THIS FILE NEEDS TO BE CHANGED TO COMPENSATE FOR THE AUDIO MODULE CLAMPING THE VALUES TO [0,1]

  device = AL_DEFAULT_INPUT; // Use default input
  
  // Check the sampling-rate
  pv.param = AL_RATE;
  if (alGetParams(AL_DEFAULT_INPUT, &pv, 1) < 0)  {
    syslog(SYSLOG_FATAL,"SND","alGetParams failed: %s",alGetErrorString(oserror()));
    return(1);
  }
  samprate = alFixedToDouble(pv.value.ll);
  if(samprate < 0) {
    syslog(SYSLOG_FATAL,"SND","Unable to get sampling-rate.");
  } else {
    syslog(SYSLOG_INFO,"SND","Sampling-rate: %.0fHz",samprate);
  }

  buf = (short *)malloc((int)(samprate / 2) * 2);
  if(buf == NULL) {
    syslog(SYSLOG_FATAL,"SND","Unable to allocate input-buffer");
    return(1); 
  }

  // Open the input-port
  portconfig = alNewConfig();
  alSetChannels(portconfig, 1);         // Mono
  alSetWidth(portconfig, AL_SAMPLE_16); // 16 bits
  alSetQueueSize(portconfig, (int)samprate ); // 1sec buffer
  alSetDevice(portconfig, device);
  port = alOpenPort("PPM input", "r", portconfig);
  if(!port) {
    syslog(SYSLOG_FATAL,"SND","alOpenPort failed: %s",alGetErrorString(oserror()));
    return(1);
  }

  return(0);
}

float Audio_GetSamplingRate(void) {
  return(samprate);
}

int16 *Audio_GetBuffer(uint32 samples) {
  alReadFrames(port,buf,samples);
  return(buf);
}

void Audio_SendBuffer(void) {
    syslog(SYSLOG_FATAL,"SND","AudioSendBuffer isnt implemented yet");
}
