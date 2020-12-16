/*
	0~13 까지 모드 선택
*/
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define _USE_MATH_DEFINES
#include <math.h>

typedef unsigned char uchar;

using namespace std;
using namespace cv;

uchar** uc_alloc(int size_x, int size_y)		// 메모리 할당받는 함수(unsinged char)
{
	uchar** m;
	int i;
	if ((m = (uchar**)calloc(size_y, sizeof(uchar*))) == NULL)
	{
		printf("uc_alloc error 1\7\n");
		exit(0);
	}
	for (i = 0; i < size_y; i++)
		if ((m[i] = (uchar*)calloc(size_x, sizeof(uchar))) == NULL)
		{
			printf("uc_alloc error 2\7\n");
			exit(0);
		}
	return m;
}
double** d_alloc(int size_x, int size_y)		// 메모리 할당받는 함수(double)
{
	double** m;
	int i;
	if ((m = (double**)calloc(size_y, sizeof(double*))) == NULL)
	{
		printf("d_alloc error 1\7\n");
		exit(0);
	}
	for (i = 0; i < size_y; i++)
		if ((m[i] = (double*)calloc(size_x, sizeof(double))) == NULL)
		{
			printf("d_alloc error 2\7\n");
			exit(0);
		}
	return m;
}
void uc_free(int size_x, int size_y, uchar** ucmatrix)
{
	int i;

	for (i = 0; i < size_y; i++)
		free(ucmatrix[i]);

	free(ucmatrix);
}
void read_ucmatrix(int size_x, int size_y, uchar** ucmatrix, const char* filename)		// 하드디스크에서 영상을 읽어주는 함수
{
	int i;
	FILE* f;

	if ((f = fopen(filename, "rb")) == NULL)
	{
		printf("%s File open Error! \n", filename);
		printf("argv[5] is histogram reference image\n");
		exit(0);
	}
	for (i = 0; i < size_y; i++)
		if (fread(ucmatrix[i], sizeof(uchar), size_x, f) != size_x)
		{
			printf("Data Read Error! \n");
			exit(0);
		}
	fclose(f);
}
void write_ucmatrix(int size_x, int size_y, uchar** ucmatrix, const char* filename)		// 영상을 저장해주는 함수
{
	int i;
	FILE* f;

	if ((f = fopen(filename, "wb")) == NULL)
	{
		printf("%s File open Error! \n", filename);
		exit(0);
	}
	for (i = 0; i < size_y; i++)
		if (fwrite(ucmatrix[i], sizeof(uchar), size_x, f) != size_x)
		{
			printf("Data Write Error! \n");
			exit(0);
		}
	fclose(f);
}
void Negative(int Row, int Col, uchar** img, uchar** res)
{
	int i, j;

	for (i = 0; i < Col; i++)
		for (j = 0; j < Row; j++)
			res[i][j] = 255 - img[i][j];
}
void Mosaic(uchar** img, uchar** res, int Row, int Col, int block)
{
	int i, j, x, y, tmp, count;

	for (i = 0; i < Row; i += block)
		for (j = 0; j < Col; j += block)
		{
			tmp = 0;
			count = 0;
			
			for (y = 0; y < block; y++)
			{
				for (x = 0; x < block; x++)
				{
					tmp += img[i + y][j + x];
					count++;
				}
			}

			tmp /= count;

			for (y = 0; y < block; y++)
			{
				for (x = 0; x < block; x++)
				{
					res[i + y][j + x] = tmp;
				}
			}
		}
}
void makeBinary(uchar** img, uchar** res, int Row, int Col, double avg)
{
	int i, j;

	for (i = 0; i < Row; i++)
	{
		for (j = 0; j < Col; j++)
		{
			if (img[i][j] > avg)
				res[i][j] = 255;
			else
				res[i][j] = 0;
		}
	}
}
void PowImg(uchar** img, uchar** Result, int Row, int Col, double gamma)				// 감마 정정 함수
{
	int i, j;
	double tmp;

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			tmp = pow(img[i][j] / 255., 1 / gamma);
			// tmp = 해당 픽셀의 값을 255로 나눈후 (결과 0~1), ^(1/매개변수)의 연산
			// 감마의 역수 승만큼 제곱하므로 감마가 작을 수록 높은 tmp 의 값 (0.0~ 1.0)^ (1/gamma)
			if (tmp * 255 > 255) tmp = 1;
			else if (tmp * 255 < 0) tmp = 0;
			// tmp 값이 1 초과이거나 0 미만일 경우 각각 0,1 로 초기화 시켜줌
			Result[i][j] = tmp * 255;
			// 나누어진 255 값을 다시 곱하여 줌
		}
}
double average(uchar** img, int Row, int Col)											// 평균값 구하는 함수
{
	double sum = 0, avg;
	int i, j;

	for (i = 0; i < Col; i++)
	{
		for (j = 0; j < Row; j++)
			sum += img[i][j];
	}

	avg = sum / ((double)Col * Row);

	//printf("avg = %lf\n", avg);

	return avg;
}

