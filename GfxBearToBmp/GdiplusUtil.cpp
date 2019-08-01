#include "GDIPlusUtil.h"
#include <stdio.h>


ULONG_PTR GdiplusUtil::token = 0;

void GdiplusUtil::Init()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput = { 0 };
	Gdiplus::GdiplusStartup(&token, &gdiplusStartupInput, NULL);
}

void GdiplusUtil::Shutdown()
{
	Gdiplus::GdiplusShutdown(token);
}

void GdiplusUtil::SaveBmp(Gdiplus::Bitmap* pBitmap, const wchar_t* fileName)
{
	printf("Saving to output file : %ws...\n", fileName);
	CLSID bmpClsid = { 0 };
	GetEncoderClsid(L"image/bmp", &bmpClsid);
	Gdiplus::Status st = pBitmap->Save(fileName, &bmpClsid, NULL);
	if (st != Gdiplus::Status::Ok)
	{
		printf("ERROR: %d\n", st);
	}
	else
	{
		printf("OK\n");
	}
}

int GdiplusUtil::GetEncoderClsid(const wchar_t* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}
