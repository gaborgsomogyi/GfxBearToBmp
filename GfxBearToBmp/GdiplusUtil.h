#pragma once


#include <windows.h>
#include <gdiplus.h>


class GdiplusUtil
{
	private:
		static ULONG_PTR token;

	public:
		static void Init();
		static void Shutdown();
		static void SaveBmp(Gdiplus::Bitmap* pBitmap, const wchar_t* fileName);

	private:
		static int GetEncoderClsid(const wchar_t* format, CLSID* pClsid);
};
