#include <opencv2\core\core.hpp>  
#include <opencv2\imgproc\imgproc.hpp>  
#include <opencv2\opencv.hpp>
#include <iostream>
#include <opencv2\highgui\highgui.hpp>
#include <cmath>
using namespace std;
using namespace cv;
//bool isClip = true;
#define SHOW_LINE  

#define BASE 100  

static string source_window = "source";
static string window_name = "image rotate";
static Mat src;
static int rotateDegree = 0 + BASE;
static int clip = 0;
//src为原图像， dst为新图像, angle为旋转角度(正值为顺时针旋转,负值为逆时针旋转) 
/**
* 智能检测图像倾斜度
* 返回值：返回0表示无检测结果，返回非0表示摆正图象需要旋转的角度（-10至10度）
*/
double detectRotation(InputArray src)
{
	double max_angle = 6; //可旋转的最大角度  

	Mat in = src.getMat();
	if (in.empty()) return 0;

	Mat input;

	//转为灰度图  
	if (in.type() == CV_8UC1)
		input = in;
	else if (in.type() == CV_8UC3)
		cvtColor(in, input, CV_BGR2GRAY);
	else if (in.type() == CV_8UC3)
		cvtColor(in, input, CV_BGRA2GRAY);
	else
		return 0;

	Mat dst, cdst;

	//执行Canny边缘检测(检测结果为dst, 为黑白图)  
	double threshold1 = 90;
	Canny(src, 
		dst,        //单通道图像
		threshold1, //下限阈值，
		threshold1 * 3, //上限阈值
		3);          //sobel算子模型

	//将Canny边缘检测结果转化为彩色图像(cdst)  
	cvtColor(dst, cdst, CV_GRAY2BGR);  //是三通道
	int channels = dst.channels();//()不能丢
	int channels1 = cdst.channels();
	cout << channels1 << endl;
	cout << channels << endl;
	//执行霍夫线变换，检测直线  
	vector<Vec4i> lines; //存放检测结果的vector（是一个四元组）。 注意到拟合的结果Vec4i类型的line中的前两个值 给出的是直线的方向的单位向量，后两个值给出的是该直线通过的一个点。
	double minLineLength = std::min(dst.cols, dst.rows) * 0.25; //最短线长度  
	double maxLineGap = std::min(dst.cols, dst.rows) * 0.03; //最小线间距  
	int threshold = 90;
	HoughLinesP(dst,      //输入图像
		lines,            //得到输出直线为四元组，所以是vector<Vec4i> lines，得到是直线两个端点的四个坐标值
		1,                //距离的分辨率
		CV_PI / 180,      //角度的分辨率
		threshold,        //点的阈值，只取大于阈值的点来得到直线
		minLineLength,    //直线阈值，选大于此的直线
		maxLineGap);      //间断线段阈值，判断是两条线还是一条线

	//分析所需变量  
	int x1, y1, x2, y2; //直线的两个端点  
	int x, y;  //直线的中点  
	double angle, rotate_angle; //直线的角度，摆正直线需要旋转的角度  
	double line_length; //直线长度  
	double position_weighted; //直线的位置权重：靠图像中央的线权重为1, 越靠边的线权重越小  
	double main_lens[2]; //用于存放最长的二条直线长度的数组 (这两条直线即是主线条)  
	double main_angles[2];//用于存放最长的二条直线的摆正需要旋转的角度  
	main_lens[0] = main_lens[1] = 0;      //初始化
	main_angles[0] = main_angles[1] = 0;

	//逐个分析各条直线，判断哪个是主线条  
	for (size_t i = 0; i < lines.size(); i++) {
		//取得直线的两个端点座标  
		x1 = lines[i][0]; y1 = lines[i][1]; x2 = lines[i][2]; y2 = lines[i][3];  //vector的每一个元素都是四元组，所以既可以说是一维的又可以说是四维的
		x = (x1 + x2) / 2; y = (y1 + y2) / 2;//知道一个点，知道一个向量。
		//计算直线的角度  
		angle = (x1 == x2) ? 90 : (atan((y1 - y2) * 1.0 / (x2 - x1))) / CV_PI * 180;
		//摆正直线需要旋转的角度. 如果超出可旋转的最大角度,则忽略这个线。  
		if (fabs(angle - 0) <= max_angle) {
			rotate_angle = angle - 0;
		}
		else if (fabs(angle - 90) <= max_angle) {
			rotate_angle = angle - 90;
		}
		else {
			continue;
		}

		//计算线的长度  
		line_length = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
		//计算直线的位置权重：靠图像中央的线权重为1, 越靠边的线权重越小  
		position_weighted = 1;
		if (x < dst.cols / 4 || x > dst.cols * 3 / 4) position_weighted *= 0.8;
		if (x < dst.cols / 6 || x > dst.cols * 5 / 6) position_weighted *= 0.5;
		if (x < dst.cols / 8 || x > dst.cols * 7 / 8) position_weighted *= 0.5;
		if (y < dst.rows / 4 || y > dst.rows * 3 / 4) position_weighted *= 0.8;
		if (y < dst.rows / 6 || y > dst.rows * 5 / 6) position_weighted *= 0.5;
		if (y < dst.rows / 8 || y > dst.rows * 7 / 8) position_weighted *= 0.5;

		//如果 直线长度 * 位置权重 < 最小长度， 则这条线无效  
		line_length = line_length * position_weighted;
		if (line_length < minLineLength) continue;



		//如果长度为前两名，则存入数据  
		if (line_length > main_lens[1])  {
			if (line_length > main_lens[0]) {
				main_lens[1] = main_lens[0];
				main_lens[0] = line_length;
				main_angles[1] = main_angles[0];
				main_angles[0] = rotate_angle;
				//如果定义了 SHOW_LINE, 则将该线条画出来  
#ifdef SHOW_LINE                  //此函数作用防止双重定义，并且使用此定义仅在#ifdef  #endif内部使用，好处是开头定义一下，之后不断调用即可，和函数类似 
				line(cdst,        //要画线的图像
					Point(x1, y1),  //线的两个端点
					Point(x2, y2), 
					Scalar(0, 0, 255), //颜色，Scalar初始化赋值
					3,                //线条的粗细
					CV_AA);           //线条的画法，比如四邻画，八邻画，
				                       //还有一个参数，坐标小数点
#endif  
			}
			else {
				main_lens[1] = line_length;
				main_angles[1] = rotate_angle;
			}
		}
	}

	//如果定义了 SHOW_LINE, 则在source_window中显示cdst  
#ifdef SHOW_LINE  
	imshow(source_window, cdst);
#endif  

	//最后，分析最长的二条直线，得出结果  
	if (main_lens[0] > 0) {
		//如果最长的线 与 次长的线 两者长度相近，则返回两者需要旋转的角度的平均值  
		if (main_lens[1] > 0 && (main_lens[0] - main_lens[1] / main_lens[0] < 0.2)) {
			return (main_angles[0] + main_angles[1]) / 2;
		}
		else {
			return main_angles[0];   //否则，返回最长的线需要旋转的角度  
		}
	}
	else {
		return 0;
	}
}
Mat imageRotate1(Mat src, double angle)
{
	bool isClip = true;
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
	double out_width = height * fabs(a) + width * fabs(b);     //fabs来自<cmath>，作用是对double取绝对值
	double out_height = width * fabs(a) + height * fabs(b);
	

	int new_width, new_height;
	if (!isClip) {
		new_width = cvRound(out_width);
		new_height = cvRound(out_height);
	}
	else {
		//calculate width and height of clip rect  
		double angle2 = fabs(atan(height * 1.0 / width)); //即角度 b  
		double len = width * fabs(b);
		double Y = len / (1 / fabs(tan(angle1)) + 1 / fabs(tan(angle2)));
		double X = Y * 1 / fabs(tan(angle2));
		new_width = cvRound(out_width - X * 2);
		new_height = cvRound(out_height - Y * 2);
	}
	//在旋转变换矩阵中加入平移量  
	trans_mat.at<double>(0, 2) += cvRound((new_width - width) / 2);  //返回的是中心点的地址，所以旋转模型的中心点平移
	trans_mat.at<double>(1, 2) += cvRound((new_height - height) / 2);
	
	//仿射变换  
	warpAffine(input, dst, trans_mat, Size(new_width, new_height));   //要求trans_mat必须是2*3矩阵，其他还有一些参数
	                                                                  //可以在VS中勾选函数，查看定义

	//仿射后图像中心未变，需要重新确定



	return dst;
}

