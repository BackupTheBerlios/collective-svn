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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "collective.h"

#define CONFIG_RADIO_JR     0x00
#define CONFIG_RADIO_FUTABA 0x01
#define CONFIG_RADIO_HITEC  0x02
#define CONFIG_RADIO_END    0x03

typedef struct {
	struct {
		uint8 passthrough;       // Bool, decides if the software mixing/functions should be disabled
		uint8 chassignment[MAX_CHANNELS];  // Which "function" (pitch,coll,thr) each channel belongs to
		uint8 type;
		
		// Whats below is only used by the software radio
		uint8 chreverse[MAX_CHANNELS];
		float trim[MAX_CHANNELS];
		float epa[MAX_CHANNELS];
	} radio;
} config_t;

extern config_t config;

void Config_read(char *str);
int  Config_getint(char *key);
int  Config_getbool(char *key);
int  Config_getstring(char *key,char *dest);

#endif
