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
#include <math.h>

#include "collective.h"
#include "config.h"
#include "logger.h"
#include "model.h"
#include "mixers.h"
#include "physics.h"
#include "texturemanager.h"
#include "gui-font.h"

/*
* GL_BGR is not available on windows, and is implemented as part of an extension as GL_BGR_EXT
*/
#ifdef WIN32
#define GL_BGR GL_BGR_EXT
#endif

extern int win[2];

#define GUI_MAIN_PAGE    0x00
#define GUI_MONITOR_PAGE 0x01
#define GUI_REVERSE_PAGE 0x02
#define GUI_SETUP_PAGE   0x03

#define GUI_BUTTON_UP   0x00
#define GUI_BUTTON_DOWN 0x01

struct {
	float position[4][2];
	uint8 activepage;
} display;

#define CRASH(x) { printf("%s",x); exit(-1); }

static int32 guibackgroundtexture;
GLuint interfacetexture;
unsigned char *rawinterfacetexture;

config_t config;

void GUI_Widget_VMeter(int px,int py,int w,int h,float value) {
	int x,y;

	for(y=(py+(float)h*value);y<(py+h);y++) {
		for(x=px;x<(px+w);x++) {
			rawinterfacetexture[(y*512+x)*3+0] = 0x77;
			rawinterfacetexture[(y*512+x)*3+1] = 0x77;
			rawinterfacetexture[(y*512+x)*3+2] = 0x77;
		}
	}
	
	for(x=px;x<(px+w);x++) {
		rawinterfacetexture[(py*512+x)*3+0] = 0xFF;
		rawinterfacetexture[(py*512+x)*3+1] = 0xFF;
		rawinterfacetexture[(py*512+x)*3+2] = 0xFF;		
		rawinterfacetexture[((py+h)*512+x)*3+0] = 0xFF;
		rawinterfacetexture[((py+h)*512+x)*3+1] = 0xFF;
		rawinterfacetexture[((py+h)*512+x)*3+2] = 0xFF;		
	}
	for(y=py;y<(py+h);y++) {
		rawinterfacetexture[(y*512+px)*3+0] = 0xFF;
		rawinterfacetexture[(y*512+px)*3+1] = 0xFF;
		rawinterfacetexture[(y*512+px)*3+2] = 0xFF;		
		rawinterfacetexture[(y*512+px+w)*3+0] = 0xFF;
		rawinterfacetexture[(y*512+px+w)*3+1] = 0xFF;
		rawinterfacetexture[(y*512+px+w)*3+2] = 0xFF;		
	}


	for(x=px;x<(px+w);x++) {
		rawinterfacetexture[((py+(h/2))*512+x)*3+0] = 0xFF;
		rawinterfacetexture[((py+(h/2))*512+x)*3+1] = 0xFF;
		rawinterfacetexture[((py+(h/2))*512+x)*3+2] = 0xFF;		
	}
}

void GUI_Widget_Button(int px,int py,int w,int h,uint8 state,char *text) {
	int x,y,cy,cx,strx,stry;
	unsigned char shade[2];
	
	if(state == GUI_BUTTON_UP) {
		shade[0] = 0xCC;
		shade[1] = 0x77;
	} else {
		shade[0] = 0x77;
		shade[1] = 0xCC;
	}
	
	// Upper and lower
	for(x=px;x<(px+w);x++) {
			rawinterfacetexture[(py*512+x)*3+0] = shade[0];
			rawinterfacetexture[(py*512+x)*3+1] = shade[0];
			rawinterfacetexture[(py*512+x)*3+2] = shade[0];
			rawinterfacetexture[((py+1)*512+x)*3+0] = shade[0];
			rawinterfacetexture[((py+1)*512+x)*3+1] = shade[0];
			rawinterfacetexture[((py+1)*512+x)*3+2] = shade[0];

			rawinterfacetexture[((py+h)*512+x)*3+0] = shade[1];
			rawinterfacetexture[((py+h)*512+x)*3+1] = shade[1];
			rawinterfacetexture[((py+h)*512+x)*3+2] = shade[1];		
			rawinterfacetexture[((py+h-1)*512+x)*3+0] = shade[1];
			rawinterfacetexture[((py+h-1)*512+x)*3+1] = shade[1];
			rawinterfacetexture[((py+h-1)*512+x)*3+2] = shade[1];		
	}

	// Right and left
	for(y=py;y<(py+h);y++) {
			rawinterfacetexture[(y*512+px)*3+0] = shade[0];
			rawinterfacetexture[(y*512+px)*3+1] = shade[0];
			rawinterfacetexture[(y*512+px)*3+2] = shade[0];		
			rawinterfacetexture[(y*512+px+1)*3+0] = shade[0];
			rawinterfacetexture[(y*512+px+1)*3+1] = shade[0];
			rawinterfacetexture[(y*512+px+1)*3+2] = shade[0];		

			rawinterfacetexture[(y*512+px+w)*3+0] = shade[1];
			rawinterfacetexture[(y*512+px+w)*3+1] = shade[1];
			rawinterfacetexture[(y*512+px+w)*3+2] = shade[1];		
			rawinterfacetexture[(y*512+px+w-1)*3+0] = shade[1];
			rawinterfacetexture[(y*512+px+w-1)*3+1] = shade[1];
			rawinterfacetexture[(y*512+px+w-1)*3+2] = shade[1];		
	}
	
	// Filling
	for(y=(py+2);y<(py+h-1);y++) {
		for(x=(px+2);x<(px+w-1);x++) {
			rawinterfacetexture[(y*512+x)*3+0] = 0x96;
			rawinterfacetexture[(y*512+x)*3+1] = 0x96;
			rawinterfacetexture[(y*512+x)*3+2] = 0x96;
		}
	}

	// The text
	strx = px + 4;
	stry = py + (h/2) - 7;
	while( *text ) {
	
		for(cy=0;cy<16;cy++) {
			for(cx=0;cx<8;cx++) {
				if( font8x16[(int)*text][cy] & (1<<(7-cx)) ) {
					rawinterfacetexture[((cy+stry)*512+cx+strx)*3+0] = 0x00;
					rawinterfacetexture[((cy+stry)*512+cx+strx)*3+1] = 0x00;
					rawinterfacetexture[((cy+stry)*512+cx+strx)*3+2] = 0x00;
					
				}
			}
		}
		strx += 9;
		*text++;
	}

}

