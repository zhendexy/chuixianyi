#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessagebox>
#include <QFileDialog>
#include <QMouseEvent>
#include <QTimer>
#include "ui_chuixianyi.h"


#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvPipeline.h>
#include <PvBuffer.h>
//PV_INIT_SIGNAL_HANDLER();
#define BUFFER_COUNT ( 16 )
#define IMAGE_WIDTH ( 2560 )
#define IMAGE_HEIGHT ( 2048 )
#define LABEL_WIDTH (500)
#define LABEL_HEIGHT (400)


#include "opencv2/core/core_c.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <numeric>

#define LEFT_FOLDER "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\04\\left\\"
#define RIGHT_FOLDER "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\04\\right\\"
#define DATA_FOLDER "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\04\\"


using namespace std;
using namespace cv;



class chuixianyi : public QMainWindow
{
    Q_OBJECT

public:
    chuixianyi(QWidget *parent = Q_NULLPTR);



	QImage CvMat2QImage(const Mat &mat);
	Mat QImage2CvMat(const QImage &image);
	void readRectifyParams();
	void prepareFrameLR();



	//读取相机数据需要的函数
	bool SelectDevice(PvString *aConnectionID0, PvString *aConnectionID1, PvDeviceInfoType *aType0 = NULL, PvDeviceInfoType *aType1 = NULL);
	PvDevice *ConnectToDevice(const PvString &aConnectionID);
	PvStream *OpenStream(const PvString &aConnectionID);
	void ConfigureStream(PvDevice *aDevice, PvStream *aStream);
	PvPipeline* CreatePipeline(PvDevice *aDevice, PvStream *aStream);
	void AcquireImages(PvDevice *aDevice, PvDevice *bDevice, PvStream *aStream, PvStream *bStream, PvPipeline *aPipeline, PvPipeline *bPipeline);
	void StopAcquireImages(PvDevice *aDevice, PvDevice *bDevice, PvStream *aStream, PvStream *bStream, PvPipeline *aPipeline, PvPipeline *bPipeline);
	void startCamera();
	void endCamera();

	

	//相机标定用到的函数
	bool singleCameraCalibrate(const char* imageFolder, const char* imageList, const char* singleCalibrateResult, vector<vector<Point3f>>& objectPoints,
		vector<vector<Point2f>>& corners_seq, Mat& cameraMatrix, Mat& distCoeffs, Size& imageSize, Size patternSize, Size chessboardSize);
	bool stereoCalibrate(const char* stereoCalibrateResult, vector<vector<Point3f>> objectPoints, vector<vector<Point2f>> imagePoints1, vector<vector<Point2f>> imagePoints2,
		Mat& cameraMatrix1, Mat& distCoeffs1, Mat& cameraMatrix2, Mat& distCoeffs2, Size& imageSize, Mat& R, Mat& T, Mat& E, Mat& F);
	Rect stereoRectification(const char* stereoRectifyParams, Mat& cameraMatrix1, Mat& distCoeffs1, Mat& cameraMatrix2, Mat& distCoeffs2,
		Size& imageSize, Mat& R, Mat& T, Mat& R1, Mat& R2, Mat& P1, Mat& P2, Mat& Q, Mat& mapl1, Mat& mapl2, Mat& mapr1, Mat& mapr2);
	bool computeDisparityImage(const char* imageName1, const char* imageName2, Mat& img1_rectified,
		Mat& img2_rectified, Mat& mapl1, Mat& mapl2, Mat& mapr1, Mat& mapr2, Rect validRoi[2], Mat& disparity);
	bool computeDisparityImage(Mat& img1, Mat& img2, Mat& img1_rectified,
		Mat& img2_rectified, Mat& mapl1, Mat& mapl2, Mat& mapr1, Mat& mapr2, Rect validRoi[2], Mat& disparity);
	Point3f uv2xyz(Point2f uvLeft, Point2f uvRight);



	//激光平面标定用到的函数
	vector<Point> getRayLinePoints();
	Point2f uvzw2xwyw(Point uv, float zw, Mat &P);



	//图像处理部分用到的函数
	void ThinSubiteration1(Mat & pSrc, Mat & pDst);
	void ThinSubiteration2(Mat & pSrc, Mat & pDst);
	void normalizeLetter(Mat & inputarray, Mat & outputarray);
	void Line_reflect(Mat & inputarray, Mat & outputarray);
	void Delete_smallregions(Mat & pSrc, Mat & pDst);
	void GetRedComponetBySplit(Mat srcImg, Mat &red);



	//读取相机数据需要的变量
private:
	PvDevice *lDeviceL = NULL, *lDeviceR = NULL;
	PvStream *lStreamL = NULL, *lStreamR = NULL;
	PvPipeline *lPipelineL = NULL, *lPipelineR = NULL;
	PvBuffer *blBuffer = NULL, *alBuffer = NULL;
	Mat frameLeft, frameRight;//存储当前帧的图像数据

	bool flagShot = false;
	bool flagStartCamera = false;
	bool flagEndCamera = false;
	QTimer *timer;//定时器
	double rate = 30;//FPS
	int num = 0;//保存图片的命名



