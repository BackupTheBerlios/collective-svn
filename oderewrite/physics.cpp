/*
 *  physics.cpp
 *  Collective3
 *
 *  Created by James Jacobsson on 4/25/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "collective.h"
#include "physics.h"

static dWorldID world;
static dSpaceID space;
static dJointGroupID contactgroup;

static void ODE_nearCallback(void *data,dGeomID o1,dGeomID o2);

void Physics_Initialize(void) {
	world = dWorldCreate();
	dWorldSetGravity(world,0.0,0.0,0.0);
	
	space = dSimpleSpaceCreate(0);

	contactgroup = dJointGroupCreate(0);
}

static double lastTime;

void Physics_Update(void) {
	double deltaTime,currentTime;
	double *inputs;
	
	currentTime = Architecture_GetHighresTime();
	
	if( lastTime == 0.0 ) {
		deltaTime = 0.00001;
	} else {
		deltaTime   = (currentTime - lastTime);
	}
	lastTime    = currentTime;
	
	inputs = Darwin_Audio_GetInputs();
	//printf("i0=%f\n",inputs[0]);
	
	dBodyAddRelForce(heli.obj->bodyID,0.0,100.0*inputs[0],0.0);  // Collective
	//dBodyAddTorque(heli.obj->bodyID,0.0,0.0,2.0 * -inputs[1]); // Aileron
	//dBodyAddTorque(heli.obj->bodyID,0.0,2.0 * inputs[3],0.00); // Yaw
	dBodyAddTorque(heli.obj->bodyID,1.0 * inputs[2],0.00,0.0); // Elevator
	//dBodyAddRelTorque(heli.obj->bodyID,0.0,0.0,0.05);
	//dBodyAddRelTorque(heli.obj->bodyID,0.0,1.0,0.00); // Yaw
	
	dSpaceCollide(space,0,&ODE_nearCallback);
	dWorldStep(world,deltaTime);
	dJointGroupEmpty(contactgroup);
}

void Physics_RegisterObject(object_t *obj,int flags) {
	dMass m;

	dMassSetBox(&m,1,0.5,0.5,0.5);
	dMassAdjust(&m,1.0);

	if( flags & PHYSICS_HELI ) {
		heli.obj = obj;
		
		obj->bodyID = dBodyCreate(world);
		dBodySetMass(obj->bodyID,&m);
		dBodySetPosition(obj->bodyID,0.0,30.0,0.0);

		//obj->geomID = dCreateBox(space,3.0,1.0,3.0);
		obj->geomID = dCreateBox(space,obj->boundingbox[0]*2.0,obj->boundingbox[1]*2.0,obj->boundingbox[2]*2.0);
        dGeomSetBody(obj->geomID,obj->bodyID);
	} else if( flags & PHYSICS_SCENERY ) {
		obj->bodyID = dBodyCreate(world);

		dBodySetPosition(obj->bodyID,0.0,100.0,0.0);
		dBodySetMass(obj->bodyID,&m);		
		dBodySetGravityMode(obj->bodyID,0); // Scenery isnt affected by gravity

		obj->geomID = dCreateBox(space,1000.0,1.1,1000.0);
		dGeomSetPosition(obj->geomID,0.0,-12.0,0.0);
		dGeomSetBody(obj->geomID,0); // Makes this object immovable
	}
}

void Physics_Destroy(void) {
	dWorldDestroy(world);
}

/*
 * INTERNAL functions
 */

#define MAX_CONTACTS 100

static void ODE_nearCallback(void *data,dGeomID o1,dGeomID o2) {
        int i,numc;
        dContact contact[MAX_CONTACTS];
        dBodyID b1,b2;
        
        b1 = dGeomGetBody(o1);
        b2 = dGeomGetBody(o2);

        if (b1 && b2 && dAreConnectedExcluding (b1,b2,dJointTypeContact)) return;
        //printf("Collision..\n");
        
        for(i=0;i<MAX_CONTACTS;i++) {
                contact[i].surface.mode       = dContactBounce | dContactSoftCFM | dContactSoftERP;
                contact[i].surface.mode       = dContactBounce | dContactSoftERP;
                contact[i].surface.mu         = dInfinity;
                contact[i].surface.mu         = 100.01;
                contact[i].surface.mu2        = 0;
                contact[i].surface.bounce     = 0.3;
                contact[i].surface.bounce_vel = 0.02;
                contact[i].surface.soft_cfm   = 0.1;
                contact[i].surface.soft_erp   = 0.2;
        }
        numc = dCollide(o1,o2,MAX_CONTACTS,&contact[0].geom,sizeof(dContact));
        if(numc) {
                //const dReal ss[3] = {0.02,0.02,0.02};
                //printf("Coll between %i points (%i,%i)..\n",numc,b1,b2);
                for (i=0; i<numc; i++) {
                        dJointID c = dJointCreateContact (world,contactgroup,contact+i);
                        dJointAttach (c,b1,b2);
                }
        }
} 


