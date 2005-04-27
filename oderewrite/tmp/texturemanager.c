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
#include <string.h>
#include <GLUT/glut.h>

#include "collective.h"
#include "logger.h"

#define TEXTURE_FORMAT_TGA 0
#define TEXTURE_FORMAT_BMP 1


#include "texturemanager.h"

/*
* Uniformed texture-loader
*
* + Currently understands TGA and BMP formats
* + Automatic MipMap generation
* + Rescaling of textures to fit GL_TEXTURE_SIZE
*/

#define MAX_TEXTURES 20

texture_t texture[MAX_TEXTURES];

static uint32 majors = 0;

/*
 * Rescales a texture to fit within GL_MAX_TEXTURE_SIZE
 */
static int Texture_Rescale(int texIndex) {
	GLint  max_size;
	double scaling[2],srcx,srcy;
	uint32 x,y;
	void  *newtexture;
	uint8  tmpbyte;
	
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_size);
	if( (texture[texIndex].width <= max_size) && (texture[texIndex].height <= max_size) ) return(texIndex);
	
	//logger(SYSLOG_INFO,"TEX","Rescaling texture %u from %ux%u to %ux%u",texIndex,texture[texIndex].width,texture[texIndex].height,max_size,max_size);
	
	/* Okay, we get here if the texture was bigger than recommended.. Lets cut it down to size */
	scaling[0] = (float)texture[texIndex].width  / (float)max_size;
	scaling[1] = (float)texture[texIndex].height / (float)max_size;
	
	newtexture = malloc(max_size * max_size * 3);
	if(newtexture == NULL) logger(SYSLOG_FATAL,"TEX","Unable to allocate for resized texture");
	
	for(y=0;y<max_size;y++) {
		for(x=0;x<max_size;x++) {
			srcx = x * scaling[0];
			srcy = y * scaling[1];
			
			tmpbyte = *(uint8*)((uint8*)texture[texIndex].rawtexture + ((int)srcy*texture[texIndex].width)*3 + ((int)srcx)*3 );
			*(uint8*)((uint8*)newtexture + (y*max_size+x)*3) = tmpbyte;
			tmpbyte = *(uint8*)((uint8*)texture[texIndex].rawtexture + ((int)srcy*texture[texIndex].width)*3 + ((int)srcx)*3 + 1);
			*(uint8*)((uint8*)newtexture + (y*max_size+x)*3 + 1) = tmpbyte;
			tmpbyte = *(uint8*)((uint8*)texture[texIndex].rawtexture + ((int)srcy*texture[texIndex].width)*3 + ((int)srcx)*3 + 2);
			*(uint8*)((uint8*)newtexture + (y*max_size+x)*3 + 2) = tmpbyte;
		}
	}
	
	free(texture[texIndex].rawtexture);
	
	texture[texIndex].rawtexture = newtexture;
	texture[texIndex].width  = max_size;
	texture[texIndex].height = max_size;
	
	
	return(texIndex);
}

