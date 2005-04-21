#ifndef _Input_H_
#define _Input_H_

int Input_Initialize(void);
int Input_GetNumChannels(void);
float *Input_GetChannels(void);
void Input_AudioDecode(uint16 *buffer,uint32 framesamples);

#endif

