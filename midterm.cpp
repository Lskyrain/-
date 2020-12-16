#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdio.h>

typedef unsigned char uchar;

using namespace std;
using namespace cv;

uchar** uc_alloc(int size_x, int size_y)		// 메모리 할당받는 함수
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
double average(uchar** img, int Row, int Col)	// 감마 평균 정정을 위한 평균값 구하는 함수
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

double powAvgImg(uchar** img, uchar** res, int Row, int Col)	// 감마 평균 정정 함수
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

					tmp = sqrt(ySquare + xSquare);	// tmp -> 각각의 픽셀이 한가운데의 중앙 점으로부터 떨어져 있는 정도.
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

int main(int argc, char* argv[])
{
	
	int i, j, Row, Col, arg, er_cnt=0;
	double arg2;
	uchar** img, ** res, **ref;

	if (argc != 5)
	{
		fprintf(stderr,"\n %s InputImg Row Col ResultImg !!\n",argv[0]);
		exit(0);
	}

	Row = atoi(argv[2]);
	Col = atoi(argv[3]);
	img = uc_alloc(Row, Col);
	res = uc_alloc(Row, Col);
	ref = uc_alloc(Row, Col);
	read_ucmatrix(Row, Col, img, argv[1]);

	printf("Select Mode \n0: Basic\n1: Negative\n2: Mosaic\n3: Binary\n4: Gamma Correction\n5: Gamma Average Correction\n6: Circle\n7: Midterm Exam\n\n");
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
		default:
			printf("input err\n");
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
				cvImg.at<uchar>(i, j) = res[i][j];	// 출력영상을 기본이미지 파일(img)이 아닌 작업을 처리한 결과이미지(res)로 바꿔줌
			}

		write_ucmatrix(Row, Col, res, argv[4]);
		printf("%s is saved\n", argv[4]);
		free(img);
		free(res);

		namedWindow(argv[1], WINDOW_AUTOSIZE);
		imshow(argv[1], cvImg);
	}
	else
		er_cnt = 0;

	waitKey(0);
	return 0;
}