	//相机标定和测量用到的变量
private:
	const char* imageName_L = "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\04\\left\\left02.jpg"; // 用于检测深度的图像
	const char* imageName_R = "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\04\\right\\right02.jpg";
	const char* imageList_L = "caliberationpics_L.txt"; // 左相机的标定图片名称列表
	const char* imageList_R = "caliberationpics_R.txt"; // 右相机的标定图片名称列表
	const char* singleCalibrate_result_L = "calibrationresults_L.yml"; // 存放左相机的标定结果
	const char* singleCalibrate_result_R = "calibrationresults_R.yml"; // 存放右相机的标定结果
	const char* stereoCalibrate_result_L = "stereocalibrateresult_L.yml"; // 存放立体标定结果
	const char* stereoCalibrate_result_R = "stereocalibrateresult_R.yml";
	const char* stereoRectifyParams = "stereoRectifyParams.yml"; // 存放立体校正参数
	vector<vector<Point2f>> corners_seq_L; // 所有角点坐标
	vector<vector<Point2f>> corners_seq_R;
	vector<vector<Point3f>> objectPoints_L; // 三维坐标
	vector<vector<Point3f>> objectPoints_R;
	Mat cameraMatrix_L = Mat(3, 3, CV_32FC1, Scalar::all(0)); // 相机的内参数
	Mat cameraMatrix_R = Mat(3, 3, CV_32FC1, Scalar::all(0)); // 初始化相机的内参数
	Mat distCoeffs_L = Mat(1, 5, CV_32FC1, Scalar::all(0)); // 相机的畸变系数
	Mat distCoeffs_R = Mat(1, 5, CV_32FC1, Scalar::all(0)); // 初始化相机的畸变系数
	Mat extrinsicMatrix_L = Mat(3, 4, CV_32FC1, Scalar::all(0)); // 第一张图片标定板对应的世界坐标系的外参数
	Mat extrinsicMatrix_R = Mat(3, 4, CV_32FC1, Scalar::all(0)); // 初始化相机的外参数
	Mat rotationMatrix_L = Mat(3, 3, CV_32FC1, Scalar::all(0)); // 第一张图片标定板对应的世界坐标系的旋转矩阵
	Mat rotationMatrix_R = Mat(3, 3, CV_32FC1, Scalar::all(0));
	Mat translationMatrix_L = Mat(3, 1, CV_32FC1, Scalar::all(0)); // 第一张图片标定板对应的世界坐标系的平移矩阵
	Mat translationMatrix_R = Mat(3, 1, CV_32FC1, Scalar::all(0));
	Mat projectionMatrix_L = Mat(3, 4, CV_32FC1, Scalar::all(0)); // 第一张图片标定板对应的世界坐标系的投影矩阵
	Mat projectionMatrix_R = Mat(3, 4, CV_32FC1, Scalar::all(0));
	Mat R, T, E, F; // 立体标定参数
	Mat R1, R2, P1, P2, Q; // 立体校正参数
	Mat mapl1, mapl2, mapr1, mapr2; // 图像重投影映射表
	Mat img1_rectified, img2_rectified, disparity, result3DImage; // 校正图像 视差图 深度图
	Size patternSize = Size(17, 17); // 行列内角点个数
	Size chessboardSize = Size(10, 10); // 棋盘上每个棋盘格的大小10mm
	Size imageSize = Size(IMAGE_WIDTH, IMAGE_HEIGHT); // 图像尺寸
	Rect validRoi[2];
	string imageNameLeft, imageNameRight; // 矫正校正图像时用的变量名
	Mat imageLeft, imageRight; // 矫正校正图像时用的变量名
	bool flagIsMeasuring = false;// 双目视觉手动选点测量
	unsigned xoffset, yoffset; // 获取窗体label上的真实图像坐标
	vector<QPoint> labelPoints;
	vector<Vec4d> realPoints;

	

	//激光平面标定和测量用到的变量
private:
	const char* imageList_L_rayLine = "caliberationpics_L_rayLine.txt"; // 左相机激光平面标定的图片名称列表
	const char* imageList_R_ratLine = "caliberationpics_R_rayLine.txt"; // 右相机激光平面标定的图片名称列表
	const char* rayLineCalibrate_result_L = "rayLinecalibrationresults_L.yml"; // 存放左相机激光平面的标定结果
	const char* rayLineCalibrate_result_R = "rayLinecalibrationresults_R.yml"; // 存放右相机激光平面的标定结果
	Mat srcImage, grayImage, redImage, rayLineImage;
	int T_rayLine = 50; // 图像二值化的阈值
	int redtolerant = 3, graytolerant = 10;
	Mat G, L, S;	// S = [A B C]-1		S = (GT * G)-1 * GT * L



	//图像处理部分用到的变量
private:
	bool flagIsMatched = false; // 特征匹配

	bool flagIsDetectedCross = false; // 检测激光平面与垂线的交点
	Mat g_srcImage, g_dstImage, g_midImage, g_grayImage, imgHSVMask, g_redImage;//原始图、中间图和效果图
	int threshold_value = 20;	//阈值
	int msize = 100;				//面积因子
	float start_time, end_time, sum_time;	//处理时间




private slots:
void on_startCameraBtn_clicked();
void on_endCameraBtn_clicked();
void on_saveFrameBtn_clicked();
void on_calibrateBtn_clicked();
void on_rectifyBtn_clicked();
void on_measureBtn_clicked();
void on_autoMatchBtn_clicked();
void on_detectCrossBtn_clicked();
void on_planeCalibrateBtn_clicked();
void on_liveShowCoordinateBtn_clicked();

void readFrame(); // 用定时器控制读取相机数据的帧率


protected:
	void mousePressEvent(QMouseEvent *event);


private:
    Ui::chuixianyiClass ui;
};
