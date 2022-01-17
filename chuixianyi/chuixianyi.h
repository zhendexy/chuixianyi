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

#define LEFT_FOLDER "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\08\\left\\"
#define RIGHT_FOLDER "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\08\\right\\"
#define DATA_FOLDER "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\08\\"


using namespace std;
using namespace cv;



class chuixianyi : public QMainWindow
{
    Q_OBJECT

public:
    chuixianyi(QWidget *parent = Q_NULLPTR);



	QImage CvMat2QImage(const Mat &mat);
	Mat QImage2CvMat(const QImage &image);
	void prepareFrameLR();



	//��ȡ���������Ҫ�ĺ���
	bool SelectDevice(PvString *aConnectionID0, PvString *aConnectionID1, PvDeviceInfoType *aType0 = NULL, PvDeviceInfoType *aType1 = NULL);
	PvDevice *ConnectToDevice(const PvString &aConnectionID);
	PvStream *OpenStream(const PvString &aConnectionID);
	void ConfigureStream(PvDevice *aDevice, PvStream *aStream);
	PvPipeline* CreatePipeline(PvDevice *aDevice, PvStream *aStream);
	void AcquireImages(PvDevice *aDevice, PvDevice *bDevice, PvStream *aStream, PvStream *bStream, PvPipeline *aPipeline, PvPipeline *bPipeline);
	void StopAcquireImages(PvDevice *aDevice, PvDevice *bDevice, PvStream *aStream, PvStream *bStream, PvPipeline *aPipeline, PvPipeline *bPipeline);
	void startCamera();
	void endCamera();

	

	//����궨�õ��ĺ���
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
	Point3f uv2xwywzw(Point2f uvLeft, Point2f uvRight);



	//����ƽ��궨�õ��ĺ���
	//bool rayPlaneCalibrate(const char* imageFolder, const char* imageList, const char* rayPlaneCalibrateResult, 
	//	const char* realRayPlanePointsList, Mat& rayPlaneParams, Mat& projectionMatrix, Mat& cameraMatrix, Mat& distCoeffs);
	bool rayPlaneCalibrate(const char* imageFolder, const char* imageList, const char* uvPoints, const char* realPoints, const char* caliPoints,
		const char* caliErr, const char*rayPlaneCali_result, Mat& rayPlaneParams, Mat& projectionMatrix, Mat& cameraMatrix, Mat& distCoeffs);
	void getRayLinePoints(Mat& srcImage, vector<Point>& linePoints);
	void deleteSmallRegions(Mat &pSrc, Mat &pDst);
	void findSmallRegions(Mat &pSrc, Mat &pDst);
	Point2f uvzw2xwyw(Point uv, float zw, Mat &P);
	void uvzw2xwyw(Point &uv, float zw, Point2f &xwyw, Mat &M);
	Point3f uv2xwywzw(Point uv, Mat& M);
	void uv2xwywzw(Point &uv, Point3f &xwywzw, Mat& M);



	//ͼ�������õ��ĺ���
	void ThinSubiteration1(Mat & pSrc, Mat & pDst);
	void ThinSubiteration2(Mat & pSrc, Mat & pDst);
	void normalizeLetter(Mat & inputarray, Mat & outputarray);
	void Line_reflect(Mat & inputarray, Mat & outputarray);
	void Delete_smallregions(Mat & pSrc, Mat & pDst);
	void GetRedComponetBySplit(Mat srcImg, Mat &red);



	//��ȡ���������Ҫ�ı���
private:
	PvDevice *lDeviceL = NULL, *lDeviceR = NULL;
	PvStream *lStreamL = NULL, *lStreamR = NULL;
	PvPipeline *lPipelineL = NULL, *lPipelineR = NULL;
	PvBuffer *blBuffer = NULL, *alBuffer = NULL;
	Mat frameLeft, frameRight;//�洢��ǰ֡��ͼ������

