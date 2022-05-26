#include "PngProc.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <vector>

template <typename T>
void PrintArray(T* array) {
	int count = 0;
	for (int y = 0; y < 16; ++y)
	{
		std::cout << array[y] << " ";
		count += array[y];
	}
	std::cout << '\n';
	/*for (int y = 0; y < 255; ++y)
	{
		if (array[y] != 0) {
			std::cout << y << " ";
		}
	}*/
	std::cout << '\n' << "count: " << count << '\n';
}

template <typename png>
void Hist(png* picture, int iHeight, int iWidth,
	int iWidthBytes, double* pulHist)
{
	// начальное обнуление массива гистограммы
	memset(pulHist, 0, 256 * sizeof(*pulHist));
	// вычисление гистограммы
	for (int y = 0; y < iHeight; ++y)
	{
		for (int x = 0; x < iWidth; ++x)
		{
			pulHist[picture[iWidthBytes * y + x]]++;
			//PrintArray(pulHist);
		}
	}
}

void ProbabilityDensity(double* pulHist, int iHeight, int iWidth) {
	for (size_t i = 0; i < 256; ++i) {
		pulHist[i] /= (iHeight * iWidth);
	}
}

void Mean(double* pulHist, double& mean, double& En, double& H, double& E, double& sigma, double& A) {

	for (size_t i = 0; i < 256; ++i) {
		mean += i * pulHist[i];
	}
	for (size_t i = 0; i < 256; ++i) {
		En += pulHist[i] * pulHist[i];
		//log(0) не определен, поэтому делаю малую добавку
		H -= pulHist[i] * log2(pulHist[i] + 1e-16);
		sigma += + pow((i - mean), 2) * pulHist[i];
		A += pow((i - mean), 3) * pulHist[i];
		E += pow((i - mean), 4) * pulHist[i];
		}
	E = (E / pow(sigma, 2)) - 3;
	A /= pow(sqrt(sigma), 3);
	}

template <typename png>
void comatrix(png* picture, const int iHeight, const int iWidth, unsigned long* comat, float* P, float& B, const int r, const int c, const int N ) {
	memset(comat, 0, N*N * sizeof(*comat));
	memset(P, 0, N * N * sizeof(P));
	for (size_t y = 0; y < iHeight - c; ++y)
	{
		for (size_t x = 0; x < iWidth - r; ++x)
		{
			comat[picture[iWidth * y + x] + N* picture[iWidth * (y + c) + (x + r)]]++;
		}
	}
	for (size_t i = 0; i < N * N; i++)
	{
		P[i] = (float) comat[i] / ((iWidth - c) * (iHeight - r));
		B += pow(P[i], 2);

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
	 
	//Выделяем память для гистограммы
	double* pulHist = new double[256];
	//Вычисляем гистограмму
	Hist(pInputBits, nHeight, nWidth, nWidth, pulHist);
	//Нормируем 
	ProbabilityDensity(pulHist, nHeight, nWidth);
	double mean = 0.0,  /*Мат. ожидание*/
		En = 0.0, /*Энергия*/
		H = 0.0, /*Энтропия*/
		E = 0.0, /*Коэффициент эксцесса*/
		sigma = 0.0,/*Дисперсия*/
		A = 0.0; /*Коэффициент асимметрии*/
	//Вычислим гистограммные признаки
	Mean(pulHist, mean, En, H, E, sigma, A);
	std::cout << "Mean: " << mean << '\n';
	int N = 256;

	/*int* test = new int[25]{1 , 2, 0, 1, 0,
	3, 2, 1, 0, 0,
	1, 1, 2, 3, 0,
	3, 3, 3, 1, 0,
	1, 2, 1, 0, 3};
	*/

	//Выделяем память для гистограммы второго порядка и текстурных признаков
	unsigned long* comat = new unsigned long[N*N];
	float* P = new float[N * N];
	float B = 0; /*Энергия матрицы совместной встречаемости*/
	//Вычисляем гистограмму второго порядка и текстурные признаки
	comatrix(pInputBits, nHeight, nWidth, comat, P, B, 0, 1, N);

	//	PrintArray(comat);


	if (NPngProc::writePngFile(szOutputFileName, pOutputBits, nWidth, nHeight, nBPP) == NPngProc::PNG_ERROR)
	{
		printf("\nError ocuured during png file was written");
		return -1;
	}
	//Очищаем память
	delete[] comat, pulHist, P;
	return 0;
}

