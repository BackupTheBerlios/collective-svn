/************************************************************************/
/* This code is MESSY. It needs to be cleaned up, but I will get around */
/* to that once I have a better understanding of how it really works.   */
/* As it currently stands, it is just a quick hack to get my joystick   */
/* to work, since I am TIRED of crashing into the ground constantly     */
/************************************************************************/

#include "joystick.h"
#include "rcfs.h"

#define STRICT
#define DIRECTINPUT_VERSION 0x0800
#define CINTERFACE

#define MAX_CHANNELS 16

#include <dinput.h>
#include <stdio.h>

static LPDIRECTINPUT8       g_pDI              = NULL;         
static LPDIRECTINPUTDEVICE8 g_pJoystick        = NULL;     

static HINSTANCE hInstDI;

static float channel[MAX_CHANNELS];

BOOL CALLBACK    EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );

BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
								  VOID* pContext )
{
	//Set the range of the returned axes
	// For axes that are returned, set the DIPROP_RANGE property for the
	// enumerated axis in order to scale min/max values.
	if( pdidoi->dwType & DIDFT_AXIS )
	{
		DIPROPRANGE diprg; 
		diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		diprg.diph.dwHow        = DIPH_BYID; 
		diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
		diprg.lMin              = -1000; 
		diprg.lMax              = +1000; 

		// Set the range for the axis
		if( FAILED(IDirectInputDevice_SetProperty(g_pJoystick, DIPROP_RANGE, &diprg.diph ) ) ) 
		{
			return DIENUM_STOP;
		}

	}
	return DIENUM_CONTINUE;
}

int Joystick_Initialize(void)
{
	char versionInfo[15];

	HRESULT hr;
	HWND hWnd = NULL;

	sprintf(versionInfo,"RCFS v%s",RCFS_VERSION);

	hWnd = FindWindow("GLUT", versionInfo);

	if( FAILED( hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
		&IID_IDirectInput8A, (VOID**)&g_pDI, NULL )))
		return hr;

	//Get the first joystick. This should be expanded to get more
	hr = IDirectInput8_CreateDevice(g_pDI, &GUID_Joystick, &g_pJoystick, NULL);

	if (g_pJoystick == NULL)
	{
		printf("Unable to open joystick");
		return 0;
	}

	//tell the joystick which format we want
	if (FAILED(hr = IDirectInputDevice8_SetDataFormat(g_pJoystick, &c_dfDIJoystick2)))
	{
		printf("Unable to set joystick data format");
		return 0;
	}
	//set the cooperative level
	if(FAILED(hr = IDirectInputDevice8_SetCooperativeLevel(g_pJoystick, hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND)))
	{
		printf("Unable to set joystick cooperation level");
		return 0;
	}

	//Enumerate the joystick objects so that we can set the joystick range
	if(FAILED(hr = IDirectInputDevice8_EnumObjects(g_pJoystick, EnumObjectsCallback, (VOID*)hWnd, DIDFT_ALL)))
	{
		printf("Unable to enumerate the joystick objects");
		return 0;
	}

	return 0;
}

float *Joystick_Poll(void)
{

	HRESULT hr;
	DIJOYSTATE2 js; //joystick state

	if (g_pJoystick == NULL)
		return(channel);

	hr = IDirectInputDevice8_Poll(g_pJoystick);
	if (FAILED(hr))
	{
		//Try to reacquire the joystick
		hr = IDirectInputDevice8_Acquire(g_pJoystick);
		//Keep on trying untill we can acquire the joystick again
		//UGLY UGLY HACK. This should be replaced
		while (hr == DIERR_INPUTLOST)
			hr = IDirectInputDevice8_Acquire(g_pJoystick);
	}

	//Now get the state
	hr = IDirectInputDevice8_GetDeviceState(g_pJoystick, sizeof(DIJOYSTATE2), &js);
	if (FAILED(hr))
	{
		if(hr == DIERR_OTHERAPPHASPRIO)
			printf("OW");
		return(channel);
	}

	//Need to make this configurable sometime
	//On my joystick, controls are as follows:
	// Elevator : js.lY
	// Aileron : js.lX
	// Throttle : js.rglSlider[0]
	// Rudder : js.lRz
	// 3 Buttons : 00, 01, 02
	//Now for some trickery with the values
	//Values should be between -1 and 1. Should be easy, since we set the joystick range to +-1000
	
	channel[MIXER_AILERON] = (float) js.lX / (float) 1000;
	channel[MIXER_ELEVATOR] = (float) js.lY / (float) 1000;
	channel[MIXER_THROTTLE] = (float) js.rglSlider[0] / (float) 1000;
	channel[MIXER_RUDDER] = (float) js.lRz / (float) 1000;

	//We are going to assume a linear pitch curve for now. Will need to add a way to define the throttle curve later
	channel[MIXER_COLLECTIVE] = channel[MIXER_THROTTLE];

	return(channel);
}

void Joystick_Release(void)
{
	// Unacquire the device one last time just in case 
	// the app tried to exit while the device is still acquired.
	if( g_pJoystick ) 
		IDirectInputDevice8_Unacquire(g_pJoystick);


	// Release any DirectInput objects.
	IDirectInputDevice8_Release( g_pJoystick );
	IDirectInputDevice8_Release( g_pDI );
}

float *Joystick_GetChannels(void)
{
	return Joystick_Poll();
}

int Joystick_GetNumChannels(void) {
	return(MAX_CHANNELS);
}

int Joystick_Audio_Initialize(int mode) {

	/*
	* Just a stub for now. This must still be implemented
	*/
	return(0);
}

float Joystick_GetSamplingRate(void) {
	return(0);
}
