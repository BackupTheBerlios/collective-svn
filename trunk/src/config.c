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
 * $RCFS: config.c,v 1.1.1.1 2005/03/17 17:41:54 jaja Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>

/*
 * these files do not exist on windows
 */
#ifndef WIN32
#include <syslog.h>
#include <unistd.h>
#endif

/*
 * On windows, snprintf is called _snprintf.
 * Also, _getwcd is in direct.h
 */
#ifdef WIN32
#include <direct.h>
#define snprintf _snprintf
#define getcwd   _getcwd
#endif

#include "rcfs.h"
#include "logger.h"

/*
 * Originally from FPREM. As I am the original author, I'm relicensing it to BSD.
 *    James Jacobsson 5/4-2005
 */

char *conf;

char logbuff[100];

void Config_read(char *str) {
	FILE *in;
	int   sz;
	char  tmpstr[100];
	char *foo;
	
	in = fopen(str,"rb");
	if(in == NULL) {
		printf("Unable to open config-file \"%s\"\n",str);
		exit(-1);
	}  
	fseek(in,0,SEEK_END);
	sz = ftell(in);
	fseek(in,0,SEEK_SET);
	
	conf = (char *)malloc(sz);
	assert(conf != NULL);
	
	/*
	 * Copy a single character into the conf string, otherwise we get memory corruption on windows
	 */
	strcpy(conf, "");
	
	while( !feof(in) ) {
		foo = fgets(tmpstr,100,in);
		
		if( ferror( in ) )
		{
			perror( "Read error" );
			break;
		}
		
		if (!foo) continue;
		
		
		if( (tmpstr[0] >= 'a') && (tmpstr[0] <= 'z') ) {
			strcat(conf,tmpstr);
		}
	}
	
	fclose(in);
}

int Config_getint(char *key) {
	char *nstr;
	int ret;
	
	nstr = strstr(conf,key);
	snprintf(logbuff,100,"Unable to find key \"%s\"\n",key);
	if(nstr == NULL) logger(SYSLOG_FATAL,"CONF",logbuff);
	
	while(*nstr++ != '=');
	while(*nstr == ' ') nstr++;
	
	ret = atoi(nstr);
	
	return(ret);
}

int Config_getbool(char *key) {
	char *nstr;
	
	nstr = strstr(conf,key);
	snprintf(logbuff,100,"Unable to find key \"%s\"\n",key);
	if(nstr == NULL) logger(SYSLOG_FATAL,"CONF",logbuff);
	
	while(*nstr++ != '=');
	while(*nstr == ' ') nstr++;
	
	if( toupper(nstr[0]) == 'N') return(0);
	if( toupper(nstr[0]) == 'F') return(0);
	if(		  nstr[0]  == '0') return(0);
	
	return(1);
}

int Config_getstring(char *key,char *dest) {
	char *nstr;
	
	nstr = strstr(conf,key);
	snprintf(logbuff,100,"Unable to find key \"%s\"\n",key);
	if(nstr == NULL) logger(SYSLOG_FATAL,"CONF",logbuff);
	
	while(*nstr++ != '=');
	while(*nstr == ' ') nstr++;
	
	while((*nstr != '\r') && (*nstr != '\n')) {
		*dest++ = *nstr++;
	}
	*dest = 0;
	
	return(0);
}

