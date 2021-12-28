#include "chuixianyi.h"

PV_INIT_SIGNAL_HANDLER();//此行有重定义的错误 LNK2005 放在这里不会出错 不能放回chuixianyi.h中

chuixianyi::chuixianyi(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	ui.labelLeft->setAlignment(Qt::AlignCenter);
	ui.labelRight->setAlignment(Qt::AlignCenter);

	timer = new QTimer(this);
	timer->setInterval(1000 / rate);//set timer match with FPS
	
	timer->start();

	//信号与槽函数
	connect(ui.exitBtn, &QPushButton::clicked, this, &chuixianyi::close);
	connect(timer, SIGNAL(timeout()), this, SLOT(readFrame()));
}


void chuixianyi::on_startCameraBtn_clicked()
{
	timer->start();
	startCamera();
	flagStartCamera = true;
	flagEndCamera = false;
	ui.textBrowser->append(QString::fromLocal8Bit("已打开摄像头!"));
	QCoreApplication::processEvents();
}

void chuixianyi::on_endCameraBtn_clicked()
{
	timer->stop();
	endCamera();
	flagStartCamera = false;
	flagEndCamera = true;
	ui.textBrowser->append(QString::fromLocal8Bit("已关闭摄像头!"));
	QCoreApplication::processEvents();
}

void chuixianyi::on_saveFrameBtn_clicked()
{
	if (!flagStartCamera)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("相机没有打开"));
		return;
	}


	Size size(IMAGE_WIDTH, IMAGE_HEIGHT);

	PvImage *alInputImage = alBuffer->GetImage();
	alInputImage->Alloc(IMAGE_WIDTH, IMAGE_HEIGHT, PvPixelMono8);
	Mat lMat(size, CV_8UC1, alInputImage->GetDataPointer());
	frameLeft = lMat.clone();

	PvImage *blInputImage = blBuffer->GetImage();
	blInputImage->Alloc(IMAGE_WIDTH, IMAGE_HEIGHT, PvPixelMono8);
	Mat rMat(size, CV_8UC1, blInputImage->GetDataPointer());
	frameRight = rMat.clone();


	string nameLeft, nameRight;
	if (num < 10)
	{
		nameLeft = "left0" + to_string(num) + ".jpg";
		nameRight = "right0" + to_string(num) + ".jpg";
	}
	else
	{
		nameLeft = "left" + to_string(num) + ".jpg";
		nameRight = "right" + to_string(num) + ".jpg";
	}
	imwrite(nameLeft, frameLeft);
	imwrite(nameRight, frameRight);


	++num;

	ui.textBrowser->append(QString::fromLocal8Bit("已保存当前帧画面!"));
	QCoreApplication::processEvents();
}





void chuixianyi::startCamera()
{
	lDeviceL = NULL, lDeviceR = NULL;
	lStreamL = NULL, lStreamR = NULL;

	PV_SAMPLE_INIT();


	PvString lConnectionID0, lConnectionID1;
	if (SelectDevice(&lConnectionID0, &lConnectionID1))
	{
		lDeviceL = ConnectToDevice(lConnectionID0);
		lDeviceR = ConnectToDevice(lConnectionID1);
		if (lDeviceL != NULL&&lDeviceR != NULL)
		{
			lStreamL = OpenStream(lConnectionID0);
			lStreamR = OpenStream(lConnectionID1);
			if (lStreamL != NULL&&lStreamR != NULL)
			{
				lPipelineL = NULL;
				lPipelineR = NULL;

				ConfigureStream(lDeviceL, lStreamL);
				ConfigureStream(lDeviceR, lStreamR);
				lPipelineL = CreatePipeline(lDeviceL, lStreamL);
				lPipelineR = CreatePipeline(lDeviceR, lStreamR);
				if (lPipelineL&&lPipelineR)
					AcquireImages(lDeviceL, lDeviceR, lStreamL, lStreamR, lPipelineL, lPipelineR);
			}
		}
	}
}

void chuixianyi::endCamera()
{
	if (!flagStartCamera || flagEndCamera)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("还没打开相机，无法关闭相机"));
		return;
	}
	flagEndCamera = false;


	if (lPipelineL&&lPipelineR)
		StopAcquireImages(lDeviceL, lDeviceR, lStreamL, lStreamR, lPipelineL, lPipelineR);;

	delete lPipelineL;
	delete lPipelineR;

	// Close the stream
	cout << "Closing stream" << endl;
	lStreamL->Close();
	lStreamR->Close();
	PvStream::Free(lStreamL);
	PvStream::Free(lStreamR);


	// Disconnect the device
	cout << "Disconnecting device" << endl;
	lDeviceL->Disconnect();
	lDeviceR->Disconnect();
	PvDevice::Free(lDeviceL);
	PvDevice::Free(lDeviceR);



	PV_SAMPLE_TERMINATE();
}

bool chuixianyi::SelectDevice(PvString *aConnectionID0, PvString *aConnectionID1, PvDeviceInfoType *aType0, PvDeviceInfoType *aType1)
{
	PvResult lResult;
	const PvDeviceInfo *lSelectedDI0 = NULL;
	const PvDeviceInfo *lSelectedDI1 = NULL;
	PvSystem lSystem;

	if (gStop)
	{
		return false;
	}
	lSystem.Find();

	// Detect, select device.
	vector<const PvDeviceInfo *> lDIVector;
	for (uint32_t i = 0; i < lSystem.GetInterfaceCount(); i++)
	{
		const PvInterface *lInterface = dynamic_cast<const PvInterface *>(lSystem.GetInterface(i));
		if (lInterface != NULL)
		{
			for (uint32_t j = 0; j < lInterface->GetDeviceCount(); j++)
			{
				const PvDeviceInfo *lDI = dynamic_cast<const PvDeviceInfo *>(lInterface->GetDeviceInfo(j));
				if (lDI != NULL)
					lDIVector.push_back(lDI);
			}
		}
	}

	if (lDIVector.size() == 0)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("找不到相机"));
		return false;
	}


	// Read device selection, optional new IP address.
	PV_DISABLE_SIGNAL_HANDLER();
	PV_ENABLE_SIGNAL_HANDLER();

	//分配左右相机
	lSelectedDI0 = lDIVector[1];
	lSelectedDI1 = lDIVector[0];

	// If the IP Address valid?
	if (lSelectedDI0->IsConfigurationValid()&& lSelectedDI1->IsConfigurationValid())
	{
		*aConnectionID0 = lSelectedDI0->GetConnectionID();
		*aConnectionID1 = lSelectedDI1->GetConnectionID();
		if (aType0 != NULL || aType1 != NULL)
		{
			*aType0 = lSelectedDI0->GetType();
			*aType1 = lSelectedDI1->GetType();
		}

		return true;
	}
	return false;
}

PvDevice *chuixianyi::ConnectToDevice(const PvString &aConnectionID)
{
	PvDevice *lDevice;
	PvResult lResult;

	// Connect to the GigE Vision or USB3 Vision device
	lDevice = PvDevice::CreateAndConnect(aConnectionID, &lResult);
	if (lDevice == NULL)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("Unable to connect to device."));
	}

	return lDevice;
}

PvStream *chuixianyi::OpenStream(const PvString &aConnectionID)
{
	PvStream *lStream;
	PvResult lResult;

	// Open stream to the GigE Vision or USB3 Vision device
	lStream = PvStream::CreateAndOpen(aConnectionID, &lResult);
	if (lStream == NULL)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("Unable to stream from device."));
	}

	return lStream;
}

void chuixianyi::ConfigureStream(PvDevice *aDevice, PvStream *aStream)
{
	// If this is a GigE Vision device, configure GigE Vision specific streaming parameters
	PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV *>(aDevice);
	if (lDeviceGEV != NULL)
	{
		PvStreamGEV *lStreamGEV = static_cast<PvStreamGEV *>(aStream);

		// Negotiate packet size
		lDeviceGEV->NegotiatePacketSize();

		// Configure device streaming destination
		lDeviceGEV->SetStreamDestination(lStreamGEV->GetLocalIPAddress(), lStreamGEV->GetLocalPort());
	}
}

PvPipeline *chuixianyi::CreatePipeline(PvDevice *aDevice, PvStream *aStream)
{
	// Create the PvPipeline object
	PvPipeline* lPipeline = new PvPipeline(aStream);

	if (lPipeline != NULL)
	{
		// Reading payload size from device
		uint32_t lSize = aDevice->GetPayloadSize();

		// Set the Buffer count and the Buffer size
		lPipeline->SetBufferCount(BUFFER_COUNT);
		lPipeline->SetBufferSize(lSize);
	}

	return lPipeline;
}

