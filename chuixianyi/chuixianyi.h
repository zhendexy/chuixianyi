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
	Point3f uv2xyz(Point2f uvLeft, Point2f uvRight);



	//����ƽ��궨�õ��ĺ���
	vector<Point> getRayLinePoints();
	Point2f uvzw2xwyw(Point uv, float zw, Mat &P);



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
	const char* imageName_L = "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\04\\left\\left02.jpg"; // ���ڼ����ȵ�ͼ��
	const char* imageName_R = "C:\\Users\\jzpwh\\Desktop\\xy\\calibrateExperiments\\04\\right\\right02.jpg";
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
	Mat extrinsicMatrix_L = Mat(3, 4, CV_32FC1, Scalar::all(0)); // ��һ��ͼƬ�궨���Ӧ����������ϵ�������
	Mat extrinsicMatrix_R = Mat(3, 4, CV_32FC1, Scalar::all(0)); // ��ʼ������������
	Mat rotationMatrix_L = Mat(3, 3, CV_32FC1, Scalar::all(0)); // ��һ��ͼƬ�궨���Ӧ����������ϵ����ת����
	Mat rotationMatrix_R = Mat(3, 3, CV_32FC1, Scalar::all(0));
	Mat translationMatrix_L = Mat(3, 1, CV_32FC1, Scalar::all(0)); // ��һ��ͼƬ�궨���Ӧ����������ϵ��ƽ�ƾ���
	Mat translationMatrix_R = Mat(3, 1, CV_32FC1, Scalar::all(0));
	Mat projectionMatrix_L = Mat(3, 4, CV_32FC1, Scalar::all(0)); // ��һ��ͼƬ�궨���Ӧ����������ϵ��ͶӰ����
	Mat projectionMatrix_R = Mat(3, 4, CV_32FC1, Scalar::all(0));
	Mat R, T, E, F; // ����궨����
	Mat R1, R2, P1, P2, Q; // ����У������
	Mat mapl1, mapl2, mapr1, mapr2; // ͼ����ͶӰӳ���
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
	const char* imageList_L_rayLine = "caliberationpics_L_rayLine.txt"; // ���������ƽ��궨��ͼƬ�����б�
	const char* imageList_R_ratLine = "caliberationpics_R_rayLine.txt"; // ���������ƽ��궨��ͼƬ�����б�
	const char* rayLineCalibrate_result_L = "rayLinecalibrationresults_L.yml"; // ������������ƽ��ı궨���
	const char* rayLineCalibrate_result_R = "rayLinecalibrationresults_R.yml"; // ������������ƽ��ı궨���
	Mat srcImage, grayImage, redImage, rayLineImage;
	int T_rayLine = 50; // ͼ���ֵ������ֵ
	int redtolerant = 3, graytolerant = 10;
	Mat G, L, S;	// S = [A B C]-1		S = (GT * G)-1 * GT * L



	//ͼ�������õ��ı���
private:
	bool flagIsMatched = false; // ����ƥ��

	bool flagIsDetectedCross = false; // ��⼤��ƽ���봹�ߵĽ���
	Mat g_srcImage, g_dstImage, g_midImage, g_grayImage, imgHSVMask, g_redImage;//ԭʼͼ���м�ͼ��Ч��ͼ
	int threshold_value = 20;	//��ֵ
	int msize = 100;				//�������
	float start_time, end_time, sum_time;	//����ʱ��




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

void readFrame(); // �ö�ʱ�����ƶ�ȡ������ݵ�֡��


protected:
	void mousePressEvent(QMouseEvent *event);


private:
    Ui::chuixianyiClass ui;
};
