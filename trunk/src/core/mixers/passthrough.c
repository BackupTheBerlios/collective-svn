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
 
#include "collective.h"
#include "config.h"

 /*
  * Simple passthrough mixer. Far from what it is supposed to be..
  */

static float nch[MAX_CHANNELS];

float *Mixer_GetControls(void) {
	static float *ch;
	
	ch = inputdev.GetChannels();

	switch( config.radio.type ) {
		case CONFIG_RADIO_JR:
			nch[MIXER_THROTTLE]   = ch[0];
			nch[MIXER_COLLECTIVE] = ch[5];
			nch[MIXER_AILERON]    = ch[1];
			nch[MIXER_ELEVATOR]   = ch[2];
			nch[MIXER_RUDDER]     = ch[3];
			break;
		case CONFIG_RADIO_FUTABA:
		case CONFIG_RADIO_HITEC:
			nch[MIXER_THROTTLE]   = ch[2];
			nch[MIXER_COLLECTIVE] = ch[5];
			nch[MIXER_AILERON]    = ch[0];
			nch[MIXER_ELEVATOR]   = ch[1];
			nch[MIXER_RUDDER]     = ch[3];
			break;
	}
	
/*
#if 0
#ifdef WIN32
	ch[MIXER_THROTTLE] = -ch[MIXER_THROTTLE];
	if (ch[MIXER_THROTTLE] < 0)
		ch[MIXER_THROTTLE] = -ch[MIXER_THROTTLE];
	ch[MIXER_COLLECTIVE] = -ch[MIXER_COLLECTIVE];
#endif
#endif
	*/
//	if(ch[MIXER_THROTTLE]<-1.0) ch[MIXER_THROTTLE] = -1.0;
/*	ch[MIXER_AILERON ] = 0.0001 * 4.0;
	ch[MIXER_ELEVATOR] = 0.0002 * 4.0;
	ch[MIXER_RUDDER  ] = 0.00015 * 4.0;*/
	
	return(nch);
}
