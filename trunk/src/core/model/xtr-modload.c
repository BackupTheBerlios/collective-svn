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
 * $RCFS: Core/Model/xtr-modload.c,v 1.1.1.1 2005/04/02 17:41:54 jaja Exp $
 *
 */

/*
 * Loads a .mod file from Reflex XTR(r) and creates the OpenGL displaylists
 * to display it
 *
 * Status: Still in development, but very functional
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcfs.h"
#include "logger.h"
#include "texturemanager.h"

typedef struct {
	unsigned int   fsize;
	unsigned int   width,height,origwidth,origheight;
	float          xscale,yscale;
	unsigned char *data;
	GLuint         texture;
} texture_t;

typedef struct {
	float r,g,b,a;
} material_t;

typedef struct {
	unsigned int *point;
	float        *uv;
	unsigned int  points;
	unsigned int  material;
	unsigned int  texture;
	//unsigned char isF;
} face_t;

typedef struct {
	unsigned int p1,p2;
	float        width;
	unsigned int material;
} line_t;

struct {
	unsigned int  vertices;
	float        *vertex;
	float        *normal;
	unsigned int  faces;
	face_t       *face;
	unsigned int  lines;
	line_t       *line;
	unsigned int  materials;
	material_t   *material;
	unsigned int  textures;
	texture_t    *texture;

	unsigned int mainrotor[2];
	unsigned int tailrotor[2];
	
	float        smoke[3];

} modeldata;

#define CRASH(x) { printf("%s",x); exit(-1); }

static void LoadMOD(char *fname) {
	FILE *in;
	char *buffer;
	unsigned int vertexIndex,faceIndex,lineIndex,materialIndex,textureIndex,utmp[10],uvIndex;
	float tmp[10];
	int tmpCount,points;
	
	in = fopen(fname,"rb");
	if(in == NULL) CRASH("Unable to open file");
	
	buffer = (char *)malloc(1024);
	if(buffer == NULL) CRASH("Unable to allocate RAM for temp buffer");
	
	// Count the number of vertices and faces in the file
	modeldata.vertices = 0;
	modeldata.faces = 0;
	
	while(!feof(in)) {
		fgets(buffer,1022,in);
		
		if(		  strncmp(buffer,"P ",2) == 0) { // Vertex definition
			modeldata.vertices++;
		} else if(strncmp(buffer,"F ",2) == 0) { // Face definition
			sscanf(buffer,"F %u",&utmp[0]);
			modeldata.faces += utmp[0];
		} else if(strncmp(buffer,"B ",2) == 0) { // Doublesided face definition
			sscanf(buffer,"B %u",&utmp[0]);
			modeldata.faces += utmp[0];
		} else if(strncmp(buffer,"L ",2) == 0) { // Line definition
			modeldata.lines++;
		} else if(strncmp(buffer,"Y ",2) == 0) { // Line-strip definition
			sscanf(buffer,"Y %u",&utmp[0]);
			modeldata.lines += utmp[0] - 1;
		} else if(strncmp(buffer,"RGBAM ",6) == 0) { // Material definition
			modeldata.materials++;
		} else if(strncmp(buffer,"TEXDATA ",8) == 0) { // Texture definition
			modeldata.textures++;
		}
	}
	fseek(in,0,SEEK_SET);
	
	modeldata.vertex = (float *)malloc( modeldata.vertices * 4 * sizeof(float) );
	if(modeldata.vertex == NULL) CRASH("Unable to allocate RAM for vertices");
	modeldata.normal = (float *)malloc( modeldata.vertices * 4 * sizeof(float) );
	if(modeldata.normal == NULL) CRASH("Unable to allocate RAM for normals");
	modeldata.face   = (face_t *)malloc(modeldata.faces * sizeof(face_t) * 20 ); // To be sure, since face calculation isnt correct
	if(modeldata.face   == NULL) CRASH("Unable to allocate RAM for faces");
	modeldata.line   = (line_t *)malloc(modeldata.lines * sizeof(line_t) * 10);
	if(modeldata.line   == NULL) CRASH("Unable to allocate RAM for lines");
	modeldata.material = (material_t *)malloc(modeldata.materials * sizeof(material_t) );
	if(modeldata.material == NULL) CRASH("Unable to allocate RAM for materials");
	modeldata.texture = (texture_t *)malloc(modeldata.textures * sizeof(texture_t) );
	if(modeldata.texture == NULL) CRASH("Unable to allocate RAM for texture-struct");

	vertexIndex   = 0;
	faceIndex     = 0;
	lineIndex     = 0;
	materialIndex = 0;
	textureIndex  = 0;
	uvIndex       = 0;
	
	while(!feof(in)) {
		fgets(buffer,1022,in);
		
		if(		  strncmp(buffer,"P ",2) == 0) { // Vertex definition
			utmp[0] = sscanf(buffer,"P %f, %f, %f, %f, %f",&modeldata.vertex[vertexIndex*4+0],&modeldata.vertex[vertexIndex*4+1],&modeldata.vertex[vertexIndex*4+2],&tmp[0],&modeldata.vertex[vertexIndex*4+3]);
			if( utmp[0] != 5 ) modeldata.vertex[vertexIndex*4+3] = 0xFFFFFFFF;

			//printf("Vg: %f\n",modeldata.vertex[vertexIndex*4+3]);

			vertexIndex++;
		} else if(strncmp(buffer,"F ",2) == 0) { // Face definition
			tmpCount = sscanf(buffer,"F %i, %f, %f, %f, %f, %f, %f, %f, %f, %f",
			&points,
			&tmp[1],
			&tmp[2],
			&tmp[3],
			&tmp[4],
			&tmp[5],
			&tmp[6],
			&tmp[7],
			&tmp[8],
			&tmp[9]);

			modeldata.face[faceIndex].points   = points;
			modeldata.face[faceIndex].material = tmp[points + 1];
			modeldata.face[faceIndex].point    = (unsigned int *)malloc(points * sizeof(unsigned int));
			modeldata.face[faceIndex].uv	   = (float *)malloc(sizeof(float) * 2 * points);
			if(modeldata.face[faceIndex].point == NULL) CRASH("Unable to allocate for face-points");
			if(modeldata.face[faceIndex].uv    == NULL) CRASH("Unable to allocate for face-uvs");

			for(tmpCount=0;tmpCount<modeldata.face[faceIndex].points;tmpCount++) {
				modeldata.face[faceIndex].point[tmpCount] = tmp[1+tmpCount];
			}

			modeldata.face[faceIndex].texture = 0;

			//modeldata.face[faceIndex].isF = 1;

			uvIndex = 0;
			faceIndex++;
		} else if(strncmp(buffer,"B ",2) == 0) { // Face definition
			tmpCount = sscanf(buffer,"B %i, %f, %f, %f, %f, %f, %f, %f, %f, %f",
			&points,
			&tmp[1],
			&tmp[2],
			&tmp[3],
			&tmp[4],
			&tmp[5],
			&tmp[6],
			&tmp[7],
			&tmp[8],
			&tmp[9]);

			modeldata.face[faceIndex].points   = points;
			modeldata.face[faceIndex].material = tmp[points + 1];
			modeldata.face[faceIndex].point    = (unsigned int *)malloc(points * sizeof(unsigned int));
			modeldata.face[faceIndex].uv	   = (float *)malloc(sizeof(float) * 2 * points * 2); // Why last *2?
			if(modeldata.face[faceIndex].point == NULL) CRASH("Unable to allocate for face-points");

			for(tmpCount=0;tmpCount<modeldata.face[faceIndex].points;tmpCount++) {
				modeldata.face[faceIndex].point[tmpCount] = tmp[1+tmpCount];
			}

			modeldata.face[faceIndex].texture = 0;

			uvIndex = 0;
			faceIndex++;
		} else if(strncmp(buffer,"L ",2) == 0) { // Line definition
			tmpCount = sscanf(buffer,"L %u, %u, %f, %u",
			&modeldata.line[lineIndex].p1,
			&modeldata.line[lineIndex].p2,
			&modeldata.line[lineIndex].width,
			&modeldata.line[lineIndex].material);
			
			lineIndex++;
		} else if(strncmp(buffer,"Y ",2) == 0) { // Line-strip definition
			tmpCount = sscanf(buffer,"Y %u, %f, %f, %f, %f, %f, %f, %f, %f",
			&utmp[0],
			&tmp[0],&tmp[1],&tmp[2],&tmp[3],&tmp[4],&tmp[5],&tmp[6],&tmp[7]);

			for(tmpCount=0;tmpCount<(utmp[0]-1);tmpCount++) {
				modeldata.line[lineIndex].p1       = tmp[tmpCount];
				modeldata.line[lineIndex].p2       = tmp[tmpCount+1];
				modeldata.line[lineIndex].width    = tmp[utmp[0]];
				modeldata.line[lineIndex].material = tmp[utmp[0]+1];

				lineIndex++;
			}
		}  else if(strncmp(buffer,"RGBAM ",6) == 0) { // Line definition
			tmpCount = sscanf(buffer,"RGBAM %u, %u, %u, %u, %f",
			&utmp[0],
			&utmp[1],
			&utmp[2],
			&utmp[3],
			&modeldata.material[materialIndex].a);
			
			modeldata.material[materialIndex].r = (float)utmp[1] / 255.0f;
			modeldata.material[materialIndex].g = (float)utmp[2] / 255.0f;
			modeldata.material[materialIndex].b = (float)utmp[3] / 255.0f;
			
			materialIndex++;
		}  else if(strncmp(buffer,"TEXDATA ",8) == 0) { // Line definition
			void *rawtexture;
			
			tmpCount = sscanf(buffer,"TEXDATA %u, %u, %u, %u",
			&utmp[0],
			&utmp[1],
			&utmp[2],
			&utmp[3]);

			modeldata.texture[textureIndex].fsize  = utmp[0];
			modeldata.texture[textureIndex].width  = utmp[1];
			modeldata.texture[textureIndex].height = utmp[2];

			rawtexture = malloc(modeldata.texture[textureIndex].fsize);
			if(rawtexture==NULL) logger(SYSLOG_FATAL,"XTR","Unable to allocate %u bytes for texture",modeldata.texture[textureIndex].fsize);
			fread(rawtexture,1,modeldata.texture[textureIndex].fsize,in);

			modeldata.texture[textureIndex].texture = Texture_LoadFromMemory(rawtexture,TEXTURE_FORMAT_BMP);
			for(utmp[0]=0;utmp[0]<1000;utmp[0]++);
			free(rawtexture);
			
			//modeldata.texture[textureIndex].data = LoadBMP(in,&modeldata.texture[textureIndex]);

			Texture_BindTexture(modeldata.texture[textureIndex].texture);
/*			glGenTextures(1,&modeldata.texture[textureIndex].texture);
			glBindTexture(GL_TEXTURE_2D,modeldata.texture[textureIndex].texture);
			glTexImage2D(GL_TEXTURE_2D,0,3,modeldata.texture[textureIndex].width,modeldata.texture[textureIndex].height,0,GL_RGB,GL_UNSIGNED_BYTE,modeldata.texture[textureIndex].data);
			if( glGetError() != GL_NO_ERROR ) {
				CRASH("TexImageError..  Texture might be too big");
			}
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);*/

			textureIndex++;
		}  else if(strncmp(buffer,"TEX ",4) == 0) { // UV definition
			tmpCount = sscanf(buffer,"TEX %f, %f, %u",
			&tmp[0],
			&tmp[1],
			&utmp[0]);

			if(tmpCount != 3) CRASH("TEX count error");

			modeldata.face[faceIndex-1].uv[uvIndex*2+0] = tmp[0];
			modeldata.face[faceIndex-1].uv[uvIndex*2+1] = tmp[1];
			modeldata.face[faceIndex-1].texture         = utmp[0];

			uvIndex++;
		}  else if(strncmp(buffer,"HAUPTROTOR ",11) == 0) { // Main rotor definition
			tmpCount = sscanf(buffer,"HAUPTROTOR %u, %u, %u",
			&utmp[0],
			&utmp[1],
			&utmp[2]);

			if(tmpCount != 3) CRASH("HAUPTROTOR count error");

			modeldata.mainrotor[0] = utmp[0];
			modeldata.mainrotor[1] = utmp[1];
			
			//printf("MainRotor def: %u, %u\n",modeldata.mainrotor[0],modeldata.mainrotor[1]);
		}  else if(strncmp(buffer,"RAUCH ",6) == 0) { // Main rotor definition
			tmpCount = sscanf(buffer,"RAUCH %f, %f, %f",
			&modeldata.smoke[0],
			&modeldata.smoke[1],
			&modeldata.smoke[2]);
			
			physics.heli.engine.smoke[0] = modeldata.smoke[0] * 6.0;
			physics.heli.engine.smoke[1] = modeldata.smoke[1] * 6.0;
			physics.heli.engine.smoke[2] = modeldata.smoke[2] * 6.0;
			
			printf("SMOKE emitter: %f, %f, %f\n",modeldata.smoke[0],modeldata.smoke[1],modeldata.smoke[2]);

			if(tmpCount != 3) CRASH("RAUCH count error");
		}
	}
	modeldata.faces = faceIndex;
	fclose(in);
	
	logger(SYSLOG_INFO,"XTR","Done parsing mod-file..");

