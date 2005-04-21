#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>


snd_pcm_t *capture_handle;
snd_pcm_hw_params_t *hw_params;

int Audio_Initialize(int mod) 
{
	capture_handle = NULL;
	int i,err;
	short buf[128];
	char  alsa_device[80];
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	unsigned int rate = 44100;

	
	sprintf(alsa_device,"plughw:0,0");
	
	//try to open up 
	if ((err = snd_pcm_open (&capture_handle, alsa_device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
				 argv[1],
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
				
	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	snd_pcm_hw_params_free(hw_params);
	
	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
				
	for (i = 0; i < 10; ++i) {
		if ((err = snd_pcm_readi (capture_handle, buf, 128)) != 128) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
					 snd_strerror (err));
			exit (1);
		}
	}
				
	snd_pcm_close (capture_handle);
	exit (0);
}

float Audio_GetSamplingRate(void) 
{
  return 0.0f;
}

void Audio_SendBuffer(void) 
{
}

void Audio_Release(void) 
{
  if (capture_handle == NULL)
    snd_pcm_close(capture_handle);
}

#if 0
#define SYSLOG_FATAL      10
#define SYSLOG_SERIOUS     9
#define SYSLOG_UNSUPPORTED 8
#define SYSLOG_WARNING     7
#define SYSLOG_INFO        5
#define SYSLOG_DEBUG       3
#define SYSLOG_TRACE       1

void logger(unsigned char level, char *subsys, char *message, ...) {
	va_list arglist;
	FILE *syslog_fd = stdout;
	unsigned char logging_level = 0;
	
	if( level >= logging_level ) {
		va_start(arglist,message);
		fprintf(syslog_fd,"[%5s] ",subsys);
		vfprintf(syslog_fd,message,arglist);
		fprintf(syslog_fd,"\n");
		va_end(arglist);
	}
	
	if(level >= SYSLOG_FATAL) {
		exit(-1);
	}
}


int main(int argc,char **argv) {
	int i,err;
	short buf[128];
	char  alsa_device[80];
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	unsigned int rate = 44100;

	
	sprintf(alsa_device,"plughw:0,0");
	
	if ((err = snd_pcm_open (&capture_handle, alsa_device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
				 argv[1],
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
				
	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
	
	snd_pcm_hw_params_free(hw_params);
	
	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
				 snd_strerror (err));
		exit (1);
	}
				
	for (i = 0; i < 10; ++i) {
		if ((err = snd_pcm_readi (capture_handle, buf, 128)) != 128) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
					 snd_strerror (err));
			exit (1);
		}
	}
				
	snd_pcm_close (capture_handle);
	exit (0);
}
#endif
