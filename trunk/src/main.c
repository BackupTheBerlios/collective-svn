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
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h> 
#endif

#ifdef WIN32
#include <direct.h>
#endif

#include "collective.h"
#include "dynload.h"
#include "config.h"
#include "logger.h"
#include "physics.h"
#include "graphics.h"
#include "model.h"
#include "string.h"
#include "joystick.h"

audio_handler_t audiodev;
input_handler_t inputdev;
int inputType = INPUT_AUDIO;

void Main_Idleloop(void) {
	Physics_Update();
	Graphics_Redraw();
}

int main(int argc,char **argv) {
	char* dir = NULL;
	char  tmpstr[255],cwdcfg[255];
	int iLoop = 0;
	char str[100]; // !!! <- Buffer overflow waiting to happend


	for (iLoop = 1; iLoop < argc; iLoop++)
	{
		if(strncmp(argv[iLoop], "-dir", 4) == 0)
		{
			if(++iLoop >= argc)
				printf("-dir parameter must be followed by directory name");

			dir = argv[iLoop];
		}
	}

	if (dir == NULL)
	{
		sprintf(cwdcfg,"%s/collective.cfg",getcwd(tmpstr,100));
	}
	else
	{
		sprintf(cwdcfg,"%s/collective.cfg",dir);
	}

	Config_read(cwdcfg);
	
	audiodev.Initialize = NULL;
	inputdev.Initialize = NULL;
	
	//This should probably be moved somewhere else
	//Figure out the control interface
	Config_getstring("input.input",str);

	if (strncmp(str, "audio", 5) == 0)
		inputType = INPUT_AUDIO;
	else if (strncmp(str, "joystick", 8) == 0)
		inputType = INPUT_JOYSTICK;
	else
	{
		printf("Unable to determine input type");
		exit(0);
	}

	Dynload_Initialize();
	Dynload_LoadModules(inputType);

	Physics_Initialize();

	Graphics_Initialize(&argc,argv);
	
	Joystick_Initialize();

	Model_Initialize();
	
	audiodev.Initialize(AUDIO_INPUT);
	inputdev.Initialize();
	logger(SYSLOG_INFO,"MAIN","Channels from inputdev: %i",inputdev.GetNumChannels());
	
	Graphics_Mainloop();

	Joystick_Release();
}