#if 0
	// Correct all UV coords
	for(faceIndex=0;faceIndex<modeldata.faces;faceIndex++) {
		for(uvIndex=0;uvIndex<modeldata.face[faceIndex].points;uvIndex++) {
			modeldata.face[faceIndex].uv[uvIndex*2+0] = (modeldata.face[faceIndex].uv[uvIndex*2+0] / (float)modeldata.texture[ modeldata.face[faceIndex].texture ].origwidth ) * modeldata.texture[ modeldata.face[faceIndex].texture ].xscale;
			modeldata.face[faceIndex].uv[uvIndex*2+1] = (((float)modeldata.texture[ modeldata.face[faceIndex].texture ].origheight - modeldata.face[faceIndex].uv[uvIndex*2+1]) / (float)modeldata.texture[ modeldata.face[faceIndex].texture ].origheight) * modeldata.texture[ modeldata.face[faceIndex].texture ].yscale;
		}
	}
	/*
	for(faceIndex=0;faceIndex<modeldata.faces;faceIndex++) {
		for(uvIndex=0;uvIndex<modeldata.face[faceIndex].points;uvIndex++) {
				modeldata.face[faceIndex].uv[uvIndex*2+0] = (modeldata.face[faceIndex].uv[uvIndex*2+0] / (float)modeldata.texture[ modeldata.face[faceIndex].texture ].origwidth ) * modeldata.texture[ modeldata.face[faceIndex].texture ].xscale;
				modeldata.face[faceIndex].uv[uvIndex*2+1] = (((float)modeldata.texture[ modeldata.face[faceIndex].texture ].origheight - modeldata.face[faceIndex].uv[uvIndex*2+1]) / (float)modeldata.texture[ modeldata.face[faceIndex].texture ].origheight) * modeldata.texture[ modeldata.face[faceIndex].texture ].yscale;

				//modeldata.face[faceIndex].uv[uvIndex*2+0] = (modeldata.face[faceIndex].uv[uvIndex*2+0] / (float)modeldata.texture[ modeldata.face[faceIndex].texture ].width)  * modeldata.texture[ modeldata.face[faceIndex].texture ].xscale;
				//modeldata.face[faceIndex].uv[uvIndex*2+1] = (((float)modeldata.texture[ modeldata.face[faceIndex].texture ].height - modeldata.face[faceIndex].uv[uvIndex*2+1]) / (float)modeldata.texture[ modeldata.face[faceIndex].texture ].height) * modeldata.texture[ modeldata.face[faceIndex].texture ].yscale;
		}
	}
	*/
