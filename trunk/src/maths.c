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
 
 #include <math.h>
 
 #include "collective.h"
 
 void Maths_IdentityMatrix(float *m) {
	m[ 0] = 1.0f; m[ 1] = 0.0f; m[ 2] = 0.0f; m[ 3] = 0.0f;
	m[ 4] = 0.0f; m[ 5] = 1.0f; m[ 6] = 0.0f; m[ 7] = 0.0f;
	m[ 8] = 0.0f; m[ 9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
	m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
 }
 
void Maths_EulerRotation(float *m,float x,float y,float z) {
	static float A,B,C,D,E,F,AD,BD;

	Maths_IdentityMatrix(m);
	
	A     = cos(x);
    B     = sin(x);
    C     = cos(y);
    D     = sin(y);
    E     = cos(z);
    F     = sin(z);
    AD    =   A * D;
    BD    =   B * D;
    m[0]  =   C * E;
    m[1]  =  -C * F;
    m[2]  =   D;
    m[4]  =  BD * E + A * F;
    m[5]  = -BD * F + A * E;
    m[6]  =  -B * C;
    m[8]  = -AD * E + B * F;
    m[9]  =  AD * F + B * E;
    m[10] =   A * C;
    m[3]  =  m[7] = m[11] = m[12] = m[13] = m[14] = 0;
    m[15] =  1;
 }
 
 void Maths_MatrixMultiply(float *d,float *a) {
	static float td[16];
    static int i, j;             // row and column counters

    for (j = 0; j < 4; j++)         // transform by columns first
        for (i = 0; i < 4; i++)     // then by rows
            td[i*4+j] = a[i*4+0] * d[0*4+j] + a[i*4+1] * d[1*4+j] +
                        a[i*4+2] * d[2*4+j] + a[i*4+3] * d[3*4+j];

    for (i = 0; i < 16; i++) {         // copy temporary matrix into "d"
		d[i] = td[i];
	}
}

void Maths_Translation(float *m,float x,float y,float z) {
	Maths_IdentityMatrix(m);
	
	m[12] = x;
	m[13] = y;
	m[14] = z;
}

void Maths_VectorMatrixMultiply(float *d,float *v,float *m) {
  d[0] = m[0] * v[0] + m[4] * v[1] + m[ 8] * v[2] + m[12] * v[3];
  d[1] = m[1] * v[0] + m[5] * v[1] + m[ 9] * v[2] + m[13] * v[3];
  d[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
  d[3] = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];

}

void Maths_VectorAdd(float *d,float *v) {
	d[0] = d[0] + v[0];
	d[1] = d[1] + v[1];
	d[2] = d[2] + v[2];
	d[3] = d[3] + v[3];
	d[3] = 1.0f;
}

void Maths_VectorZero(float *v) {
	v[0] = v[1] = v[2] = 0.0f;
	v[3] = 1.0f;
}

void Maths_VectorCopy(float *d,float *s) {
	d[0] = s[0];
	d[1] = s[1];
	d[2] = s[2];
	d[3] = s[3];
}

void Maths_VectorScale(float *v,float scale) {
	v[0] = v[0] * scale;
	v[1] = v[1] * scale;
	v[2] = v[2] * scale;
}

void Maths_MatrixAdd(float *d,float *a) {
	static int i;
	
	for(i=0;i<16;i++)
		d[i] += a[i];
	
}