int Texture_LoadFromMemory(void *texdata,uint8 format) {

	if(format == TEXTURE_FORMAT_TGA) {
		uint8  idlength,tmpcol,bpp;
		uint32 x,y,shift;

		texture[majors].swapper = 0;

		idlength = *(uint8 *)(texdata);

		bpp = *(uint8*)((uint8*)texdata + 16);

		texture[majors].origwidth  = *(uint16 *)((uint8*)texdata + 12);
		texture[majors].origheight = *(uint16 *)((uint8*)texdata + 14);

#if (defined BIG_ENDIAN && !defined LINUX)
		texture[majors].origheight = ((texture[majors].origheight & 0xFF) << 8) + ((texture[majors].origheight & 0xFF00) >> 8);
		texture[majors].origwidth  = ((texture[majors].origwidth  & 0xFF) << 8) + ((texture[majors].origwidth  & 0xFF00) >> 8);
#endif

		/* Rounds up the height/width to the closest n^2 */
		for(shift=1;shift<16;shift++) if(((float)texture[majors].origwidth  / (float)(1<<shift)) <= 1.0) break;
		texture[majors].width = 1 << shift;
		for(shift=1;shift<16;shift++) if(((float)texture[majors].origheight / (float)(1<<shift)) <= 1.0) break;
		texture[majors].height = 1 << shift;

		texture[majors].rawtexture = malloc(texture[majors].height * texture[majors].width * (bpp/8));

		//logger(SYSLOG_INFO,"TEX","Loading %ix%i %ibit TGA texture",texture[majors].origwidth,texture[majors].origheight,bpp);

		bpp = bpp / 8;

		for(y=0;y<texture[majors].origheight;y++) {
			memcpy( (void *)((uint8 *)texture[majors].rawtexture + texture[majors].width*y*bpp),
				(void *)((uint8 *)texdata + (texture[majors].origwidth*y*bpp) + 18 + idlength),
				texture[majors].origwidth * bpp);
			// BGR to RGB fixing
			for(x=0;x<texture[majors].origwidth;x++) {
				tmpcol = *(uint8 *)((uint8*)(texture[majors].rawtexture) + texture[majors].width*y*bpp + x*bpp); // tmp = B
				*(uint8 *)((uint8*)(texture[majors].rawtexture) + texture[majors].width*y*bpp + x*bpp) = *((uint8 *)(texture[majors].rawtexture) + texture[majors].width*y*bpp + x*bpp + 2); // B = R
				*(uint8 *)((uint8*)(texture[majors].rawtexture) + texture[majors].width*y*bpp + x*bpp + 2) = tmpcol; // R = tmp
				if(bpp == 4)
					*(uint8 *)((uint8*)(texture[majors].rawtexture) + texture[majors].width*y*bpp + x*bpp + 3) = 255 - *(uint8 *)((uint8*)(texture[majors].rawtexture) + texture[majors].width*y*bpp + x*bpp + 2);
			} 
		}

		texture[majors].scaling[0] = (float)texture[majors].origwidth  / (float)texture[majors].width;
		texture[majors].scaling[1] = (float)texture[majors].origheight / (float)texture[majors].height;

		//printf("Scaling: %f, %f\n",texture[majors].scaling[0],texture[majors].scaling[1]);

		Texture_Rescale(majors);

		// GL Stuff
		glGenTextures(1,&texture[majors].gltexture);
		glBindTexture(GL_TEXTURE_2D,texture[majors].gltexture);
		if(bpp == 3)
			glTexImage2D(GL_TEXTURE_2D,0,bpp,texture[majors].width,texture[majors].height,0,GL_RGB,GL_UNSIGNED_BYTE,texture[majors].rawtexture);
		if(bpp == 4)
			glTexImage2D(GL_TEXTURE_2D,0,bpp,texture[majors].width,texture[majors].height,0,GL_RGBA,GL_UNSIGNED_BYTE,texture[majors].rawtexture);
		if( glGetError() != GL_NO_ERROR ) {
			logger(SYSLOG_FATAL,"TEX","TexImageError..  Texture might be too big");
		}
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	} else if( format == TEXTURE_FORMAT_BMP ) {
		uint32 dataoffset,mempointer;
		uint32 x,y,shift;
		uint8  tmp;

		texture[majors].swapper = 1;
		
		if( (*(uint8*)((uint8*)texdata) != 'B') || (*(uint8*)((uint8*)texdata+1) != 'M') ) logger(SYSLOG_FATAL,"TEX","Told to load something as a BMP, which isnt a BMP..");

#ifndef SIXTYFOURBIT
		dataoffset = *(uint32 *)((uint8*)texdata + 10);
		texture[majors].origwidth  = *(uint32 *)((uint8*)texdata + 18);
		texture[majors].origheight = *(uint32 *)((uint8*)texdata + 22);
#else
		{
			uint64 tmp,tmp2;
			
			tmp = *(uint64 *)(texdata + 8);
			tmp = tmp >> 16;
			dataoffset = tmp;
			printf("Do=%u\n",tmp);

			tmp = *(uint64 *)(texdata + 16);
			tmp = tmp >> 16;
			texture[majors].origwidth = tmp;

			tmp  = *(uint64 *)(texdata + 16);
			tmp  = (tmp & 0xFF);
			tmp2 = *(uint64 *)(texdata + 24);
			tmp2 = tmp2 >> 40;
			texture[majors].origheight = (tmp << 16) + tmp2;
		}
#endif

#if (defined BIG_ENDIAN && !defined LINUX)
		dataoffset = dataoffset >> 16;
		dataoffset = ((dataoffset & 0xFF00) >> 8) + ((dataoffset & 0xFF) << 8);
		texture[majors].origwidth = texture[majors].origwidth >> 16;
		texture[majors].origwidth = ((texture[majors].origwidth & 0xFF00) >> 8) + ((texture[majors].origwidth & 0xFF) << 8);
		texture[majors].origheight = texture[majors].origheight >> 16;
		texture[majors].origheight = ((texture[majors].origheight & 0xFF00) >> 8) + ((texture[majors].origheight & 0xFF) << 8);
#endif

		/* Rounds up the height/width to the closest n^2 */
		for(shift=1;shift<16;shift++) if(((float)texture[majors].origwidth  / (float)(1<<shift)) <= 1.0) break;
		texture[majors].width = 1 << shift;
		for(shift=1;shift<16;shift++) if(((float)texture[majors].origheight / (float)(1<<shift)) <= 1.0) break;
		texture[majors].height = 1 << shift;

		texture[majors].rawtexture = malloc(texture[majors].height * texture[majors].width * 3);

		//logger(SYSLOG_INFO,"TEX","Loading %ix%i BMP texture",texture[majors].origwidth,texture[majors].origheight);

		mempointer = dataoffset + 13;
		for(y=0;y<texture[majors].origheight;y++) {
			memcpy( (void *)(((uint8 *)texture[majors].rawtexture) + texture[majors].width*y*3),
					(void *)(((uint8 *)(texdata) + dataoffset)),
					texture[majors].origwidth * 3 );

			for(x=0;x<texture[majors].width;x++) {
				tmp = *(uint8*)(((uint8*)(texture[majors].rawtexture) + texture[majors].width*y*3 + x*3 + 0)); // tmp = B 
				*(uint8*)(((uint8*)(texture[majors].rawtexture) + texture[majors].width*y*3 + x*3 + 0)) = *(uint8*)(((uint8*)(texture[majors].rawtexture) + texture[majors].width*y*3 + x*3 + 2)); // B = R
				*(uint8*)(((uint8*)(texture[majors].rawtexture) + texture[majors].width*y*3 + x*3 + 2)) = tmp;
			} 

			dataoffset += ((texture[majors].origwidth*3) & 0xFFFFFFFC) + 4;
		}

		texture[majors].scaling[0] = (float)texture[majors].origwidth  / (float)texture[majors].width;
		texture[majors].scaling[1] = (float)texture[majors].origheight / (float)texture[majors].height;

		Texture_Rescale(majors);

		// GL Stuff
		glGenTextures(1,&texture[majors].gltexture);
		glBindTexture(GL_TEXTURE_2D,texture[majors].gltexture);
		glTexImage2D(GL_TEXTURE_2D,0,3,texture[majors].width,texture[majors].height,0,GL_RGB,GL_UNSIGNED_BYTE,texture[majors].rawtexture);
		if( glGetError() != GL_NO_ERROR ) {
			logger(SYSLOG_FATAL,"TEX","TexImageError..  Texture might be too big");
		}
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}

	majors++;
	return(majors-1);
}