	bool flagShot = false;
	bool flagStartCamera = false;
	bool flagEndCamera = false;
	QTimer *timer;//��ʱ��
	double rate = 30;//FPS
	int num = 0;//����ͼƬ������



	//����궨�Ͳ����õ��ı���
private:
	const char* imageName_L = "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\08\\left\\left01_10.jpg"; // ���ڼ����ȵ�ͼ��
	const char* imageName_R = "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\08\\right\\right00_0.jpg";
	const char* imageList_L = "caliberationpics_L.txt"; // ������ı궨ͼƬ�����б�
	const char* imageList_R = "caliberationpics_R.txt"; // ������ı궨ͼƬ�����б�
	const char* singleCalibrate_result_L = "calibrationresults_L.yml"; // ���������ı궨���
	const char* singleCalibrate_result_R = "calibrationresults_R.yml"; // ���������ı궨���
	const char* stereoCalibrate_result_L = "stereocalibrateresult_L.yml"; // �������궨���
	const char* stereoCalibrate_result_R = "stereocalibrateresult_R.yml";
	const char* stereoRectifyParams = "stereoRectifyParams.yml"; // �������У������
	vector<vector<Point2f>> corners_seq_L; // ���нǵ�����
	vector<vector<Point2f>> corners_seq_R;
	vector<vector<Point3f>> objectPoints_L; // ��ά����
	vector<vector<Point3f>> objectPoints_R;
	Mat cameraMatrix_L = Mat(3, 3, CV_32FC1, Scalar::all(0)); // ������ڲ���
	Mat cameraMatrix_R = Mat(3, 3, CV_32FC1, Scalar::all(0)); // ��ʼ��������ڲ���
	Mat distCoeffs_L = Mat(1, 5, CV_32FC1, Scalar::all(0)); // ����Ļ���ϵ��
	Mat distCoeffs_R = Mat(1, 5, CV_32FC1, Scalar::all(0)); // ��ʼ������Ļ���ϵ��
	Mat rotationVector_L = Mat(3, 1, CV_32FC1, Scalar::all(0)); // ��һ��ͼƬ�궨���Ӧ����������ϵ����ת����
	Mat rotationVector_R = Mat(3, 1, CV_32FC1, Scalar::all(0));
	Mat rotationMatrix_L = Mat(3, 3, CV_32FC1, Scalar::all(0)); // ��һ��ͼƬ�궨���Ӧ����������ϵ����ת����
	Mat rotationMatrix_R = Mat(3, 3, CV_32FC1, Scalar::all(0));
	Mat translationVector_L = Mat(3, 1, CV_32FC1, Scalar::all(0)); // ��һ��ͼƬ�궨���Ӧ����������ϵ��ƽ������
	Mat translationVector_R = Mat(3, 1, CV_32FC1, Scalar::all(0));
	Mat extrinsicMatrix_L = Mat(3, 4, CV_32FC1, Scalar::all(0)); // ��һ��ͼƬ�궨���Ӧ����������ϵ�������
	Mat extrinsicMatrix_R = Mat(3, 4, CV_32FC1, Scalar::all(0));
	Mat projectionMatrix_L = Mat(3, 4, CV_32FC1, Scalar::all(0)); // ��һ��ͼƬ�궨���Ӧ����������ϵ��ͶӰ����
	Mat projectionMatrix_R = Mat(3, 4, CV_32FC1, Scalar::all(0));
	Mat R, T, E, F; // ����궨����
	Mat R1, R2, P1, P2, Q; // ����У������
	Mat mapl1, mapl2, mapr1, mapr2; // ͼ����ͶӰӳ���	�������У��
	Mat maplx, maply, maprx, mapry; // ͼ����ͶӰӳ���	���������Ŀ����
	Mat img1_rectified, img2_rectified, disparity, result3DImage; // У��ͼ�� �Ӳ�ͼ ���ͼ
	Size patternSize = Size(17, 17); // �����ڽǵ����
	Size chessboardSize = Size(10, 10); // ������ÿ�����̸�Ĵ�С10mm
	Size imageSize = Size(IMAGE_WIDTH, IMAGE_HEIGHT); // ͼ��ߴ�
	Rect validRoi[2];
	string imageNameLeft, imageNameRight; // ����У��ͼ��ʱ�õı�����
	Mat imageLeft, imageRight; // ����У��ͼ��ʱ�õı�����
	bool flagIsMeasuring = false;// ˫Ŀ�Ӿ��ֶ�ѡ�����
	unsigned xoffset, yoffset; // ��ȡ����label�ϵ���ʵͼ������
	vector<QPoint> labelPoints;
	vector<Vec4d> realPoints;

	