#endif

	logger(SYSLOG_INFO,"XTR","XTR Modeldata: V:%u F:%u L:%u M:%u T:%u",modeldata.vertices,modeldata.faces,modeldata.lines,modeldata.materials,modeldata.textures);
#if 0
	printf("-------------\n");
	printf("Vertices : %u\n",modeldata.vertices);
	printf("Faces    : %u\n",modeldata.faces);
	printf("Lines    : %u\n",modeldata.lines);
	printf("Materials: %u\n",modeldata.materials);
	printf("Textures : %u\n",modeldata.textures);
#endif
}

#define MAIN_LIST  0
#define ROTOR_LIST 1

GLuint callList[3];

static GLuint *BuildLists(void) {
	unsigned int faceIndex,pointIndex,lineIndex;
	
	logger(SYSLOG_INFO,"XTR","Building OpenGL display-lists....");
	
	callList[0] = glGenLists(1);
	callList[1] = glGenLists(1);

	glNewList(callList[0],GL_COMPILE);
	//glColor3f(1.0,1.0,1.0);
	// First, all the faces that doesnt belong to the main rotor
	for(faceIndex=0;faceIndex<modeldata.faces;faceIndex++) {
		//glBindTexture(GL_TEXTURE_2D,modeldata.texture[ modeldata.face[faceIndex].texture ].texture);
		Texture_BindTexture(modeldata.texture[ modeldata.face[faceIndex].texture ].texture);

		// Check if the face is part of the main rotor
		if( (modeldata.vertex[ modeldata.face[faceIndex].point[0] * 4 + 3 ] >= (modeldata.mainrotor[0] - modeldata.mainrotor[1])) &&
			(modeldata.vertex[ modeldata.face[faceIndex].point[0] * 4 + 3 ] <= (modeldata.mainrotor[0] + modeldata.mainrotor[1])) ) {
			// It is, so dont put it in this list
		} else {
			// It's not, so put it in the list
			glBegin(GL_POLYGON);
			for(pointIndex=0;pointIndex<modeldata.face[faceIndex].points;pointIndex++) {
				Texture_MapUV( modeldata.face[faceIndex].uv[pointIndex * 2 + 0],
							  modeldata.face[faceIndex].uv[pointIndex * 2 + 1] );
				//glTexCoord2f( modeldata.face[faceIndex].uv[pointIndex * 2 + 0],
				//			  modeldata.face[faceIndex].uv[pointIndex * 2 + 1] );
				glVertex3f( modeldata.vertex[ modeldata.face[faceIndex].point[pointIndex] * 4 + 0 ],
							modeldata.vertex[ modeldata.face[faceIndex].point[pointIndex] * 4 + 1 ],
							modeldata.vertex[ modeldata.face[faceIndex].point[pointIndex] * 4 + 2 ]);
			}
			glEnd();
		}
	}
#if 1
	// Now, all the lines that doesnt belong to the main rotor
	for(lineIndex=0;lineIndex<modeldata.lines;lineIndex++) {
		glLineWidth( modeldata.line[lineIndex].width * 1.0f );

		// Check if the line is part of the main rotor
		if( (modeldata.vertex[ modeldata.line[lineIndex].p1 * 4 + 3] >= (modeldata.mainrotor[0] - modeldata.mainrotor[1])) &&			
			(modeldata.vertex[ modeldata.line[lineIndex].p2 * 4 + 3] <= (modeldata.mainrotor[0] + modeldata.mainrotor[1])) ) {
			// It is, so dont put it in this list
		} else {
			// It's not, so put it in the list
			//glColor3f(1.0,1.0,1.0);

			glBegin(GL_LINES);

			/*glColor3f( modeldata.material[ modeldata.line[lineIndex].material-1 ].r,
					   modeldata.material[ modeldata.line[lineIndex].material-1 ].g,
					   modeldata.material[ modeldata.line[lineIndex].material-1 ].b);*/
		
			glVertex3f(modeldata.vertex[ modeldata.line[lineIndex].p1*4+0],
					   modeldata.vertex[ modeldata.line[lineIndex].p1*4+1],
					   modeldata.vertex[ modeldata.line[lineIndex].p1*4+2]);
			glVertex3f(modeldata.vertex[ modeldata.line[lineIndex].p2*4+0],
					   modeldata.vertex[ modeldata.line[lineIndex].p2*4+1],
					   modeldata.vertex[ modeldata.line[lineIndex].p2*4+2]);

			glEnd();
		}
	}
#endif
	glEndList();

	glNewList(callList[1],GL_COMPILE);

	// First, all the faces that DO belong to the main rotor
	for(faceIndex=0;faceIndex<modeldata.faces;faceIndex++) {
		Texture_BindTexture(modeldata.texture[ modeldata.face[faceIndex].texture ].texture);
		//glBindTexture(GL_TEXTURE_2D,modeldata.texture[ modeldata.face[faceIndex].texture ].texture);

		// Check if the face is part of the main rotor
		if( (modeldata.vertex[ modeldata.face[faceIndex].point[0] * 4 + 3 ] >= (modeldata.mainrotor[0] - modeldata.mainrotor[1])) &&
			(modeldata.vertex[ modeldata.face[faceIndex].point[0] * 4 + 3 ] <= (modeldata.mainrotor[0] + modeldata.mainrotor[1])) ) {
			// It is, so put it in this list
			glBegin(GL_POLYGON);
			for(pointIndex=0;pointIndex<modeldata.face[faceIndex].points;pointIndex++) {
				Texture_MapUV( modeldata.face[faceIndex].uv[pointIndex * 2 + 0],
							  modeldata.face[faceIndex].uv[pointIndex * 2 + 1] );
				//glTexCoord2f( modeldata.face[faceIndex].uv[pointIndex * 2 + 0],
				//			  modeldata.face[faceIndex].uv[pointIndex * 2 + 1] );
				glVertex3f( modeldata.vertex[ modeldata.face[faceIndex].point[pointIndex] * 4 + 0 ],
							modeldata.vertex[ modeldata.face[faceIndex].point[pointIndex] * 4 + 1 ],
							modeldata.vertex[ modeldata.face[faceIndex].point[pointIndex] * 4 + 2 ]);
			}
			glEnd();
		} else {
			// It's not, so dont put it in the list
		}
	}

	// Now, all the lines that DO belong to the main rotor
	for(lineIndex=0;lineIndex<modeldata.lines;lineIndex++) {
		glLineWidth( modeldata.line[lineIndex].width * 1.0f );

		// Check if the line is part of the main rotor
		if( (modeldata.vertex[ modeldata.line[lineIndex].p1 * 4 + 3] >= (modeldata.mainrotor[0] - modeldata.mainrotor[1])) &&			
			(modeldata.vertex[ modeldata.line[lineIndex].p2 * 4 + 3] <= (modeldata.mainrotor[0] + modeldata.mainrotor[1])) ) {
			// It is, so put it in this list
			glBegin(GL_LINES);

			glColor3f( modeldata.material[ modeldata.line[lineIndex].material-1 ].r,
					   modeldata.material[ modeldata.line[lineIndex].material-1 ].g,
					   modeldata.material[ modeldata.line[lineIndex].material-1 ].b);
		
			glVertex3f(modeldata.vertex[ modeldata.line[lineIndex].p1*4+0],
					   modeldata.vertex[ modeldata.line[lineIndex].p1*4+1],
					   modeldata.vertex[ modeldata.line[lineIndex].p1*4+2]);
			glVertex3f(modeldata.vertex[ modeldata.line[lineIndex].p2*4+0],
					   modeldata.vertex[ modeldata.line[lineIndex].p2*4+1],
					   modeldata.vertex[ modeldata.line[lineIndex].p2*4+2]);

			glEnd();
		} else {
			// It's not, so dont put it in the list
		}
	}
	glEndList();

	return(callList);
}

GLuint *Model_LoadMOD(char *fname) {
	GLuint *lists;

	LoadMOD(fname);
	logger(SYSLOG_INFO,"XTR","Done loading model");

	lists = BuildLists();

	return(lists);
}