void GUI_Draw_Background(void) {
	int x,y;
	
	for(y=0;y<256;y++) {
		for(x=0;x<512;x++) {
			rawinterfacetexture[(y*512+x)*3+0] = 0x33;
			rawinterfacetexture[(y*512+x)*3+1] = 0x33;
			rawinterfacetexture[(y*512+x)*3+2] = 0xCC;
		}
	}
}

void GUI_Draw_Reverse(void) {
	GUI_Widget_Button(0,0,40,20,GUI_BUTTON_UP,"Back");

	GUI_Widget_Button(60, 30,100,20,GUI_BUTTON_UP  ,"Elevator");
	GUI_Widget_Button(60, 60,100,20,GUI_BUTTON_UP  ,"Aileron");
	GUI_Widget_Button(60, 90,100,20,GUI_BUTTON_DOWN,"Throttle");
	GUI_Widget_Button(60,120,100,20,GUI_BUTTON_UP  ,"Yaw");
	GUI_Widget_Button(60,150,100,20,GUI_BUTTON_UP  ,"Collective");

}

void GUI_Draw_Monitor(void) {
	int channel;
	float *ch;
	
	ch = Mixer_GetControls();
	
	for(channel=0;channel<8;channel++) {
		GUI_Widget_VMeter(40+30*channel,30,20,100,ch[channel] / 2.0 + 0.5);
	}
	
	GUI_Widget_Button(0,0,40,20,GUI_BUTTON_UP,"Back");
}

void GUI_Draw_Setup(void) {
	GUI_Widget_Button(0,0,40,20,GUI_BUTTON_UP,"Back");

	if( config.radio.passthrough ) {
		GUI_Widget_Button( 50,30,80,20,GUI_BUTTON_UP  ,"Radio");
		GUI_Widget_Button(150,30,80,20,GUI_BUTTON_DOWN,"Joystick");
	} else {
		GUI_Widget_Button( 50,30,80,20,GUI_BUTTON_DOWN,"Radio");
		GUI_Widget_Button(150,30,80,20,GUI_BUTTON_UP  ,"Joystick");
		
		switch( config.radio.type ) {
			case CONFIG_RADIO_JR:
				GUI_Widget_Button( 50,80,80,20,GUI_BUTTON_UP  ,"JR");
				break;
			case CONFIG_RADIO_FUTABA:
				GUI_Widget_Button( 50,80,80,20,GUI_BUTTON_UP  ,"Futaba");
				break;
			case CONFIG_RADIO_HITEC:
				GUI_Widget_Button( 50,80,80,20,GUI_BUTTON_UP  ,"Hitec");
				break;
		}
	}
}

void GUI_Draw_Main(void) {
	
	GUI_Widget_Button( 20, 20, 60, 20,GUI_BUTTON_UP,"Setup");
	GUI_Widget_Button( 20, 50,100, 20,GUI_BUTTON_UP,"Ch monitor");
	//GUI_Widget_Button( 20, 80,100, 20,GUI_BUTTON_UP,"Ch assign");
	GUI_Widget_Button( 20,110,100, 20,GUI_BUTTON_UP,"Ch reverse");
}

