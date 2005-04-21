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
 * $RCFS: Core/Physics/physics.c,v 1.1.1.1 2005/03/17 17:41:54 jaja Exp $
 *
 */
#include <stdio.h>
#include <stdlib.h> 

/*
 * On windows you need to define _USE_MATH_DEFINES before you can get access to M_PI
 */
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include "rcfs.h"
#include "maths.h"
#include "mixers.h"
#include "model.h"

#define GRAVITY 9.82f

static float lastTime = 0.0,deltaTime;
static float finalVector[4],prevVector[4];

static model_t *model = NULL;

void Physics_Initialize(void) {
	Maths_IdentityMatrix(physics.heli.rotationMatrix);
	Maths_IdentityMatrix(physics.heli.finalMatrix);

	Maths_VectorZero(finalVector);
	Maths_VectorZero(prevVector);

	physics.heli.pos[0] =  0.0f;
	physics.heli.pos[1] =  8.0f;
	physics.heli.pos[2] = -4.0f;
	physics.heli.pos[3] =  1.0f;

	physics.heli.speed[0] = 0.0f;
	physics.heli.speed[1] = 0.0001f;
	physics.heli.speed[2] = 0.0f;
	physics.heli.speed[3] = 1.0f;
	
	physics.heli.weight = 3.0; // 3.0Kg heli

	physics.heli.rotor.rpm = 1800;

	/* This should be read from a parameters file */
	if(model == NULL) {
		model = (model_t *)malloc(sizeof(model_t));
		model->parameters.rotor.chord  = 0.05;
		model->parameters.rotor.radius = 0.55;
	}
}

void Physics_Reset(void) {
	Physics_Initialize();
}

/*
 * Calculates the lift of the main rotor perpendicular to the main shaft
 * Takes into account:
 *  * Air density at various altitudes (Accurate to a height of 140Km)
 *  * Angle of incidence of main blades
 *  * Chord of main blades
 *  * RPM of main rotor
 * Doesnt take into account:
 *  * Angle of attack
 *  * Blade-stalls
 *  * Compressible air
 *  * Tip vortices
 */
 
#define AIR_DENSITY_SEALEVEL 1.225f*9.82f

static float Physics_CalcRotorLift(void) {
	float BladeTipHeight;         // The area exposed forward of a tilted blade (0 for 0deg pitch)
	float displacementRevolution; // How much air we displace for one revolution of the blades
	float displacementTotal;      // How much air we displace in a second
	float lift;                   // Lift, taking into account density

	
	BladeTipHeight = model->parameters.rotor.chord * sin(physics.heli.rotor.pitch * M_PI / 180.0);

	displacementRevolution  = BladeTipHeight; // Should be divided by two, but we have two blades.. :)
	displacementRevolution *= M_PI * model->parameters.rotor.radius * model->parameters.rotor.radius * 2; // Times two, since we have two blades

	displacementTotal = displacementRevolution * physics.heli.rotor.rpm;
	
	lift = displacementTotal * AIR_DENSITY_SEALEVEL * (1.0f - (0.01f*(physics.heli.pos[1]/80))); // Air-pressure decreases with 1% per 80m

	//printf("Lift: %.2fN - ",lift);

	return(lift * 1.8);
}

/*
 * Calculates the amount of lift generated from the angle of attack
 * between the direction of travel and the tilt of the rotor-disc
 */
