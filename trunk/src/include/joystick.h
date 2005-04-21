#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

int Joystick_Initialize(void);
int Joystick_GetNumChannels(void);
float *Joystick_GetChannels(void);
float *Joystick_Poll(void);
void Joystick_Release(void);

//Need to fix this requirement
int Joystick_Audio_Initialize(int mod);
//Only here untill I can figure out what it is used for
float Joystick_GetSamplingRate(void);

#endif

