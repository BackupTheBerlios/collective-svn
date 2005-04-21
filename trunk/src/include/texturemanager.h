#ifndef _TEXTUREMANAGER_H_
#define _TEXTUREMANAGER_H_

#include "collective.h"

#define TEXTURE_FORMAT_TGA 0
#define TEXTURE_FORMAT_BMP 1

int  Texture_LoadFromMemory(void *texdata,uint8 format);
int  Texture_LoadFromFile(char *filename);
void Texture_BindTexture(uint32 i);
void Texture_MapUV(float u,float v);

#endif
