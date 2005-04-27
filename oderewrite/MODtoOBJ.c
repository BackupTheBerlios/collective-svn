#include <stdio.h>
#include <stdlib.h>

#define CRASH(x) { printf("%s",x); exit(-1); }

unsigned int vertices = 0,faces = 0;

typedef struct {
	unsigned int p[3];
	double       uv[3][2];
} face_t;

double       *vertex;
face_t       *face;

int main(int argc,char **argv) {
	FILE        *in,*out;
	char         buffer[1024];
	unsigned int utmp[20];
	float        ftmp[20];
	unsigned int vi,fi,ti,i,preface,line = 0;
	
	printf("Analyzing %s\n",argv[1]);
	in = fopen(argv[1],"rt");
	if(in == NULL) CRASH("Cant open input-file\n");
	
	while(!feof(in)) {
		fgets(buffer,1024,in);
		if(      strncmp(buffer,"P ",2) == 0 ) vertices++;
		else if( strncmp(buffer,"F ",2) == 0 ) {
			sscanf(buffer,"F %u",&utmp[0]);
			faces += utmp[0] - 2;
		} else if( strncmp(buffer,"B ",2) == 0 ) {
			sscanf(buffer,"B %u",&utmp[0]);
			faces += utmp[0] - 2;
		}
	}
	fseek(in,0,SEEK_SET);
	
	vertex  = (double *)malloc( sizeof(double) * vertices * 3 );
	face    = (face_t *)malloc( sizeof(face_t) * faces );
	
	vi = fi = 0;
	while(!feof(in)) {
		fgets(buffer,1024,in);
		line++;
		if(      strncmp(buffer,"P ",2) == 0 ) {
			sscanf(buffer,"P %f, %f, %f",&ftmp[0],&ftmp[1],&ftmp[2]);
			vertex[vi * 3 + 0] = ftmp[0] * 20.0;
			vertex[vi * 3 + 1] = ftmp[1] * 20.0;
			vertex[vi * 3 + 2] = ftmp[2] * 20.0;
			vi++;
		} else if( strncmp(buffer,"F ",2) == 0 ) {
			sscanf(buffer,"F %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u",&utmp[0],&utmp[1],&utmp[2],&utmp[3],&utmp[4],&utmp[5],&utmp[6],&utmp[7],&utmp[8],&utmp[9],&utmp[10],&utmp[11]);
			preface = fi;
			ti = 0;
			for(i=0;i<(utmp[0]-2);i++) {
				face[fi].p[0] = utmp[1+i];
				face[fi].p[1] = utmp[2+i];
				face[fi].p[2] = utmp[3+i];
				
				if(face[fi].p[0] == face[fi].p[2]) CRASH("NOW");
				
				fi++;
			}
		} else if( strncmp(buffer,"B ",2) == 0 ) {
			sscanf(buffer,"B %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u",&utmp[0],&utmp[1],&utmp[2],&utmp[3],&utmp[4],&utmp[5],&utmp[6],&utmp[7],&utmp[8],&utmp[9],&utmp[10],&utmp[11]);
			preface = fi;
			ti = 0;
			for(i=0;i<(utmp[0]-2);i++) {
				face[fi].p[0] = utmp[1+i];
				face[fi].p[1] = utmp[2+i];
				face[fi].p[2] = utmp[3+i];

				if(face[fi].p[0] == face[fi].p[2]) printf("Dupe at line %u\n",line);

				fi++;
			}
		} else if( strncmp(buffer,"TEX ",4) == 0 ) {
			sscanf(buffer,"TEX %u, %u",&utmp[0],&utmp[1]);
		
			face[preface].uv[ti][0] = (double)utmp[0];
			face[preface].uv[ti][1] = (double)utmp[1];

			ti++;
			if( ti == 3 ) {
				face[preface+1].uv[0][0] = face[preface].uv[1][0];
				face[preface+1].uv[0][1] = face[preface].uv[1][1];
				face[preface+1].uv[1][0] = face[preface].uv[2][0];
				face[preface+1].uv[1][1] = face[preface].uv[2][1];
				
				ti = 2;
				preface++;
			}
		} else if( strncmp(buffer,"TEXDATA ",8) == 0 ) {
			void *rawtexture;
			FILE *outtex;
			
			sscanf(buffer,"TEXDATA %u",&utmp[0]);
			rawtexture = malloc(utmp[0]);
			if( rawtexture == NULL) CRASH("Couldnt allocate RAM for texture");
			fread(rawtexture,1,utmp[0],in);
			outtex = fopen("TEX0.bmp","wb");
			fwrite(rawtexture,1,utmp[0],outtex);
			fclose(outtex);
		}
	}
	
	if( argc < 3 ) {
		printf("Vertices: %u\n",vertices);
		printf("Faces   : %u\n",faces);
		
		printf("Face0\n");
		printf("P = %u - %u - %u\n",face[0].p[0],face[0].p[1],face[0].p[2]);
		printf("T = %f - %f\n",face[0].uv[0][0],face[0].uv[0][1]);
		printf("T = %f - %f\n",face[0].uv[1][0],face[0].uv[1][1]);
		printf("T = %f - %f\n",face[0].uv[2][0],face[0].uv[2][1]);
		printf("Face1\n");
		printf("P = %u - %u - %u\n",face[1].p[0],face[1].p[1],face[1].p[2]);
		printf("T = %f - %f\n",face[1].uv[0][0],face[1].uv[0][1]);
		printf("T = %f - %f\n",face[1].uv[1][0],face[1].uv[1][1]);
		printf("T = %f - %f\n",face[1].uv[2][0],face[1].uv[2][1]);
	} else {
		char mtlname[100];
		
		sprintf(mtlname,"%s.mtl",argv[2]);
		out = fopen(mtlname,"wt");
		fprintf(out,"newmtl defaultMaterial\n");
		fprintf(out,"illum 4\n");
		fprintf(out,"Kd 0.00 0.00 0.00\n");
		fprintf(out,"Ka 0.00 0.00 0.00\n");
		fprintf(out,"Tf 1.00 1.00 1.00\n");
		fprintf(out,"map_Kd TEX0.bmp\n");
		fprintf(out,"Ni 1.00\n");
		fclose(out);
		
		out = fopen(argv[2],"wt");


		fprintf(out,"mtllib %s.mtl\n",argv[2]);

		for(i=0;i<vertices;i++)
			fprintf(out,"v %f %f %f\n",vertex[i * 3 + 0],vertex[i * 3 + 1],vertex[i * 3 + 2]);
		for(i=0;i<faces;i++) {
			fprintf(out,"vt %f %f\n",face[i].uv[0][0],face[i].uv[0][1]);
			fprintf(out,"vt %f %f\n",face[i].uv[1][0],face[i].uv[1][1]);
			fprintf(out,"vt %f %f\n",face[i].uv[2][0],face[i].uv[2][1]);
		}
		for(i=0;i<vertices;i++)
			fprintf(out,"vn %f %f %f\n",0.1,0.1,0.1);

		fprintf(out,"usemtl defaultMaterial\n");
		for(i=0;i<faces;i++) {
			fprintf(out,"f %u/%u/%u %u/%u/%u %u/%u/%u\n",face[i].p[0],i*3+0+1,1,face[i].p[1],i*3+1+1,1,face[i].p[2],i*3+2+1,1); /* Normal fixed to 0 right now, pending normal calculations */
		}
		fclose(out);
	}
	fclose(in);
}