void chuixianyi::AcquireImages(PvDevice *aDevice, PvDevice *bDevice, PvStream *aStream, PvStream *bStream, PvPipeline *aPipeline, PvPipeline *bPipeline)
{
	// Get device parameters need to control streaming
	PvGenParameterArray *alDeviceParams = aDevice->GetParameters();
	PvGenParameterArray *blDeviceParams = bDevice->GetParameters();

	// Map the GenICam AcquisitionStart and AcquisitionStop commands
	PvGenCommand *alStart = dynamic_cast<PvGenCommand *>(alDeviceParams->Get("AcquisitionStart"));
	PvGenCommand *blStart = dynamic_cast<PvGenCommand *>(blDeviceParams->Get("AcquisitionStart"));

	// Note: the pipeline must be initialized before we start acquisition
	// std::cout << "Starting pipeline" << endl;
	aPipeline->Start();
	bPipeline->Start();

	// Get stream parameters
	PvGenParameterArray *alStreamParams = aStream->GetParameters();
	PvGenParameterArray *blStreamParams = bStream->GetParameters();

	// Map a few GenICam stream stats counters
	PvGenFloat *alFrameRate = dynamic_cast<PvGenFloat *>(alStreamParams->Get("AcquisitionRate"));
	PvGenFloat *alBandwidth = dynamic_cast<PvGenFloat *>(alStreamParams->Get("Bandwidth"));
	PvGenFloat *blFrameRate = dynamic_cast<PvGenFloat *>(blStreamParams->Get("AcquisitionRate"));
	PvGenFloat *blBandwidth = dynamic_cast<PvGenFloat *>(blStreamParams->Get("Bandwidth"));

	// Enable streaming and send the AcquisitionStart command
	// cout << "Enabling streaming and sending AcquisitionStart command." << endl;
	aDevice->StreamEnable();
	bDevice->StreamEnable();
	alStart->Execute();
	blStart->Execute();
}

void chuixianyi::StopAcquireImages(PvDevice *aDevice, PvDevice *bDevice, PvStream *aStream, PvStream *bStream, PvPipeline *aPipeline, PvPipeline *bPipeline)
{
	// Get device parameters need to control streaming
	PvGenParameterArray *alDeviceParams = aDevice->GetParameters();
	PvGenParameterArray *blDeviceParams = bDevice->GetParameters();

	// Map the GenICam AcquisitionStart and AcquisitionStop commands
	PvGenCommand *alStop = dynamic_cast<PvGenCommand *>(alDeviceParams->Get("AcquisitionStop"));
	PvGenCommand *blStop = dynamic_cast<PvGenCommand *>(blDeviceParams->Get("AcquisitionStop"));


	// Tell the device to stop sending images.
	// cout << "Sending AcquisitionStop command to the device" << endl;
	alStop->Execute();
	blStop->Execute();

	// Disable streaming on the device
	// cout << "Disable streaming on the controller." << endl;
	aDevice->StreamDisable();
	bDevice->StreamDisable();

	// Stop the pipeline
	// cout << "Stop pipeline" << endl;
	aPipeline->Stop();
	bPipeline->Stop();
}


void chuixianyi::readFrame()
{
	if (!flagStartCamera)
		return;


	PvResult alOperationResult;
	PvResult blOperationResult;

	// Retrieve next buffer
	PvResult alResult = lPipelineL->RetrieveNextBuffer(&alBuffer, 1000, &alOperationResult);
	PvResult blResult = lPipelineR->RetrieveNextBuffer(&blBuffer, 1000, &blOperationResult);
	if (alResult.IsOK() && blResult.IsOK())
	{
		if (alOperationResult.IsOK() && blOperationResult.IsOK())
		{
			//
			// We now have a valid buffer. This is where you would typically process the buffer.
			// -----------------------------------------------------------------------------------------
			// ...

			QImage lQimage(alBuffer->GetDataPointer(), IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_Grayscale8);
			QImage rQimage(blBuffer->GetDataPointer(), IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_Grayscale8);

			QPixmap lQPixmap = QPixmap::fromImage(lQimage);
			QPixmap rQPixmap = QPixmap::fromImage(rQimage);
			lQPixmap = lQPixmap.scaled(LABEL_WIDTH, LABEL_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			rQPixmap = rQPixmap.scaled(LABEL_WIDTH, LABEL_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);

			ui.labelLeft->setPixmap(lQPixmap);
			ui.labelRight->setPixmap(rQPixmap);
		}
		// Release the buffer back to the pipeline
		lPipelineL->ReleaseBuffer(alBuffer);
		lPipelineR->ReleaseBuffer(blBuffer);
	}
}


QImage chuixianyi::CvMat2QImage(const Mat &mat)
{
	// 图像的通道
	int channel = mat.channels();

	// 设立一个表 直接查询 其中 0 2 是无效值 1 3 4 对应的转换值
	const std::map<int, QImage::Format> img_cvt_map{
		{ 1, QImage::Format_Grayscale8 },
		{ 3, QImage::Format_RGB888 },
		{ 4, QImage::Format_ARGB32 }
	};

	QImage image(mat.data, mat.cols, mat.rows,
		static_cast<int>(mat.step),
		img_cvt_map.at(channel));

	// 三通道图像 值做 通道转换
	return channel == 3 ? image.rgbSwapped() : image;
}

Mat chuixianyi::QImage2CvMat(const QImage &image)
{
	cv::Mat mat;
	const std::map<QImage::Format, int> img_cvt_map{
		{ QImage::Format_Grayscale8, 1 },
		{ QImage::Format_RGB888, 3 },
		{ QImage::Format_ARGB32, 4 }
	};

	auto aa = image.format();
	return cv::Mat(image.height(), image.width(), img_cvt_map.at(image.format()));
}





void chuixianyi::on_calibrateBtn_clicked()
{
	if (flagStartCamera && !flagEndCamera)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请先关闭相机！"));
		return;
	}

	singleCameraCalibrate(LEFT_FOLDER, imageList_L, singleCalibrate_result_L, objectPoints_L, corners_seq_L, cameraMatrix_L,
		distCoeffs_L, imageSize, patternSize, chessboardSize);
	ui.textBrowser->append(QString::fromLocal8Bit("已完成左相机的标定!"));
	QCoreApplication::processEvents();
	singleCameraCalibrate(RIGHT_FOLDER, imageList_R, singleCalibrate_result_R, objectPoints_R, corners_seq_R, cameraMatrix_R,
		distCoeffs_R, imageSize, patternSize, chessboardSize);
	ui.textBrowser->append(QString::fromLocal8Bit("已完成右相机的标定!"));
	QCoreApplication::processEvents();
	stereoCalibrate(stereoCalibrate_result_L, objectPoints_L, corners_seq_L, corners_seq_R, cameraMatrix_L, distCoeffs_L,
		cameraMatrix_R, distCoeffs_R, imageSize, R, T, E, F);
	ui.textBrowser->append(QString::fromLocal8Bit("相机立体标定完成!"));
	QCoreApplication::processEvents();
	//stereoCalibrate(stereoCalibrate_result_R, objectPoints_R, corners_seq_L, corners_seq_R, cameraMatrix_L, distCoeffs_L,
	//	cameraMatrix_R, distCoeffs_R, imageSize, R2, T2, E2, F2);
	//cout << "右相机立体标定完成！" << endl;
	validRoi[0], validRoi[1] = stereoRectification(stereoRectifyParams, cameraMatrix_L, distCoeffs_L, cameraMatrix_R, distCoeffs_R,
		imageSize, R, T, R1, R2, P1, P2, Q, mapl1, mapl2, mapr1, mapr2);
	ui.textBrowser->append(QString::fromLocal8Bit("已创建图像重投影映射表!"));
	QCoreApplication::processEvents();
}

void chuixianyi::on_rectifyBtn_clicked()
{
	if (flagStartCamera && !flagEndCamera)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请先关闭相机！"));
		return;
	}

	readRectifyParams();

	QStringList file_name_list;
	QStringList file_path_list;
	QStringList path_list = QFileDialog::getOpenFileNames(this,
		QString::fromLocal8Bit("请选择要校正的左右相机图片"),
		QString::fromLocal8Bit(DATA_FOLDER),
		QString::fromLocal8Bit("Image Files(*.jpg *.png *.bmp *.pgm *.pbm);;All(*.*)"));
	for (int i = 0; i < path_list.size(); i++)
	{
		//单个文件路径
		QString path = path_list[i];
		QFileInfo file = QFileInfo(path);
		//获得文件名
		QString file_name = file.fileName();

		file_path_list.append(path);
		file_name_list.append(file_name);
	}

	if (path_list.isEmpty())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("未选择图片，将读取默认路径的图片"));
		computeDisparityImage(imageName_L, imageName_R, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	}
	//需选择两张图，选多张图也只取前两张
	if (path_list.size() >= 2)
	{
		//QString路径添加双斜杠\\，并转const char*路径
		imageNameLeft = file_path_list[0].toStdString();
		imageNameRight = file_path_list[1].toStdString();
		string::size_type pos0 = 0, pos1 = 0;
		while ((pos0 = imageNameLeft.find('/', pos0)) != string::npos)
		{
			imageNameLeft.replace(pos0, 1, "\\", 0, 2);//替换
		}
		while ((pos1 = imageNameRight.find('/', pos1)) != string::npos)
		{
			imageNameRight.replace(pos1, 1, "\\", 0, 2);//替换
		}

		computeDisparityImage(imageNameLeft.c_str(), imageNameRight.c_str(), img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	}
	//若只选了一张，就再选一张
	if (path_list.size() == 1)
	{
		QString another_file_path = QFileDialog::getOpenFileName(this,
			QString::fromLocal8Bit("请选择要校正的右相机图片"),
			QString::fromLocal8Bit(DATA_FOLDER),
			QString::fromLocal8Bit("Image Files(*.jpg *.png *.bmp *.pgm *.pbm);;All(*.*)"));

		if (another_file_path != "")
		{
			//QString路径添加双斜杠\\，并转const char*路径
			imageNameLeft = file_path_list[1].toStdString();
			imageNameRight = another_file_path.toStdString();
			string::size_type pos0 = 0, pos1 = 0;
			while ((pos0 = imageNameLeft.find('/', pos0)) != string::npos)
			{
				imageNameLeft.replace(pos0, 1, "\\", 0, 2);//替换
			}
			while ((pos1 = imageNameRight.find('/', pos1)) != string::npos)
			{
				imageNameRight.replace(pos1, 1, "\\", 0, 2);//替换
			}

			computeDisparityImage(imageNameLeft.c_str(), imageNameRight.c_str(), img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
		}
		else
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("未选择右相机图片，将读取默认路径的图片"));
			computeDisparityImage(imageName_L, imageName_R, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
		}
	}
	ui.textBrowser->append(QString::fromLocal8Bit("已完成左右图像校正!"));
	QCoreApplication::processEvents();

	// 从三维投影获得深度映射
	reprojectImageTo3D(disparity, result3DImage, Q);
	imwrite(DATA_FOLDER + string("视差图.jpg"), disparity);
	//namedWindow("视差图", WINDOW_NORMAL);
	//imshow("视差图", disparity);
	//setMouseCallback("视差图", onMouse);
	//waitKey(0);
	//destroyAllWindows();
}

