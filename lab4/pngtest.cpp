#include "PngProc.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <vector>

template<class png>
void Check(png* InPic, int Width, const size_t x, const size_t y, int c, int r, int& error, float coeff) {
	int derror = InPic[(x + c) + (y + r) * Width] + error * (coeff);
	derror > 255 ? derror = 255 : derror=derror;
	derror < 0 ? derror = 0 : derror=derror;
	InPic[(x + c) + (y + r) * Width] = derror;
}

template<class png>
void Transformation(png* InPic, const int Width,  const int Height, const size_t x, const size_t y, int& error) {
	bool flag;
	InPic[x + y * Width] > 127 ? flag = true : flag = false;
	if (flag == true) {
		error = InPic[x + y * Width] - 255;
		InPic[x + y * Width] = 255;
	}
	else {
		error = InPic[x + y * Width];
		InPic[x + y * Width] = 0;
	}
}

template<class png>
void FS(png* InPic, const int Width, const int Height) {
	bool flag;
	int error = 0;
	int derror = 0;
	for (size_t y = 0; y < Height; y++) {
		for (size_t x = 0; x < Width; x++) {
			Transformation(InPic, Width, Height, x, y, error);
			//Рассеиваем ошибку на следующий элемент, нижний левый, нижний соседний и на нижний правый элемент
			if (x != 0 && x != Width - 1 && y != Height - 1) {
				Check(InPic, Width, x, y, 1, 0, error, (7.0 / 16.0));
				Check(InPic, Width, x, y, -1, 1, error, (3.0 / 16.0));
				Check(InPic, Width, x, y, 0, 1, error, (5.0 / 16.0));
				Check(InPic, Width, x, y, 1, 1, error, (1.0 / 16.0));
			}
			//Если мы оказались в начале массива, то рассеиваем ошибку на следующий, нижний соседний и нижний правый элемент
			else if (x == 0 && y != Height - 1) {
				Check(InPic, Width, x, y, 1, 0, error, (7.0 / 16.0));
				Check(InPic, Width, x, y, 0, 1, error, (5.0 / 16.0));
				Check(InPic, Width, x, y, 1, 1, error, (1.0 / 16.0));
			}
			//Если мы оказались на последнем элементе строки, то рассеиваем ошибку на нижний левый и на нижний соседний элемент

			else if (x == Width - 1 && y != Height - 1) {
				Check(InPic, Width, x, y, 0, 1, error, (5.0 / 16.0));
				Check(InPic, Width, x, y, -1, 1, error, (3.0 / 16.0));
			}
			//Если мы оказались на последней строке массива, то рассеиваем ошибку только на следующий элемент
			else if (y == Height - 1 && x != Width - 1) {
				Check(InPic, Width, x, y, 1, 0, error, (7.0 / 16.0));
			}

		}
	}
}



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

	char	szInputFileName[256] = "Lena.png";
	char    szOutputFileName[256] = "out.png";


	size_t nReqSize = NPngProc::readPngFile(szInputFileName, 0, 0, 0, 0);
	if (nReqSize == NPngProc::PNG_ERROR)
	{
		printf("\nError ocured while pngfile was read");
		return -1;
	}
	
	

	unsigned char* pInputBits = new unsigned char[nReqSize];
	if (!pInputBits)
	{
		printf("\nCan't allocate memory for image, required size is %u", nReqSize);
		return -1;
	}

	size_t nWidth, nHeight;
	unsigned int nBPP;

	size_t nRetSize0 = NPngProc::readPngFileGray(szInputFileName, pInputBits, &nWidth, &nHeight/*, &nBPP*/);

	nBPP = 8;

	FS(pInputBits, nWidth, nHeight);

	if (NPngProc::writePngFile(szOutputFileName, pInputBits, nWidth, nHeight, nBPP) == NPngProc::PNG_ERROR)
	{
		printf("\nError ocuured during png file was written");
		return -1;
	}
	return 0;
}

