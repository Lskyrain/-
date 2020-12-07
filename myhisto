/*
	직접 짜본 histogram 함수 코드들
*/

#include <opencv2/opencv.hpp>
#include <iostream>

typedef unsigned char uchar;

using namespace std;
using namespace cv;

void getHisto(uchar** img, int Row, int Col)
{
	int i, j;

	Mat himg(Row, Col, CV_8UC(1));
	MatND histogram;

	const int* channel_numbers = { 0 };
	float channel_range[] = { 0.0, 255.0 };
	const float* channel_ranges = channel_range;
	int number_bins = 255;

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			himg.at<uchar>(i, j) = img[i][j];
		}

	calcHist(&himg, 1, channel_numbers, Mat(), histogram, 1, &number_bins, &channel_ranges);

	int hist_w = himg.cols;
	int hist_h = himg.rows;
	int bin_w = cvRound((double)hist_w / number_bins);

	Mat hist_img(hist_h, hist_w, CV_8UC1, Scalar::all(0));

	// 0부터 히스토그램 행 크기로 값들의 범위 변환
	normalize(histogram, histogram, 0, hist_img.rows, NORM_MINMAX);

	for (int i = 1; i < number_bins; i++)
	{
		// line함수: 선을 그려준다. ; 파라미터 값은 차례대로 영상Mat, 좌표점1, 좌표점2, 색상, 두께, 타입(CV_AA 이면 안티에일리징 라인), 좌표 시프트연산
		// line(Mat& img, Point pt1, Point pt2, const Scalar& color, int thickness, int lineType, int shift);
		line(hist_img, Point(bin_w * (i - 1), hist_h - cvRound(histogram.at<float>(i - 1))), Point(bin_w * (i), hist_h - cvRound(histogram.at<float>(i))), Scalar(255, 0, 0), 1, 8, 0);
	}

	free(img);

	namedWindow("Original", WINDOW_AUTOSIZE);
	imshow("Original", himg);
	namedWindow("pdf", WINDOW_AUTOSIZE);
	imshow("pdf", hist_img);
	waitKey(0);
}
void HistoEqual(uchar** img, int Row, int Col)
{
	int i, j;

	Mat himg(Row, Col, CV_8UC(1));
	Mat himg_equal(Row, Col, CV_8UC(1));

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			himg.at<uchar>(i, j) = img[i][j];
		}

	equalizeHist(himg, himg_equal);

	namedWindow("Original", WINDOW_AUTOSIZE);
	imshow("Original", himg);
	namedWindow("Histogram Equalizing", WINDOW_AUTOSIZE);
	imshow("Histogram Equalizing", himg_equal);
	waitKey(0);
}

void HistoMatch(uchar** img, uchar** ref, int Row, int Col, Mat& result)
{
	int i, j;

	Mat target(Row, Col, CV_8UC(1));
	Mat refer(Row, Col, CV_8UC(1));
	Mat original(Row, Col, CV_8UC(1));
	Mat reference(Row, Col, CV_8UC(1));;

	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			target.at<uchar>(i, j) = img[i][j];
			original.at<uchar>(i, j) = img[i][j];
		}
	for (i = 0; i < Row; i++)
		for (j = 0; j < Col; j++)
		{
			refer.at<uchar>(i, j) = ref[i][j];
			reference.at<uchar>(i, j) = ref[i][j];
		}

	const float HISMATCH = 0.000001;	// thresold of histogram difference
	double min, max;

	int histSize = 256;	// histogram을 256 level로 작성
	float range[] = { 0, 256 };	// 최소와 최대 bin
	const float* histRange = { range };
	bool uniform = true;

	Mat ref_hist, tgt_hist; // 생성된 히스토그램 저장
	Mat ref_hist_accum, tgt_hist_accum; // 히스토그램 CDF계산

	calcHist(&refer, 1, 0, Mat(), ref_hist, 1, &histSize, &histRange, uniform);
	calcHist(&target, 1, 0, Mat(), tgt_hist, 1, &histSize, &histRange, uniform);

	minMaxLoc(ref_hist, &min, &max);
	if (max == 0)
	{
		cout << "ERROR: max is 0 in ref_hist" << endl;
	}
	normalize(ref_hist, ref_hist, min / max, 1.0, NORM_MINMAX);

	//CDF계산
	ref_hist.copyTo(ref_hist_accum);
	tgt_hist.copyTo(tgt_hist_accum);

	float* src_cdf_data = ref_hist_accum.ptr<float>();
	float* dst_cdf_data = tgt_hist_accum.ptr<float>();

	for (j = 0; j < 256; j++)
	{
		src_cdf_data[j] += src_cdf_data[j - 1];
		dst_cdf_data[j] += dst_cdf_data[j - 1];
	}

	//계산된 CDF를 normalize
	minMaxLoc(ref_hist_accum, &min, &max);
	normalize(ref_hist_accum, ref_hist_accum, min / max, 1.0, NORM_MINMAX);
	minMaxLoc(tgt_hist_accum, &min, &max);
	normalize(tgt_hist_accum, tgt_hist_accum, min / max, 1.0, NORM_MINMAX);

	// Histogram Matching
	Mat lut(1, 256, CV_8UC(1));	// Lookup Table을 만든다
	uchar* M = lut.ptr<uchar>();
	uchar last = 0;

	for (j = 0; j < tgt_hist_accum.rows; j++)
	{
		float F1 = dst_cdf_data[j];

		//참조 이미지에서 유사한 CDF값을 갖는 픽셀 intensity를 찾는다
		for (uchar k = last; k < ref_hist_accum.rows; k++)
		{
			float F2 = src_cdf_data[k];
			if (abs(F2 - F1) < HISMATCH || F2 > F1)
			{
				M[j] = k;	// 변경대상 이미지의 intensity j는 intensity k
				last = k;	// 다음 검색을 시작할 위치
				break;	// 다음 intensity로
			}
		}
	}
	LUT(target, lut, target);	// Lookup Table을 이용한 색깔 변경

	namedWindow("target", WINDOW_AUTOSIZE);
	imshow("target", original);
	namedWindow("reference", WINDOW_AUTOSIZE);
	imshow("reference", reference);
	//namedWindow("Reuslt", WINDOW_AUTOSIZE);
	imshow("Result", target);
	waitKey(0);
}