void GUI_Handle_Mouse(int x,int y) {
	switch( display.activepage ) {
		case GUI_MAIN_PAGE:
			if(      (x > 20) && (x < 120) && (y >  50) && (y <  70) ) display.activepage = GUI_MONITOR_PAGE;
			else if( (x > 20) && (x < 120) && (y > 110) && (y < 130) ) display.activepage = GUI_REVERSE_PAGE;
			else if( (x > 20) && (x <  80) && (y >  20) && (y <  40) ) display.activepage = GUI_SETUP_PAGE;
			break;
		case GUI_MONITOR_PAGE:
			if( (x < 40) && (y < 20) ) display.activepage = GUI_MAIN_PAGE;
			break;
		case GUI_REVERSE_PAGE:
			if( (x < 40) && (y < 20) ) display.activepage = GUI_MAIN_PAGE;
			break;
		case GUI_SETUP_PAGE:
			if(      (x <  40) && (y <  20) ) display.activepage = GUI_MAIN_PAGE;
			else if( (x >  50) && (x < 130) && (y > 30) && (y <  50) ) config.radio.passthrough = 0;
			else if( (x > 150) && (x < 230) && (y > 30) && (y <  50) ) config.radio.passthrough = 1;
			else if( (x >  50) && (x < 130) && (y > 80) && (y < 110) && !config.radio.passthrough ) {
				config.radio.type++;
				if(config.radio.type >= CONFIG_RADIO_END) config.radio.type = 0x00;
			}
			break;
	}
}

void IsInDisplay(int x,int y) {
	float cross[4],angle[2],length[2];
	
	//printf("Pre: %ix%i - ",x,y);
	
	x = x - ((win[0] - 800) / 2);
	y = y - ((win[1] - 600) / 2);
	y = 600 - y;
	
	cross[0] = (y - display.position[0][1])*(display.position[1][0]-display.position[0][0]) - (x - display.position[0][0])*(display.position[1][1] - display.position[0][1]);
	cross[1] = (y - display.position[1][1])*(display.position[2][0]-display.position[1][0]) - (x - display.position[1][0])*(display.position[2][1] - display.position[1][1]);
	cross[2] = (y - display.position[2][1])*(display.position[3][0]-display.position[2][0]) - (x - display.position[2][0])*(display.position[3][1] - display.position[2][1]);
	cross[3] = (y - display.position[3][1])*(display.position[0][0]-display.position[3][0]) - (x - display.position[3][0])*(display.position[0][1] - display.position[3][1]);
	
	// Calc angle between P0-XY P1-XY
	length[0] = sqrt( (x - display.position[0][0])*(x - display.position[0][0]) +
					  (y - display.position[0][1])*(y - display.position[0][1]) );

	angle[0] = asin( (y - display.position[0][1]) / length[0] );
	//printf("Angle: %.4f\n",angle[0]);
	
	if( (cross[0] < 0.0) && (cross[1] < 0.0) && (cross[2] < 0.0) && (cross[3] < 0.0)) {
		int relx,rely;
		float tmpx;
		
		tmpx = (-cross[3] / (-cross[1] + -cross[3])) * 512.0;
		relx = tmpx;
		tmpx = (-cross[0] / (-cross[2] + -cross[0])) * 256.0;
		rely = tmpx;
		
		//relx = (float)(x - display.position[0][0]) * (512.0 / (float)(display.position[1][0] - display.position[0][0]));
		//rely = (float)(y - display.position[0][1]) * (256.0 / (float)(display.position[3][1] - display.position[0][1]));
		
		GUI_Handle_Mouse(relx,rely);
		//printf("Inside..(%ix%i)\n",relx,rely);
	} else {
		//printf("not inside (%ix%i)\n",x,y);
	}
}

void Graphics_GUI_Mousefunction(int button,int state,int x,int y) {
	if(state == GLUT_DOWN) IsInDisplay(x,y);
}

void Graphics_GUI_Idleloop(void) {
	glutPostRedisplay();
}

