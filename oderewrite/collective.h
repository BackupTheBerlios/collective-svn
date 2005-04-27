/*
 *  collective.h
 *  Collective3
 *
 *  Created by James Jacobsson on 4/25/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _COLLECTIVE_H_
#define _COLLECTIVE_H_

#include "ode/ode.h"

/* Apparently these are already defined by ODE */
#if 0
typedef unsigned char      uint8;
typedef char                int8;
typedef unsigned short     uint16;
typedef short               int16;
typedef unsigned int       uint32;
typedef int                 int32;
typedef unsigned long long uint64;
typedef long long           int64;
#endif

typedef struct {
	uint32 p[3]; // Point / Vertex
	uint32 n[3]; // Normal
	uint32 t[3]; // Texture-coords
} face_t;

typedef struct {
        char        *texture;
        char        *name;
        int          texnum;
        double       diffuse[3],ambient[3];
        face_t      *face;
        uint32       faces;
		uint8		 textured;
} material_t;

typedef struct {
        struct {
			uint32      vertices,mappings,normals,materials;
			double     *vertex,*normal,*mapping;
			material_t *material;
        } geometry;
        uint8 isRBD;
		double boundingbox[3];
        //double pos[3];
        //double rot[16];
        dBodyID bodyID;
        dGeomID geomID;
} object_t;

typedef struct {
        object_t *obj;
        double    throttle,pitch;
        double    weight,rotorRadius;
} heli_t;

extern heli_t heli;

#ifdef __cplusplus
extern "C" {
#endif

void   Main_Update(void);

void   Architecture_Initialize(void);
double Architecture_GetHighresTime(void);

double *Darwin_Audio_GetInputs(void);

void      Physics_Initialize(void);
double   *Physics_GetRotation(object_t *obj);
double   *Physics_GetPosition(object_t *obj);
void      Physics_RegisterObject(object_t *obj,int flags);
void      Physics_Update(void);

#ifdef __cplusplus
}
#endif

#endif