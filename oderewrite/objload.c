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
