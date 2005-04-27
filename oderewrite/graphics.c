/*
 *  graphics.c
 *  Collective2
 *
 *  This is the OpenGL implementation of the graphics library for Collective
 *
 *  Created by James Jacobsson on 4/24/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
#include <GLUT/glut.h>
#include "collective.h"
#include "logger.h"
#include "texturemanager.h"

int win[2],window;
double lookat[3];

#define MAX_OBJECTS 10

object_t *object[MAX_OBJECTS];
uint8 objects = 0;

void Graphics_LookAt(object_t *obj) {
	const dReal *pos;
	
	pos = dBodyGetPosition(obj->bodyID);
	lookat[0] = pos[0];
	lookat[1] = pos[1];
	lookat[2] = pos[2];
}

void Graphics_RegisterObject(object_t *obj) {
	if(objects >= (MAX_OBJECTS-1)) logger(SYSLOG_FATAL,"GFX","Tried to register one too many objects");
	object[objects++] = obj;
}

static void Graphics_DrawObject(object_t *obj);

void Graphics_DrawObjects(void) {
	int i;
	
	for(i=0;i<objects;i++)
		Graphics_DrawObject(object[i]);
}

static void Graphics_DrawObject(object_t *obj) {
	const dReal *pos,*rot;
	double rotMatrix[16];
	unsigned int i;
	
	glPushMatrix();

	pos = dBodyGetPosition(obj->bodyID);
	rot = dBodyGetRotation(obj->bodyID);
	glTranslated(pos[0],pos[1],pos[2]);

	rotMatrix[ 0] = rot[ 0];
	rotMatrix[ 1] = rot[ 1];
	rotMatrix[ 2] = rot[ 2];
	rotMatrix[ 3] = rot[ 3];
	rotMatrix[ 4] = rot[ 4];
	rotMatrix[ 5] = rot[ 5];
	rotMatrix[ 6] = rot[ 6];
	rotMatrix[ 7] = rot[ 7];
	rotMatrix[ 8] = rot[ 8];
	rotMatrix[ 9] = rot[ 9];
	rotMatrix[10] = rot[10];
	rotMatrix[11] = rot[11];
	rotMatrix[12] = 0.0;
	rotMatrix[13] = 0.0;
	rotMatrix[14] = 0.0;
	rotMatrix[15] = 1.0;
	
	glMultMatrixd(rotMatrix);
	
	//glRotatef(180.0,0.0,0.0,1.0);
	
	if(obj->geometry.material[0].faces < 1000) glColor3f(0.7,0.7,0.7);
	else glColor3f(1.0,1.0,0.0);

	if( obj->geometry.material[0].textured ) {
		glEnable(GL_TEXTURE_2D);
		Texture_BindTexture( obj->geometry.material[0].texnum );
	} else {
		glDisable(GL_TEXTURE_2D);
	}
	
	glBegin(GL_TRIANGLES);
	for(i=0;i<obj->geometry.material[0].faces;i++) {
#if 1
		Texture_MapUV( obj->geometry.mapping[ obj->geometry.material[0].face[i].t[0] * 2 + 0 ],
					   obj->geometry.mapping[ obj->geometry.material[0].face[i].t[0] * 2 + 1 ] );	
		glVertex3d( obj->geometry.vertex[ obj->geometry.material[0].face[i].p[0] * 3 + 0],
					obj->geometry.vertex[ obj->geometry.material[0].face[i].p[0] * 3 + 1],
					obj->geometry.vertex[ obj->geometry.material[0].face[i].p[0] * 3 + 2]);
		Texture_MapUV( obj->geometry.mapping[ obj->geometry.material[0].face[i].t[1] * 2 + 0 ],
					   obj->geometry.mapping[ obj->geometry.material[0].face[i].t[1] * 2 + 1 ] );	
		glVertex3d( obj->geometry.vertex[ obj->geometry.material[0].face[i].p[1] * 3 + 0],
					obj->geometry.vertex[ obj->geometry.material[0].face[i].p[1] * 3 + 1],
					obj->geometry.vertex[ obj->geometry.material[0].face[i].p[1] * 3 + 2]);
		Texture_MapUV( obj->geometry.mapping[ obj->geometry.material[0].face[i].t[2] * 2 + 0 ],
					   obj->geometry.mapping[ obj->geometry.material[0].face[i].t[2] * 2 + 1 ] );	
		glVertex3d( obj->geometry.vertex[ obj->geometry.material[0].face[i].p[2] * 3 + 0],
					obj->geometry.vertex[ obj->geometry.material[0].face[i].p[2] * 3 + 1],
					obj->geometry.vertex[ obj->geometry.material[0].face[i].p[2] * 3 + 2]);
#endif
	}
	glEnd();
	
	glColor3f(0.0,0.0,0.0);
	glBegin(GL_POINTS);
	for(i=0;i<obj->geometry.vertices;i++) {
		glVertex3d( obj->geometry.vertex[i * 3 + 0],
					obj->geometry.vertex[i * 3 + 1],
					obj->geometry.vertex[i * 3 + 2]);
	}
	glEnd();
	
	glColor3f(1.0,0.0,0.0);
	glBegin(GL_LINES);
		// Back
		glVertex3d( -(obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );
		glVertex3d(  (obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );

		glVertex3d( -(obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );
		glVertex3d( -(obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );

		glVertex3d(  (obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );
		glVertex3d( -(obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );

		glVertex3d(  (obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );
		glVertex3d(  (obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );

		// Front
		glVertex3d( -(obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );
		glVertex3d(  (obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );

		glVertex3d( -(obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );
		glVertex3d( -(obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );

		glVertex3d(  (obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );
		glVertex3d( -(obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );

		glVertex3d(  (obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );
		glVertex3d(  (obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );

		// Between
		glVertex3d( -(obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );
		glVertex3d( -(obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );

		glVertex3d(  (obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );
		glVertex3d(  (obj->boundingbox[0] / 2.0),-(obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );

		glVertex3d( -(obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );
		glVertex3d( -(obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );

		glVertex3d(  (obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0), -(obj->boundingbox[2] / 2.0) );
		glVertex3d(  (obj->boundingbox[0] / 2.0), (obj->boundingbox[1] / 2.0),  (obj->boundingbox[2] / 2.0) );

	glEnd();
	
	glPopMatrix();
}

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

float grot = 0.0;

static void Graphics_Display(void) {
	glClearDepth(1.0f);
	glClearColor(0.3,0.3,0.3,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glLoadIdentity();

	gluLookAt(10.0,2.0,10.0,  // Eye
			  lookat[0],lookat[1],lookat[2],  // Target
			  0.0,1.0,0.0); // UP

	Main_Update();
	
	glutSwapBuffers();
}

static void Graphics_Idleloop(void) {
	glutPostRedisplay();
}

void Graphics_Mainloop(void) {
	glutMainLoop();
}

void Graphics_Initialize(int *argc,char **argv) {
	glutInit(argc,argv);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
	window = glutCreateWindow("Collective2");
	glutSetWindow(window);
	glShadeModel(GL_SMOOTH);
	
	glutReshapeFunc( Graphics_Reshape);
	glutDisplayFunc( Graphics_Display);
	//glutKeyboardFunc(Keyboard_Callback);
	glutIdleFunc(    Graphics_Idleloop);
}