double powAvgImg(uchar** img, uchar** res, int Row, int Col)							// 감마 평균 정정 함수
{
	/*
		k = 감마 , 감마를 0부터 10까지 0.001씩 대입해서
		출력되는 이미지의 평균이 127.0 초과 129.0 미만이 될 때
		반복문 동작을 멈춤. 출력되는 이미지는 결과 이미지에 자동으로 입력 후
		감마값 반환
	*/
	int i, j;
	double tmp, k;
	
	for (k = 0.0; k < 10; k += 0.001)
	{
		for (i = 0; i < Row; i++)
			for (j = 0; j < Row; j++)
			{
				tmp = pow(img[i][j] / 255., 1 / k);

				if (tmp * 255 > 255)	tmp = 1;
				else if (tmp * 255 < 0)	tmp = 0;

				res[i][j] = tmp * 255;
			}
		if (average(res, Row, Col) > 127. && average(res, Row, Col) < 129.)
			break;
	}
	printf("반환된 감마값: %lf ", k);
	return k;
}
void Circle(uchar** img, uchar** Result, int Row, int Col, double diameter)
{
	int i, j;
	double tmp, xSqure, ySqure;

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			ySqure = (Row / 2 - i) * (Row / 2 - i);
			xSqure = (Col / 2 - j) * (Col / 2 - j);

			tmp = sqrt(ySqure + xSqure);

			if (tmp < diameter)
				Result[i][j] = 255;
			else
				Result[i][j] = 0;
		}
}
void midterm(uchar** img, uchar** res, int Row, int Col, int mode)
{
	int i, j, sum, count, x, y, block=0, row_pos, col_pos;
	double tmp, xSquare, ySquare, diameter, avg;

	if (mode == 1)
	{
		printf("Insert parameter (blocksize diameter row_pos col_pos) : ");
		scanf_s("%d", &block);
		scanf_s("%lf", &diameter);
		scanf_s("%d", &row_pos);
		scanf_s("%d", &col_pos);
	}
	else
	{
		printf("Insert parameter (diameter row_pos col_pos) : ");
		scanf_s("%lf", &diameter);
		scanf_s("%d", &row_pos);
		scanf_s("%d", &col_pos);
	}

	switch (mode)
	{
		case 0:		// 원하는 영역 Negative처리
			for (i = 0; i < Row; i++)
				for (j = 0; j < Col; j++)
				{
					ySquare = (col_pos - i) * (col_pos - i);
					xSquare = (row_pos - j) * (row_pos - j);

					tmp = sqrt(ySquare + xSquare);							// tmp -> 각각의 픽셀이 한가운데의 중앙 점으로부터 떨어져 있는 정도.
																			// 빗변의 길이 = (a^2+b^2)의 제곱근 -> tmp = sqrt(ysquare +xsquare)
					if (tmp<diameter)
						// 원 안의 영역은 영상을 처리
						res[i][j] = 255 - img[i][j];
					else
						// 원 밖의 영역은 기존 영상 그대로
						res[i][j] = img[i][j];
				}
			break;
		case 1:		// 원하는 영역 Mosaic처리
			for (i = 0; i < Row; i += block)
				for (j = 0; j < Col; j += block)
				{
					sum = 0;
					count = 0;

					for (y = 0; y < block; y++)
						for (x = 0; x < block; x++)
						{
							sum += img[i + y][j + x];
							count++;
						}
					/*
					모자이크를 위한 작업
					sum(합계)을 count(픽셀 개수)로 나누어 블럭 내 픽셀의 평균 값을 구함
					이후 구해진 값을 블럭 내 모든 픽셀에 입력해 블럭 내 모든 픽셀이 같은 값을 갖게함.
					*/

					sum /= count;

					for (y = 0; y < block; y++)
						for (x = 0; x < block; x++)
						{
							ySquare = (col_pos - (i + y)) * (col_pos - (i + y));
							xSquare = (row_pos - (j + x)) * (row_pos - (j + x));

							tmp = sqrt(ySquare + xSquare);

							if (tmp < diameter)
								res[i + y][j + x] = sum;
							else
								res[i + y][j + x] = img[i + y][j + x];
						}
				}
			break;
		case 2:		// 원하는 영역 Binary처리
			avg = average(img, Row, Col);

			for (i = 0; i < Row; i++)
				for (j = 0; j < Col; j++)
				{
					ySquare = (col_pos - i) * (col_pos - i);
					xSquare = (row_pos - j) * (row_pos - j);

					tmp = sqrt(ySquare + xSquare);

					if (tmp < diameter)
					{
						if (img[i][j] > avg)
							res[i][j] = 255;
						else
							res[i][j] = 0;
					}
					else
						res[i][j] = img[i][j];
				}
			break;
		default:
			printf("input err\n");
			break;
		}
}
void convolution(double** mask, int F_length, int size_x, int size_y, uchar** img, uchar** res)
{
	// F_LENGTH = 필터 사이즈
	int i, j, x, y;
	int margin, indexX, indexY;
	double sum, coeff;

	margin = (int)(F_length / 2);

	for (i = 0; i < size_y; i++)
		for (j = 0; j < size_x; j++)
		{
			sum = 0;
			// X, Y = 필터 내부에서 동작하는 반복문
			// 필터 사이즈만큼 반복하여 매 픽셀마다 해당 픽셀부터 F_LENGTH^2 만큼 반복하여
			// 필터를 통과시킨다.
			for (y = 0; y < F_length; y++)
			{
				//x,y = 필터의 중앙값이 위치할 픽셀
				//indexX, Y = 해당 필터를 통과하는 첫번째 인덱스
				indexY = i - margin + y;

				//해당 인덱스가 영상의 외곽을 통과할 경우 처리
				if (indexY < 0)
					indexY = -indexY;
				else if (indexY >= size_y)
					indexY = (2 * size_y - indexY - 1);

				for (x = 0; x < F_length; x++)
				{
					indexX = j - margin + x;
					if (indexX < 0)
						indexX = -indexX;
					else if (indexX >= size_x)
						indexX = (2 * size_x - indexX - 1);			//외곽처리( 대칭기법 )

					sum += mask[y][x] * (double)img[indexY][indexX];	// 필터를 통과한 값들을 합한다. (총 9개의 값)
				}
			}
			//sum += 128;

			//클리핑 처리
			if (sum < 0.)
				sum = 0.;
			else if (sum > 255)
				sum = 255.;

			res[i][j] = sum;
		}
}
void convolution2(double** mask, int F_length, int size_x, int size_y, uchar** img, uchar** res)
{
	// F_LENGTH = 필터 사이즈
	int i, j, x, y;
	int margin, indexX, indexY;
	double sum, coeff;
	Mat tmp_res(size_x, size_y, CV_8UC(1));
	uchar** tmp_img;

	tmp_img = uc_alloc(size_x, size_y);

	margin = (int)(F_length / 2);

	for (i = 0; i < size_y; i++)
		for (j = 0; j < size_x; j++)
		{
			sum = 0;
			// X, Y = 필터 내부에서 동작하는 반복문
			// 필터 사이즈만큼 반복하여 매 픽셀마다 해당 픽셀부터 F_LENGTH^2 만큼 반복하여
			// 필터를 통과시킨다.
			for (y = 0; y < F_length; y++)
			{
				//x,y = 필터의 중앙값이 위치할 픽셀
				//indexX, Y = 해당 필터를 통과하는 첫번째 인덱스
				indexY = i - margin + y;

				//해당 인덱스가 영상의 외곽을 통과할 경우 처리
				if (indexY < 0)
					indexY = -indexY;
				else if (indexY >= size_y)
					indexY = (2 * size_y - indexY - 1);

				for (x = 0; x < F_length; x++)
				{
					indexX = j - margin + x;
					if (indexX < 0)
						indexX = -indexX;
					else if (indexX >= size_x)
						indexX = (2 * size_x - indexX - 1);			//외곽처리( 대칭기법 )

					sum += mask[y][x] * (double)img[indexY][indexX];	// 필터를 통과한 값들을 합한다. (총 9개의 값)
				}
			}
			//sum += 128;

			//클리핑 처리
			if (sum < 0.)
				sum = 0.;
			else if (sum > 255)
				sum = 255.;

			res[i][j] = sum;
			tmp_res.at<char>(i, j) = res[i][j];
		}
		for (i = 0; i < size_y; i++)
			for (j = 0; j < size_x; j++)
			{
				tmp_img[i][j] = res[i][j];
			}
		// 구한 엣지 값들을 원본영상에 더해주기
		for (i = 0; i < size_y; i++)
			for (j = 0; j < size_x; j++)
			{
				res[i][j] = img[i][j] + tmp_img[i][j];
			}
		//namedWindow("edge value", WINDOW_AUTOSIZE);
		//imshow("edge value", tmp_res);
}
void conv(double** mask, int F_length, int size_x, int size_y, uchar** img, uchar** res)	
{
	// F_LENGTH = 필터 사이즈
	int i, j, x, y;
	int margin, indexX, indexY;
	double sum, coeff;

	margin = (int)(F_length / 2);

	for (i = 0; i < size_y; i++)
		for (j = 0; j < size_x; j++)
		{
			sum = 0;
			// X, Y = 필터 내부에서 동작하는 반복문
			// 필터 사이즈만큼 반복하여 매 픽셀마다 해당 픽셀부터 F_LENGTH^2 만큼 반복하여
			// 필터를 통과시킨다.
			for (y = 0; y < F_length; y++)
			{
				//x,y = 필터의 중앙값이 위치할 픽셀
				//indexX, Y = 해당 필터를 통과하는 첫번째 인덱스
				indexY = i - margin + y;

				//해당 인덱스가 영상의 외곽을 통과할 경우 처리
				if (indexY < 0)
					indexY = -indexY;
				else if (indexY >= size_y)
					indexY = (2 * size_y - indexY - 1);

				for (x = 0; x < F_length; x++)
				{
					indexX = j - margin + x;
					if (indexX < 0)
						indexX = -indexX;
					else if (indexX >= size_x)
						indexX = (2 * size_x - indexX - 1);			//외곽처리( 대칭기법 )

					sum += mask[y][x] * (double)img[indexY][indexX];	// 필터를 통과한 값들을 합한다. (총 9개의 값)
				}
			}
			sum += 128;
		/*
			//클리핑 처리
			if (sum < 0.)
				sum = 0.;
			else if (sum > 255)
				sum = 255.;
		*/
			res[i][j] = sum;
		}
}
void Filtering(uchar** img, uchar** res, int Row, int Col)
{
	double** gaussMask, ** aveMask, **sobelYMask, **sobelXMask, **PrewittMask, **RobertsMask,
			** Laplace4Mask, **Laplace8Mask, **embosMask;
	int flag, block_size = 3;

	gaussMask = d_alloc(block_size, block_size);
	aveMask = d_alloc(block_size, block_size);
	sobelYMask = d_alloc(block_size, block_size);
	sobelXMask = d_alloc(block_size, block_size);
	PrewittMask = d_alloc(block_size, block_size);
	RobertsMask = d_alloc(block_size, block_size);
	Laplace4Mask = d_alloc(block_size, block_size);
	Laplace8Mask = d_alloc(block_size, block_size);
	embosMask = d_alloc(block_size, block_size);

	gaussMask[0][0] = 1 / 16.;	aveMask[0][0] = 1 / 9.;
	gaussMask[0][1] = 2 / 16.;	aveMask[0][1] = 1 / 9.;
	gaussMask[0][2] = 1 / 16.;	aveMask[0][2] = 1 / 9.;
	gaussMask[1][0] = 1 / 16.;	aveMask[1][0] = 1 / 9.;
	gaussMask[1][1] = 2 / 16.;	aveMask[1][1] = 1 / 9.;
	gaussMask[1][2] = 4 / 16.;	aveMask[1][2] = 1 / 9.;
	gaussMask[2][0] = 2 / 16.;	aveMask[2][0] = 1 / 9.;
	gaussMask[2][1] = 2 / 16.;	aveMask[2][1] = 1 / 9.;
	gaussMask[2][2] = 1 / 16.;	aveMask[2][2] = 1 / 9.;

	sobelYMask[0][0] =  1.;	sobelXMask[0][0] = -1.;
	sobelYMask[0][1] =  0.;	sobelXMask[0][1] = -2.;
	sobelYMask[0][2] = -1.;	sobelXMask[0][2] = -1.;
	sobelYMask[1][0] =  2.;	sobelXMask[1][0] =  0.;
	sobelYMask[1][1] =  0.;	sobelXMask[1][1] =  0.;
	sobelYMask[1][2] = -2.;	sobelXMask[1][2] =  0.;
	sobelYMask[2][0] =  1.;	sobelXMask[2][0] =  1.;
	sobelYMask[2][1] =  0.;	sobelXMask[2][1] =  2.;
	sobelYMask[2][2] = -1.;	sobelXMask[2][2] =  1.;

	Laplace4Mask[0][0] = 0.;	Laplace8Mask[0][0] = -1.;	embosMask[0][0] = -1.;
	Laplace4Mask[0][1] = -1.;	Laplace8Mask[0][1] = -1.;	embosMask[0][1] =  0.;
	Laplace4Mask[0][2] =  0.;	Laplace8Mask[0][2] = -1.;	embosMask[0][2] =  0.;
	Laplace4Mask[1][0] = -1.;	Laplace8Mask[1][0] = -1.;	embosMask[1][0] =  0.;
	Laplace4Mask[1][1] =  4.;	Laplace8Mask[1][1] =  8.;	embosMask[1][1] =  0.;
	Laplace4Mask[1][2] = -1.;	Laplace8Mask[1][2] = -1.;	embosMask[1][2] =  0.;
	Laplace4Mask[2][0] =  0.;	Laplace8Mask[2][0] = -1.;	embosMask[2][0] =  0.;
	Laplace4Mask[2][1] = -1.;	Laplace8Mask[2][1] = -1.;	embosMask[2][1] =  0.;
	Laplace4Mask[2][2] =  0.;	Laplace8Mask[2][2] = -1.;	embosMask[2][2] =  1.;

	printf("Select Filter\n0: Gaussian\n1: Average\n2: Sobel Y\n3: Sobel X\n4: Laplace(Center Val 4)\n5: Laplace(Center Val 8)\n6: Embossing\n\n");
	scanf_s("%d", &flag);

	switch (flag)
	{
		case 0:
			convolution(gaussMask, block_size, Row, Col, img, res);
			break;
		case 1:
			convolution(aveMask, block_size, Row, Col, img, res);
			break;
		case 2:
			convolution2(sobelYMask, block_size, Row, Col, img, res);
			break;
		case 3:
			convolution2(sobelXMask, block_size, Row, Col, img, res);
			break;
		case 4:
			convolution2(Laplace4Mask, block_size, Row, Col, img, res);
			break;
		case 5:
			convolution2(Laplace8Mask, block_size, Row, Col, img, res);
			break;
		case 6:
			conv(embosMask, block_size, Row, Col, img, res);
			break;
		default:
			printf("input err(Filter Select)");
	}
}
void Bubble_sort(uchar* Sort, uchar* median_value, int Mode, int filterSize)	//medianfilter의 값 정렬을 위한 버블 정렬
{
	int i, x;
	uchar temp, swap;
	//필터사이즈 ^2 만큼 반복( 필터에 들어가는 총 값의 수)
	for (x = 0; x < filterSize * filterSize; x++)
	{
		//비교를 위한 값 대입
		temp = Sort[x];
		for (i = x; i < filterSize * filterSize - 1; i++)
		{
			//버블 정렬 동작
			if (temp >= Sort[i + 1])
			{
				swap = temp;
				temp = Sort[i + 1];
				Sort[i + 1] = swap;
			}
		}
		Sort[x] = temp;
	}
	if (Mode == -1)
		*median_value = (uchar)Sort[0]; // median filter의 중앙값을 필터내의 최솟값으로 설정
	else if (Mode == 0)
		*median_value = (uchar)Sort[filterSize * filterSize / 2]; // median filter의 중앙값을 필터내의 중간값으로 설정
	else if (Mode == 1)
		*median_value = (uchar)Sort[filterSize * filterSize - 1]; // median filter의 중앙값을 필터내의 최대값으로 설정
}
void median(uchar** img, uchar** res, int Row, int Col, int Mode, int filterSize) // Median Filtering
{
	int i, j, x, y, z, count = 0, filterSize2;
	uchar median_value; // 필터의 중앙값
	uchar* Sort;
	// 필터의 마스크값을 정렬해 저장할 포인터 배열 동적할당
	filterSize2 = filterSize * filterSize;
	Sort = (uchar*)calloc(filterSize2, sizeof(uchar));

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
			res[i][j] = img[i][j];

	//모든 픽셀에 대하여 영상의 외곽을 초과하지 않는 범위 내에서 필터링
	for (i = 0; i < Row - filterSize; i++)
		for (j = 0; j < Col - filterSize; j++)
		{
			//필터내의 모든 값을 sort 배열에 입력
			for (x = 0; x < filterSize; x++)
				for (y = 0; y < filterSize; y++)
					Sort[filterSize * x + y] = img[i + x][j + y];
			//입력된 배열을 정렬
			Bubble_sort(Sort, &median_value, Mode, filterSize);
			//필터의 중앙에 위치한 픽셀에 median value를 입력
			res[i + 1][j + 1] = median_value;
		}
	free(Sort);
}
int rearrange(double* X, int N)
{
	int i, j, *power_of_2, *pos, stage, num_of_stages = 0;
	double temp;

	for (i = N; i > 1; i >>= 1, num_of_stages++);

	if ((power_of_2 = (int*)malloc(sizeof(int) * num_of_stages)) == NULL)
		return -1;
	if ((pos = (int*)malloc(sizeof(int) * N)) == NULL)
		return -1;

	power_of_2[0] = 1;

	for (stage = 1; stage < num_of_stages; stage++)
		power_of_2[stage] = power_of_2[stage - 1] << 1;

	for (i = 1; i < N - 1; i++)
		pos[i] = 0;
	for (i = 1; i < N - 1; i++)
	{
		if (!pos[i])
		{
			for (j = 0; j < num_of_stages; j++)
			{
				if (i & power_of_2[j])
					pos[i] += power_of_2[num_of_stages - 1 - j];
			}

			temp = X[i];
			X[i] = X[pos[i]];
			X[pos[i]] = temp;
			pos[pos[i]] = 1;
		}
	}
	free(power_of_2);
	free(pos);

	return 0;

}
void fft(double* X_re, double* X_im, int N)
{
	double X_temp_re, X_temp_im;
	double phase;

	int num_of_stages = 0, num_of_elements, num_of_sections, size_of_butterfly;
	int i, j, stage, m1, m2;

	for (i = N; i > 1; i >>= 1, num_of_stages++);

	num_of_elements = N;
	num_of_sections = 1;
	size_of_butterfly = N >> 1;

	for (stage = 0; stage < num_of_stages; stage++)
	{
		m1 = 0;
		m2 = size_of_butterfly;

		for (i = 0; i < num_of_sections; i++)
		{
			for (j = 0; j < size_of_butterfly; j++, m1++, m2++)
			{
				X_temp_re = X_re[m1] - X_re[m2];
				X_temp_im = X_im[m1] - X_im[m2];

				X_re[m1] = X_re[m1] + X_re[m2];
				X_im[m1] = X_im[m1] + X_im[m2];

				phase = -2.0 * M_PI * j / num_of_elements;

				X_re[m2] = X_temp_re * cos(phase) - X_temp_im * sin(phase);
				X_im[m2] = X_temp_re * sin(phase) + X_temp_im * cos(phase);
			}

			m1 += size_of_butterfly;
			m2 += size_of_butterfly;

		}
		num_of_elements >>= 1;
		num_of_sections <<= 1;
		size_of_butterfly >>= 1;

	}

	rearrange(X_re, N);
	rearrange(X_im, N);

}
int fft_2d(double** X_re, double** X_im, int N, int Mode)
{
	int i, j;
	double *temp_re, *temp_im;

	//temp 메모리 할당
	if ((temp_re = (double*)malloc(sizeof(double) * N)) == NULL)
		return -1;
	if ((temp_im = (double*)malloc(sizeof(double) * N)) == NULL)
		return -1;

	if (Mode == 0)
	{
		//row만큼 횟수로  fft 수행
		for (i = 0; i < N; i++)
			fft(X_re[i], X_im[i], N);

		for (j = 0; j < N; j++)
		{
			for (i = 0; i < N; i++)
			{
				temp_re[i] = X_re[i][j];
				temp_im[i] = X_im[i][j];
			}
			fft(temp_re, temp_im, N);

			for (i = 0; i < N; i++)
			{
				X_re[i][j] = temp_re[i] / N;
				X_im[i][j] = temp_im[i] / N;
			}
		}
	}
	else if (Mode == 1)
	{
		for (i = 0; i < N; i++)
			fft(X_re[i], X_im[i], N);

		for (j = 0; j < N; j++)
		{
			for (i = 0; i < N; i++)
			{
				temp_re[i] = X_re[i][j];
				temp_im[i] = X_im[i][j];
			}
			fft(temp_re, temp_im, N);

			for (i = 0; i < N; i++)
			{
				X_re[i][j] = temp_re[i] / N;
				X_im[i][j] = temp_im[i] / N;
			}
		}
	}
	else if (Mode == 2)
	{
		for (i = 0; i < N; i++)
		{
			for (j = 0; j < N; j++)
			{
				temp_re[j] = X_re[i][j] * pow(-1, j);
				temp_im[j] = X_im[i][j] * pow(-1, j);
			}

			fft(temp_re, temp_im, N);

			for (j = 0; j < N; j++)
			{
				X_re[i][j] = temp_re[j];
				X_im[i][j] = temp_im[j];
			}

		}
		for (j = 0; j < N; j++)
		{
			for (i = 0; i < N; i++)
			{
				temp_re[i] = X_re[i][j] * pow(-1, i);
				temp_im[i] = X_im[i][j] * pow(-1, i);
			}

			fft(temp_re, temp_im, N);

			for (i = 0; i < N; i++)
			{
				X_re[i][j] = temp_re[i] / N;
				X_im[i][j] = temp_im[i] / N;
			}
		}
	}

	free(temp_re);
	free(temp_im);

	return 0;
}
void fftInit(uchar** img, uchar** res, int Row, int Col, int Mode)
{
	int i, j;
	double** t_img, ** t_tmp;
	t_img = d_alloc(Row, Col);
	t_tmp = d_alloc(Row, Col);

	//결과 이미지 및 입력 이미지를 double로 할당, 대입함
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			t_img[i][j] = (double)img[i][j];
		}

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			t_tmp[i][j] = 0.;
		}

	//2분할 fft 진행, row를 매개변수로 넘김
	fft_2d(t_img, t_tmp, Row, Mode);

	//클리핑동작
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			if (t_img[i][j] > 255.)
				t_img[i][j] = 255.;
			else if (t_img[i][j] < 0.)
				t_img[i][j] = 0.;

			//결과 영상에 연산된 결과 영상 대입
			res[i][j] = t_img[i][j];
		}
}
void Ideal(uchar** img, uchar** res, int Row, int Col)
{
	int i, j, Mode = 2;
	double max = -10E30, min = 10E30;
	double** fourier_img, ** imaginary_img;
	Mat tmp_res(Row, Col, CV_8UC(1));

	fourier_img = (double**)d_alloc(Row, Col);
	imaginary_img = (double**)d_alloc(Row, Col);

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			fourier_img[i][j] = (double)img[i][j];
			imaginary_img[i][j] = 0.;
		}
	fft_2d(fourier_img, imaginary_img, Row, Mode);

	//frequency Filtering Process
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			double diameter;

			diameter = sqrt((Row / 2. - i) * (Row / 2. - i) + (Col / 2. - j) * (Col / 2. - j));

			if (fabs(diameter) < 10)
			{
				fourier_img[i][j] = 0;
				imaginary_img[i][j] = 0;
			}
			else
				imaginary_img[i][j] = -imaginary_img[i][j];
		}
	fft_2d(fourier_img, imaginary_img, Row, Mode-1);

	// Shift Process
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			fourier_img[i][j] = fourier_img[i][j] * pow(-1, i + j);
		}
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			if (max < fourier_img[i][j])
				max = fourier_img[i][j];
			if (min > fourier_img[i][j])
				min = fourier_img[i][j];
			if (fourier_img[i][j] > 255)
				fourier_img[i][j] = 255;
			if (fourier_img[i][j] < 0)
				fourier_img[i][j] = 0;
			tmp_res.at<uchar>(i, j) = (uchar)fourier_img[i][j];
		}
	printf("Fourier Max Min Value = %lf %lf \n", max, min);
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			if (fourier_img[i][j] > 255)
				fourier_img[i][j] = 255;
			if (fourier_img[i][j] < 0)
				fourier_img[i][j] = 0;
			res[i][j] = (uchar)fourier_img[i][j];
		}

}
void Butterworth(uchar** img, uchar** res, int Row, int Col)
{
	int i, j, Mode = 2;
	double max = -10E30, min = 10E30;
	double** fourier_img, ** imaginary_img;
	double** Butterworth;

	fourier_img = (double**)d_alloc(Row, Col);
	imaginary_img = (double**)d_alloc(Row, Col);
	Butterworth = (double**)d_alloc(Row, Col);

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			fourier_img[i][j] = (double)img[i][j];
			imaginary_img[i][j] = 0.;
		}
	fft_2d(fourier_img, imaginary_img, Row, Mode);

	int n = 2;
	int D0 = 50;
	double diameter;

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			diameter = sqrt((Row / 2. - i) * (Row / 2. - i) + (Col / 2. - j) * (Col / 2. - j));

			Butterworth[i][j] = 1 / (1 + pow(diameter / D0, 2 * n));

			fourier_img[i][j] *= Butterworth[i][j];
			imaginary_img[i][j] *= -Butterworth[i][j];
		}
	fft_2d(fourier_img, imaginary_img, Row, Mode - 1);

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			fourier_img[i][j] = fourier_img[i][j] * pow(-1, i + j);
		}
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			if (max < fourier_img[i][j])
				max = fourier_img[i][j];
			if (min > fourier_img[i][j])
				min = fourier_img[i][j];
		}
	printf("Fourier Max Min Value = %lf %lf \n", max, min);

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			fourier_img[i][j] -= min;
		}
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			if (fourier_img[i][j] > 255)
				fourier_img[i][j] = 255;
			if (fourier_img[i][j] < 0)
				fourier_img[i][j] = 0;
			res[i][j] = (double)fourier_img[i][j];
		}
}
void final(uchar** img, uchar** res, int Row, int Col)
{
	int i, j;
	double avg_diff;
	uchar** spa_img, **freq_img, **difference;
	Mat  org_img(Row, Col, CV_8UC(1)), tmp_img(Row, Col, CV_8UC(1));

	spa_img = uc_alloc(Row, Col);
	freq_img = uc_alloc(Row, Col);
	difference = uc_alloc(Row, Col);

	// 원본 영상 convolution
	Filtering(img, res, Row, Col);	
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			spa_img[i][j] = res[i][j];
			tmp_img.at<char>(i, j) = res[i][j];
			org_img.at<char>(i, j) = img[i][j];
		}

	// convolution한 영상을 frequency domain으로 변환시켰다가 다시 spatial domain으로 변환
	//Ideal(img, res, Row, Col);	
	Butterworth(spa_img, res, Row, Col);
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			freq_img[i][j] = res[i][j];
		}
	// 처리한 영상 2개를 비교하여 difference 구하기
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			difference[i][j] = spa_img[i][j] - freq_img[i][j];
		}
	// difference의 평균 구하기 
	avg_diff = average(difference, Row, Col);
	printf("Difference Avg = %lf\n", avg_diff);

	namedWindow("original disp", WINDOW_AUTOSIZE);
	imshow("original disp", org_img);
	namedWindow("convolution disp", WINDOW_AUTOSIZE);
	imshow("convolution disp", tmp_img);
}

