/*
 *  main.c
 *  Collective3
 *
 *  Created by James Jacobsson on 4/25/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "collective.h"
#include "physics.h"
#include "graphics.h"
#include "input.h"
#include "objload.h"

object_t *heliObject,*sceneryObject;

heli_t heli;

/*
 * This gets called once per frame, and is responsible for calling all the subsystems
 * so that they do their updating for this frame
 */
void Main_Update(void) {
	Physics_Update();
	
	Graphics_LookAt(heli.obj);
	
	Graphics_DrawObjects();
	//Graphics_DrawObject(heli.obj);
}

int main(int argc,char **argv) {
	Input_Initialize();  /* Input needs to be initialized before architecture, since architecture registers its input-devices into the input subsystem */

	Architecture_Initialize();
	
	Physics_Initialize();
	
	Graphics_Initialize(&argc,argv);

	//heliObject    = Object_LoadObject("/Users/slowcoder/src/Collective/models/Fury.mod",PHYSICS_HELI);
	heliObject    = Object_LoadObject("/Users/slowcoder/src/Collective3/test.obj",PHYSICS_HELI);
	sceneryObject = Object_LoadObject("/Users/slowcoder/Documents/MayaScenes/Meridian.obj",PHYSICS_SCENERY);
	
	Graphics_RegisterObject(   heliObject);
	Graphics_RegisterObject(sceneryObject);
	
	Graphics_Mainloop();
	
	return(0);
}
