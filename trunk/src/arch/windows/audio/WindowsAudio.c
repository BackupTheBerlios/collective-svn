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
* $RCFS: main.c,v 1.1.1.1 2005/03/17 17:41:54 jaja Exp $
*
*/

#include <stdio.h>
#include <signal.h>

#include <dsound.h>

#include "rcfs.h"
#include "logger.h"
#include "audio.h"

static int    audio_mode = 0;

static double        samprate;
static int16        *buffer;

/*Format for our audio in*/
static WAVEFORMATEX waveFormat;
/*Handle for wavein*/
static HWAVEIN	hWaveIn;
/*Two headers for double buffering*/
static WAVEHDR	hWaveHdr[2];
static BOOL bRecord;
static double minVal = 0;
static double maxVal = 0;
static int highCount = 0;
static int lowCount = 0;

#define BUFFERSIZE 2048

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
/************************************************************************/
/* This needs to be replaced with the DirectSound code, but for now     */
/* we'll just use the straight windows audio api, since that is easier  */
/* to get going and I need more time to resolve the DirectSound stuff   */
/************************************************************************/
int Audio_Initialize(int mode) 
{
	MMRESULT mResult;

	audio_mode = mode;
	buffer = (uint16 *) calloc(BUFFERSIZE, sizeof(uint16));

	ZeroMemory(&waveFormat, sizeof(waveFormat));
	//44.1 kHz, mono, 8-bit is our preferred format
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 1;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 8;
	waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample/8);
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	//Clear out the wave headers
	ZeroMemory(&hWaveHdr[0], sizeof(WAVEHDR) * 2);

	mResult = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, (DWORD)waveInProc, 0, CALLBACK_FUNCTION);
	if (mResult)
	{
		printf("Error open default audio in device\n");
		return 0;
	}

	hWaveHdr[0].dwBufferLength = BUFFERSIZE;//waveFormat.nAvgBytesPerSec << 1;
	hWaveHdr[1].dwBufferLength = BUFFERSIZE; //waveFormat.nAvgBytesPerSec << 1;
	
	//Allocate enough memory for both buffers
	if (!(hWaveHdr[0].lpData = (char *)VirtualAlloc(0, hWaveHdr[0].dwBufferLength * 2, MEM_COMMIT, PAGE_READWRITE)))
	{
		printf("Unable to allocate memory for buffers");
		//TODO Add some cleanup code
		return 0;
	}

	//Set the buffer starting addresses
	hWaveHdr[1].lpData = hWaveHdr[0].lpData + hWaveHdr[0].dwBufferLength;

	//Prepare the headers
	if (mResult = waveInPrepareHeader(hWaveIn, &hWaveHdr[0], sizeof(WAVEHDR)))
	{
		printf("Unable to prepare header 0: %08X\n", mResult);
		//cleanup
		return 0;
	}
	if ((mResult = waveInPrepareHeader(hWaveIn, &hWaveHdr[1], sizeof(WAVEHDR))))
	{
		printf("Unable to prepare header 1: %08X\n", mResult);
		//cleanup
		return 0;
	}

	//Queue the headers
	if (mResult = waveInAddBuffer(hWaveIn, &hWaveHdr[0], sizeof(WAVEHDR)))
	{
		printf("Unable to queue header 0: %08X\n", mResult);
		//cleanup
		return 0;
	}
	if (mResult = waveInAddBuffer(hWaveIn, &hWaveHdr[1], sizeof(WAVEHDR)))
	{
		printf("Unable to queue header 1: %08X\n", mResult);
		//cleanup
		return 0;
	}

	//Now start the actual input
	bRecord = TRUE;
	waveInStart(hWaveIn);

	/*
	 * Just a stub for now. This must still be implemented
	 */
	return(0);
}

float Audio_GetSamplingRate(void) {
	/*
	* Just a stub for now. This must still be implemented
	*/
	return (float) waveFormat.nSamplesPerSec;
}

int16 *Audio_GetBuffer(uint32 samples) {
	/*
	* Just a stub for now. This must still be implemented
	*/
	return(buffer);
}

void Audio_SendBuffer(void) {
	/*
	* Just a stub for now. This must still be implemented
	*/
}

void Audio_Release(void)
{
	MMRESULT mResult;
	
	bRecord = FALSE;

	//Stop recording
	waveInStop(hWaveIn);

	//Unprepare the wave headers
	if (mResult = waveInUnprepareHeader(hWaveIn, &hWaveHdr[0], sizeof(WAVEHDR)))
	{
		printf("Unable to unprepare header 0: %08X\n", mResult);
		//cleanup
	}
	if (mResult = waveInUnprepareHeader(hWaveIn, &hWaveHdr[1], sizeof(WAVEHDR)))
	{
		printf("Unable to unprepare header 1: %08X\n", mResult);
		//cleanup
	}

	//free the memory for the buffers
	if (hWaveHdr[0].lpData) VirtualFree(hWaveHdr[0].lpData, 0, MEM_RELEASE);
	
	waveInClose(hWaveIn);
	free(buffer);
}

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, WAVEHDR* inBuffer, DWORD dwParam2)
{
	int iLoop = 0;
	uint16 prevSample = 0;
	uint16 sample = 0;
	uint16 newprevSample = 0;
	uint16 newsample = 0;
	unsigned char dataVal = 0;
	double threshold;
	char audioBuf[500];

	if (uMsg == WIM_DATA)
	{

		int iSize = waveFormat.nBlockAlign;
		int iLength = inBuffer->dwBytesRecorded / iSize;

//		FILE* audio = fopen("audio.dat", "wt");
//		fputs("dataVal,sample,threshold,newsample\n", audio);

		for (iLoop = 0; iLoop < iLength; iLoop++)
		{
			if (!bRecord)
				return;

			dataVal = (unsigned char) inBuffer->lpData[iLoop];
			
			if(dataVal < 100) newsample = 1;
			else if(dataVal > 150) newsample = 0;
			else newsample = newprevSample;
			newprevSample = newsample;

			buffer[iLoop] = newsample;

/*
			maxVal -= 0.1;
			minVal += 0.1;
			if (maxVal < minVal) 
				maxVal = minVal + 1;

			if (dataVal> maxVal) 
				maxVal = dataVal;
			else if (dataVal < minVal) 
				minVal = dataVal;
			threshold = (minVal + maxVal) / 2;

			if (dataVal > threshold) 
			{
				highCount++;
				if (lowCount) 
				{
					lowCount = 0;
					sample = 0;
				}
			} 
			else 
			{
				lowCount++;
				if (highCount) 
				{
					highCount = 0;
					sample = 1;
				} 
			}
			sprintf(audioBuf, "%d,%d,%e,%d\n", dataVal,sample,threshold,newsample);
			fputs(audioBuf, audio);

			buffer[iLoop] = sample;

*/
		}
//		fflush(audio);
//		fclose(audio);
		inputdev.AudioDecode(buffer,iLength);

		if (bRecord) 
			waveInAddBuffer(hWaveIn, &hWaveHdr[inBuffer->dwUser], sizeof(WAVEHDR));
	}
}

// Since DirectSound does not work with callbacks for audio processing, use this to simulate it
void Audio_Poll(void)
{
}

