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
* $RCFS: Input/darwinjoystick.c,v 1.1.1.1 2005/03/17 17:41:54 jaja Exp $
*
*/

/*
 * Placeholder for joystick functions for Darwin, so we avoid compilation errors
 */
#include <stdio.h>

#include "joystick.h"
#include "collective.h"

#define MAX_CHANNELS 16


static float channel[MAX_CHANNELS];

int Joystick_Audio_Initialize(int mode) {
	return(0);
}

int Joystick_Initialize(void) {
	return(0);
}

float *Joystick_Poll(void) {
	return(channel);
}

void Joystick_Release(void) {
}

float *Joystick_GetChannels(void) {
	return(Joystick_Poll());
}

int Joystick_GetNumChannels(void) {
	return(MAX_CHANNELS);
}

float Joystick_GetSamplingRate(void) {
	return(0);
}
