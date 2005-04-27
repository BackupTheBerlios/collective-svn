#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

void      Graphics_Initialize(int *argc,char **argv);
void      Graphics_RegisterObject(object_t *obj);
void      Graphics_DrawObjects(void);
void      Graphics_LookAt(object_t *obj);
void      Graphics_Mainloop(void);

#endif