Mat imageRotate4(InputArray src, OutputArray dst, double angle, bool isClip)
{
	Mat input = src.getMat();


	//得到图像大小  
	int width = input.cols;
	int height = input.rows;

	//计算图像中心点  
	Point2f center;
	center.x = width / 2.0;
	center.y = height / 2.0;

	//获得旋转变换矩阵  
	double scale = 1.0;
	Mat trans_mat = getRotationMatrix2D(center, -angle, scale);

	//计算新图像大小  
	double angle1 = angle  * CV_PI / 180.;
	double a = sin(angle1) * scale;
	double b = cos(angle1) * scale;
	double out_width = height * fabs(a) + width * fabs(b); //外边框长度  
	double out_height = width * fabs(a) + height * fabs(b);//外边框高度  

	int new_width, new_height;
	if (!isClip) {
		new_width = cvRound(out_width);
		new_height = cvRound(out_height);
	}
	else {
		//calculate width and height of clip rect  
		double angle2 = fabs(atan(height * 1.0 / width)); //即角度 b  
		double len = width * fabs(b);
		double Y = len / (1 / fabs(tan(angle1)) + 1 / fabs(tan(angle2)));
		double X = Y * 1 / fabs(tan(angle2));
		new_width = cvRound(out_width - X * 2);
		new_height = cvRound(out_height - Y * 2);
	}

	//在旋转变换矩阵中加入平移量  
	trans_mat.at<double>(0, 2) += cvRound((new_width - width) / 2);
	trans_mat.at<double>(1, 2) += cvRound((new_height - height) / 2);

	//仿射变换  
	warpAffine(input, dst, trans_mat, Size(new_width, new_height));

	return dst.getMat();
}