int main(int argc, char* argv[])
{
	int i, j, Row, Col, arg, er_cnt=0;
	double arg2;
	uchar** img, ** res;

	if (argc != 5)
	{
		fprintf(stderr,"\n %s InputImg Row Col ResultImg !!\n",argv[0]);
		exit(0);
	}

	Row = atoi(argv[2]);
	Col = atoi(argv[3]);
	img = uc_alloc(Row, Col);
	res = uc_alloc(Row, Col);
	read_ucmatrix(Row, Col, img, argv[1]);

	printf("Select Mode \n0: Basic\n1: Negative\n2: Mosaic\n3: Binary\n4: Gamma Correction\n5: Gamma Average Correction\n6: Circle\n7: Midterm Exam\n8: Filtering\n9: Median Filtering\n10: Fft\n11: Ideal\n12: Butterworth Lowpass\n13: Final Exam\n\n");
	scanf_s("%d", &arg);

	switch (arg)
		{
		case 0:
			printf("Mode: Basic\n");
			for (i = 0; i < Col; i++)
				for (j = 0; j < Row; j++)
					res[i][j] = img[i][j];
			break;
		case 1:
			printf("Mode: Negative\n");
			Negative(Row, Col, img, res);
			break;
		case 2:
			printf("Mode: Mosaic\nInput block: ");
			scanf_s("%lf", &arg2);
			Mosaic(img, res, Row, Col, arg2);
			break;
		case 3:
			printf("Mode: Binary\n");
			arg2 = average(img, Row, Col);
			makeBinary(img, res, Row, Col, arg2);
		case 4:
			printf("Mode: Gamma Correction\nInput: ");
			scanf_s("%lf", &arg2);
			PowImg(img, res, Row, Col, arg2);
			break;
		case 5:
			printf("Mode: Gamma Average Correction\n");
			powAvgImg(img, res, Row, Col);
			break;
		case 6:
			printf("Mode: Circle\nInput diameter: ");
			scanf_s("%lf", &arg2);
			Circle(img, res, Row, Col, arg2);
			break;
		case 7:
			printf("Mode: Midterm Exam\nSelect Detailed Menu\n0: Negative\n1: Mosaic\n2: Binary\nInput: ");
			scanf_s("%lf", &arg2);
			midterm(img, res, Row, Col, arg2);
			break;
		case 8:
			printf("Mode: Filtering\n");
			Filtering(img, res, Row, Col);
			break;
		case 9:
			printf("Mode: Median Filtering\nSelect Mode\n-1: Min\n 0: Median\n 1: Max\n\n");
			scanf_s("%lf", &arg2);
			median(img, res, Row, Col, arg2, 3);
			break;
		case 10:
			printf("Mode: Fft\n");
			fftInit(img, res, Row, Col, 2);
			break;
		case 11:
			printf("Mode: Ideal Filter\n");
			Ideal(img, res, Row, Col);
			break;
		case 12:
			printf("Mode: Butterworth Lowpass Filter\n");
			Butterworth(img, res, Row, Col);
			break;
		case 13:
			final(img, res, Row, Col);
			break;
		default:
			printf("input err(Mode Select)\n");
			er_cnt=1;
			break;
		}

	Mat cvImg(Row, Col, CV_8UC(1));
	/*
		Mat 함수 = opencv에서 가장 기본이 되는 데이터 타입으로 행렬(Matrix) 구조체
		CV_8UC(1)이 자리하고 있는 파라미터에 들어가는 내용은 행렬의 요소가 어떤 타입의 데이터인지 정해준다.
	*/

	if (er_cnt == 0)
	{
		for (i = 0; i < Row; i++)
			for (j = 0; j < Col; j++)
			{
				// (cvImg.data+cvImg.step*i)[j] = img[i][j]; // Think this
				cvImg.at<uchar>(i, j) = res[i][j];		// 출력영상을 기본이미지 파일(img)이 아닌 작업을 처리한 결과이미지(res)로 바꿔줌
			}

		write_ucmatrix(Row, Col, res, argv[4]);
		printf("%s is saved\n", argv[4]);
		free(img);
		free(res);

		namedWindow("Result", WINDOW_AUTOSIZE);
		imshow("Result", cvImg);
	}
	else
		er_cnt = 0;

	waitKey(0);
	return 0;
}