void chuixianyi::on_measureBtn_clicked()
{
	//测量按钮点击一次开始测量，再次点击就退出测量
	if (flagStartCamera && !flagEndCamera)
	{
		if (flagIsMeasuring)
		{
			timer->start();
			flagIsMeasuring = false;
			return;
		}
	}
	flagIsMeasuring = true;

	prepareFrameLR();
	readRectifyParams();

	//这里就将校正后的frameLeft和frameRight显示在label上了
	computeDisparityImage(frameLeft, frameRight, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	ui.textBrowser->append(QString::fromLocal8Bit("已将左右图像校正!"));
	ui.textBrowser->append(QString::fromLocal8Bit("请手动选择两对点!"));
	QCoreApplication::processEvents();
	//while (labelPoints.size() != 4)
	//{

	//}


}

void chuixianyi::mousePressEvent(QMouseEvent *event)
{
	if (!flagIsMeasuring)
		return;

	//鼠标点击左键 获取鼠标坐标
	if (event->button() == Qt::LeftButton)
	{
		//QLabel内容相对于QLbael本身的偏移量
		//xoffset
		//yoffset

		QPoint pt;
		//if (ui.labelLeft->geometry().contains(ui.labelLeft->mapFromGlobal(QCursor::pos())))
		if (ui.labelLeft->underMouse())
		{
			xoffset = (ui.labelLeft->contentsRect().width() - ui.labelLeft->pixmap()->rect().width()) / 2;
			yoffset = (ui.labelLeft->contentsRect().height() - ui.labelLeft->pixmap()->rect().height()) / 2;

			pt = event->globalPos();
			pt = ui.labelLeft->mapFromGlobal(pt);
			pt = pt - QPoint(xoffset, yoffset);
			labelPoints.push_back(pt);

			ui.textBrowser->append(QString::fromLocal8Bit("左图的图像坐标：") + QString::number(pt.x()*5.12) + QString("  ") + QString::number(pt.y()*5.12));
			QCoreApplication::processEvents();
		}
		//else if (ui.labelRight->geometry().contains(ui.labelRight->mapFromGlobal(QCursor::pos())))
		else if (ui.labelRight->underMouse())
		{
			xoffset = (ui.labelRight->contentsRect().width() - ui.labelRight->pixmap()->rect().width()) / 2;
			yoffset = (ui.labelRight->contentsRect().height() - ui.labelRight->pixmap()->rect().height()) / 2;

			pt = event->globalPos();
			pt = ui.labelRight->mapFromGlobal(pt);
			pt = pt - QPoint(xoffset, yoffset);
			labelPoints.push_back(pt);

			ui.textBrowser->append(QString::fromLocal8Bit("右图的图像坐标：") + QString::number(pt.x()*5.12) + QString("  ") + QString::number(pt.y()*5.12));
			QCoreApplication::processEvents();
		}

		//选择了两对点后开始计算两点距离
		if (labelPoints.size() == 4)
		{
			Point3f Pw1 = uv2xyz(Point2f(labelPoints[0].x()*5.12, labelPoints[0].y()*5.12), Point2f(labelPoints[1].x()*5.12, labelPoints[1].y()*5.12));
			Point3f Pw2 = uv2xyz(Point2f(labelPoints[2].x()*5.12, labelPoints[2].y()*5.12), Point2f(labelPoints[3].x()*5.12, labelPoints[3].y()*5.12));
			double distance = sqrt((Pw1.x - Pw2.x)*(Pw1.x - Pw2.x) + (Pw1.z - Pw2.z)*(Pw1.y - Pw2.y) + (Pw1.z - Pw2.z)*(Pw1.z - Pw2.z));
			
			ui.textBrowser->append(QString::fromLocal8Bit("第一个点的坐标："));
			ui.textBrowser->append(QString::number(Pw1.x) + QString("  ") + QString::number(Pw1.y) + QString("  ") + QString::number(Pw1.z));
			ui.textBrowser->append(QString::fromLocal8Bit("第二个点的坐标："));
			ui.textBrowser->append(QString::number(Pw2.x) + QString("  ") + QString::number(Pw2.y) + QString("  ") + QString::number(Pw2.z));
			ui.textBrowser->append(QString::fromLocal8Bit("两点之间的距离为：") + QString::number(distance));

			labelPoints.clear();
		}
	}
}

void chuixianyi::on_autoMatchBtn_clicked()
{
	//测量按钮点击一次开始测量，再次点击就退出测量
	if (flagStartCamera && !flagEndCamera)
	{
		if (flagIsMatched)
		{
			timer->start();
			flagIsMatched = false;
			return;
		}
	}
	flagIsMatched = true;

	prepareFrameLR();
	readRectifyParams();

	//这里就将校正后的frameLeft和frameRight显示在label上了
	computeDisparityImage(frameLeft, frameRight, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	ui.textBrowser->append(QString::fromLocal8Bit("已将左右图像校正!"));

	//-- 读取图像
	Mat img1 = img1_rectified;
	Mat img2 = img2_rectified;
	//Mat img1 = imread(DATA_FOLDER + string("左相机校正图像.jpg"));
	//Mat img2 = imread(DATA_FOLDER + string("右相机校正图像.jpg"));




	//SURF算子	KNN特征匹配
	vector<KeyPoint> keypoints_1, keypoints_2;
	Mat descriptor_1, descriptor_2;
	int minHessian = 400;
	Ptr<Feature2D> surf = xfeatures2d::SURF::create(minHessian);

	surf->detectAndCompute(img1, noArray(), keypoints_1, descriptor_1);
	surf->detectAndCompute(img2, noArray(), keypoints_2, descriptor_2);
	cout << keypoints_1.size() << " " << keypoints_2.size() << endl;
	Mat outimg1;
	drawKeypoints(img1, keypoints_1, outimg1, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
	imwrite(DATA_FOLDER + string("SURF特征点.jpg"), outimg1);
	ui.textBrowser->append(QString::fromLocal8Bit("已提取SURF特征点!"));
	QCoreApplication::processEvents();

	vector<DMatch> matches;
	vector<vector<DMatch>> knn_matches;

	BFMatcher matcher(NORM_L2);
	matcher.knnMatch(descriptor_1, descriptor_2, knn_matches, 2);

	for (size_t r = 0; r < knn_matches.size(); ++r)
	{
		if (knn_matches[r][0].distance > 0.8*knn_matches[r][1].distance) continue;
		matches.push_back(knn_matches[r][0]);
	}

	Mat img_match;
	Mat img_goodmatch;
	drawMatches(img1, keypoints_1, img2, keypoints_2, matches, img_goodmatch);
	imwrite(DATA_FOLDER + string("SURF_KNN特征匹配.jpg"), img_goodmatch);
	ui.textBrowser->append(QString::fromLocal8Bit("已完成KNN特征匹配!"));
	QCoreApplication::processEvents();


	//https://blog.csdn.net/m0_37598482/article/details/78782252
	//极线约束和RANSAC算法剔除误匹配
	int ptCount = (int)matches.size();
	Mat p1(ptCount, 2, CV_32F);
	Mat p2(ptCount, 2, CV_32F);

	// 把Keypoint转换为Mat  
	Point2f pt;
	for (int i = 0; i<ptCount; i++)
	{
		pt = keypoints_1[matches[i].queryIdx].pt;
		p1.at<float>(i, 0) = pt.x;
		p1.at<float>(i, 1) = pt.y;

		pt = keypoints_2[matches[i].trainIdx].pt;
		p2.at<float>(i, 0) = pt.x;
		p2.at<float>(i, 1) = pt.y;
	}
	// 用RANSAC方法计算F  
	// Mat m_Fundamental;  
	// 上面这个变量是基本矩阵  
	vector<uchar> m_RANSACStatus;
	// 上面这个变量已经定义过，用于存储RANSAC后每个点的状态  
	//m_Fundamental = findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);  
	Mat m_Fundamental = findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);
	cout << m_Fundamental << endl;
	// 计算野点个数  
	int OutlinerCount = 0;
	for (int i = 0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] == 0) // 状态为0表示野点  
		{
			OutlinerCount++;
		}
	}

	// 计算内点  
	vector<Point2f> m_LeftInlier;
	vector<Point2f> m_RightInlier;
	vector<DMatch> m_InlierMatches;
	// 上面三个变量用于保存内点和匹配关系  
	int InlinerCount = ptCount - OutlinerCount;
	m_InlierMatches.resize(InlinerCount);
	m_LeftInlier.resize(InlinerCount);
	m_RightInlier.resize(InlinerCount);
	InlinerCount = 0;
	for (int i = 0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] != 0)
		{
			m_LeftInlier[InlinerCount].x = p1.at<float>(i, 0);
			m_LeftInlier[InlinerCount].y = p1.at<float>(i, 1);
			m_RightInlier[InlinerCount].x = p2.at<float>(i, 0);
			m_RightInlier[InlinerCount].y = p2.at<float>(i, 1);
			m_InlierMatches[InlinerCount].queryIdx = InlinerCount;
			m_InlierMatches[InlinerCount].trainIdx = InlinerCount;
			InlinerCount++;
		}
	}

	// 把内点转换为drawMatches可以使用的格式  
	vector<KeyPoint> key1(InlinerCount);
	vector<KeyPoint> key2(InlinerCount);
	KeyPoint::convert(m_LeftInlier, key1);
	KeyPoint::convert(m_RightInlier, key2);
	//for (int i = 0; i<10; i++)
	//	cout << key1[i].pt << " " << key2[i].pt << " " << key1[i].pt.x - key2[i].pt.x << endl;

	Mat img_inliermatches;
	drawMatches(img1, key1, img2, key2, m_InlierMatches, img_inliermatches);
	imwrite(DATA_FOLDER + string("SURF_KNN_RANSAC特征匹配.jpg"), img_inliermatches);
	ui.textBrowser->append(QString::fromLocal8Bit("已完成极线约束RANSAC剔除误匹配!"));
	QCoreApplication::processEvents();



	//读取KeyPoint中的对应关系	并计算对应点的空间坐标
	int inlier_ptCount = (int)m_InlierMatches.size();
	Mat p3(ptCount, 2, CV_32F);
	Mat p4(ptCount, 2, CV_32F);
	Point2f pt_l, pt_r;
	Point3f real_pt;
	fstream fs(DATA_FOLDER + string("匹配点的坐标.txt"), ios::out);
	if (fs.is_open())
	{
		for (int i = 0; i < inlier_ptCount; ++i)
		{
			pt_l = key1[m_InlierMatches[i].queryIdx].pt;;
			p3.at<float>(i, 0) = pt_l.x;
			p3.at<float>(i, 1) = pt_l.y;

			pt_r = key2[m_InlierMatches[i].trainIdx].pt;
			p4.at<float>(i, 0) = pt_r.x;
			p4.at<float>(i, 1) = pt_r.y;

			real_pt = uv2xyz(pt_l, pt_r);
			fs << real_pt.x << " " << real_pt.y << " " << real_pt.z << endl;
		}
	}
	fs.close();
	ui.textBrowser->append(QString::fromLocal8Bit("已写入匹配点的三维坐标!"));
	QCoreApplication::processEvents();
	















	//Mat test(img1.rows, img1.cols, CV_32FC1);
	//float a = 0.0105591872, b = 0.1024243227, c = 10.106085341;
	//for (int i = 0; i<test.rows; i++)
	//{
	//	float* pt = test.ptr<float>(i);
	//	for (int j = 0; j<test.cols; j++)
	//	{
	//		float val = j*a + i*b + c;
	//		pt[j] = val;
	//	}
	//}
	////cout<<test<<endl;  
	//cv::normalize(test, test, 0, 1, CV_MINMAX);
	//cv::convertScaleAbs(test, test, 255);
	//test.convertTo(test, CV_8UC1);
	//imwrite(DATA_FOLDER + string("test.jpg"), test);











	
	
	//ORB算子	BF特征匹配

	////初始化
	//vector<KeyPoint> keypoints_1, keypoints_2;   //关键点/角点
	//											 /**
	//											 opencv中keypoint类的默认构造函数为：
	//											 CV_WRAP KeyPoint() : pt(0,0), size(0), angle(-1), response(0), octave(0), class_id(-1) {}
	//											 pt(x,y):关键点的点坐标； // size():该关键点邻域直径大小； // angle:角度，表示关键点的方向，值为[0,360)，负值表示不使用。
	//											 response:响应强度，选择响应最强的关键点;   octacv:从哪一层金字塔得到的此关键点。
	//											 class_id:当要对图片进行分类时，用class_id对每个关键点进行区分，默认为-1。
	//											 **/
	//Mat descriptors_1, descriptors_2;      //描述子
	//									   //创建ORB对象，参数为默认值
	//Ptr<FeatureDetector> detector = ORB::create();
	//Ptr<DescriptorExtractor> descriptor = ORB::create();
	//Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
	///**
	//“Ptr<FeatureDetector> detector = ”等价于 “FeatureDetector * detector =”
	//Ptr是OpenCV中使用的智能指针模板类，可以轻松管理各种类型的指针。
	//特征检测器FeatureDetetor是虚类，通过定义FeatureDetector的对象可以使用多种特征检测及匹配方法，通过create()函数调用。
	//描述子提取器DescriptorExtractor是提取关键点的描述向量类抽象基类。
	//描述子匹配器DescriptorMatcher用于特征匹配，"Brute-force-Hamming"表示使用汉明距离进行匹配。
	//**/

	////第一步，检测Oriented Fast角点位置
	//chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
	//detector->detect(img_1, keypoints_1);     //对参数1图像进行特征的提取，并存放入参数2的数组中
	//detector->detect(img_2, keypoints_2);

	////第二步，根据角点计算BREIF描述子
	//descriptor->compute(img_1, keypoints_1, descriptors_1);   //computer()计算关键点的描述子向量（注意思考参数设置的合理性）
	//descriptor->compute(img_2, keypoints_2, descriptors_2);
	//chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
	//chrono::duration<double> time_used = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
	//cout << "extract ORB cost = " << time_used.count() << " seconds. " << endl;
	//Mat outimg1;
	//drawKeypoints(img_1, keypoints_1, outimg1, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
	////imshow("ORB features", outimg1);
	//imwrite(DATA_FOLDER + string("ORB特征点.jpg"), outimg1);

	////第三步， 对两幅图像中的描述子进行匹配，使用hamming距离
	//vector<DMatch> matches;    //DMatch是匹配关键点描述子 类, matches用于存放匹配项
	//t1 = chrono::steady_clock::now();
	//matcher->match(descriptors_1, descriptors_2, matches); //对参数1 2的描述子进行匹配，并将匹配项存放于matches中
	//t2 = chrono::steady_clock::now();
	//time_used = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
	//cout << "match the ORB cost: " << time_used.count() << "seconds. " << endl;

	////第四步，匹配点对筛选
	////计算最小距离和最大距离
	//auto min_max = minmax_element(matches.begin(), matches.end(),
	//	[](const DMatch &m1, const DMatch &m2) { return m1.distance < m2.distance; });
	//// auto 可以在声明变量的时候根据变量初始值的类型自动为此变量选择匹配的类型
	//// minmax_element()返回指向范围内最小和最大元素的一对迭代器。参数1 2为起止迭代器范围
	//// 参数3是二进制函数，该函数接受范围内的两个元素作为参数，并返回可转换为bool的值。
	//// 返回的值指示作为第一个参数传递的元素是否小于第二个。该函数不得修改其任何参数。
	//double min_dist = min_max.first->distance;  // min_max存储了一堆迭代器，first指向最小元素
	//double max_dist = min_max.second->distance; // second指向最大元素

	//printf("-- Max dist : %f \n", max_dist);
	//printf("-- Min dist : %f \n", min_dist);

	////当描述子之间的距离大于两倍最小距离时，就认为匹配有误。但有时最小距离会非常小，所以要设置一个经验值30作为下限。
	//vector<DMatch> good_matches;  //存放良好的匹配项
	//for (int i = 0; i < descriptors_1.rows; ++i)
	//{
	//	if (matches[i].distance <= max(2 * min_dist, 30.0))
	//	{
	//		good_matches.push_back(matches[i]);
	//	}
	//}

	////第五步，绘制匹配结果
	//Mat img_match;         //存放所有匹配点
	//Mat img_goodmatch;     //存放好的匹配点
	//					   // drawMatches用于绘制两幅图像的匹配关键点。
	//					   // 参数1是第一个源图像，参数2是其关键点数组；参数3是第二张原图像，参数4是其关键点数组
	//					   // 参数5是两张图像的匹配关键点数组,参数6用于存放函数的绘制结果
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_match);
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, good_matches, img_goodmatch);
	//imwrite(DATA_FOLDER + string("所有匹配点对.jpg"), img_match);
	//imwrite(DATA_FOLDER + string("优化后匹配点对.jpg"), img_goodmatch);
	//waitKey(0);


}

