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
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "collective.h"
#include "maths.h"
#include "logger.h"
#include "config.h"
#include "model.h"
#include "mixers.h"
#include "physics.h"
#include "texturemanager.h"

int window,win[2];
static GLuint *aircraftlist;

/*
 * Start of shadow-code
 */
float g_shadowMatrix[16];

typedef struct {
    float nx, ny, nz;
    float x, y, z;
} Vertex;

Vertex g_floorQuad[] =
{
    { 0.0f, 1.0f,  0.0f,  -150.0f, 0.0f, -150.0f },
    { 0.0f, 1.0f,  0.0f,  -150.0f, 0.0f,  150.0f },
    { 0.0f, 1.0f,  0.0f,   150.0f, 0.0f,  150.0f },
    { 0.0f, 1.0f,  0.0f,   150.0f, 0.0f, -150.0f },
};

void buildShadowMatrix( float fMatrix[16], float fLightPos[4], float fPlane[4] )
{
    float dotp;

    // Calculate the dot-product between the plane and the light's position
    dotp = fPlane[0] * fLightPos[0] + 
           fPlane[1] * fLightPos[1] + 
           fPlane[1] * fLightPos[2] + 
           fPlane[3] * fLightPos[3];

    // First column
    fMatrix[0]  = dotp - fLightPos[0] * fPlane[0];
    fMatrix[4]  = 0.0f - fLightPos[0] * fPlane[1];
    fMatrix[8]  = 0.0f - fLightPos[0] * fPlane[2];
    fMatrix[12] = 0.0f - fLightPos[0] * fPlane[3];

    // Second column
    fMatrix[1]  = 0.0f - fLightPos[1] * fPlane[0];
    fMatrix[5]  = dotp - fLightPos[1] * fPlane[1];
    fMatrix[9]  = 0.0f - fLightPos[1] * fPlane[2];
    fMatrix[13] = 0.0f - fLightPos[1] * fPlane[3];

    // Third column
    fMatrix[2]  = 0.0f - fLightPos[2] * fPlane[0];
    fMatrix[6]  = 0.0f - fLightPos[2] * fPlane[1];
    fMatrix[10] = dotp - fLightPos[2] * fPlane[2];
    fMatrix[14] = 0.0f - fLightPos[2] * fPlane[3];

    // Fourth column
    fMatrix[3]  = 0.0f - fLightPos[3] * fPlane[0];
    fMatrix[7]  = 0.0f - fLightPos[3] * fPlane[1];
    fMatrix[11] = 0.0f - fLightPos[3] * fPlane[2];
    fMatrix[15] = dotp - fLightPos[3] * fPlane[3];
}

void findPlane( GLfloat plane[4], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3] )
{
    GLfloat vec0[3], vec1[3];

    // Need 2 vectors to find cross product
    vec0[0] = v1[0] - v0[0];
    vec0[1] = v1[1] - v0[1];
    vec0[2] = v1[2] - v0[2];

    vec1[0] = v2[0] - v0[0];
    vec1[1] = v2[1] - v0[1];
    vec1[2] = v2[2] - v0[2];

    // Find cross product to get A, B, and C of plane equation
    plane[0] =   vec0[1] * vec1[2] - vec0[2] * vec1[1];
    plane[1] = -(vec0[0] * vec1[2] - vec0[2] * vec1[0]);
    plane[2] =   vec0[0] * vec1[1] - vec0[1] * vec1[0];

    plane[3] = -(plane[0] * v0[0] + plane[1] * v0[1] + plane[2] * v0[2]);
}

void renderFloor(void) {
    glColor3f( 1.0f, 1.0f, 1.0f );
    glInterleavedArrays( GL_N3F_V3F, 0, g_floorQuad );
    glDrawArrays( GL_QUADS, 0, 4 );
}

float g_lightPosition[] = { 2.0f, 6.0f, 0.0f, 1.0f };


/*
 * End of shadow-code
 */


static void Graphics_Reshape(int w,int h) {
	win[0] = w;
	win[1] = h;

	glViewport     ( 0, 0, w, h );
	glMatrixMode   ( GL_PROJECTION );  // Select The Projection Matrix
	glLoadIdentity ( );                // Reset The Projection Matrix
	if( h == 0 )  // Calculate The Aspect Ratio Of The Window
		gluPerspective ( 70, (float)w, 1.0, 10000.0 );
	else
		gluPerspective ( 70, (float)w / (float)h, 1.0,10000.0 );
	glMatrixMode   ( GL_MODELVIEW );  // Select The Model View Matrix
	glLoadIdentity ( );    // Reset The Model View Matrix
}

