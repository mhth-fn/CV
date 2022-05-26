#include "PngProc.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <cmath>
#include <vector>
#include <algorithm>

# define M_PI           3.14159265358979323846  /* pi */
# define angel			45

//ядро кубической свертки
float u(float v) {
	float a = -0.5;
	if (abs(v) <= 1) {
		return ((a + 2) * pow(abs(v), 3) - (a + 3) * v * v + 1);
	}else if (abs(v) <= 2) {
		return (a * pow(abs(v), 3) - 5 * a * v * v + 8 * a * abs(v) - 4 * a);
	}else {
		return 0;
	}
}

//ќдномерна€ кубическа€ свертка
template <class T>
float CubicConvolution(T* InPic, int Width, int x, int y, float v) {	
	int Pm1 = InPic[(x - 1) + y * Width],
		P0 = InPic[x + y * Width],
		P1 = InPic[(x + 1) + y * Width],
		P2 = InPic[(x + 2) + y * Width];
	float f = Pm1 * u(v - 1) + P0 * u(v) + P1 * u(v + 1) + P2 * u(v + 2);
	return f;
}

//јффинный поворот
void AffineRotate(int i, int j, float& xx, float& yy, float phi) {
		yy = j * sin(phi) + i * cos(phi),
		xx = - i * sin(phi) + j * cos(phi);
}

//¬ычисление нового размера
int NewSize(float lh, float lr, float ld, float rd){
	float maximum = std::max(lh, std::max(lr, std::max(ld, rd)));
	float minimum = std::min(lh, std::min(lr, std::min(ld, rd)));
	return ceil(abs(maximum) + abs(minimum));
}

/* оординаты угловых пикселов исходного изображени€
/умножаютс€ на пр€мую матрицу трансформации дл€ нахождени€
/координат угловых пикселов и размера нового изображени€ */
template <class T>
void FirstStep(T* InPic, const int Width, const int Height, int& nWidth, int& nHeight, int& shiftyy, int& shiftxx, const float phi) {
	//ѕараметры преобразовани€

	float lhxx, lhyy,
		rhxx, rhyy,
		ldxx, ldyy,
		rdxx, rdyy;
	//Ќаходим координаты угловых пикселов
	AffineRotate(0, 0, lhxx, lhyy, phi);
	AffineRotate(0, Width - 1, rhxx, rhyy, phi);
	AffineRotate(Height - 1, 0, ldxx, ldyy, phi);
	AffineRotate(Height - 1, Width - 1, rdxx, rdyy, phi);
	//Ќаходим размер
	nWidth = NewSize(lhxx, rhxx, ldxx, rdxx) ;
	nHeight = NewSize(lhyy, rhyy, ldyy, rdyy) ;
	//Ќаходим смещение
	phi > 0 ? shiftyy = rhyy : shiftyy = lhyy;
	phi > 0 ? shiftxx = 0 : shiftxx = nWidth - rhxx;
}

//¬ычисление значени€ в вершине
template <class T>
int f(T* InPic, int x, const int y, const int Width, const int c, const int r) {
	return InPic[(x+c) + (y+r) * Width];
}

//Ѕикубическа€ интерпол€ци€
template <class T>
int DirectlyBicubic(T* InPic, int Width, const int xx, const int yy, const float x, const float y) {
	float b1 = (x - 1) * (x - 2) * (x + 1) * (y - 1) * (y - 2) * (y + 1) / 4,
		b2 = -x * (x - 2) * (x + 1) * (y - 1) * (y - 2) * (y + 1) / 4,
		b3 = -(x - 1) * (x - 2) * (x + 1) * y * (y - 2) * (y + 1) / 4,
		b4 = x * (x - 2) * (x + 1) * y * (y - 2) * (y + 1) / 4,
		b5 = -x * (x - 1) * (x - 2) * (y - 1) * (y - 2) * (y + 1) / 12,
		b6 = -(x - 1) * (x - 2) * (x + 1) * y * (y - 1) * (y - 2) / 12,
		b7 = x * (x - 1) * (x - 2) * y * (y - 2) * (y + 1) / 12,
		b8 = x * (x - 2) * (x + 1) * y * (y - 1) * (y - 2) / 12,
		b9 = x * (x - 1) * (x + 1) * (y - 1) * (y - 2) * (y + 1) / 12,
		b10 = (x - 1) * (x - 2) * (x + 1) * y * (y - 1) * (y + 1) / 12,
		b11 = x * (x - 1) * (x - 2) * y * (y - 1) * (y - 2) / 36,
		b12 = -x * (x - 1) * (x + 1) * y * (y - 2) * (y + 1) / 12,
		b13 = -x * (x - 2) * (x + 1) * y * (y - 1) * (y + 1) / 12,
		b14 = -x * (x - 1) * (x + 1) * y * (y - 1) * (y - 2) / 36,
		b15 = -x * (x - 1) * (x - 2) * y * (y - 1) * (y + 1) / 36,
		b16 = x * (x - 1) * (x + 1) * y * (y - 1) * (y + 1) / 36;
	return (b1 * f(InPic, xx, yy, Width, 0, 0) +
		b2 * f(InPic, xx, yy, Width, 0, 1) +
		b3 * f(InPic, xx, yy, Width, 1, 0) +
		b4 * f(InPic, xx, yy, Width, 1, 1) +
		b5 * f(InPic, xx, yy, Width, 0, -1) +
		b6 * f(InPic, xx, yy, Width, -1, 0) +
		b7 * f(InPic, xx, yy, Width, 1, -1) +
		b8 * f(InPic, xx, yy, Width, -1, 1) +
		b9 * f(InPic, xx, yy, Width, 0, 2) +
		b10 * f(InPic, xx, yy, Width, 2, 0) +
		b11 * f(InPic, xx, yy, Width, -1, -1) +
		b12 * f(InPic, xx, yy, Width, 1, 2) +
		b13 * f(InPic, xx, yy, Width, 2, 1) +
		b14 * f(InPic, xx, yy, Width, -1, 2) +
		b15 * f(InPic, xx, yy, Width, 2, -1) +
		b16 * f(InPic, xx, yy, Width, 2, 2));
}