void chuixianyi::on_detectCrossBtn_clicked()
{
	//测量按钮点击一次开始测量，再次点击就退出测量
	if (flagStartCamera && !flagEndCamera)
	{
		if (flagIsDetectedCross)
		{
			timer->start();
			flagIsDetectedCross = false;
			return;
		}
	}
	flagIsDetectedCross = true;

	prepareFrameLR();
	readRectifyParams();

	//这里就将校正后的frameLeft和frameRight显示在label上了
	computeDisparityImage(frameLeft, frameRight, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	ui.textBrowser->append(QString::fromLocal8Bit("已将左右图像校正!"));

	//载入原始图
	g_srcImage = img1_rectified.clone();
	cvtColor(g_srcImage, g_srcImage, COLOR_GRAY2RGB);

	GetRedComponetBySplit(g_srcImage, g_redImage);	//提取红色分量
	cvtColor(g_redImage, g_grayImage, COLOR_RGB2GRAY);	//显示灰度图 
	imwrite(DATA_FOLDER + string("Grey_imageLeft.jpg"), g_grayImage);

	//二值化
	threshold(g_grayImage, imgHSVMask, threshold_value, 255, THRESH_BINARY);
	g_midImage = Mat::zeros(imgHSVMask.size(), CV_8UC1);  //绘制

	Delete_smallregions(imgHSVMask, g_midImage);		//去除小面积区域
	imwrite(DATA_FOLDER + string("Target_imageLeft.jpg"), g_midImage);

	//normalizeLetter显示效果图  
	normalizeLetter(g_midImage, g_dstImage);

	//曲线映射到原图
	/*	threshold(g_grayImage, g_midImage, threshold_value, 255, CV_THRESH_BINARY);	*/
	/*	imshow("【二值化图】", g_midImage);											*/
	Line_reflect(g_dstImage, g_midImage);
	imwrite(DATA_FOLDER + string("Reflect_imageLeft.jpg"), g_midImage);

	//转换类型，保存skeleton图像
	normalize(g_dstImage, g_midImage, 0, 255, NORM_MINMAX, CV_8U);
	imwrite(DATA_FOLDER + string("Thinning_imageLeft.jpg"), g_midImage);




	//载入原始图
	g_srcImage = img2_rectified.clone();
	cvtColor(g_srcImage, g_srcImage, COLOR_GRAY2RGB);

	GetRedComponetBySplit(g_srcImage, g_redImage);	//提取红色分量
	cvtColor(g_redImage, g_grayImage, COLOR_RGB2GRAY);	//显示灰度图 
	imwrite(DATA_FOLDER + string("Grey_imageRight.jpg"), g_grayImage);

	//二值化
	threshold(g_grayImage, imgHSVMask, threshold_value, 255, THRESH_BINARY);
	g_midImage = Mat::zeros(imgHSVMask.size(), CV_8UC1);  //绘制

	Delete_smallregions(imgHSVMask, g_midImage);		//去除小面积区域
	imwrite(DATA_FOLDER + string("Target_imageRight.jpg"), g_midImage);

	//normalizeLetter显示效果图  
	normalizeLetter(g_midImage, g_dstImage);

	//曲线映射到原图
	/*	threshold(g_grayImage, g_midImage, threshold_value, 255, CV_THRESH_BINARY);	*/
	/*	imshow("【二值化图】", g_midImage);											*/
	Line_reflect(g_dstImage, g_midImage);
	imwrite(DATA_FOLDER + string("Reflect_imageRight.jpg"), g_midImage);

	//转换类型，保存skeleton图像
	normalize(g_dstImage, g_midImage, 0, 255, NORM_MINMAX, CV_8U);
	imwrite(DATA_FOLDER + string("Thinning_imageRight.jpg"), g_midImage);

	ui.textBrowser->append(QString::fromLocal8Bit("已检测激光光条中心!"));
	QCoreApplication::processEvents();
}

void chuixianyi::GetRedComponetBySplit(Mat srcImg, Mat &red)
{
	//将整幅图中的红色分量都提取出来
	Mat imgROI;
	vector<Mat>channels;
	split(srcImg, channels);
	Mat blueComponet = channels.at(0);
	Mat greenComponet = channels.at(1);
	blueComponet = Mat::zeros(srcImg.size(), CV_8UC1);//Mat相当于指针，会对chnnels.at(0)重新赋值
	greenComponet = Mat::zeros(srcImg.size(), CV_8UC1);
	merge(channels, imgROI);//仅仅保留红色分量，其他分量赋值为0
	red = imgROI;
}

void chuixianyi::ThinSubiteration1(Mat & pSrc, Mat & pDst)
{
	int rows = pSrc.rows;
	int cols = pSrc.cols;
	pSrc.copyTo(pDst);
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			if (pSrc.at<float>(i, j) == 1.0f)
			{
				/// get 8 neighbors
				/// calculate C(p)
				int neighbor0 = (int)pSrc.at<float>(i - 1, j - 1);
				int neighbor1 = (int)pSrc.at<float>(i - 1, j);
				int neighbor2 = (int)pSrc.at<float>(i - 1, j + 1);
				int neighbor3 = (int)pSrc.at<float>(i, j + 1);
				int neighbor4 = (int)pSrc.at<float>(i + 1, j + 1);
				int neighbor5 = (int)pSrc.at<float>(i + 1, j);
				int neighbor6 = (int)pSrc.at<float>(i + 1, j - 1);
				int neighbor7 = (int)pSrc.at<float>(i, j - 1);
				int C = int(~neighbor1 & (neighbor2 | neighbor3)) +
					int(~neighbor3 & (neighbor4 | neighbor5)) +
					int(~neighbor5 & (neighbor6 | neighbor7)) +
					int(~neighbor7 & (neighbor0 | neighbor1));
				if (C == 1)
				{
					/// calculate N
					int N1 = int(neighbor0 | neighbor1) +
						int(neighbor2 | neighbor3) +
						int(neighbor4 | neighbor5) +
						int(neighbor6 | neighbor7);
					int N2 = int(neighbor1 | neighbor2) +
						int(neighbor3 | neighbor4) +
						int(neighbor5 | neighbor6) +
						int(neighbor7 | neighbor0);
					int N = min(N1, N2);
					if ((N == 2) || (N == 3))
					{
						/// calculate criteria 3
						int c3 = (neighbor1 | neighbor2 | ~neighbor4) & neighbor3;
						if (c3 == 0)
						{
							pDst.at<float>(i, j) = 0.0f;
						}
					}
				}
			}
		}
	}
}