static float rotorRot = 0.0;
static int   elapsedTime[2],frames = 0;
uint32       currentTime,deltaTime,lastTime;
uint8		 fullscreen = 0,guiSwitch = 0;
int          winPos[2], winSize[2];

static void Graphics_Fullscreen(void) {
	fullscreen = !fullscreen;
	if(!fullscreen) {
		glutReshapeWindow(winSize[0],winSize[1]);
		glutPositionWindow(winPos[0],winPos[1]);
	} else {
		winPos[0] = glutGet(GLUT_WINDOW_X);
		winPos[1] = glutGet(GLUT_WINDOW_Y);
		winSize[0] = glutGet(GLUT_WINDOW_WIDTH);
		winSize[1] = glutGet(GLUT_WINDOW_HEIGHT);
		glutFullScreen();
	}
}

void Graphics_DisplayGUI(void);
static void Graphics_Particle_Display(void);
static void Graphics_Particle_Update(uint32 deltaTime);

static void Graphics_DisplaySimulator(void) {
	GLfloat shadowPlane[4];
	GLfloat v0[3],v1[3],v2[3];
	float   lightPosition[4];
	float  *input;
	
	input = Mixer_GetControls();

	// SHADOW code
	// To define a plane that matches the floor, we need to 3 vertices from it
    v0[0] = g_floorQuad[0].x;
    v0[1] = g_floorQuad[0].y;
    v0[2] = g_floorQuad[0].z;

    v1[0] = g_floorQuad[1].x;
    v1[1] = g_floorQuad[1].y;
    v1[2] = g_floorQuad[1].z;

    v2[0] = g_floorQuad[2].x;
    v2[1] = g_floorQuad[2].y;
    v2[2] = g_floorQuad[2].z;

    findPlane( shadowPlane, v0, v1, v2 );

	// Recalc light-position and draw lightsource
	lightPosition[0] = 20.0;
	lightPosition[1] = 800.0;
	lightPosition[2] = 20.0;

	g_lightPosition[0] = lightPosition[0];
	g_lightPosition[1] = lightPosition[1];
	g_lightPosition[2] = lightPosition[2];
	g_lightPosition[3] = 1.0f;

    //
    // Build a shadow matrix using the light's current position and the plane
    //
    buildShadowMatrix( g_shadowMatrix, g_lightPosition, shadowPlane );
	
	// EOS
	
	glClearDepth(1.0f);
	glClearColor(0.3,0.3,1.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	glDisable(GL_LIGHTING);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.0f, 2.0f,0.0f,
			  physics.heli.pos[0],physics.heli.pos[1],physics.heli.pos[2],
			  0.0f, 1.0f,0.0f);

	// 1st, we render the world
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	//renderFloor();
	glCallList(aircraftlist[2]);

	glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_COLOR_MATERIAL);

	glColor3f(0.3,0.3,0.3);
	glPushMatrix();
		glMultMatrixf((GLfloat *)g_shadowMatrix);
		glTranslatef(physics.heli.pos[0],physics.heli.pos[1],physics.heli.pos[2]);
		glMultMatrixf(physics.heli.finalMatrix);
		glScalef(6.0,6.0,6.0);
		glRotatef(-90.0,0.0,1.0,0.0);
		glColor4f(0.0,0.0,0.0,0.4);
		glCallList(aircraftlist[0]);
	glPopMatrix();


	glEnable(GL_DEPTH_TEST);

	/* Draw the helicopter */
	//glPushMatrix();
	glMatrixMode   ( GL_PROJECTION );  // Select The Projection Matrix
	glLoadIdentity ( );                // Reset The Projection Matrix
	gluPerspective ( 50, (float)win[0] / (float)win[1], 1.0,10000.0 );
	glMatrixMode   ( GL_MODELVIEW );  // Select The Model View Matrix


	glTranslatef(physics.heli.pos[0],physics.heli.pos[1],physics.heli.pos[2]);
	glMultMatrixf(physics.heli.finalMatrix);

	glScalef(6.0,6.0,6.0);
	glRotatef(-90.0,0.0,1.0,0.0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glColor3f(1.0,1.0,1.0);
	glCallList(aircraftlist[0]);

	glPushMatrix();
		glRotatef(rotorRot,0.0,1.0,0.0);
		glCallList(aircraftlist[1]);
	glPopMatrix();

	elapsedTime[1] = glutGet(GLUT_ELAPSED_TIME);
	currentTime    = elapsedTime[1];
	deltaTime      = currentTime - lastTime;
	lastTime       = currentTime;

	// Particle system
	glLoadIdentity();

	gluLookAt(0.0f, 2.0f,0.0f,
			  physics.heli.pos[0],physics.heli.pos[1],physics.heli.pos[2],
			  0.0f, 1.0f,0.0f);

	Graphics_Particle_Update(deltaTime);
	Graphics_Particle_Display();

	glutSwapBuffers();
	
	frames++;
		
	rotorRot += (physics.heli.rotor.rpm / 60.0) * deltaTime;
	
	if(elapsedTime[1] >= (elapsedTime[0]+1000)) {
		elapsedTime[0] = elapsedTime[1];
		logger(SYSLOG_DEBUG,"GFX","FPS: %.1f",(float)frames);
		frames = 0;
	}
}

