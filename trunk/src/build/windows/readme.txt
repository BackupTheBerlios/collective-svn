Compiling rcfs on Windows XP with Visual Studio.NET 2003

You need to download GLUT, it can be found at http://www.xmission.com/~nate/glut.html
Extract all the files to a subdirectory called GLUT in the rcfs source directory.

1. Solving the C2381 error:
You need to modify the GLUT header files to get rid of this. It is caused by an incompatilibity with the windows.h file in VS.NET

Find this bit of code:
#if defined(_WIN32)
# ifndef GLUT_BUILDING_LIB
extern _CRTIMP void __cdecl exit(int);
# endif

Replace it with this:
#if defined(_WIN32) 
# ifndef GLUT_BUILDING_LIB /*extern _CRTIMP void __cdecl exit(int);*/ 
_CRTIMP __declspec(noreturn) void __cdecl exit(int); 
# endif

2. Modify Model.c to load your model file and scenery (you will probably have to put in the full path for now)
3. Modify main.c to load the config file (Again, provide the full path here)

Once we modify it so that you can provide the 2 settings above as parameters, it should not be neccesary anymore.

Please note that I have not actually gotten rcfs to be usable on windows yet, since there is no input code, but this will at least allow the windows coders to get started.
