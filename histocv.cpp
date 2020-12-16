#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

typedef unsigned char uchar;
using namespace std;
using namespace cv;

int histogram[256], cdfOfHisto[256], histogramEqual[256];

uchar **uc_alloc(int size_x, int size_y)
{
	uchar **m;
	int i;

	if ((m =(uchar **)calloc(size_y, sizeof(uchar *)))==NULL) 
	{
		printf("d_alloc error 1\7\n");
		exit(0);
	}
	for (i=0; i<size_y; i++)
		if ((m[i] =(uchar *)calloc(size_x, sizeof(uchar)))==NULL) 
		{
			printf("d_alloc error 2\7\n");
			exit(0);
		}
	return m;
}

void read_ucmatrix(int size_x, int size_y, uchar **ucmatrix, const char *filename)
{
	int i;
	FILE *f;

	if ((f =fopen(filename, "rb")) == NULL) 
	{
		printf("%s File open Error!\n", filename);
		exit(0);
	}
	for (i=0; i<size_y; i++)
		if (fread(ucmatrix[i], sizeof(uchar), size_x, f) != size_x) 
		{
			printf("Data Read Error!\n");
			exit(0);
		}
	fclose(f);
}	


void write_ucmatrix(int size_x, int size_y, uchar **ucmatrix, const char *filename)
{
	int i;
	FILE *f;

	if ((f =fopen(filename, "wb")) == NULL) 
	{
		printf("%s File open Error!\n", filename);
		exit(0);
	}
	for (i=0; i<size_y; i++)
		if (fwrite(ucmatrix[i], sizeof(uchar), size_x, f) != size_x) 
		{
			printf("Data Write Error!\n");
			exit(0);
		}
	fclose(f);
}	

void get_hist1(uchar **img, int Col, int Row)
{
  	int i,j, tmp;
	double tmp1;
  	int t, tp, range, hrange;

	Mat imgHisto(Row, Col, CV_8UC(1));
	Mat cdfImgHisto(Row, Col, CV_8UC(1));

	for(i=0;i<Row;i++)
	for(j=0;j<Col;j++)
	{
		imgHisto.at<uchar>(i,j)= 0;
		cdfImgHisto.at<uchar>(i,j)= 0;
	}

  	tp = Col*Row;

  	for (i = 0; i < 256; i++)
    		histogram[i] = 0;

  	for (i = 0; i < Row; i++)
    	for (j = 0; j < Col; j++)
      		histogram[img[i][j]]++;

	// Find the maximum histogram value
	tmp1 =0;
  	for (i = 0; i < 256; ++i)
	{
		tmp1 = tmp1 > histogram[i] ? tmp1 : histogram[i];
	}

  	for (i = 0; i < 256; ++i)
	{
		tmp = (int)255.*(histogram[i]/tmp1);
		line(imgHisto,Point(i,255),Point(i,256-tmp),CV_RGB(255,255,255),1,8,0);
	}

//	NamedWindow("Histo Equal",2);
	imshow("Histo Equal ",imgHisto);

  	cdfOfHisto[0] = histogram[0];
  	for (i = 1; i < 256; i++)
	{
        	cdfOfHisto[i] = cdfOfHisto[i-1] + histogram[i];
	}

	// Draw the CDF of Histogram 
	tmp1 = (double)cdfOfHisto[255];
  	for (i = 0; i < 256; ++i)
	{
		tmp = (int)(Col-1)*(cdfOfHisto[i]/tmp1);
		line(cdfImgHisto,Point(i,Col-1),Point(i,Col-tmp),CV_RGB(255,255,255),1,8,0);
	}
	imshow("CDF of Histogram ",cdfImgHisto);

//  printf("****\n");
 /* range = tp - histogram[0];
 */ 

	range = cdfOfHisto[255] - cdfOfHisto[0];
 // 	printf("%d %d\n",tp, range);


  	histogramEqual[0] = 0;
  	for (i = 1; i < 256; ++i) {
    		t = (int)ceil(((cdfOfHisto[i] - cdfOfHisto[0]) * 255.0) / range);
    		histogramEqual[i] = (t < 0) ? 0 : (t > 255) ? 255 : t;
  	}


  	for (i = 0; i < Row; ++i)
    	for (j = 0; j < Col; ++j)
      		img[i][j] = histogramEqual[img[i][j]];
}


