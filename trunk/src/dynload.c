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
 */
 
#include "collective.h"
#include "logger.h"
#include "audio.h"
#include "input.h"
#include "joystick.h"

#include <stdio.h>

/*
 * Hack until I can figure out if this affects the windows build
 */
#ifndef WIN32
#include <dlfcn.h>
#endif

/*
 * This was originally a loader for dynamically loaded modules, but due to cross
 * platform issues, it's no longer that. The abstration is still here though, 
 * if we decide to do it sometime in the future
 */

int Dynload_Initialize(void) {
 return(0);
}

int Dynload_LoadModules(int inputType) {

	switch (inputType)
	{
	case INPUT_AUDIO:
		audiodev.Initialize = (int (*)(int))Audio_Initialize;
		audiodev.GetSamplingRate = (float (*)(void))Audio_GetSamplingRate;

		inputdev.Initialize = (int (*)(void))Input_Initialize;
		inputdev.GetNumChannels = (int (*)(void))Input_GetNumChannels;
		inputdev.GetChannels = (float *(*)(void))Input_GetChannels;
		inputdev.AudioDecode = (void (*)(uint16 *,uint32))Input_AudioDecode;
		inputdev.Release = (void (*) (void)) Audio_Release;
		break;
	case INPUT_JOYSTICK:
		audiodev.Initialize = (int (*)(int))Joystick_Audio_Initialize;
		audiodev.GetSamplingRate = (float (*)(void))Joystick_GetSamplingRate;

		inputdev.Initialize = (int (*)(void))Joystick_Initialize;
		inputdev.GetNumChannels = (int (*)(void))Joystick_GetNumChannels;
		inputdev.GetChannels = (float *(*)(void))Joystick_GetChannels;
		inputdev.AudioDecode = NULL;
		inputdev.Release = (void (*) (void)) Joystick_Release;
		break;
	default:
		return(1);
		break;
	}
	
#if 0
	void *handle[2];
	const char *error;
	
	handle[0] = dlopen("audio.so",RTLD_NOW | RTLD_LOCAL);
	if( (error = dlerror()) != NULL) {
		logger(SYSLOG_FATAL,"DYNL","dlopen() failed because: %s",error);
		return(1);
	}
	audiodev.Initialize = (int (*)(int))dlsym(handle[0],"Audio_Initialize");
	if( (error = dlerror()) != NULL) {
		logger(SYSLOG_FATAL,"DYNL","Couldn't find symbol \"%s\"",error);
		return(1);
	}
	/* audiodev.GetBuffer = (int16 *(*)(uint32))dlsym(handle[0],"Audio_GetBuffer");
	if( (error = dlerror()) != NULL) {
		logger(SYSLOG_FATAL,"DYNL","Couldn't find symbol \"%s\"",error);
		return(1);
	}*/
	audiodev.GetSamplingRate = (float (*)(void))dlsym(handle[0],"Audio_GetSamplingRate");
	if( (error = dlerror()) != NULL) {
		logger(SYSLOG_FATAL,"DYNL","Couldn't find symbol \"%s\"",error);
		return(1);
	}
	
	handle[1] = dlopen("input.so",RTLD_LAZY | RTLD_LOCAL);
	if( (error = dlerror()) != NULL) {
		logger(SYSLOG_FATAL,"DYNL","Couldn't find symbol \"%s\"",error);
		return(1);
	}
	inputdev.Initialize = (int (*)(void))dlsym(handle[1],"Input_Initialize");
	if( (error = dlerror()) != NULL) {
		logger(SYSLOG_FATAL,"DYNL","Couldn't find symbol \"%s\"",error);
		return(1);
	}
	inputdev.GetNumChannels = (int (*)(void))dlsym(handle[1],"Input_GetNumChannels");
	if( (error = dlerror()) != NULL) {
		logger(SYSLOG_FATAL,"DYNL","Couldn't find symbol \"%s\"",error);
		return(1);
	}
	inputdev.GetChannels = (float *(*)(void))dlsym(handle[1],"Input_GetChannels");
	if( (error = dlerror()) != NULL) {
		logger(SYSLOG_FATAL,"DYNL","Couldn't find symbol \"%s\"",error);
		return(1);
	}
	inputdev.AudioDecode = (void (*)(uint16 *,uint32))dlsym(handle[1],"Input_AudioDecode");
	if( (error = dlerror()) != NULL) {
		logger(SYSLOG_FATAL,"DYNL","Couldn't find symbol \"%s\"",error);
		return(1);
	}
#endif	
	return(0);
}
