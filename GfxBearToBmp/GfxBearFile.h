#pragma once


#include <windows.h>


class GfxBearFile
{
	private:
		enum Type
		{
			NONE,
			GFX,
			MAP
		};
		Type m_type;
		long m_fileSize;
		unsigned char* m_rawData;
		long m_plttFileSize;
		unsigned char* m_plttRawData;

	public:
		GfxBearFile();
		~GfxBearFile();
		bool Load(const wchar_t* typeStr, const wchar_t* fileName, const wchar_t* plttFileName);
		void PrintInfo();
		bool SaveBitmap(const wchar_t* fileName);

	private:
		bool Load(const wchar_t* fileName, long* fileSize, unsigned char** rawData);
		void CheckLoaded();
};