void get_hist(uchar **img, int Col, int Row)
{
  	int i,j, tmp;
	double tmp1;
  	int t, tp, range, hrange;

	Mat imgHisto(Row, Col, CV_8UC(1));
	Mat cdfImgHisto(Row, Col, CV_8UC(1));

	for(i=0;i<Row;i++)
	for(j=0;j<Col;j++)
	{
		imgHisto.at<uchar>(i,j)= 0;
		cdfImgHisto.at<uchar>(i,j)= 0;
	}

  	tp = Col*Row;

  	for (i = 0; i < 256; i++)
    		histogram[i] = 0;

  	for (i = 0; i < Row; i++)
    	for (j = 0; j < Col; j++)
      		histogram[img[i][j]]++;

	tmp1 =0;
  	for (i = 0; i < 256; ++i)
	{
		tmp1 = tmp1 > histogram[i] ? tmp1 : histogram[i];
	}

  	for (i = 0; i < 256; ++i)
	{
		tmp = (int)255*(histogram[i]/tmp1);
		line(imgHisto,Point(i,255),Point(i,256-tmp),CV_RGB(255,255,255),1,8,0);
	}

//	cvNamedWindow("HistoLine",1);
	imshow("Histo Line ",imgHisto);

  	cdfOfHisto[0] = histogram[0];
  	for (i = 1; i < 256; i++)
        	cdfOfHisto[i] = cdfOfHisto[i-1] + histogram[i];

	// Draw the CDF of Histogram 
	tmp1 = (double)cdfOfHisto[255];
  	for (i = 0; i < 256; ++i)
	{
		tmp = (int)(Col-1)*(cdfOfHisto[i]/tmp1);
		line(cdfImgHisto,Point(i,Col-1),Point(i,Col-tmp),CV_RGB(255,255,255),1,8,0);
	}
	imshow("Original CDF of Histogram ",cdfImgHisto);

//  printf("****\n");
 /* range = tp - histogram[0];
 */ 

	range = cdfOfHisto[255] - cdfOfHisto[0];
  //	printf("%d %d\n",tp, range);


  	histogramEqual[0] = 0;
  	for (i = 1; i < 256; ++i) {
    		t = (int)ceil(((cdfOfHisto[i] - cdfOfHisto[0]) * 255.0) / range);
    		histogramEqual[i] = (t < 0) ? 0 : (t > 255) ? 255 : t;
  	}


  	for (i = 0; i < Row; ++i)
    	for (j = 0; j < Col; ++j)
      		img[i][j] = histogramEqual[img[i][j]];
}

int main(int argc, char *argv[])
{
	int i,j;
	int Row, Col;
	uchar **img;

	if(argc !=4)
	{
		printf("Exe imgData x_size(Column)  y_size(Row) \n");
		exit(0);
	}

	Col = atoi(argv[2]);
	Row = atoi(argv[3]);
	img = uc_alloc(Col,Row);
	read_ucmatrix(Col,Row,img,argv[1]);
	
	Mat cvImg(Row, Col, CV_8UC(1));

	for(i=0;i<Row;i++)
	for(j=0;j<Col;j++)
	{
		cvImg.at<uchar>(i,j)= img[i][j];
	}

	namedWindow(argv[1],WINDOW_AUTOSIZE);
	imshow(argv[1],cvImg);
//	printf("Please Wait\n");
//	getchar();
	get_hist(img, Col, Row);

	for(i=0;i<Row;i++)
	for(j=0;j<Col;j++)
	{
		cvImg.at<uchar>(i,j)= img[i][j];
	}

	imshow("HistoGram Equalizer...",cvImg);

	get_hist1(img, Col, Row);

	waitKey(0);
	return 0;
}