float Physics_CalculateAoALift(void) {
	static float heliUpVector[4] = { 0.0f, -1.0f, 0.0f, 1.0f };
	static float discVector[4]; //,scaledSpeed[4];
	//static float dot,alpha,lift,speedLength;
	static float vlen[2];
	static float displacementRevolution;
	static float cosalpha,alpha,lift;
		
	//cos(a) = (1x2 + 2x1)/(||V0|| ||V1||)
	
	Maths_VectorMatrixMultiply(discVector,heliUpVector,physics.heli.rotationMatrix);
	vlen[0] = sqrt( (physics.heli.speed[0]*physics.heli.speed[0]) + (physics.heli.speed[1]*physics.heli.speed[1]) + (physics.heli.speed[2]*physics.heli.speed[2]) );
	vlen[1] = sqrt( (discVector[0]*discVector[0]) + (discVector[1]*discVector[1]) + (discVector[2]*discVector[2]) );
	
	if( (vlen[0] == 0.0) && (vlen[1] == 0.0) ) return(0.0);
	
	cosalpha = (discVector[0]*physics.heli.speed[0] + discVector[1]*physics.heli.speed[1] + discVector[2]*physics.heli.speed[2]) /
			   (vlen[0] * vlen[1]);
	
	alpha = acos(cosalpha);

	displacementRevolution = sin(alpha * 2.0) * M_PI * model->parameters.rotor.radius * model->parameters.rotor.radius * 2; // Times two, since we have two blades

	lift = displacementRevolution * AIR_DENSITY_SEALEVEL * (1.0f - (0.01f*(physics.heli.pos[1]/80))); // Air-pressure decreases with 1% per 80m
	return( lift );
	
	
	lift = sin(alpha * 2.0) * (vlen[0] / 1.0);
	
	//printf("Lift: %f\n",lift);
	//printf("Alpha: %.4f - cosAlpha: %.4f - Lift: %.4f\n",alpha / M_PI * 180.0,cosalpha,sin(alpha * 2.0));

	return(lift);
}


