#include "PngProc.h"
#include <string.h>
#include <stdio.h>
#include <cmath>

void ImageMirroring(unsigned char* pOut
	, unsigned char* pIn
	, size_t nWidth
	, size_t nHeight);

void ReverseMirroring(unsigned char* pOut
	, unsigned char* pIn
	, size_t nWidth
	, size_t nHeight);

void Blending(unsigned char* pOut
	, unsigned char* pIn0
	, unsigned char* pIn1
	, unsigned char* pIn2
	, size_t nWidth
	, size_t nHeight);


void Transpose(unsigned char* pOut
	, unsigned char* pIn
	, size_t nWidth
	, size_t nHeight);
int main(int argc, char* argv[])
{
	class CBitsPtrGuard
	{
	public:
		CBitsPtrGuard(unsigned char** pB) : m_ppBits(pB) { }
		~CBitsPtrGuard() { if (*m_ppBits) delete *m_ppBits, *m_ppBits = 0; }
	protected:
		unsigned char** m_ppBits;
	};

	// parse input parameters
	char	szInputFileName0[256] = "baboon.png";
	char	szInputFileName1[256] = "barbara.png";
	char	szInputFileName2[256] = "Goldhill.png";
	char    szOutputFileName[256] = "out.png";
	/*if (argc < 2)
		printf("\nformat: pngtest <input_file> [<output_file>]");
	else 
	{
		strcpy(szInputFileName, argv[1]);
		if (argc > 2)
			strcpy(szOutputFileName, argv[2]);
		else
		{
			strcpy(szOutputFileName, szInputFileName);
			strcat(szOutputFileName, "_out.png");
		}
	}
	*/

	size_t nReqSize0 = NPngProc::readPngFile(szInputFileName0, 0, 0, 0, 0);
	if (nReqSize0 == NPngProc::PNG_ERROR)
	{
		printf("\nError ocured while pngfile was read");
		return -1;
	}
	size_t nReqSize1 = NPngProc::readPngFile(szInputFileName1, 0, 0, 0, 0);
	if (nReqSize1 == NPngProc::PNG_ERROR)
	{
		printf("\nError ocured while pngfile was read");
		return -1;
	}
	size_t nReqSize2 = NPngProc::readPngFile(szInputFileName2, 0, 0, 0, 0);
	if (nReqSize2 == NPngProc::PNG_ERROR)
	{
		printf("\nError ocured while pngfile was read");
		return -1;
	}
	

	unsigned char* pInputBits0 = new unsigned char[nReqSize0];
	if (!pInputBits0)
	{
		printf("\nCan't allocate memory for image, required size is %u", nReqSize0);
		return -1;
	}
	unsigned char* pInputBits1 = new unsigned char[nReqSize1];
	if (!pInputBits1)
	{
		printf("\nCan't allocate memory for image, required size is %u", nReqSize1);
		return -1;
	}
	unsigned char* pInputBits2 = new unsigned char[nReqSize2];
	if (!pInputBits2)
	{
		printf("\nCan't allocate memory for image, required size is %u", nReqSize2);
		return -1;
	}
	CBitsPtrGuard InputBitsPtrGuard0(&pInputBits0);
	CBitsPtrGuard InputBitsPtrGuard1(&pInputBits1);
	CBitsPtrGuard InputBitsPtrGuard2(&pInputBits2);



	unsigned char* pOutputBits = new unsigned char[nReqSize0];
	if (!pOutputBits)
	{
		printf("\nCan't allocate memory for image, required size is %u", nReqSize0);
		return -1;
	}


	CBitsPtrGuard OutputBitsPtrGuard(&pOutputBits);

	size_t nWidth, nHeight;
	unsigned int nBPP;

	size_t nRetSize0 = NPngProc::readPngFileGray(szInputFileName0, pInputBits0, &nWidth, &nHeight/*, &nBPP*/);
	size_t nRetSize1 = NPngProc::readPngFileGray(szInputFileName1, pInputBits1, &nWidth, &nHeight/*, &nBPP*/);
	size_t nRetSize2 = NPngProc::readPngFileGray(szInputFileName2, pInputBits2, &nWidth, &nHeight/*, &nBPP*/);
	nBPP = 8;



	//Блендинг с нормальным режимом смешивания
	Blending(pOutputBits, pInputBits0, pInputBits1, pInputBits2, nWidth, nHeight);
	unsigned char* Buffer = new unsigned char[nReqSize0];
	memcpy(Buffer, pOutputBits, nReqSize0);
	//Зеркальное отражение пикселов изображения относительно вертикальной оси
	ImageMirroring(pOutputBits, Buffer ,nWidth, nHeight);
	//Зеркальное отражение пикселов изображения относительно горизонтальной оси
	ReverseMirroring(pOutputBits, Buffer, nWidth, nHeight);
	//Транспонирование изображения
	Transpose(pOutputBits, Buffer, nWidth, nHeight);

	if (NPngProc::writePngFile(szOutputFileName, pOutputBits, nWidth, nHeight, nBPP) == NPngProc::PNG_ERROR)
	{
		printf("\nError ocuured during png file was written");
		return -1;
	}
	delete[] Buffer;
	return 0;
}

void Blending(unsigned char* pOut
	, unsigned char* pIn0
	, unsigned char* pIn1
	, unsigned char* pIn2
	, size_t nWidth
	, size_t nHeight) {
	for (size_t y = 0; y < nHeight; ++y)
	{
		for (size_t x = 0; x < nWidth; ++x)
		{			
			pOut[x + y * nWidth] = (1 - pIn2[x + y * nWidth] / 255.0) * (pIn2[x + y * nWidth] / 255.0) * pIn0[x + y * nWidth]
				+ (1 - pIn2[x + y * nWidth] / 255.0) * (pIn2[x + y * nWidth] / 255.0) * pIn1[x + y * nWidth]
				+ (pIn2[x + y * nWidth] / 255.0) * (pIn2[x + y * nWidth] / 255.0) * (pIn1[x + y * nWidth] / 255.0 * pIn0[x + y * nWidth] / 255.0) * 255.0;		
		}
	}
} 

void ImageMirroring(unsigned char* pOut
	, unsigned char* pIn
	, size_t nWidth
	, size_t nHeight)
{

	for (size_t y = 0; y < nHeight; ++y)
	{
		for (size_t x = 0; x < nWidth; ++x)
		{
			pOut[x + y * nWidth] = pIn[(nWidth - x - 1) + y * nWidth];
		}
	}
	return;
}

void ReverseMirroring(unsigned char* pOut
	, unsigned char* pIn
	, size_t nWidth
	, size_t nHeight)
{
	for (size_t y = 0; y < nHeight; ++y)
	{
		for (size_t x = 0; x < nWidth; ++x)
		{
		pOut[x + y * nWidth] = pIn[x + nWidth *(nHeight - y - 1)];
		}
	}
} 

void Transpose(unsigned char* pOut
	, unsigned char* pIn
	, size_t nWidth
	, size_t nHeight)
{
	for (size_t y = 0; y < nHeight; ++y)
	{
		for (size_t x = 0; x < nWidth; ++x)
		{
			pOut[x + y * nWidth] = pIn[y + x*nWidth];
		}
	}
}