void chuixianyi::ThinSubiteration2(Mat & pSrc, Mat & pDst)
{
	int rows = pSrc.rows;
	int cols = pSrc.cols;
	pSrc.copyTo(pDst);
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			if (pSrc.at<float>(i, j) == 1.0f)
			{
				/// get 8 neighbors
				/// calculate C(p)
				int neighbor0 = (int)pSrc.at<float>(i - 1, j - 1);
				int neighbor1 = (int)pSrc.at<float>(i - 1, j);
				int neighbor2 = (int)pSrc.at<float>(i - 1, j + 1);
				int neighbor3 = (int)pSrc.at<float>(i, j + 1);
				int neighbor4 = (int)pSrc.at<float>(i + 1, j + 1);
				int neighbor5 = (int)pSrc.at<float>(i + 1, j);
				int neighbor6 = (int)pSrc.at<float>(i + 1, j - 1);
				int neighbor7 = (int)pSrc.at<float>(i, j - 1);
				int C = int(~neighbor1 & (neighbor2 | neighbor3)) +
					int(~neighbor3 & (neighbor4 | neighbor5)) +
					int(~neighbor5 & (neighbor6 | neighbor7)) +
					int(~neighbor7 & (neighbor0 | neighbor1));
				if (C == 1)
				{
					/// calculate N
					int N1 = int(neighbor0 | neighbor1) +
						int(neighbor2 | neighbor3) +
						int(neighbor4 | neighbor5) +
						int(neighbor6 | neighbor7);
					int N2 = int(neighbor1 | neighbor2) +
						int(neighbor3 | neighbor4) +
						int(neighbor5 | neighbor6) +
						int(neighbor7 | neighbor0);
					int N = min(N1, N2);
					if ((N == 2) || (N == 3))
					{
						int E = (neighbor5 | neighbor6 | ~neighbor0) & neighbor7;
						if (E == 0)
						{
							pDst.at<float>(i, j) = 0.0f;
						}
					}
				}
			}
		}
	}
}

void chuixianyi::normalizeLetter(Mat & inputarray, Mat & outputarray)
{
	bool bDone = false;
	int rows = inputarray.rows;
	int cols = inputarray.cols;

	inputarray.convertTo(inputarray, CV_32FC1);

	inputarray.copyTo(outputarray);

	outputarray.convertTo(outputarray, CV_32FC1);

	/// pad source
	Mat p_enlarged_src = Mat(rows + 2, cols + 2, CV_32FC1);
	for (int i = 0; i < (rows + 2); i++)
	{
		p_enlarged_src.at<float>(i, 0) = 0.0f;
		p_enlarged_src.at<float>(i, cols + 1) = 0.0f;
	}
	for (int j = 0; j < (cols + 2); j++)
	{
		p_enlarged_src.at<float>(0, j) = 0.0f;
		p_enlarged_src.at<float>(rows + 1, j) = 0.0f;
	}
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			if (inputarray.at<float>(i, j) >= threshold_value)
			{			//调参
				p_enlarged_src.at<float>(i + 1, j + 1) = 1.0f;
			}
			else
				p_enlarged_src.at<float>(i + 1, j + 1) = 0.0f;
		}
	}

	/// start to thin
	Mat p_thinMat1 = Mat::zeros(rows + 2, cols + 2, CV_32FC1);
	Mat p_thinMat2 = Mat::zeros(rows + 2, cols + 2, CV_32FC1);
	Mat p_cmp = Mat::zeros(rows + 2, cols + 2, CV_8UC1);

	while (bDone != true)
	{
		/// sub-iteration 1
		ThinSubiteration1(p_enlarged_src, p_thinMat1);
		/// sub-iteration 2
		ThinSubiteration2(p_thinMat1, p_thinMat2);
		/// compare
		compare(p_enlarged_src, p_thinMat2, p_cmp, CMP_EQ);	//比较输入的src1和src2中的元素，真为255，否则为0
															/// check
		int num_non_zero = countNonZero(p_cmp);					//返回灰度值不为0的像素数
		if (num_non_zero == (rows + 2) * (cols + 2))
		{
			bDone = true;
		}
		/// copy
		p_thinMat2.copyTo(p_enlarged_src);
	}
	// copy result
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			outputarray.at<float>(i, j) = p_enlarged_src.at<float>(i + 1, j + 1);
		}
	}
}

