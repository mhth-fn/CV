#include "PngProc.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>

void PrintNewMass(int* p, int NX, int NY)
{
	std::cout << "\nMassiv:" << std::endl; // выводим результат
	for (int i = 0; i < NY; i++) {
		for (int j = 0; j < NX; j++)
		{
			std::cout << p[j + i * NX] << " ";
		}
		std::cout << std::endl;
	}
}
template <class T>
void Convolution2D(T* pSrc, T* pRes, int iWidth,
	int iHeight, float* pKernel, int iKernelSize) {

	memcpy(pRes, pSrc, sizeof(*pRes) * iWidth * iHeight);

	for (int y = 0; y <= iHeight - iKernelSize; ++y) {
		for (int x = 0; x <= iWidth - iKernelSize; ++x) {

			float Sum = 0;

			for (int j = 0; j < iKernelSize; ++j) {
				for (int i = 0; i < iKernelSize; ++i)

					Sum += pSrc[(y + j) * iWidth + (x + i)] * pKernel[j * iKernelSize + i];
			}

			if (Sum > 255) Sum = 255;
			else
				if (Sum < 0) Sum = 0;

			pRes[(y + (iKernelSize / 2)) * iWidth + (x + (iKernelSize / 2))] = (int)Sum;
		}
	}

}

template <class T>
void convolution2D(T* InPic, T* OutPic, int Width, int Height,
	float* kernel, int kWidth, int kHeight) {

	float bufsum; 
	int revkx, revky; //"обратные" значения индексов ядра

	int suby, subx;

	//Найдем центральный индекс ядра исходя из логики, что
	//если поделить нечетное число на два и присвоить значение
	//переменной int, то получим как раз центральный индекс
	int kCX = kWidth / 2; 
	int kCY = kHeight / 2;

	for (int y = 0; y < Height; ++y)
	{
		for (int x = 0; x < Width; ++x)
		{
			//Обновляем сумму для каждого пикселя
			bufsum = 0;
			for (int ky = 0; ky < kHeight; ++ky)
			{
				revky = (kHeight - 1) - ky;       //"Обратное" значение индекса ядра по y 
				for (int kx = 0; kx < kWidth; ++kx)
				{
					revkx = (kWidth - 1) - kx;   //"Обратное" значение индекса ядра по y 

					suby = y + (kCY - revky);
					subx = x + (kCX - revkx);
					//Если элемент ядра выходят за границы нашей картинки, то ничего не прибавляем (прибавляем ноль)
					//По сути, просто добавляем "нулевое граничное условие".
					if (suby >= 0 && suby < Height && subx >= 0 && subx < Width) {
						bufsum += InPic[Width * suby + subx] * kernel[kWidth * revky + revkx];
					}
				}
			}
			bufsum < 0 ? bufsum = 0 : bufsum = bufsum;
			bufsum > 255 ? bufsum = 255 : bufsum = bufsum;
			OutPic[x + y * Width] = bufsum;
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

	CBitsPtrGuard InputBitsPtrGuard0(&pInputBits);



	unsigned char* pOutputBits = new unsigned char[nReqSize];
	if (!pOutputBits)
	{
		printf("\nCan't allocate memory for image, required size is %u", nReqSize);
		return -1;
	}

	CBitsPtrGuard OutputBitsPtrGuard(&pOutputBits);

	size_t nWidth, nHeight;
	unsigned int nBPP;

	size_t nRetSize0 = NPngProc::readPngFileGray(szInputFileName, pInputBits, &nWidth, &nHeight/*, &nBPP*/);

	nBPP = 8;
	//Тест из лекции (прошел успешно)
	int* test1 = new int[20]
	{ 5, 7, 8, 1, 3,
	3, 2, 4, 1, 3,
	6, 4, 0, 1, 1,
	1, 3, 1, 2 ,2 };
	int* test1out = new int[20]; 
	 
	//Ядро для низкочастотной фильтрации:  (1.0/5.0)*5 = 1
	float* LPF = new float[9]{
	0, 1.0 / 5, 0,
	1.0 / 5, 1.0 / 5, 1.0 / 5,
	0, 1.0 / 5, 0 };

	//Ядро для фильтрации, повышающей резкость: 8*(-2.0/8.0) + 3 = 1
	float* SHARPNESS= new float[9]{
	-2.0/8.0, -2.0 / 8, -2.0 / 8.0,
	-2.0 / 8.0, 3, -2.0 / 8.0,
	-2.0 / 8.0, -2.0 / 8.0, -2.0 / 8.0 };

	//Ядро для высокочастотной фильтрации: 4*(-1.0/4.0) + 1 = 0
	float* HPF = new float[9]{0, -1.0 / 4, 0,
	-1.0 / 4, 1, -1.0 / 4,
	0, -1.0 / 4, 0 };

	float* F = new float[25]{
	0, 0, -1, 0, 0,
	0, -1, -2, -1, 0,
	-1, -2, 16, -2, -1,
	0, -1, -2, -1, 0,
	0, 0, -1, 0, 0};

	Convolution2D(test1, test1out, 5, 4, LPF, 3);
	//convolution2D(pInputBits, pOutputBits, nWidth, nHeight, F, 5, 5);

	PrintNewMass(test1out, 5, 4);
	if (NPngProc::writePngFile(szOutputFileName, pOutputBits, nWidth, nHeight, nBPP) == NPngProc::PNG_ERROR)
	{
		printf("\nError ocuured during png file was written");
		return -1;
	}
	delete[] LPF, SHARPNESS, HPF, test1, test1out;
	return 0;
}

