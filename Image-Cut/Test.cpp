#include <opencv2\core\core.hpp>  
#include <opencv2\imgproc\imgproc.hpp>  
#include <opencv2\opencv.hpp>
#include <iostream>
#include <opencv2\highgui\highgui.hpp>
#include <cmath>


using namespace std;
using namespace cv;


//图像剪切  
//参数：src为源图像， dst为结果图像, rect为剪切区域  
//返回值：返回0表示成功，否则返回错误代码  
int imageCrop(InputArray src, OutputArray dst, Rect rect)
{
	Mat input = src.getMat();
	if (input.empty()) {
		return -1;
	}

	//计算剪切区域：  剪切Rect与源图像所在Rect的交集  
	Rect srcRect(0, 0,     //矩形的初始坐标
		input.cols, input.rows);    //创建的矩形框的大小，矩形对角两端点
	rect = rect & srcRect;
	cout << rect << endl;
	if (rect.width <= 0 || rect.height <= 0){
		return -2;
	}

	//创建结果图像  
	dst.create(Size(rect.width, rect.height), src.type());
	cout << dst.getMat() << endl;
	Mat output = dst.getMat();  //此即剪切，前面是得到想要部分,但是没有赋值
	if (output.empty()) return -1;

	try {      //try catch捕捉异常
		//复制源图像的剪切区域 到结果图像  
		input(rect).copyTo(output);    //真赋值
		cout << output << endl;
		return 0;
	}
	catch (...) {
		return -3;
	}
}

//========================  主程序开始 ==========================  

static string window_name = "Draw a Rect to crop";
static Mat src;  //源图片  
bool  isDrag = false;
Point point1; //矩形的第一个点  
Point point2; //矩形的第二个点  

static void callbackMouseEvent(int mouseEvent, int x, int y, int flags, void* param)   //鼠标响应事件的常见的参数设定
{
	if (mouseEvent == CV_EVENT_MOUSEMOVE)
	{
		cout << "触发鼠标移动事件" << endl;
	}
	switch (mouseEvent) {

	case CV_EVENT_LBUTTONDOWN:  //左键按下
		point1 = Point(x, y);
		point2 = Point(x, y);
		isDrag = true;          //此处改变isDrag的值结束，响应函数实在不断调用的过程，不改变则进入函数没有处理
		break;

	case CV_EVENT_MOUSEMOVE:    //鼠标移动
		if (isDrag) {
			point2 = Point(x, y);
			Mat dst = src.clone();
			Rect rect(point1, point2); //得到矩形  
			rectangle(dst, rect, Scalar(0, 0, 255));//画矩形  
			imshow(window_name, dst); //显示图像  此处显示是那个红色线框，还是大图，结果原图加矩形
		}
		break;

	case CV_EVENT_LBUTTONUP:    //左键抬起
		if (isDrag) {
			isDrag = false;
			Rect rect(point1, point2); //得到矩形  
			imageCrop(src, src, rect); //图像剪切  
			imshow(window_name, src); //显示图像  
		}
		break;

	}

	return;
}


int main()
{
	//read image file  
	src = imread("F:\\1_test.jpg");
	if (!src.data) {
		cout << "error read image" << endl;
		return -1;
	}

	//create window  
	namedWindow(window_name);
	imshow(window_name, src);

	//set mouse event call back  
	setMouseCallback(window_name,//鼠标触发窗口响应的窗口名
		callbackMouseEvent,   //鼠标响应函数，监视到鼠标操作后调用并处理相应动作 
		NULL);                //鼠标响应处理函数的ID，识别号  类型名void *userdata

	waitKey();

	return 0;

}
