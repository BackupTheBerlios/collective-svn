/*
 *  objload-obj.c
 *  Collective3
 *
 *  Created by James Jacobsson on 4/25/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
#include "collective.h"
#include "logger.h"
#include "texturemanager.h"

#include "objload-obj.h"

static void Object_OBJ_LoadMaterial(char *fname,object_t *obj) {
	FILE *inmtl;
	char  buffer[1024];
	int   material;
	
	inmtl = fopen(fname,"rt");
	if(inmtl == NULL) logger(SYSLOG_FATAL,"OBJ","Unable to open OBJ materials file %s",fname);
	
	material = -1;
	
	while(!feof(inmtl)) {
		fgets(buffer,1024,inmtl);
		
		if(        strncmp(buffer,"newmtl ",7) == 0 ) {
			material++;
			obj->geometry.material[material].name = (char *)malloc(80);
			sscanf(buffer,"newmtl %80s",obj->geometry.material[material].name);
		} else if( strncmp(buffer,"map_Kd ",7) == 0 ) {
			obj->geometry.material[material].texture = (char *)malloc(80);
			sscanf(buffer,"map_Kd %80s",obj->geometry.material[material].texture);

			//obj->geometry.material[material].texnum = Texture_LoadFromFile(obj->geometry.material[material].texture);
		}
	}
	
	fclose(inmtl);
}

object_t *Object_LoadOBJ(char *fname,object_t *obj) {
	FILE        *in;
	char         buffer[1024],filename[80];
	float        ftmp[3];
	double       box[3];
	unsigned int utmp[9],vertex,face,mapping,normal;
	int material;

	box[0] = box[1] = box[2] = 0.0;

	obj->geometry.vertices  = 0;
	obj->geometry.mappings  = 0;
	obj->geometry.normals   = 0;
	obj->geometry.materials = 0;
	
	in = fopen(fname,"rt");
	if(in == NULL) logger(SYSLOG_FATAL,"OBJ","Unable open OBJ file %s",fname);
	
	while(!feof(in)) {
		fgets(buffer,1024,in);
		
		if(        strncmp(buffer,"v ",2) == 0 ) {
			obj->geometry.vertices++;
		} else if( strncmp(buffer,"vn ",2)  == 0 ) {
			obj->geometry.normals++;
		} else if( strncmp(buffer,"vt ",2)  == 0 ) {
			obj->geometry.mappings++;
		} else if( strncmp(buffer,"usemtl ",7)  == 0 ) {
			obj->geometry.materials++;
		}
	}
	fseek(in,0,SEEK_SET); 

	obj->geometry.vertex   = (double     *)malloc( sizeof(double)     * obj->geometry.vertices * 3 );
	obj->geometry.normal   = (double     *)malloc( sizeof(double)     * obj->geometry.normals  * 3 );
	obj->geometry.mapping  = (double     *)malloc( sizeof(double)     * obj->geometry.mappings * 2 );
	obj->geometry.material = (material_t *)malloc( sizeof(material_t) * obj->geometry.materials );

	material = -1;

	/* Need to do a little loop here to figure out how many faces in each material */
	while(!feof(in)) {
		fgets(buffer,1024,in);

		if(        strncmp(buffer,"f ",2) == 0 ) {
			obj->geometry.material[material].faces++;
		} else if( strncmp(buffer,"usemtl ",7)  == 0 ) {
			if(material >= 0) printf("Face: %u\n",obj->geometry.material[material].faces);
			material++;
			obj->geometry.material[material].faces = 0;
		}
	}
	fseek(in,0,SEEK_SET); 

	for(material=0;material<obj->geometry.materials;material++) {
		obj->geometry.material[material].face = (face_t *)malloc( sizeof(face_t) * obj->geometry.material[material].faces );
	}
	
	vertex = mapping = normal = face = 0;
	material = -1;
	
	while(!feof(in)) {
		fgets(buffer,1024,in);
		
		if(        strncmp(buffer,"v ",2) == 0 ) {
			sscanf(buffer,"v %f %f %f",&ftmp[0],&ftmp[1],&ftmp[2]);
			obj->geometry.vertex[vertex * 3 + 0] = ftmp[0];
			obj->geometry.vertex[vertex * 3 + 1] = ftmp[1];
			obj->geometry.vertex[vertex * 3 + 2] = ftmp[2];
			
			if(ftmp[0] > box[0]) box[0] = ftmp[0];
			if(ftmp[1] > box[1]) box[1] = ftmp[1];
			if(ftmp[2] > box[2]) box[2] = ftmp[2];
			
			vertex++;
		} else if( strncmp(buffer,"vn ",2)  == 0 ) {
			sscanf(buffer,"vn %f %f %f",&ftmp[0],&ftmp[1],&ftmp[2]);
			obj->geometry.normal[normal * 3 + 0] = ftmp[0];
			obj->geometry.normal[normal * 3 + 1] = ftmp[1];
			obj->geometry.normal[normal * 3 + 2] = ftmp[2];
			normal++;
		} else if( strncmp(buffer,"vt ",2)  == 0 ) {
			sscanf(buffer,"vt %f %f",&ftmp[0],&ftmp[1]);
			obj->geometry.mapping[mapping * 2 + 0] = ftmp[0];
			obj->geometry.mapping[mapping * 2 + 1] = ftmp[1];
			mapping++;
		} else if( strncmp(buffer,"usemtl ",7)  == 0 ) {
			material++;
			face = 0;
		} else if( strncmp(buffer,"f ",2)  == 0 ) {
			sscanf(buffer,"f %u/%u/%u %u/%u/%u %u/%u/%u",
					&utmp[0],&utmp[1],&utmp[2],
					&utmp[3],&utmp[4],&utmp[5],
					&utmp[6],&utmp[7],&utmp[8]);
			
			obj->geometry.material[material].face[face].p[0] = utmp[0] - 1;
			obj->geometry.material[material].face[face].p[1] = utmp[3] - 1;
			obj->geometry.material[material].face[face].p[2] = utmp[6] - 1;
			obj->geometry.material[material].face[face].t[0] = utmp[1] - 1;
			obj->geometry.material[material].face[face].t[1] = utmp[4] - 1;
			obj->geometry.material[material].face[face].t[2] = utmp[7] - 1;
			obj->geometry.material[material].face[face].n[0] = utmp[2] - 1;
			obj->geometry.material[material].face[face].n[1] = utmp[5] - 1;
			obj->geometry.material[material].face[face].n[2] = utmp[8] - 1;

			//printf("Mat = %u\n",material);

			face++;
		} else if( strncmp(buffer,"mtllib ",7)  == 0 ) {
			sscanf(buffer,"mtllib %80s",filename);

			Object_OBJ_LoadMaterial(filename,obj);
		}
	}
	
	
	printf("Vertices : %u\n",obj->geometry.vertices);
	printf("Materials: %u\n",obj->geometry.materials);
	
	fclose(in);
	
	return(obj);
}