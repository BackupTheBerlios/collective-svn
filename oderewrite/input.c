/*
 *  input.c
 *  Collective3
 *
 *  Created by James Jacobsson on 4/25/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "collective.h"
#include "input.h"

#define MAX_INPUT_DEVS 10

input_device_t *inputdevice[MAX_INPUT_DEVS];
uint8 inputdevices;

void Input_Initialize(void) {
	int i;

	for(i=0;i<MAX_INPUT_DEVS;i++) {
		inputdevice[i] = NULL;
	}
	
	inputdevices = 0;
}

uint8 Input_GetNumDevices(void) {
	return(inputdevices);
}

input_device_t *Input_GetDevice(int i) {
	return( inputdevice[i] );
}

void Input_RegisterDevice(input_device_t *dev) {
	inputdevice[inputdevices++] = dev;
}

double *Input_GetInput(input_device_t *dev) {
	return( dev->GetInputs() );
}

void      Input_Release(void) {
	int i;
	
	for(i=0;i<inputdevices;i++)
		inputdevice[i]->Release();
}
