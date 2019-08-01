#include "GfxBearFile.h"
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include "GdiplusUtil.h"


#define MAGIC "BEAR3zzzz"

#pragma pack(1)
typedef struct
{
	unsigned char magic[9];
	unsigned char c;
	unsigned char t;
	unsigned short width;
	unsigned short height;
	unsigned char maxColorID;
} BearHader;

class GfxBearFilePalette
{
	private:
		enum Type
		{
			VECTOR,
			MAP
		};
		Type m_type;
		std::vector<Gdiplus::Color> m_vectorPalette;
		std::unordered_map<int, Gdiplus::Color> m_mapPalette;

	public:

		GfxBearFilePalette(unsigned char* colorPtr, unsigned char maxColorID):
		m_type(VECTOR),
		m_vectorPalette(),
		m_mapPalette()
		{
			for (int i = 0; i <= maxColorID; ++i)
			{
				m_vectorPalette.push_back(Gdiplus::Color(colorPtr[0], colorPtr[1], colorPtr[2]));
				colorPtr += 3;
			}
		}

		GfxBearFilePalette():
		m_type(MAP),
		m_vectorPalette(),
		m_mapPalette()
		{
			m_mapPalette[0] = Gdiplus::Color::Black;
			m_mapPalette[1] = Gdiplus::Color::Red;
			m_mapPalette[0x40] = Gdiplus::Color::Maroon;
			m_mapPalette[0x80] = Gdiplus::Color::Maroon;
			m_mapPalette[0x81] = Gdiplus::Color::Yellow;
			m_mapPalette[0x83] = Gdiplus::Color::Lime;
			m_mapPalette[0x84] = Gdiplus::Color::Cyan;
		}

		int GetSizeInFile()
		{
			switch (m_type)
			{
			case VECTOR:
				return (int)m_vectorPalette.size() * 3;
				break;
			case MAP:
				return 0;
				break;
			default:
				throw new std::exception("Invalid palette type");
				break;
			}
		}

		const Gdiplus::Color& at(int index)
		{
			switch (m_type)
			{
			case VECTOR:
				return m_vectorPalette.at(index);
				break;
			case MAP:
				return m_mapPalette.at(index);
				break;
			default:
				throw new std::exception("Invalid palette type");
				break;
			}
		}
};

GfxBearFile::GfxBearFile():
m_type(NONE),
m_fileSize(0),
m_rawData(NULL),
m_plttFileSize(0),
m_plttRawData(NULL)
{
}

 GfxBearFile::~GfxBearFile()
{
	if (m_rawData)
	{
		delete[] m_rawData;
		m_rawData = NULL;
	}
}

bool GfxBearFile::Load(const wchar_t* typeStr, const wchar_t* fileName, const wchar_t* plttFileName)
{
	if (!wcscmp(typeStr, L"gfx"))
	{
		m_type = GFX;
	}
	else if (!wcscmp(typeStr, L"map"))
	{
		m_type = MAP;
	}
	else
	{
		throw new std::exception("Invalid conversion type");
	}

	Load(fileName, &m_fileSize, &m_rawData);
	if (plttFileName)
		Load(plttFileName, &m_plttFileSize, &m_plttRawData);

	return true;
}

bool GfxBearFile::Load(const wchar_t* fileName, long* fileSize, unsigned char** rawData)
{
	printf("Opening input file : %ws...\n", fileName);
	FILE* inFile = NULL;
	_wfopen_s(&inFile, fileName, L"rb");
	if (!inFile)
	{
		printf("Failed to open file for read.\n");
		return false;
	}
	printf("OK\n");

	printf("Getting file size...\n");
	fseek(inFile, 0, SEEK_END);
	*fileSize = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);
	printf("OK: %d\n", *fileSize);

	printf("Reading content...\n");
	*rawData = new unsigned char[*fileSize];
	if (!*rawData)
	{
		printf("Unable to allocate space for file content\n");
		fclose(inFile);
		return false;
	}
	memset(*rawData, 0xDEADCAFE, *fileSize);
	if (!fread(*rawData, 1, *fileSize, inFile))
	{
		printf("Unable to read file content\n");
		fclose(inFile);
		return false;
	}
	printf("OK\n");

	fclose(inFile);

	int magicSize = sizeof(MAGIC) - 1;
	if (*fileSize < magicSize || memcmp(*rawData, MAGIC, magicSize))
	{
		*fileSize = 0;
		delete[] * rawData;
		*rawData = NULL;
		throw new std::exception("Corrupt bear file, invalid magic");
	}

	return true;
}

