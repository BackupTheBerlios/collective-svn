/*
 *  input.h
 *  Collective3
 *
 *  Created by James Jacobsson on 4/25/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _INPUT_H_
#define _INPUT_H_

struct input_device;
typedef struct input_device {
        char   *name;
        int     (*Initialize)(void);
        int     (*GetNumInputs)(void);
        double *(*GetInputs)(void);
        void    (*Release)(void);
} input_device_t;

void            Input_Initialize(void);
uint8           Input_GetNumDevices(void);
input_device_t *Input_GetDevice(int i);
void            Input_RegisterDevice(input_device_t *dev);
double         *Input_GetInput(input_device_t *dev);
void            Input_Release(void);


#endif