//—ам поворот и интерпол€ци€
template <class T>
void BicubicRotate(T* InPic, T* OutPic, const int Width, const int Height, const int nWidth, const int nHeight, const int shiftyy, const int shiftxx, const float phi) {
	float barbar = shiftyy;
	for (int i = 0; i < nHeight; i++) {
		for (int j = 0; j < nWidth; j++) {
			/* оординаты каждого пиксела нового изображени€ умножают на
			/обратную матрицу трансформации дл€ получени€ координаты
			/соответствующего ему пиксела в исходном изображении */
			float yy =(j - shiftxx) * sin(phi) + (i - barbar) * cos(phi),
				xx =-(i - barbar) * sin(phi) + (j - shiftxx) * cos(phi);
			/*ѕровер€ем, если координата попала в пределы габаритов исходного
			/изображени€, то значение пиксела в новом изображении
			/определ€ют с помощью интерпол€ции (в данном случае бикубической) по соседним пикселам
			/исходного изображени€.*/
			if (yy >= 0 && xx >= 0 && xx <= Width-1 && yy <= Height-1) {
				int y = (int)yy,
					x = (int)xx;
				float p = yy - y,
					v = xx - x;
				//Ќе получилось реализвать кубической сверткой, на выходе получалась разбита€ на квадратики картинка
				/*int P0 = CubicConvolution(InPic, Width, x, y - 1, v),
					P1 = CubicConvolution(InPic,  Width,  x,  y,  v),
					P2 = CubicConvolution(InPic,  Width,  x,  y + 1,  v),
					P3 = CubicConvolution(InPic,  Width,  x,  y + 2,  v);
				OutPic[j + i * nWidth] = P0 * u(p - 1) + P1 * u(p) + P2 * u(p + 1) + P3 * u(p + 2);*/
				//Ѕикубическа€ интерпол€ци€
				OutPic[j + i * nWidth] = DirectlyBicubic(InPic, Width, x, y, v, p);
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

	// parse input parameters
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


	size_t nWidth, nHeight;
	unsigned int nBPP;

	size_t nRetSize0 = NPngProc::readPngFileGray(szInputFileName, pInputBits, &nWidth, &nHeight/*, &nBPP*/);
	int NewWidth = 0,
		NewHeight = 0,
		shiftyy = 0, shiftxx = 0; 			/*ѕеременные shiftxx и shiftyy отвечают за смещение, дабы картинка не выезжала за границы*/
	float phi = angel * M_PI / 180.0;;
	nBPP = 8;
	FirstStep(pInputBits, nWidth, nHeight, NewWidth, NewHeight, shiftyy, shiftxx, phi);
	unsigned char* pOutputBits = new unsigned char[NewWidth * NewHeight];
	BicubicRotate(pInputBits, pOutputBits, nWidth, nHeight, NewWidth, NewHeight, shiftyy, shiftxx, phi);
	if (NPngProc::writePngFile(szOutputFileName, pOutputBits, NewWidth, NewHeight, nBPP) == NPngProc::PNG_ERROR)
	{
		printf("\nError ocuured during png file was written");
		return -1;
	}
	return 0;
}

