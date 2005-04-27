/*
 *  texturemanager.h
 *  Collective3
 *
 *  Created by James Jacobsson on 4/25/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _TEXTUREMANAGER_H_
#define _TEXTUREMANAGER_H_

#include <GLUT/glut.h>

typedef struct {
	uint32 width,height,origheight,origwidth;
	double scaling[2],offset[2];
	GLuint gltexture;
	void  *rawtexture;
	uint8  swapper;
} texture_t;

int Texture_LoadFromFile(char *filename);
void Texture_BindTexture(uint32 i);
void Texture_MapUV(float u,float v);

#endif