void chuixianyi::Line_reflect(Mat & inputarray, Mat & outputarray)
{
	int rows = inputarray.rows;
	int cols = inputarray.cols;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			if (inputarray.at<float>(i, j) == 1.0f)
			{
				outputarray.at<float>(i, j) = 0.0f;
			}
		}
	}
}

void chuixianyi::Delete_smallregions(Mat & pSrc, Mat & pDst)
{
	// 提取连通区域，并剔除小面积联通区域
	vector<vector<Point>> contours;           //二值图像轮廓的容器
	vector<Vec4i> hierarchy;                  //4个int向量，分别表示后、前、父、子的索引编号
	findContours(pSrc, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);             //检测所有轮廓

	vector<vector<Point>>::iterator k;                    //迭代器，访问容器数据

	for (k = contours.begin(); k != contours.end();)      //遍历容器,设置面积因子
	{
		if (contourArea(*k, false) < msize)
		{//删除指定元素，返回指向删除元素下一个元素位置的迭代器
			k = contours.erase(k);
		}
		else
			++k;
	}

	//contours[i]代表第i个轮廓，contours[i].size()代表第i个轮廓上所有的像素点
	for (int i = 0; i < contours.size(); i++)
	{
		for (int j = 0; j < contours[i].size(); j++)
		{
			//获取轮廓上点的坐标
			Point P = Point(contours[i][j].x, contours[i][j].y);
		}
		drawContours(pDst, contours, i, Scalar(255), -1, 8);
	}
}



void chuixianyi::readRectifyParams()
{
	FileStorage stereoStore(DATA_FOLDER + string(stereoCalibrate_result_L), FileStorage::READ);
	stereoStore["cameraMatrix1"] >> cameraMatrix_L;
	stereoStore["cameraMatrix2"] >> cameraMatrix_R;
	stereoStore["distCoeffs1"] >> distCoeffs_L;
	stereoStore["distCoeffs2"] >> distCoeffs_R;
	stereoStore["R"] >> R;
	stereoStore["T"] >> T;
	stereoStore["E"] >> E;
	stereoStore["F"] >> F;
	stereoStore.release();

	FileStorage stereoParamsStore(DATA_FOLDER + string(stereoRectifyParams), FileStorage::READ);
	stereoParamsStore["R1"] >> R1;
	stereoParamsStore["R2"] >> R2;
	stereoParamsStore["P1"] >> P1;
	stereoParamsStore["P2"] >> P2;
	stereoParamsStore["Q"] >> Q;
	//stereoParamsStore["mapl1"] >> mapl1;
	//stereoParamsStore["mapl2"] >> mapl2;
	//stereoParamsStore["mapr1"] >> mapr1;
	//stereoParamsStore["mapr2"] >> mapr2;
	stereoParamsStore.release();


	validRoi[0], validRoi[1] = stereoRectification(stereoRectifyParams, cameraMatrix_L, distCoeffs_L, cameraMatrix_R, distCoeffs_R,
		imageSize, R, T, R1, R2, P1, P2, Q, mapl1, mapl2, mapr1, mapr2);
	ui.textBrowser->append(QString::fromLocal8Bit("已读取相机标定数据!"));
	QCoreApplication::processEvents();
}

void chuixianyi::prepareFrameLR()
{
	if (flagStartCamera && !flagEndCamera)
	{
		//摄像头画面静止
		timer->stop();
		Size size(IMAGE_WIDTH, IMAGE_HEIGHT);

		PvImage *alInputImage = alBuffer->GetImage();
		alInputImage->Alloc(IMAGE_WIDTH, IMAGE_HEIGHT, PvPixelMono8);
		Mat lMat(size, CV_8UC1, alInputImage->GetDataPointer());
		cvtColor(lMat, frameLeft, COLOR_RGB2BGR);

		PvImage *blInputImage = blBuffer->GetImage();
		blInputImage->Alloc(IMAGE_WIDTH, IMAGE_HEIGHT, PvPixelMono8);
		Mat rMat(size, CV_8UC1, blInputImage->GetDataPointer());
		cvtColor(rMat, frameRight, COLOR_RGB2BGR);


		ui.textBrowser->append(QString::fromLocal8Bit("已捕捉摄像头画面!"));
		QCoreApplication::processEvents();
	}
}


//************************************
// Description: 根据左右相机中像素坐标求解空间坐标
// Method:    uv2xyz
// FullName:  uv2xyz
// Access:    public 
// Parameter: Point2f uvLeft
// Parameter: Point2f uvRight
// Returns:   cv::Point3f
// Author:    小白
// Date:      2017/01/10
// History:
//************************************
Point3f chuixianyi::uv2xyz(Point2f uvLeft, Point2f uvRight)
{
	//     [u1]      [xw]                      [u2]      [xw]
	//zc1 *|v1| = Pl*[yw]                  zc2*|v2| = P2*[yw]
	//     [ 1]      [zw]                      [ 1]      [zw]
	//               [1 ]                                [1 ]
	//Mat mLeftRotation = Mat(3, 3, CV_32F, leftRotation);
	//Mat mLeftTranslation = Mat(3, 1, CV_32F, leftTranslation);
	//Mat mLeftRT = Mat(3, 4, CV_32F);//左相机RT矩阵
	//hconcat(mLeftRotation, mLeftTranslation, mLeftRT);
	//Mat mLeftIntrinsic = Mat(3, 3, CV_32F, leftIntrinsic);
	//Mat mLeftP = mLeftIntrinsic * mLeftRT;
	Mat mLeftP = P1;
	//cout<<"左相机P矩阵 = "<<endl<<mLeftP<<endl;

	//Mat mRightRotation = Mat(3, 3, CV_32F, rightRotation);
	//Mat mRightTranslation = Mat(3, 1, CV_32F, rightTranslation);
	//Mat mRightRT = Mat(3, 4, CV_32F);//右相机RT矩阵
	//hconcat(mRightRotation, mRightTranslation, mRightRT);
	//Mat mRightIntrinsic = Mat(3, 3, CV_32F, rightIntrinsic);
	//Mat mRightP = mRightIntrinsic * mRightRT;
	Mat mRightP = P2;
	//cout<<"右相机P矩阵 = "<<endl<<mRightP<<endl;

	//最小二乘法A矩阵
	Mat A = Mat(4, 3, CV_32F);
	A.at<float>(0, 0) = uvLeft.x * mLeftP.at<double>(2, 0) - mLeftP.at<double>(0, 0);
	A.at<float>(0, 1) = uvLeft.x * mLeftP.at<double>(2, 1) - mLeftP.at<double>(0, 1);
	A.at<float>(0, 2) = uvLeft.x * mLeftP.at<double>(2, 2) - mLeftP.at<double>(0, 2);

	A.at<float>(1, 0) = uvLeft.y * mLeftP.at<double>(2, 0) - mLeftP.at<double>(1, 0);
	A.at<float>(1, 1) = uvLeft.y * mLeftP.at<double>(2, 1) - mLeftP.at<double>(1, 1);
	A.at<float>(1, 2) = uvLeft.y * mLeftP.at<double>(2, 2) - mLeftP.at<double>(1, 2);

	A.at<float>(2, 0) = uvRight.x * mRightP.at<double>(2, 0) - mRightP.at<double>(0, 0);
	A.at<float>(2, 1) = uvRight.x * mRightP.at<double>(2, 1) - mRightP.at<double>(0, 1);
	A.at<float>(2, 2) = uvRight.x * mRightP.at<double>(2, 2) - mRightP.at<double>(0, 2);

	A.at<float>(3, 0) = uvRight.y * mRightP.at<double>(2, 0) - mRightP.at<double>(1, 0);
	A.at<float>(3, 1) = uvRight.y * mRightP.at<double>(2, 1) - mRightP.at<double>(1, 1);
	A.at<float>(3, 2) = uvRight.y * mRightP.at<double>(2, 2) - mRightP.at<double>(1, 2);

	//最小二乘法B矩阵
	Mat B = Mat(4, 1, CV_32F);
	B.at<float>(0, 0) = mLeftP.at<double>(0, 3) - uvLeft.x * mLeftP.at<double>(2, 3);
	B.at<float>(1, 0) = mLeftP.at<double>(1, 3) - uvLeft.y * mLeftP.at<double>(2, 3);
	B.at<float>(2, 0) = mRightP.at<double>(0, 3) - uvRight.x * mRightP.at<double>(2, 3);
	B.at<float>(3, 0) = mRightP.at<double>(1, 3) - uvRight.y * mRightP.at<double>(2, 3);

	Mat XYZ = Mat(3, 1, CV_32F);
	//采用SVD最小二乘法求解XYZ
	solve(A, B, XYZ, DECOMP_SVD);

	//cout<<"空间坐标为 = "<<endl<<XYZ<<endl;

	//世界坐标系中坐标
	Point3f world;
	world.x = XYZ.at<float>(0, 0);
	world.y = XYZ.at<float>(1, 0);
	world.z = XYZ.at<float>(2, 0);

	return world;
}