static void callbackAdjust(int, void *)  //规定必须是这样的参数类型，createTrackbar的回调
{
	Mat dst;

	//imageRotate1(src, dst, rotateDegree - BASE);  
	//imageRotate2(src, dst, rotateDegree - BASE);  
	//imageRotate3(src, dst, rotateDegree - BASE);  

	bool isClip = (clip == 1);
	Mat A1=imageRotate4(src, dst, rotateDegree - BASE, isClip);

	imshow(window_name, A1);
}   


/**
* 检测图像倾斜度
* 返回值：返回0表示无检测结果，返回非0表示摆正图象需要旋转的角度（-10至10度）
*/

int main(){
	src = imread("F:\\1_test.jpg", -1);
	Mat output;
	/*output=imageRotate1(src,-15);
	namedWindow("win1");
	imshow("win1",output);*/
	namedWindow(window_name);  
	createTrackbar("rotate",   //1,创建滚动条，const string&类型的trackbarname，表示轨迹条的名字，用来代表我们创建的轨迹条。
		window_name,           //2,const string&类型的winname，填窗口的名字，表示这个轨迹条会依附到哪个窗口上，即对应namedWindow（）创建窗口时填的某一个窗口名。
		&rotateDegree,         //3,int* 类型的value，一个指向整型的指针，表示滑块的位置。并且在创建时，滑块的初始位置就是该变量当前的值。
		BASE * 2,              //4,int类型的count，表示滑块可以达到的最大位置的值。注意:滑块最小的位置的值始终为0。
		callbackAdjust
		                       //5,TrackbarCallback类型的onChange，首先注意他有默认值0。
	                           /*这是一个指向回调函数的指针，每次滑块位置改变时，这个函数都会进行回调。
	                           并且这个函数的原型必须为void XXXX(int, void*); 
	                           其中第一个参数是轨迹条的位置，第二个参数是用户数据（看下面的第六个参数）。
	                           如果回调是NULL指针，表示没有回调函数的调用，仅第三个参数value有变化。*/
	    //,0                      
		                       /*6,第六个参数，void*类型的userdata，他也有默认值0。
		                       这个参数是用户传给回调函数的数据，用来处理轨迹条事件。如果使用的第三个参数value实参是全局变量的话，完全可以不去管这个userdata参数。*/				   
				);
	createTrackbar("clip", window_name, &clip, 1, callbackAdjust);

	//自动检测旋转角度  
	double angle = detectRotation(src);
	if (angle != 0) {
		rotateDegree = angle + BASE;
		setTrackbarPos("rotate", window_name, rotateDegree);
	}

	callbackAdjust(0, 0);
	namedWindow("AAA");
	Mat A2 = imageRotate1(src, angle);
	imshow("AAA",A2);
	waitKey();

	return 0;

}