void GfxBearFile::CheckLoaded()
{
	if (!m_rawData)
	{
		throw new std::exception("No file loaded");
	}
}

void GfxBearFile::PrintInfo()
{
	CheckLoaded();

	BearHader* pBearHader = (BearHader*)m_rawData;
	printf("BearHader::c: %d\n", pBearHader->c);
	printf("BearHader::t: %d\n", pBearHader->t);
	printf("BearHader::width: %d\n", pBearHader->width);
	printf("BearHader::height: %d\n", pBearHader->height);
	printf("BearHader::maxColorID: %d\n", pBearHader->maxColorID);
}

bool GfxBearFile::SaveBitmap(const wchar_t* fileName)
{
	CheckLoaded();

	BearHader* pBearHader = (BearHader*)m_rawData;
	//Gdiplus::Bitmap* pOrigBitmap = new Gdiplus::Bitmap(L"edge.bmp");
	Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(pBearHader->width, pBearHader->height, PixelFormat24bppRGB);

	printf("Converting data...\n");
	GfxBearFilePalette* palette = NULL;
	switch (m_type)
	{
	case GFX:
		switch (pBearHader->c)
		{
		case 1:
			palette = new GfxBearFilePalette(m_rawData + sizeof(BearHader), pBearHader->maxColorID);
			break;
		case 2:
			palette = new GfxBearFilePalette(m_plttRawData + sizeof(BearHader), pBearHader->maxColorID);
			break;
		default:
			throw new std::exception("Invalid palette");
			break;
		}
		break;
	case MAP:
		palette = new GfxBearFilePalette();
		break;
	default:
		throw new std::exception("Invalid conversion type");
		break;
	}

	unsigned char* pixelPtr = m_rawData + sizeof(BearHader);
	if (pBearHader->c == 1) pixelPtr += palette->GetSizeInFile();
	unsigned int x = 0;
	unsigned int y = 0;
	while (pixelPtr < m_rawData + m_fileSize)
	{
		unsigned char colorIndex = *pixelPtr++;
		Gdiplus::Color color = palette->at(colorIndex);

		unsigned int colorCount = 1;
		bool needUpdate = true;
		bool isEnd = pixelPtr >= m_rawData + m_fileSize;
		while (needUpdate && !isEnd)
		{
			if (*pixelPtr > pBearHader->maxColorID)
			{
				colorCount += *pixelPtr++ - pBearHader->maxColorID;
				needUpdate = true;
			}
			else
			{
				needUpdate = false;
			}
			isEnd = pixelPtr >= m_rawData + m_fileSize;
		}
		//printf("Position in file: %x\n", sizeof(BearHader) + (pBearHader->maxColorID + 1) * 3 + pixelPtr - colorPtr);
		//printf("colorIndex: %d\n", colorIndex);
		//printf("colorCount: %d\n", colorCount);
		for (unsigned int i = 0; i < colorCount; ++i)
		{
			/*Gdiplus::Color origColor;
			Gdiplus::Status st1 = pOrigBitmap->GetPixel(x, y, &origColor);
			if (st1 != Gdiplus::Status::Ok)
			{
				printf("Unable to get pixel at (%d,%d). Error Code: %d\n", x, y, st1);
				exit(1);
			}
			if (origColor.GetValue() != color.GetValue())
			{
				printf("Position in file: %x\n", sizeof(BearHader) + (pBearHader->maxColorID + 1) * 3 + pixelPtr - colorPtr);
				printf("colorCount: %d\n", colorCount);
				printf("pos: (%d,%d)\n", x, y);
				printf("origColor: (%x,%x,%x)\n", origColor.GetRed(), origColor.GetGreen(), origColor.GetBlue());
				printf("color[%x]: (%x,%x,%x)\n", colorIndex, color.GetRed(), color.GetGreen(), color.GetBlue());
				exit(1);
			}*/

			//printf("pos: (%d,%d)\n", x, y);
			Gdiplus::Status st2 = pBitmap->SetPixel(x, y, color);
			if (st2 != Gdiplus::Status::Ok)
			{
				printf("Unable to set pixel at (%d,%d). Error Code: %d\n", x, y, st2);
				return false;
			}
			if (++x >= pBearHader->width)
			{
				x = 0;
				++y;
			}
		}
	}
	printf("OK\n");

	GdiplusUtil::SaveBmp(pBitmap, fileName);

	delete palette;
	palette = NULL;
	//delete pOrigBitmap;
	//pOrigBitmap = NULL;
	delete pBitmap;
	pBitmap = NULL;

	return true;
}