	//����ƽ��궨�Ͳ����õ��ı���
private:
	const char* rayLineimageList_L = "rayLinecaliberationpics_L.txt"; // ���������ƽ��궨��ͼƬ�����б�
	const char* rayLineimageList_R = "rayLinecaliberationpics_R.txt"; // ���������ƽ��궨��ͼƬ�����б�
	const char* uvPoints_L = "uvPoints_L.txt"; // ���ͼ���ϼ���������ĵ�����
	const char* uvPoints_R = "uvPoints_R.txt";
	const char* realPoints_L = "realPoints_L.txt"; // ��ż���ƽ������������
	const char* realPoints_R = "realPoints_R.txt";
	const char* caliPoints_L = "caliPoints_L.txt"; // ��Ÿ��ݱ궨��������ļ���ƽ������������
	const char* caliPoints_R = "caliPoints_R.txt";
	const char* caliErr_L = "caliErr_L.txt"; // ��ű궨���
	const char* caliErr_R = "caliErr_R.txt";
	const char* rayPlaneCali_result_L = "rayPlaneParams_L.yml"; // ������������ƽ��ı궨���
	const char* rayPlaneCali_result_R = "rayPlaneParams_R.yml"; // ������������ƽ��ı궨���
	Mat rayPlaneParams_L = Mat(3, 1, CV_32FC1); // ���������ƽ��궨���� A B C
	Mat rayPlaneParams_R = Mat(3, 1, CV_32FC1);
	Mat srcImage, grayImage, redImage, rayLineImage, midImage, maskImage;
	int T_rayLine = 80; // ͼ���ֵ������ֵ
	int redtolerant = 3, graytolerant = 10;
	bool flagIsMeasuring1 = false;


	//ͼ�������õ��ı���
private:
	bool flagIsMatched = false; // ����ƥ��

	bool flagIsDetectedCross = false; // ��⼤��ƽ���봹�ߵĽ���
	Mat g_srcImage, g_dstImage, g_midImage, g_grayImage, imgHSVMask, g_redImage;//ԭʼͼ���м�ͼ��Ч��ͼ
	int threshold_value = 20;	//��ֵ
	int msize = 400;				//�������
	float start_time, end_time, sum_time;	//����ʱ��




private slots:
void on_startCameraBtn_clicked();
void on_endCameraBtn_clicked();
void on_saveFrameBtn_clicked();
void readFrame(); // �ö�ʱ�����ƶ�ȡ������ݵ�֡��

void on_OpenCVCalibrateBtn_clicked();
void on_MATLABCalibrateBtn_clicked();
void on_rayPlaneParamsBtn_clicked();

void on_calibrateBtn_clicked();
void on_rectifyBtn_clicked();
void on_measureBtn_clicked();

void on_rayPlaneCalibrateBtn_clicked();
void on_measure1Btn_clicked();

void on_autoMatchBtn_clicked();
void on_detectCrossBtn_clicked();
void on_liveShowCoordinateBtn_clicked();




protected:
	void mousePressEvent(QMouseEvent *event);


private:
    Ui::chuixianyiClass ui;
};