/*
单目标定
参数：
imageList		存放标定图片名称的txt
singleCalibrateResult	存放标定结果的txt
objectPoints	世界坐标系中点的坐标
corners_seq		存放图像中的角点,用于立体标定
cameraMatrix	相机的内参数矩阵
distCoeffs		相机的畸变系数
imageSize		输入图像的尺寸（像素）
patternSize		标定板每行的角点个数, 标定板每列的角点个数 (15, 9)
chessboardSize	棋盘上每个方格的边长（mm）
注意：亚像素精确化时，允许输入单通道，8位或者浮点型图像。由于输入图像的类型不同，下面用作标定函数参数的内参数矩阵和畸变系数矩阵在初始化时也要数据注意类型。
*/
bool chuixianyi::singleCameraCalibrate(const char* imageFolder, const char* imageList, const char* singleCalibrateResult, vector<vector<Point3f>>& objectPoints,
	vector<vector<Point2f>>& corners_seq, Mat& cameraMatrix, Mat& distCoeffs, Size& imageSize, Size patternSize, Size chessboardSize)

{
	int n_boards = 0;
	ifstream imageStore(DATA_FOLDER + string(imageList)); // 打开存放标定图片名称的txt
	FileStorage resultStore(DATA_FOLDER + string(singleCalibrateResult), FileStorage::WRITE);
	//ofstream resultStore(DATA_FOLDER + string(singleCalibrateResult)); // 保存标定结果的txt
	// 开始提取角点坐标
	vector<Point2f> corners; // 存放一张图片的角点坐标 
	string imageName; // 读取的标定图片的名称
	while (getline(imageStore, imageName)) // 读取txt的每一行（每一行存放了一张标定图片的名称）
	{
		n_boards++;
		Mat imageInput = imread(imageFolder + imageName);
		cvtColor(imageInput, imageInput, COLOR_RGB2GRAY);
		imageSize.width = imageInput.cols; // 获取图片的宽度
		imageSize.height = imageInput.rows; // 获取图片的高度
											// 查找标定板的角点
		bool found = findChessboardCorners(imageInput, patternSize, corners); // 最后一个参数int flags的缺省值为：CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE
																			  // 亚像素精确化。在findChessboardCorners中自动调用了cornerSubPix，为了更加精细化，我们自己再调用一次。
		if (found) // 当所有的角点都被找到
		{
			TermCriteria criteria = TermCriteria(TermCriteria::Type::EPS + TermCriteria::Type::MAX_ITER, 40, 0.001); // 终止标准，迭代40次或者达到0.001的像素精度
			cornerSubPix(imageInput, corners, Size(11, 11), Size(-1, -1), criteria);// 由于我们的图像只存较大，将搜索窗口调大一些，（11， 11）为真实窗口的一半，真实大小为（11*2+1， 11*2+1）--（23， 23）
			corners_seq.push_back(corners); // 存入角点序列
											// 绘制角点
											//drawChessboardCorners(imageInput, patternSize, corners, true);
											//imshow("cornersframe", imageInput);
											//waitKey(500); // 暂停0.5s

			Mat imageCorners = imageInput.clone();
			cvtColor(imageCorners, imageCorners, COLOR_GRAY2RGB);
			drawChessboardCorners(imageCorners, patternSize, corners, true);
			string cornersName = "corners_" + imageName;
			imwrite(DATA_FOLDER + string("corners\\") + cornersName, imageCorners);
		}
	}
	//destroyWindow("cornersframe");
	// 进行相机标定
	// 计算角点对应的三维坐标
	int pic, i, j;
	for (pic = 0; pic < n_boards; pic++)
	{
		vector<Point3f> realPointSet;
		for (i = 0; i < patternSize.height; i++)
		{
			for (j = 0; j < patternSize.width; j++)
			{
				Point3f realPoint;
				// 假设标定板位于世界坐标系Z=0的平面
				realPoint.x = j * chessboardSize.width;
				realPoint.y = i * chessboardSize.height;
				realPoint.z = 0;
				realPointSet.push_back(realPoint);
			}
		}
		objectPoints.push_back(realPointSet);
	}
	// 执行标定程序
	vector<Mat> rvec; // 旋转向量
	vector<Mat> tvec; // 平移向量
	cv::calibrateCamera(objectPoints, corners_seq, imageSize, cameraMatrix, distCoeffs, rvec, tvec, 0);
	// 保存标定结果
	resultStore << "cameraMatrix" << cameraMatrix;//相机内参数矩阵
	resultStore << "distCoeffs" << distCoeffs;//相机畸变系数
											  // 计算重投影点，与原图角点比较，得到误差
	double errPerImage = 0.; // 每张图像的误差
	double errAverage = 0.; // 所有图像的平均误差
	double totalErr = 0.; // 误差总和
	vector<Point2f> projectImagePoints; // 重投影点
	for (i = 0; i < n_boards; i++)
	{
		vector<Point3f> tempObjectPoints = objectPoints[i]; // 临时三维点
															// 计算重投影点
		projectPoints(tempObjectPoints, rvec[i], tvec[i], cameraMatrix, distCoeffs, projectImagePoints);
		// 计算新的投影点与旧的投影点之间的误差
		vector<Point2f> tempCornersPoints = corners_seq[i];// 临时存放旧投影点
		Mat tempCornersPointsMat = Mat(1, tempCornersPoints.size(), CV_32FC2); // 定义成两个通道的Mat是为了计算误差
		Mat projectImagePointsMat = Mat(1, projectImagePoints.size(), CV_32FC2);
		// 赋值
		for (int j = 0; j < tempCornersPoints.size(); j++)
		{
			projectImagePointsMat.at<Vec2f>(0, j) = Vec2f(projectImagePoints[j].x, projectImagePoints[j].y);
			tempCornersPointsMat.at<Vec2f>(0, j) = Vec2f(tempCornersPoints[j].x, tempCornersPoints[j].y);
		}
		// opencv里的norm函数其实把这里的两个通道分别分开来计算的(X1-X2)^2的值，然后统一求和，最后进行根号
		errPerImage = norm(tempCornersPointsMat, projectImagePointsMat, NORM_L2) / (patternSize.width * patternSize.height);
		totalErr += errPerImage;
		//resultStore << "No." + to_string(i + 1) + "errPerImage" << errPerImage;
	}
	//resultStore << "totalErr/n_boards" << totalErr / n_boards;
	imageStore.close();
	resultStore.release();
	return true;
}

/*
双目标定:计算两摄像机相对旋转矩阵 R,平移向量 T, 本征矩阵E, 基础矩阵F
参数：
stereoCalibrateResult	存放立体标定结果的txt
objectPoints			三维点
imagePoints				二维图像上的点
cameraMatrix			相机内参数
distCoeffs				相机畸变系数
imageSize				图像尺寸
R		左右相机相对的旋转矩阵
T		左右相机相对的平移向量
E		本征矩阵
F		基础矩阵
*/
bool chuixianyi::stereoCalibrate(const char* stereoCalibrateResult, vector<vector<Point3f>> objectPoints, vector<vector<Point2f>> imagePoints1, vector<vector<Point2f>> imagePoints2,
	Mat& cameraMatrix1, Mat& distCoeffs1, Mat& cameraMatrix2, Mat& distCoeffs2, Size& imageSize, Mat& R, Mat& T, Mat& E, Mat& F)

{
	//ofstream stereoStore(DATA_FOLDER + string(stereoCalibrateResult));
	FileStorage stereoStore(DATA_FOLDER + string(stereoCalibrateResult), FileStorage::WRITE);
	TermCriteria criteria = TermCriteria(TermCriteria::COUNT | TermCriteria::EPS, 30, 1e-6); // 终止条件
	cv::stereoCalibrate(objectPoints, imagePoints1, imagePoints2, cameraMatrix1, distCoeffs1,
		cameraMatrix2, distCoeffs2, imageSize, R, T, E, F, CALIB_FIX_INTRINSIC, criteria); // 注意参数顺序，可以到保存的文件中查看，避免返回时出错
	stereoStore << "cameraMatrix1" << cameraMatrix1;
	stereoStore << "cameraMatrix2" << cameraMatrix2;
	stereoStore << "distCoeffs1" << distCoeffs1;
	stereoStore << "distCoeffs2" << distCoeffs2;
	stereoStore << "R" << R;
	stereoStore << "T" << T;
	stereoStore << "E" << E;
	stereoStore << "F" << F;
	stereoStore.release();
	return true;
}

