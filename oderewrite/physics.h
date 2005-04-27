/*
 *  physics.h
 *  Collective3
 *
 *  Created by James Jacobsson on 4/25/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#include "collective.h"

#define PHYSICS_HELI    1
#define PHYSICS_SCENERY 2

void      Physics_Initialize(void);
double   *Physics_GetRotation(object_t *obj);
double   *Physics_GetPosition(object_t *obj);
void      Physics_RegisterObject(object_t *obj,int flags);
void      Physics_Update(void);


#endif