void Physics_Update(void) {
	static float gravityAccelerationVector[4] = { 0.0f, -9.82f, 0.0f, 1.0f };
	static float heliAccelerationVector[4]    = { 0.0f,  0.0f , 0.0f, 1.0f };
	static float heliAirfoilLift[4]			  = { 0.0f,  0.0f , 0.0f, 1.0f };
	static float tmpVector[4],deltaRotation[4],tmpMatrix[16];
	static float currentTime,liftForce,liftAcceleration,aoalift;
	float *input;
	
	/* !!! Ugly.. Wrong place for a GLUT call */
	currentTime = glutGet(GLUT_ELAPSED_TIME);
	if( lastTime == 0.0 ) {
		deltaTime = 0.0;
	} else {
		deltaTime   = (currentTime - lastTime) / 1000.0f;
	}
	lastTime    = currentTime;

	input = Mixer_GetControls();

	/* Add the delta-rotation to the rotation-matrix for the heli */
	deltaRotation[0] = input[MIXER_AILERON]  * deltaTime * 5.0f;
	deltaRotation[2] = input[MIXER_ELEVATOR] * deltaTime * 5.0f;
	deltaRotation[1] = input[MIXER_RUDDER]   * deltaTime * 12.0f;
		
	Maths_EulerRotation(tmpMatrix,deltaRotation[0],0.0f,0.0f);
	Maths_MatrixMultiply(physics.heli.rotationMatrix,tmpMatrix);
	Maths_EulerRotation(tmpMatrix,0.0f,deltaRotation[1],0.0f);
	Maths_MatrixMultiply(physics.heli.rotationMatrix,tmpMatrix);
	Maths_EulerRotation(tmpMatrix,0.0f,0.0f,deltaRotation[2]);
	Maths_MatrixMultiply(physics.heli.rotationMatrix,tmpMatrix);

	// 1st, lets calculate the forces. Right now, the only force we have is the one from the main rotor
	physics.heli.rotor.rpm = (input[MIXER_THROTTLE]+1.0)/2.0 * 1800;
	physics.heli.rotor.pitch = input[MIXER_COLLECTIVE] * 12.0; // +-10 degs of pitch
	liftForce = Physics_CalcRotorLift();

	// 2nd, recalculate the forces into acceleration using "a = F/m"
	liftAcceleration = liftForce / (physics.heli.weight * 9.82f);

	// 3rd, rotate the acceleration-vector
	heliAccelerationVector[0] = 0.0f;
	heliAccelerationVector[1] = liftAcceleration;
	heliAccelerationVector[2] = 0.0f;
	heliAccelerationVector[3] = 1.0f;

	Maths_VectorMatrixMultiply(tmpVector,heliAccelerationVector,physics.heli.rotationMatrix);

	aoalift = Physics_CalculateAoALift();
	heliAirfoilLift[0] = 0.0;
	heliAirfoilLift[1] = aoalift / (physics.heli.weight * 9.82f);
	heliAirfoilLift[2] = 0.0;
	heliAirfoilLift[3] = 1.0;

	// 4th, add up all the acceleration-vectors
	Maths_VectorZero(heliAccelerationVector);
	Maths_VectorAdd(heliAccelerationVector,gravityAccelerationVector);
	Maths_VectorAdd(heliAccelerationVector,heliAirfoilLift);
	Maths_VectorAdd(heliAccelerationVector,tmpVector);

	// 5th, add the acceleration to the speed
	Maths_VectorScale(heliAccelerationVector,deltaTime);
	Maths_VectorAdd(physics.heli.speed,heliAccelerationVector);


	//printf("Speed: %.4f - %.4f - %.4f\n",physics.heli.speed[0],physics.heli.speed[1],physics.heli.speed[2]);

	// Fake drag (assumes heli is 1m^2 in all directions)
	//drag = 1.0 * AIR_DENSITY_SEALEVEL * speed;
	
//	physics.heli.speed[0] = physics.heli.speed[0] / 1.01f;
//	physics.heli.speed[1] = physics.heli.speed[1] / 1.01f;
//	physics.heli.speed[2] = physics.heli.speed[2] / 1.01f;
	
	// 6th, add the speed to the position
	Maths_VectorCopy(tmpVector,physics.heli.speed);
	Maths_VectorScale(tmpVector,deltaTime);
	Maths_VectorAdd(physics.heli.pos,tmpVector);

	// Crash reset (!!! hax)
#if 1
	if(physics.heli.pos[1] < 0.5f) {
		Maths_IdentityMatrix(physics.heli.rotationMatrix);
		Maths_IdentityMatrix(physics.heli.finalMatrix);
		
		Maths_VectorZero(finalVector);
		Maths_VectorZero(prevVector);
		
		physics.heli.pos[0] =  0.0f;
		physics.heli.pos[1] =  8.0f;
		physics.heli.pos[2] = -4.0f;
		physics.heli.pos[3] =  1.0f;
		
		physics.heli.speed[0] = 0.0f;
		physics.heli.speed[1] = 0.00001f;
		physics.heli.speed[2] = 0.0f;
		physics.heli.speed[3] = 1.0f;		
	}
#endif
#if 0
	if(physics.heli.pos[1] < 0.5f) {
		float upVector[4];
		
		physics.heli.speed[1] = 0.0;
		physics.heli.pos[1] = 0.5;

		physics.heli.speed[0] = 0.0;
		physics.heli.speed[2] = 0.0;

		upVector[0] = 0.0; upVector[1] = 1.0; upVector[2] = 0.0; upVector[3] = 1.0;

		Maths_VectorMatrixMultiply(tmpVector,upVector,physics.heli.rotationMatrix);
	
		tmpVector[0] = tmpVector[0] * deltaTime;
		tmpVector[2] = tmpVector[2] * deltaTime;
	
		Maths_EulerRotation(tmpMatrix,tmpVector[2],0.0f,0.0f);
		Maths_MatrixMultiply(physics.heli.rotationMatrix,tmpMatrix);
		Maths_EulerRotation(tmpMatrix,0.0f,0.0f,tmpVector[0]);
		Maths_MatrixMultiply(physics.heli.rotationMatrix,tmpMatrix);

		/*Maths_EulerRotation(tmpMatrix,0.0f,deltaRotation[1],0.0f);
		Maths_MatrixMultiply(physics.heli.rotationMatrix,tmpMatrix);
		Maths_EulerRotation(tmpMatrix,0.0f,0.0f,deltaRotation[2]);
		Maths_MatrixMultiply(physics.heli.rotationMatrix,tmpMatrix);*/

	}
#endif
	Maths_IdentityMatrix(physics.heli.finalMatrix);
	Maths_MatrixMultiply(physics.heli.finalMatrix,physics.heli.rotationMatrix);
}
