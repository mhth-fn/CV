#include "PngProc.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>


template <class T>
void PrintNewMass(T* p, int NX, int NY)
{
	std::cout << "\nMassiv:" << std::endl; // выводим результат
	for (int i = 0; i < NY; i++) {
		for (int j = 0; j < NX; j++)
		{
			std::cout << (int)p[j + i * NX] << " ";
		}
		std::cout << std::endl;
	}
}

template <class T>
void RankFiltration(T* InPic, T* OutPic, int Width, int Height,
	float* kernel, int kWidth, int kHeight, size_t Rank) {

	std::vector<int> buf;
	int revkx, revky; //"обратные" значени€ индексов €дра

	int suby, subx; 

	//Ќайдем центральный индекс €дра исход€ из логики, что
	//если поделить нечетное число на два и присвоить значение
	//переменной int, то получим как раз центральный индекс
	int kCX = kWidth / 2; 
	int kCY = kHeight / 2;

	for (int y = 0; y < Height; ++y)
	{
		for (int x = 0; x < Width; ++x)
		{
			//ќбновл€ем сумму дл€ каждого пиксел€
			buf.clear();
			for (int ky = 0; ky < kHeight; ++ky)
			{
				revky = (kHeight - 1) - ky;       //"ќбратное" значение индекса €дра по y 
				for (int kx = 0; kx < kWidth; ++kx)
				{
					revkx = (kWidth - 1) - kx;   //"ќбратное" значение индекса €дра по y 

					suby = y + (kCY - revky);
					subx = x + (kCX - revkx);
					//≈сли попадаем в границу, то добавл€ем значение пиксел€, иначе добавл€ем ноль
					if (suby >= 0 && suby < Height && subx >= 0 && subx < Width) {
						buf.push_back(InPic[Width * suby + subx]  * kernel[kWidth * revky + revkx]);
					}
					else{
						buf.push_back(0);
					}
				}
			}
			//—ортируем полученный вектор и записываем среднее значение 
			sort(buf.begin(), buf.end());

			OutPic[x + y * Width] = buf[Rank];
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

	char	szInputFileName[256] = "Chrom.png";
	char    szOutputFileName[256] = "Chrom_Dilation.png";


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
	//“ест из лекции 
	int* test1 = new int[20]
	{ 7, 5, 8, 4,
		3, 12, 1, 3,
		6, 9, 12, 18,
		1, 5, 6, 7};
	int* test1out = new int[20]; 
	 

	float* apertura = new float[9]{
	1.0, 1.0 , 1.0,
	1.0 , 1.0 , 1.0 ,
	1.0, 1.0 , 1.0};
	
	size_t Rank = 8;
	RankFiltration(pInputBits, pOutputBits, nWidth, nHeight, apertura, 3, 3, Rank);

	//PrintNewMass(pOutputBits, nWidth, nHeight);	
	if (NPngProc::writePngFile(szOutputFileName, pOutputBits, nWidth, nHeight, nBPP) == NPngProc::PNG_ERROR)
	{
		printf("\nError ocuured during png file was written");
		return -1;
	}
	delete[] test1, apertura, test1out;
	return 0;
}