void Graphics_GUI_Initialize(void) {
	char str[100]; // !!! <- Buffer overflow waiting to happend
	int x,y;
	
	rawinterfacetexture = (unsigned char *)malloc(512*256*3);
	if(rawinterfacetexture == NULL) CRASH("No ram left");
	
	for(y=0;y<256;y++) {
		for(x=0;x<512;x++) {
			rawinterfacetexture[(y*512+x)*3+0] = 0x00;
			rawinterfacetexture[(y*512+x)*3+1] = 0x00;
			rawinterfacetexture[(y*512+x)*3+2] = 0x00;
		}
	}
	
	glGenTextures(1,&interfacetexture);
	glBindTexture(GL_TEXTURE_2D,interfacetexture);
	glTexImage2D(GL_TEXTURE_2D,0,3,512,256,0,GL_RGB,GL_UNSIGNED_BYTE,rawinterfacetexture);
	if( glGetError() != GL_NO_ERROR ) {
		logger(SYSLOG_FATAL,"TEX","TexImageError..  Texture might be too big");
	}
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	for(y=0;y<256;y++) {
		for(x=0;x<512;x++) {
			rawinterfacetexture[(y*512+x)*3+0] = 0xFF;
			rawinterfacetexture[(y*512+x)*3+1] = (x & 0x4) ? 0xFF : 0x00;
			rawinterfacetexture[(y*512+x)*3+2] = 0xFF;
		}
	}

	display.activepage = GUI_MAIN_PAGE;

#if 1
	display.position[0][0] =  57; display.position[0][1] = 345;
	display.position[1][0] = 477; display.position[1][1] = 585;
	display.position[2][0] = 558; display.position[2][1] = 493;
	display.position[3][0] = 134; display.position[3][1] = 231;
#endif
#if 0
	display.position[0][0] =  60; display.position[0][1] = 250;
	display.position[1][0] = 600; display.position[1][1] = 250;
	display.position[2][0] = 600; display.position[2][1] =  50;
	display.position[3][0] =  60; display.position[3][1] =  50;
#endif	
	/*
	 * Load the background image
	 */
	Config_getstring("gui.background",str);
	guibackgroundtexture = Texture_LoadFromFile(str);
}

void Graphics_DisplayGUI(void) {
	glClearDepth(1.0f);
	glClearColor(0.0,1.0,0.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0,win[0],0.0,win[1]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(1.0,1.0,1.0);

	glEnable(GL_TEXTURE_2D);
	Texture_BindTexture(guibackgroundtexture);
	glBegin(GL_QUADS);
	
		if( (win[0] < 800) || (win[1] < 600) ) {
			Texture_MapUV(0.0,0.0); glVertex2f(win[0] / 20     ,win[1] / 20);
			Texture_MapUV(1.0,0.0); glVertex2f(win[0] / 20 * 19,win[1] / 20);
			Texture_MapUV(1.0,1.0); glVertex2f(win[0] / 20 * 19,win[1] / 20 * 19);
			Texture_MapUV(0.0,1.0); glVertex2f(win[0] / 20     ,win[1] / 20 * 19);
		} else {
			Texture_MapUV(0.0,0.0); glVertex2f(win[0] / 2 - 400,win[1] / 2 - 300);
			Texture_MapUV(1.0,0.0); glVertex2f(win[0] / 2 + 400,win[1] / 2 - 300);
			Texture_MapUV(1.0,1.0); glVertex2f(win[0] / 2 + 400,win[1] / 2 + 300);
			Texture_MapUV(0.0,1.0); glVertex2f(win[0] / 2 - 400,win[1] / 2 + 300);
		}
	glEnd();

	GUI_Draw_Background();
	switch(display.activepage) {
		case GUI_MAIN_PAGE:
			GUI_Draw_Main();
			break;
		case GUI_MONITOR_PAGE:
			GUI_Draw_Monitor();
			break;
		case GUI_REVERSE_PAGE:
			GUI_Draw_Reverse();
			break;
		case GUI_SETUP_PAGE:
			GUI_Draw_Setup();
			break;
	}

	glBindTexture(GL_TEXTURE_2D,interfacetexture);
	glTexImage2D(GL_TEXTURE_2D,0,3,512,256,0,GL_RGB,GL_UNSIGNED_BYTE,rawinterfacetexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glBegin(GL_QUADS);
		if( (win[0] > 800) && (win[1] > 600) ) {
			int xstart,ystart;
			
			xstart = win[0] / 2 - 400;
			ystart = win[1] / 2 - 300;
			
			glTexCoord2f(0.0,0.0); glVertex2f(xstart + display.position[0][0],ystart + display.position[0][1]);
			glTexCoord2f(1.0,0.0); glVertex2f(xstart + display.position[1][0],ystart + display.position[1][1]);
			glTexCoord2f(1.0,1.0); glVertex2f(xstart + display.position[2][0],ystart + display.position[2][1]);
			glTexCoord2f(0.0,1.0); glVertex2f(xstart + display.position[3][0],ystart + display.position[3][1]);
		}
	glEnd();

	glDisable(GL_TEXTURE_2D);


	//Graphics_GUI_DrawWidgets();

#if 1
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
#endif

//	glFlush();
	glutSwapBuffers();
}
