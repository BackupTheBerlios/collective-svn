/*
 *  objload.c
 *  Collective3
 *
 *  Created by James Jacobsson on 4/25/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
#include <string.h>

#include "collective.h"
#include "logger.h"
#include "physics.h"
#include "objload-obj.h"

#include "objload.h"

static void Object_LoadMOD(char *fname,object_t *obj);
static void Object_LoadMODXX(char *fname,object_t *obj) {
	FILE *in;
	char buffer[1024];
	float ftmp[3];
	double box[3];
	unsigned int vertex;

	box[0] = box[1] = box[2] = 0.0;

	obj->geometry.vertices  = 0;
	obj->geometry.materials = 0;

	logger(SYSLOG_INFO,"OBJ","Loading MOD: %s",fname);

	in = fopen(fname,"rt");
	if( in == NULL ) logger(SYSLOG_FATAL,"OBJ","Unable to open file %s",fname);
	
	while(!feof(in)) {
		fgets(buffer,1024,in);
		if( strncmp(buffer,"P ",2) == 0 ) obj->geometry.vertices++;
	}
	fseek(in,0,SEEK_SET);

	obj->geometry.vertex = (double *)malloc( sizeof(double) * obj->geometry.vertices * 3 );

	vertex = 0;
	while(!feof(in)) {
		fgets(buffer,1024,in);

		if( strncmp(buffer,"P ",2) == 0 ) {
			sscanf(buffer,"P %f, %f, %f",&ftmp[0],&ftmp[1],&ftmp[2]);
			obj->geometry.vertex[vertex * 3 + 0] = ftmp[0] * 20.0;
			obj->geometry.vertex[vertex * 3 + 1] = ftmp[1] * 20.0;
			obj->geometry.vertex[vertex * 3 + 2] = ftmp[2] * 20.0;

			if( (ftmp[0] * 20.0) > box[0] ) box[0] = ftmp[0] * 20.0;
			if( (ftmp[1] * 20.0) > box[1] ) box[1] = ftmp[1] * 20.0;
			if( (ftmp[2] * 20.0) > box[2] ) box[2] = ftmp[2] * 20.0;

			vertex++;
		}
	}
	
	obj->boundingbox[0] = box[0] * 2.0;
	obj->boundingbox[1] = box[1] * 2.0;
	obj->boundingbox[2] = box[2] * 2.0;
	
	//printf("BBox: %f,%f,%f\n",box[0] / 10.0,box[1] / 10.0,box[2] / 10.0);
	
	fclose(in);
}

object_t *Object_LoadObject(char *filename,int flags) {
        object_t *obj;

        obj = (object_t *)malloc(sizeof(object_t));

        if(        strncmp("obj",(char *)(filename + strlen(filename) - 3),3) == 0 ) {
                Object_LoadOBJ(filename,obj);
        } else if( strncmp("mod",(char *)(filename + strlen(filename) - 3),3) == 0 ) {
                Object_LoadMOD(filename,obj);
        } else {
			logger(SYSLOG_FATAL,"OBJ","Unknown extension on object-file\n");
		}

        Physics_RegisterObject(obj,flags);

        return(obj);
}
