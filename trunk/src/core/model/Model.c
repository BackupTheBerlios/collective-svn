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
 
 /*
 * More or less concept code..
 */
 
#include "collective.h" 
#include "model.h"
#include "logger.h"
#include "config.h"
#include "texturemanager.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * On windows you need to define _USE_MATH_DEFINES before you can get access to M_PI
 */
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

/*
 * GL_BGR is not available on windows, and is implemented as part of an extension as GL_BGR_EXT
 */
#ifdef WIN32
#define GL_BGR GL_BGR_EXT
#endif

#define CRASH(x) { printf("%s",x); exit(-1); }

GLuint *callLists;

GLuint *Model_GetCallListPointer(void) {
	return(callLists);
}

#if 0
void *TGATextureLoad(char *texture) {
	FILE *in;
	unsigned char  idLen;
	unsigned short width,height;
	unsigned char  *data;
	unsigned int    ret;
	
	in = fopen(texture,"rb");
	if(in == NULL) {
		fprintf(stderr,"Unable to open file %s\n",texture);
		exit(-1);
	}
	if( fread(&idLen,1,1,in)  != 1) CRASH("TGA Couldnt load idLen"); 
	if( fseek(in,12,SEEK_SET) != 0) CRASH("TGA Couldnt skip first part of header");
	if( fread(&width ,2,1,in) != 1) CRASH("TGA Couldnt load header.width");
	if( fread(&height,2,1,in) != 1) CRASH("TGA Couldnt load header.height");

#if (defined BIG_ENDIAN && !defined LINUX)
	width  = (width  << 8) + (width  >> 8);
	height = (height << 8) + (height >> 8);
#endif
	logger(SYSLOG_INFO,"MODEL","TGA Resolution: %u x %u",width,height);

	if( fseek(in,18+idLen,SEEK_SET) != 0 ) CRASH("TGA Couldnt skip to bitmap data");

	logger(SYSLOG_INFO,"MODEL","TGA Allocating %u bytes",width*height*3);
	data = (unsigned char *)malloc(width * height * 3);
	if(data == NULL) {
		logger(SYSLOG_FATAL,"MODEL","TGA Unable to allocate memory for %s",texture);
		exit(-1);
	}
	logger(SYSLOG_INFO,"MODEL","TGA Allocation went well");
	
	ret = fread(data,1,width*height*3,in);
	if(ret != (width*height*3)) {
		logger(SYSLOG_FATAL,"MODEL","TGA couldnt load enough data from texture-file");
		exit(-1);		
	}
	
	fclose(in);

	return(data);
}
#endif

void CreateSphere(float cx,float cy,float cz,double r,int n) {
   int i,j;
   double theta1,theta2,theta3;
   float ex,ey,ez,px,py,pz;
   float vscale;
   
   vscale = (768.0/1024.0);

   if (r < 0)
      r = -r;
   if (n < 0)
      n = -n;
   if (n < 4 || r <= 0) {
      glBegin(GL_POINTS);
      glVertex3f(cx,cy,cz);
      glEnd();
      return;
   }

   for (j=0;j<n/2;j++) {
      theta1 = j * (M_PI*2) / n - (M_PI/2);
      theta2 = (j + 1) * (M_PI*2) / n - (M_PI/2);

      glBegin(GL_QUAD_STRIP);
      for (i=0;i<=n;i++) {
         theta3 = i * (M_PI*2) / n;

         ex = cos(theta2) * cos(theta3);
         ey = sin(theta2);
         ez = cos(theta2) * sin(theta3);
         px = cx + r * ex;
         py = cy + r * ey;
         pz = cz + r * ez;

         glNormal3f(ex,ey,ez);
         //glTexCoord2f(i/(double)n,(((2*(j+1)/(double)n)-0.5)*vscale)+0.5);
		 Texture_MapUV(i/(double)n,(((2*(j+1)/(double)n)-0.5)*vscale)+0.5);
         glVertex3f(px,py,pz);

         ex = cos(theta1) * cos(theta3);
         ey = sin(theta1);
         ez = cos(theta1) * sin(theta3);
         px = cx + r * ex;
         py = cy + r * ey;
         pz = cz + r * ez;

         glNormal3f(ex,ey,ez);
         //glTexCoord2f(i/(double)n,(((2*j/(double)n)-0.5)*vscale)+0.5);
		 Texture_MapUV(i/(double)n,(((2*j/(double)n)-0.5)*vscale)+0.5);
         glVertex3f(px,py,pz);
      }
      glEnd();
   }
}

static GLuint Model_GenerateScenery(char *texturefile) {
	GLuint scenerylist;
	int i;
	
	i = Texture_LoadFromFile(texturefile);

	scenerylist = glGenLists(1);
	glNewList(scenerylist,GL_COMPILE);
	
	glColor4f(1.0,1.0,1.0,1.0);
	Texture_BindTexture(i);
	CreateSphere(0,40,0,80,16);

	glEndList();

	return(scenerylist);
}

void Model_Initialize(void) {
	char str[100]; // !!! <- Buffer overflow waiting to happend
	
	Config_getstring("model.appearance",str);
	callLists = Model_LoadMOD(str);

	Config_getstring("scenery.texture",str);
	callLists[2] = Model_GenerateScenery(str);
}
