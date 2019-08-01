#include <stdio.h>
#include <algorithm>
#include <windows.h>
#include <shlwapi.h>
#include "GdiplusUtil.h"
#include "GfxBearFile.h"


#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib,"shlwapi.lib")


static int g_major = 1;
static int g_minor = 0;


int wmain(int argc, wchar_t* argv[])
{
	try
	{
		printf("Gfx bear to bmp converter %d.%d\n", g_major, g_minor);
		printf("Written by Gabor Somogyi\n");
		printf("Compile date %s %s\n\n", __DATE__, __TIME__);

		if (argc < 3)
		{
			printf("Usage : GfxBearToBmp.exe [gfx|map] input.bear <pltt.bear>\n");
			return -1;
		}

		GdiplusUtil::Init();

		GfxBearFile gfxBearFile;
		wchar_t* plttFileName = NULL;
		if (argc > 3) plttFileName = argv[3];
		if (!gfxBearFile.Load(argv[1], argv[2], plttFileName))
		{
			return 1;
		}
		gfxBearFile.PrintInfo();
		
		wchar_t* fileName = _wcsdup(argv[2]);
		size_t fileNameLen = wcslen(fileName);
		PathRemoveExtension(fileName);
		wcscat_s(fileName, fileNameLen, L".bmp");
		gfxBearFile.SaveBitmap(fileName);
		free(fileName);

		GdiplusUtil::Shutdown();
	}
	catch(std::exception* e)
	{
		printf("Exception caught: %s\n", e->what());
	}

	return 0;
}