GLuint *Model_GetCallListPointer(void);

static void Graphics_BuildDisplayList(void) {
	aircraftlist = Model_GetCallListPointer();
}

void Graphics_GUI_Idleloop(void);

void Graphics_GUI_Mousefunction(int button,int state,int x,int y);

// Swaps between the GUI and the simulaton (0=Sim, 1=GUI)
static void Graphics_GUISwitch(uint8 gui) {
	int w,h;
	
	w = win[0]; h = win[1];

	if(!gui) {
		glViewport     ( 0, 0, w, h );
		glMatrixMode   ( GL_PROJECTION );  // Select The Projection Matrix
		glLoadIdentity ( );                // Reset The Projection Matrix
		if( h == 0 )  // Calculate The Aspect Ratio Of The Window
			gluPerspective ( 70, (float)w, 1.0, 10000.0 );
		else
			gluPerspective ( 70, (float)w / (float)h, 1.0,10000.0 );
		glMatrixMode   ( GL_MODELVIEW );  // Select The Model View Matrix
		glLoadIdentity ( );    // Reset The Model View Matrix

		glutDisplayFunc( Graphics_DisplaySimulator);
		glutMouseFunc(NULL);
		glutIdleFunc(Main_Idleloop);
	} else {
		glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0.0,1024.0,1024.0,0.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

		glutDisplayFunc(Graphics_DisplayGUI);	
		glutMouseFunc(Graphics_GUI_Mousefunction);
		glutIdleFunc(Graphics_GUI_Idleloop);
	}
}

void Keyboard_Callback(unsigned char key,int x,int y) {
	switch(key) {
		case 'q':
			inputdev.Release();
			exit(0);
			break;
		case 27:
			guiSwitch = !guiSwitch;
			Graphics_GUISwitch(guiSwitch);
			break;
		case 'f':
			Graphics_Fullscreen();
			break;
		case ' ':
			Physics_Reset();
			break;
	}
}

GLfloat LightAmbient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = {80.0f, 80.0f,80.0f, 1.0f };

void Graphics_GUI_Initialize(void);

#define MAX_PARTICLES 100
static int smoketexture;

struct {
	float pos[3];
	float speed[3];
	float life;
	float scale[2];
	uint8 active;
} particle[MAX_PARTICLES];

static void Graphics_Particle_Initialize(void) {
	uint32 i;
	char   str[100];

	Config_getstring("smoke.texture",str);
	smoketexture = Texture_LoadFromFile(str);
	
	for(i=0;i<MAX_PARTICLES;i++) {
		particle[i].active = 0;
		particle[i].life   = (float)i * (1.0 / (float)MAX_PARTICLES);
	}
}

static void Graphics_Particle_Update(uint32 deltaTimeMS) {
	static float heliUpVector[4] = { 0.0f, -1.0f, 0.0f, 1.0f };
	static float discVector[4];
	static uint32 i;
	float deltaTime;
	
	Maths_VectorMatrixMultiply(discVector,heliUpVector,physics.heli.rotationMatrix);
		
	deltaTime = (float)deltaTimeMS / 1000.0;
	
	for(i=0;i<MAX_PARTICLES;i++) {
		particle[i].life -= 1.0 / 60.0;
		//particle[i].life -= (float)deltaTime / 1000.0;

		particle[i].scale[0] += deltaTime / 0.8;
		particle[i].scale[1] += deltaTime / 0.8;

		particle[i].pos[0] += particle[i].speed[0];
		particle[i].pos[1] += particle[i].speed[1];
		particle[i].pos[2] += particle[i].speed[2];

		if(particle[i].pos[1] < 0.0) {
			particle[i].pos[1] = -0.01;

			//particle[i].speed[0] = particle[i].speed[1] * 3.0;
			//particle[i].speed[2] = particle[i].speed[1] * 3.0;

			particle[i].scale[0] += deltaTime * 4.0;
			particle[i].scale[1]  = 0.6;
		
			//particle[i].speed[1] = 0.0;
		}

		if(particle[i].life <= 0.0) { // Particle has expired. Time to be reborn
			particle[i].pos[0] = physics.heli.pos[0];
			particle[i].pos[1] = physics.heli.pos[1];
			particle[i].pos[2] = physics.heli.pos[2];

			particle[i].scale[0] = 0.0;
			particle[i].scale[1] = 0.0;
			
			particle[i].speed[0] = discVector[0] * (physics.heli.rotor.pitch / 50.0);
			particle[i].speed[1] = discVector[1] * (physics.heli.rotor.pitch / 50.0);
			particle[i].speed[2] = discVector[2] * (physics.heli.rotor.pitch / 50.0);
			
			particle[i].life   = 1.0;
			particle[i].active = 1;
		}
	}
}

