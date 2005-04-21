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
 
#ifndef _COLLECTIVE_H_
#define _COLLECTIVE_H_

#if (defined WIN32 || defined DARWIN )
#include <GLUT/glut.h>
#else
#if (defined IRIX || defined LINUX)
#include <GL/glut.h>
#endif
#endif

#define COLLECTIVE_NAME "Collective"
#define COLLECTIVE_VERSION "0.0"

typedef unsigned char      uint8;
typedef char                int8;
typedef unsigned short     uint16;
typedef short               int16;
typedef unsigned int       uint32;
typedef int                 int32;
typedef unsigned long long uint64;
typedef long long           int64;

#define AUDIO_INPUT  1
#define AUDIO_OUTPUT 2

#define MAX_CHANNELS 16

#if 0
// Hitec/Futaba channels
#define MIXER_AILERON    0
#define MIXER_ELEVATOR   1
#define MIXER_THROTTLE   2
#define MIXER_RUDDER     3
#define MIXER_COLLECTIVE 5
#endif

// JR channels
#define MIXER_AILERON    1
#define MIXER_ELEVATOR   2
#define MIXER_THROTTLE   0
#define MIXER_RUDDER     3
#define MIXER_COLLECTIVE 5

//Input Type
#define INPUT_AUDIO	    0
#define INPUT_JOYSTICK	1


void Main_Idleloop(void);

struct audio_handler;
typedef struct audio_handler {
  int    (*Initialize)(int mode);
  float  (*GetSamplingRate)(void);
  int16 *(*GetBuffer)(uint32 samples);
  void   (*SendBuffer)(short *);
} audio_handler_t;

struct input_handler;
typedef struct input_handler {
  int    (*Initialize)(void);
  int    (*GetNumChannels)(void);
  float *(*GetChannels)(void);
  void   (*AudioDecode)(uint16 *buff,uint32 samples);
  void   (*Release) (void);
} input_handler_t;

extern audio_handler_t audiodev;
extern input_handler_t inputdev;
extern uint32          currentTime;
struct {
  struct {
    float matrix[16],finalMatrix[16],rotationMatrix[16];
    float pos[4],speed[4];
    float weight;
    struct {
      float pitch;
      float rpm;
    } rotor;
    struct {
      float smoke[3];
    } engine;
  } heli;
} physics;

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif


#endif