int Texture_LoadFromFile(char *filename) {
	FILE   *in;
	void   *ptr;
	uint32  fsize;
	int     ret;

	logger(SYSLOG_INFO,"TEX","Loading TGA texture : %s",filename);

	in = fopen(filename,"rb");
	if( in == NULL ) logger(SYSLOG_FATAL,"TEX","Unable to open file %s",filename);

	// Check filesize and allocate enough room for it
	fseek(in,0,SEEK_END);
	fsize = ftell(in);
	ptr = malloc(fsize);
	if(ptr == NULL) logger(SYSLOG_FATAL,"TEX","Unable to allocate enough RAM\n");
	fseek(in,0,SEEK_SET);

	fread(ptr,1,fsize,in); /* !!! Slightly inefficent */

	fclose(in);

	if(        strncmp("tga",(char *)(filename + strlen(filename) - 3),3) == 0 ) {
		ret = Texture_LoadFromMemory(ptr,TEXTURE_FORMAT_TGA);
	} else if( strncmp("bmp",(char *)(filename + strlen(filename) - 3),3) == 0 ) {
		ret = Texture_LoadFromMemory(ptr,TEXTURE_FORMAT_BMP);
	} else {
		logger(SYSLOG_FATAL,"TEX","Asked to load a fileformat we dont understand %s",filename);
		ret = 0;
	}
	
	free(ptr);

	return(ret);
}

static int texIndex = -1;

void Texture_BindTexture(uint32 i) {
	glBindTexture(GL_TEXTURE_2D,texture[i].gltexture);
	texIndex = i;
}

void Texture_MapUV(float u,float v) {
	float nu,nv;
	
	if(texIndex >= 0) {
		if(texture[texIndex].swapper) {

			nu = ( u / (float)texture[texIndex].origwidth ) * texture[texIndex].scaling[0];
			nv = (((float)texture[texIndex].origheight - v) / (float)texture[texIndex].origheight) * texture[texIndex].scaling[1];

			//printf("UVs: %f - %f\n",nu,nv);

			glTexCoord2f(nu,nv );
		} else
			glTexCoord2f( u * texture[texIndex].scaling[0], v * texture[texIndex].scaling[1] );
	}
}