/*
立体校正
参数：
stereoRectifyParams	存放立体校正结果的txt
cameraMatrix			相机内参数
distCoeffs				相机畸变系数
imageSize				图像尺寸
R						左右相机相对的旋转矩阵
T						左右相机相对的平移向量
R1, R2					行对齐旋转校正
P1, P2					左右投影矩阵
Q						重投影矩阵
map1, map2				重投影映射表
*/
Rect chuixianyi::stereoRectification(const char* stereoRectifyParams, Mat& cameraMatrix1, Mat& distCoeffs1, Mat& cameraMatrix2, Mat& distCoeffs2,
	Size& imageSize, Mat& R, Mat& T, Mat& R1, Mat& R2, Mat& P1, Mat& P2, Mat& Q, Mat& mapl1, Mat& mapl2, Mat& mapr1, Mat& mapr2)

{
	Rect validRoi[2];
	//ofstream stereoStore(DATA_FOLDER + string(stereoRectifyParams));
	FileStorage stereoStore(DATA_FOLDER + string(stereoRectifyParams), FileStorage::WRITE);
	cv::stereoRectify(cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2, imageSize,
		R, T, R1, R2, P1, P2, Q, 0, -1, imageSize, &validRoi[0], &validRoi[1]);
	// 计算左右图像的重投影映射表
	stereoStore << "R1" << R1;
	stereoStore << "R2" << R2;
	stereoStore << "P1" << P1;
	stereoStore << "P2" << P2;
	stereoStore << "Q" << Q;
	cout << "R1:" << endl;
	cout << R1 << endl;
	cout << "R2:" << endl;
	cout << R2 << endl;
	cout << "P1:" << endl;
	cout << P1 << endl;
	cout << "P2:" << endl;
	cout << P2 << endl;
	cout << "Q:" << endl;
	cout << Q << endl;
	initUndistortRectifyMap(cameraMatrix1, distCoeffs1, R1, P1, imageSize, CV_32FC1, mapl1, mapl2);
	initUndistortRectifyMap(cameraMatrix2, distCoeffs2, R2, P2, imageSize, CV_32FC1, mapr1, mapr2);
	//stereoStore << "mapl1" << mapl1;
	//stereoStore << "mapl2" << mapl2;
	//stereoStore << "mapr1" << mapr1;
	//stereoStore << "mapr2" << mapr2;
	stereoStore.release();
	return validRoi[0], validRoi[1];
}

/*
计算视差图
参数：
imageName1	左相机拍摄的图像
imageName2	右相机拍摄的图像
img1_rectified	重映射后的左侧相机图像
img2_rectified	重映射后的右侧相机图像
map	重投影映射表
*/
bool chuixianyi::computeDisparityImage(const char* imageName1, const char* imageName2, Mat& img1_rectified,
	Mat& img2_rectified, Mat& mapl1, Mat& mapl2, Mat& mapr1, Mat& mapr2, Rect validRoi[2], Mat& disparity)

{
	// 首先，对左右相机的两张图片进行重构
	Mat img1 = imread(string(imageName1));
	Mat img2 = imread(string(imageName2));
	frameLeft = img1.clone();
	frameRight = img2.clone();
	if (img1.empty() | img2.empty())
	{
		cout << "图像为空" << endl;
	}
	Mat gray_img1, gray_img2;
	cvtColor(img1, gray_img1, COLOR_BGR2GRAY);
	cvtColor(img2, gray_img2, COLOR_BGR2GRAY);
	Mat canvas(imageSize.height, imageSize.width * 2, CV_8UC1); // 注意数据类型
	Mat canLeft = canvas(Rect(0, 0, imageSize.width, imageSize.height));
	Mat canRight = canvas(Rect(imageSize.width, 0, imageSize.width, imageSize.height));
	gray_img1.copyTo(canLeft);
	gray_img2.copyTo(canRight);
	imwrite(DATA_FOLDER + string("校正前左右相机图像.jpg"), canvas);
	remap(gray_img1, img1_rectified, mapl1, mapl2, INTER_LINEAR);
	remap(gray_img2, img2_rectified, mapr1, mapr2, INTER_LINEAR);
	imwrite(DATA_FOLDER + string("左相机校正图像.jpg"), img1_rectified);
	imwrite(DATA_FOLDER + string("右相机校正图像.jpg"), img2_rectified);

	QImage QImage1 = CvMat2QImage(img1_rectified);
	QImage QImage2 = CvMat2QImage(img2_rectified);
	QPixmap QPixmap1 = QPixmap::fromImage(QImage1);
	QPixmap QPixmap2 = QPixmap::fromImage(QImage2);
	QPixmap1 = QPixmap1.scaled(500, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap2 = QPixmap2.scaled(500, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.labelLeft->setPixmap(QPixmap1);
	ui.labelRight->setPixmap(QPixmap2);

	img1_rectified.copyTo(canLeft);
	img2_rectified.copyTo(canRight);
	rectangle(canLeft, validRoi[0], Scalar(255, 255, 255), 5, 8);
	rectangle(canRight, validRoi[1], Scalar(255, 255, 255), 5, 8);
	for (int j = 0; j <= canvas.rows; j += 16)
		line(canvas, Point(0, j), Point(canvas.cols, j), Scalar(0, 255, 0), 1, 8);
	imwrite(DATA_FOLDER + string("校正后左右相机图像.jpg"), canvas);
	// 进行立体匹配
	Ptr<StereoBM> bm = StereoBM::create(16, 9); // Ptr<>是一个智能指针
	bm->compute(img1_rectified, img2_rectified, disparity); // 计算视差图
	disparity.convertTo(disparity, CV_32F, 1.0 / 16);
	// 归一化视差映射
	normalize(disparity, disparity, 0, 256, NORM_MINMAX, -1);
	return true;
}
bool chuixianyi::computeDisparityImage(Mat& img1, Mat& img2, Mat& img1_rectified,
	Mat& img2_rectified, Mat& mapl1, Mat& mapl2, Mat& mapr1, Mat& mapr2, Rect validRoi[2], Mat& disparity)

{
	// 首先，对左右相机的两张图片进行重构
	if (img1.empty() | img2.empty())
	{
		cout << "图像为空" << endl;
	}
	Mat gray_img1, gray_img2;
	cvtColor(img1, gray_img1, COLOR_BGR2GRAY);
	cvtColor(img2, gray_img2, COLOR_BGR2GRAY);
	Mat canvas(imageSize.height, imageSize.width * 2, CV_8UC1); // 注意数据类型
	Mat canLeft = canvas(Rect(0, 0, imageSize.width, imageSize.height));
	Mat canRight = canvas(Rect(imageSize.width, 0, imageSize.width, imageSize.height));
	gray_img1.copyTo(canLeft);
	gray_img2.copyTo(canRight);
	imwrite(DATA_FOLDER + string("校正前左右相机图像.jpg"), canvas);
	remap(gray_img1, img1_rectified, mapl1, mapl2, INTER_LINEAR);
	remap(gray_img2, img2_rectified, mapr1, mapr2, INTER_LINEAR);
	imwrite(DATA_FOLDER + string("左相机校正图像.jpg"), img1_rectified);
	imwrite(DATA_FOLDER + string("右相机校正图像.jpg"), img2_rectified);

	QImage QImage1 = CvMat2QImage(img1_rectified);
	QImage QImage2 = CvMat2QImage(img2_rectified);
	QPixmap QPixmap1 = QPixmap::fromImage(QImage1);
	QPixmap QPixmap2 = QPixmap::fromImage(QImage2);
	QPixmap1 = QPixmap1.scaled(LABEL_WIDTH, LABEL_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap2 = QPixmap2.scaled(LABEL_WIDTH, LABEL_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.labelLeft->setPixmap(QPixmap1);
	ui.labelRight->setPixmap(QPixmap2);

	img1_rectified.copyTo(canLeft);
	img2_rectified.copyTo(canRight);
	rectangle(canLeft, validRoi[0], Scalar(255, 255, 255), 5, 8);
	rectangle(canRight, validRoi[1], Scalar(255, 255, 255), 5, 8);
	for (int j = 0; j <= canvas.rows; j += 16)
		line(canvas, Point(0, j), Point(canvas.cols, j), Scalar(0, 255, 0), 1, 8);
	imwrite(DATA_FOLDER + string("校正后左右相机图像.jpg"), canvas);
	// 进行立体匹配
	Ptr<StereoBM> bm = StereoBM::create(16, 9); // Ptr<>是一个智能指针
	bm->compute(img1_rectified, img2_rectified, disparity); // 计算视差图
	disparity.convertTo(disparity, CV_32F, 1.0 / 16);
	// 归一化视差映射
	normalize(disparity, disparity, 0, 256, NORM_MINMAX, -1);
	return true;
}






void chuixianyi::on_planeCalibrate_clicked()
{
	//读取相机标定的数据
	//校正图片

	prepareFrameLR();
	readRectifyParams();

	//这里就将校正后的frameLeft和frameRight显示在label上了
	computeDisparityImage(frameLeft, frameRight, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	ui.textBrowser->append(QString::fromLocal8Bit("已将左右图像校正!"));


	lightPlaneCali(cameraMatrix_L, distCoeffs_L, mapl1, mapl2);
	lightPlaneCali(cameraMatrix_R, distCoeffs_R, mapr1, mapr2);
}

void chuixianyi::lightPlaneCali(Mat &cameraMatrix, Mat &distCoeffs, Mat &map1, Mat &map2)
{
	
	


}



void chuixianyi::on_liveShowCoordinate_clicked()
{

}
