#ifndef _AUDIO_H_
#define _AUDIO_H_

int Audio_Initialize(int mod);
float Audio_GetSamplingRate(void);
void Audio_SendBuffer(void);
void Audio_Release(void);

/************************************************************************/
/* Used for polling audio data                                          */
/************************************************************************/
void Audio_Poll(void);


#endif

