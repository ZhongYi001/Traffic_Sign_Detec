#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <string>

using namespace cv;  //包含cv命名空间  
using namespace std;
string filename = "..//sign02.avi";

Mat Sign(Mat srcImage);

bool cmp(double &num, const double &s, const double &l)
{
	if (num <= l && num >= s)
		return true;
	else
		return false;
}


int main()
{
	Mat frame;
	VideoCapture capture( filename);
	long totalframeNum = capture.get(CV_CAP_PROP_FRAME_COUNT);
	if (capture.isOpened())
	{
		for (size_t i = 0; i <= totalframeNum; i++)
		{
			size_t j = i;
			capture >> frame;
			if (frame.empty())
			{
				break;
			}
			else if (j != 0)
			{
				//double time0 = static_cast<double>(getTickCount());
				frame = Sign(frame);
				//time0 = ((double)getTickCount() - time0) / getTickFrequency();
				//cout << "单次检测运行时间为：" << time0 << "秒" << endl;
			}

			int c = waitKey(10);
			if ((char)c == 27)
			{
				break;
			}

		}
	}
}

Mat Sign(Mat srcImage)
{
	Mat output = srcImage.clone();
	Mat imageRGB[3];
	split(srcImage, imageRGB);
	for (int i = 0; i < 3; i++)
	{
		equalizeHist(imageRGB[i], imageRGB[i]);
	}
	merge(imageRGB, 3, srcImage);
	Mat ROI = srcImage(Rect(Point(srcImage.cols / 3, 0), Point(srcImage.cols, srcImage.rows / 2)));
	Mat HSVImage;
	cvtColor(srcImage, HSVImage, COLOR_BGR2HSV);
	//imshow("【HSV】", HSVImage);
	Mat resultGray = Mat(HSVImage.rows, HSVImage.cols, CV_8U, cv::Scalar(0));
	//imshow("[2]", resultGray);
	Mat resultColor = Mat(HSVImage.rows, HSVImage.cols, CV_8UC3, cv::Scalar(255, 255, 255));
	double H = 0.0, S = 0.0, V = 0.0;
	int S_Min1 = 120,
		S_Max1 = 255,
		S_Min2 = 50,
		S_Max2 = 60,
		H_Min1 = 0,
		H_Min2 = 145,
		H_Max1 = 5,
		H_Max2 = 149,
		V_Min = 90,
		V_Max = 250;
	for (int i = 0; i < HSVImage.rows / 2; i++)
	{
		for (int j = HSVImage.cols / 3; j < HSVImage.cols; j++)
		{
			H = HSVImage.at<Vec3b>(i, j)[0];
			V = HSVImage.at<Vec3b>(i, j)[1];
			S = HSVImage.at<Vec3b>(i, j)[2];

			//((H >= 170) || (H <= 10 && S >= 40 && V >= 30)

			/*if (V==255)*/
			/*if ((S >= 70 && S<155) || (S >= 35 && S<65))*/
			/*if ((H >= H_Min1 && H <= H_Max1) || (H >= H_Min2 && H <= H_Max2))*/
			if (
				//(H >= 6 & H <= 29 & (S >= 120) && ((V >= 55 & V <= 59) || (V >= 65 && H <= 79) || (V >= 85 & V <= 179))) ||		//yellow
				//((H >= 170) || (H <= 10 && S >= 40 && V >= 30))
				(H >= 6 & H <= 29 & (S >= 120) && ((V >= 55 & V <= 59) || (V >= 65 && H <= 79) || (V >= 85 & V <= 179))) ||		//yellow
				((H >= 170) || (H <= 10 && S >= 40 && V >= 30)) ||						// red
				(cmp(H, 96, 120) && cmp(S, 180, 254) && cmp(V, 185, 254))						//blue

				)
			{
				/*if ((H >= 0 && H < 24) && V >= 215)*/
				/*if ((S >= S_Min1 && S <= S_Max1) || (S >= S_Min2 && S <= S_Max2))*/
				{
					/*if (V >= V_Min && V <= V_Max)*/
					{
						resultGray.at<uchar>(i, j) = 255;
						resultColor.at<Vec3b>(i, j)[0] = srcImage.at<Vec3b>(i, j)[0];
						resultColor.at<Vec3b>(i, j)[1] = srcImage.at<Vec3b>(i, j)[1];
						resultColor.at<Vec3b>(i, j)[2] = srcImage.at<Vec3b>(i, j)[2];
					}
				}

			}
		}
	}
	//Mat after_inrange;
	//inRange(HSVImage, Scalar(0, 200, 0), Scalar(5, 254, 5), after_inrange);
	//imshow("【提取后】", resultGray);
	Mat Bin_img;
	threshold(resultGray, Bin_img, 0, 255, CV_THRESH_OTSU);
	//imshow("【二值化】", Bin_img);
	Mat Blured_img;
	GaussianBlur(Bin_img, Blured_img, Size(7, 7), 2, 2);
	//imshow("【滤波后】", Blured_img);
	Mat Morph_img;
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5), Point(2, 2));
	morphologyEx(Bin_img, Morph_img, CV_MOP_OPEN, element);
	//imshow("【开运算】", Morph_img);
	vector<Vec4i>hierarchy(10000);
	vector<Mat>contours(10000);//手动分配内存空间大小
	findContours(Morph_img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(Morph_img, contours, i, Scalar::all(255), CV_FILLED, 8);
	}
	imshow("【绘制后】", Morph_img);
	Mat Split_img = Mat(8, 8, CV_8U, cv::Scalar(0));
	//imshow("[3]", Split_img);
	int rows_start, cols_start, N = 0;
	double R, Rt;
	R = 0.65;
	for (int n = 0; n < ROI.rows / 8; n++)
	{
		rows_start = n * 8;
		for (int m = 0; m < ROI.cols / 8; m++)
		{
			cols_start = m * 8 + srcImage.cols / 3;
			//cout << n+1 << "*" << m+1 << "times" << endl;
			Mat Morph_ROI = Morph_img(Rect(Point(cols_start, rows_start), Point(cols_start + Split_img.cols, rows_start + Split_img.rows)));
			for (int i = 0; i < Split_img.rows; i++)
			{
				for (int j = 0; j < Split_img.cols; j++)
				{
					Split_img.at<uchar>(i, j) = Morph_img.at<uchar>(i + rows_start, j + cols_start);
					Morph_img.at<uchar>(i + rows_start, j + cols_start) = 0;
					if (Split_img.at<uchar>(i, j) == 255)
					{
						N++;
						//cout << N << endl;
					}
				}
			}
			Rt = (double)N / (Split_img.rows * Split_img.cols);
			N = 0;
			if (Rt > R)
			{
				for (int i = 0; i < Split_img.rows; i++)
				{
					for (int j = 0; j < Split_img.cols; j++)
					{
						Morph_ROI.at<uchar>(i, j) = 255;
					}
				}
				/*rectangle(ROI, Point( m * Split_img.cols, n * Split_img.rows), Point( (m+1) * Split_img.cols, (n+1)* Split_img.rows), Scalar(0, 255, 255),2);*/
				//cout << 1 << endl;
			}
			//cout << Rt << endl;
		}
		//imwrite(file_name, Split_img);
	}
	int width = 0;
	int height = 0;
	int x = 0;
	int y = 0;
	findContours(Morph_img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
	vector<Rect> boundRect(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		boundRect[i] = boundingRect(Mat(contours[i])); 
		double area = contourArea(contours[i]);
		//if (area > 500 && area < 3000)
		{
			/*RotatedRect rect = minAreaRect(contours[i]);*/
			//Point2f P[4];
			//rect.points(P);
			//for (int j = 0; j <= 3; j++)
			//{
			//	line(srcImage, P[j], P[(j + 1) % 4], Scalar(255), 2);
			//}
			boundRect[i] = boundingRect(Mat(contours[i]));
			width = boundRect[i].width;
			height = boundRect[i].height;
			x = boundRect[i].x;
			y = boundRect[i].y;
			//if ( (width/height >0.8) && (width/height < 1.2))
			//{
				rectangle(output, Rect(x, y, width, height), Scalar(0, 255, 255), 3, 8);
			//}

			
		}
	}
	imshow("1", output);


	return output;
}