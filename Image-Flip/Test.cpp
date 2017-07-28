#include <opencv2\core\core.hpp>  
#include <opencv2\imgproc\imgproc.hpp>  
#include <opencv2\opencv.hpp>
#include <iostream>
#include <opencv2\highgui\highgui.hpp>
#include <cmath>
using namespace std;
using namespace cv;
//src为原图像， dst为新图像, angle为旋转角度(正值为顺时针旋转,负值为逆时针旋转)  
Mat imageRotate1(Mat src, double angle)
{
	Mat input = src;
	Mat dst;
	if (input.empty()) {
		cout << "error" << endl;
	}

	//得到图像大小  
	int width = input.cols;
	int height = input.rows;

	//计算图像中心点  
	Point2f center;
	center.x = width / 2.0;
	center.y = height / 2.0;
	cout << center.x << endl;
	cout << center.y << endl;
	cout << center << endl;
	//获得旋转变换矩阵  
	double scale = 1.0;
	Mat trans_mat = getRotationMatrix2D(center, -angle, scale);//得到仿射变换矩阵  
	                                                           //得到的trans_mat是一个2*3矩阵
	                                                           //前两列是变换矩阵，最后一列是b1,b2这么一个线性变换的参数
	                                                           //知识点可以在CSDN中opencv基础知识中寻找
	cout << trans_mat << endl;

	//计算新图像大小  
	double angle1 = angle  * CV_PI / 180.;
	double a = sin(angle1) * scale;
	double b = cos(angle1) * scale;
	double out_width = height * fabs(a) + width * fabs(b);     //fabs来自<cmath>
	double out_height = width * fabs(a) + height * fabs(b);
	
	//在旋转变换矩阵中加入平移量  
	trans_mat.at<double>(0, 2) += cvRound((out_width - width) / 2);  //返回的是中心点的地址，所以旋转模型的中心点平移
	trans_mat.at<double>(1, 2) += cvRound((out_height - height) / 2);
	
	//仿射变换  
	warpAffine(input, dst, trans_mat, Size(out_width, out_height));   //要求trans_mat必须是2*3矩阵，其他还有一些参数
	                                                                  //可以在VS中勾选函数，查看定义

	//仿射后图像中心未变，需要重新确定



	return dst;
}
int main(){
	Mat src = imread("F:\\1.jpg", 1);
	Mat output;
	output=imageRotate1(src,60);
	namedWindow("win1");
	imshow("win1",output);
	waitKey(0);
}