static void Graphics_Particle_Display(void) {
	uint32 i;
	float  tmpMatrix[16];
	
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	Texture_BindTexture(smoketexture);

	glPushMatrix();
#if 0
	/*gluLookAt(0.0f, 2.0f,0.0f,
			  physics.heli.pos[0],physics.heli.pos[1],physics.heli.pos[2],
			  0.0f, 1.0f,0.0f);*/
	glGetFloatv(GL_MODELVIEW_MATRIX,tmpMatrix);
	tmpMatrix[0] = 1.0; tmpMatrix[4] = 0.0; tmpMatrix[8] = 0.0;
	tmpMatrix[1] = 0.0; tmpMatrix[5] = 1.0; tmpMatrix[9] = 0.0;
	tmpMatrix[2] = 0.0; tmpMatrix[6] = 0.0; tmpMatrix[10] = 1.0;
	glLoadMatrixf(tmpMatrix);

#endif
	glBegin(GL_QUADS);
		for(i=0;i<MAX_PARTICLES;i++) {
			if(particle[i].active) {
				glColor4f(1.0,1.0,1.0,particle[i].life + 0.2);
#if 1
					Texture_MapUV(1.0,1.0); glVertex3f( particle[i].pos[0] + particle[i].scale[0],particle[i].pos[1] + particle[i].scale[1],particle[i].pos[2] );
					Texture_MapUV(1.0,0.0); glVertex3f( particle[i].pos[0] + particle[i].scale[0],particle[i].pos[1] - particle[i].scale[1],particle[i].pos[2] );
					Texture_MapUV(0.0,0.0); glVertex3f( particle[i].pos[0] - particle[i].scale[0],particle[i].pos[1] - particle[i].scale[1],particle[i].pos[2] );
					Texture_MapUV(0.0,1.0); glVertex3f( particle[i].pos[0] - particle[i].scale[0],particle[i].pos[1] + particle[i].scale[1],particle[i].pos[2] );
#endif
#if 0
					Texture_MapUV(1.0,1.0); glVertex3f( particle[i].pos[0] + (1.0-particle[i].life),particle[i].pos[1] + (1.0-particle[i].life),particle[i].pos[2] );
					Texture_MapUV(1.0,0.0); glVertex3f( particle[i].pos[0] + (1.0-particle[i].life),particle[i].pos[1] - (1.0-particle[i].life),particle[i].pos[2] );
					Texture_MapUV(0.0,0.0); glVertex3f( particle[i].pos[0] - (1.0-particle[i].life),particle[i].pos[1] - (1.0-particle[i].life),particle[i].pos[2] );
					Texture_MapUV(0.0,1.0); glVertex3f( particle[i].pos[0] - (1.0-particle[i].life),particle[i].pos[1] + (1.0-particle[i].life),particle[i].pos[2] );
#endif
			}
		}
	glEnd();
	
	glPopMatrix();
}

void Graphics_Initialize(int *argc,char **argv) { /* GLut need the args, as well as SGIs MultiPipe */
	char versionInfo[15];

	glutInit(argc,argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );

	sprintf(versionInfo,"Collective v%s",COLLECTIVE_VERSION);
	window = glutCreateWindow(versionInfo);
	glutSetWindow(window);
	
	glOrtho(-10,10,-10,10,-10,10);
	glClearColor(0,0,0,1);
	
	glShadeModel(GL_SMOOTH);
	//glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	//glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);
	//glEnable(GL_LIGHT1);
	//glEnable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	
	Graphics_Particle_Initialize();
	Graphics_GUI_Initialize();
	
	logger(SYSLOG_INFO,"GFX","OpenGL: Using renderer %s",glGetString(GL_RENDERER));
}

void Graphics_Mainloop(void) {
	Graphics_BuildDisplayList();

	glutReshapeFunc( Graphics_Reshape);
	glutDisplayFunc( Graphics_DisplaySimulator);
	glutKeyboardFunc(Keyboard_Callback);

	glutIdleFunc(Main_Idleloop);

	glutMainLoop();
}

void Graphics_Redraw(void) {
	glutPostRedisplay();
}
