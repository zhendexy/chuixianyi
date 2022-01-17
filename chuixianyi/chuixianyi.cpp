#include "chuixianyi.h"

PV_INIT_SIGNAL_HANDLER();//�������ض���Ĵ��� LNK2005 �������ﲻ����� ���ܷŻ�chuixianyi.h��

chuixianyi::chuixianyi(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	ui.labelLeft->setAlignment(Qt::AlignCenter);
	ui.labelRight->setAlignment(Qt::AlignCenter);

	timer = new QTimer(this);
	timer->setInterval(1000 / rate);//set timer match with FPS
	
	timer->start();

	//�ź���ۺ���
	connect(ui.exitBtn, &QPushButton::clicked, this, &chuixianyi::close);
	connect(timer, SIGNAL(timeout()), this, SLOT(readFrame()));
}





QImage chuixianyi::CvMat2QImage(const Mat &mat)
{
	// ͼ���ͨ��
	int channel = mat.channels();

	// ����һ���� ֱ�Ӳ�ѯ ���� 0 2 ����Чֵ 1 3 4 ��Ӧ��ת��ֵ
	const std::map<int, QImage::Format> img_cvt_map{
		{ 1, QImage::Format_Grayscale8 },
		{ 3, QImage::Format_RGB888 },
		{ 4, QImage::Format_ARGB32 }
	};

	QImage image(mat.data, mat.cols, mat.rows,
		static_cast<int>(mat.step),
		img_cvt_map.at(channel));

	// ��ͨ��ͼ�� ֵ�� ͨ��ת��
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

void chuixianyi::prepareFrameLR()
{
	if (flagStartCamera && !flagEndCamera)
	{
		//����ͷ���澲ֹ
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


		ui.textBrowser->append(QString::fromLocal8Bit("�Ѳ�׽����ͷ����!"));
		QCoreApplication::processEvents();
	}
}

void chuixianyi::on_OpenCVCalibrateBtn_clicked()
{
	FileStorage singleLStore(DATA_FOLDER + string(singleCalibrate_result_L), FileStorage::READ);
	singleLStore["cameraMatrix"] >> cameraMatrix_L;
	singleLStore["distCoeffs"] >> distCoeffs_L;
	singleLStore["rotationVector"] >> rotationVector_L;
	singleLStore["rotationMatrix"] >> rotationMatrix_L;
	singleLStore["translationVector"] >> translationVector_L;
	singleLStore["extrinsicMatrix"] >> extrinsicMatrix_L;
	singleLStore["projectionMatrix"] >> projectionMatrix_L;
	singleLStore.release();


	FileStorage singleRStore(DATA_FOLDER + string(singleCalibrate_result_R), FileStorage::READ);
	singleRStore["cameraMatrix"] >> cameraMatrix_R;
	singleRStore["distCoeffs"] >> distCoeffs_R;
	singleRStore["rotationVector"] >> rotationVector_R;
	singleRStore["rotationMatrix"] >> rotationMatrix_R;
	singleRStore["translationVector"] >> translationVector_R;
	singleRStore["extrinsicMatrix"] >> extrinsicMatrix_R;
	singleRStore["projectionMatrix"] >> projectionMatrix_R;
	singleRStore.release();

	FileStorage stereoStore(DATA_FOLDER + string(stereoCalibrate_result_L), FileStorage::READ);
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

	initUndistortRectifyMap(cameraMatrix_L, distCoeffs_L, Mat(), Mat(), imageSize, CV_32FC1, maplx, maply);
	initUndistortRectifyMap(cameraMatrix_R, distCoeffs_R, Mat(), Mat(), imageSize, CV_32FC1, maprx, mapry);


	ui.textBrowser->append(QString::fromLocal8Bit("�Ѷ�ȡOpenCV����궨����!"));
	QCoreApplication::processEvents();
}

void chuixianyi::on_MATLABCalibrateBtn_clicked()
{
	FileStorage singleLStore(DATA_FOLDER + string("MATLAB����궨���\\Calib_Results_left.yml"), FileStorage::READ);
	singleLStore["cameraMatrix"] >> cameraMatrix_L;
	singleLStore["distCoeffs"] >> distCoeffs_L;
	singleLStore["rotationVector"] >> rotationVector_L;
	singleLStore["translationVector"] >> translationVector_L;
	singleLStore.release();
	Rodrigues(rotationVector_L, rotationMatrix_L);
	hconcat(rotationMatrix_L, translationVector_L, extrinsicMatrix_L);
	projectionMatrix_L = cameraMatrix_L*extrinsicMatrix_L;
	FileStorage matrixLStore(DATA_FOLDER + string("MATLAB����궨���\\Calib_Matrix_left.yml"), FileStorage::WRITE);
	matrixLStore << "rotationVector" << rotationVector_L;
	matrixLStore << "rotationMatrix" << rotationMatrix_L;
	matrixLStore << "translationVector" << translationVector_L;
	matrixLStore << "extrinsicMatrix" << extrinsicMatrix_L;
	matrixLStore << "projectionMatrix" << projectionMatrix_L;
	matrixLStore.release();

	FileStorage singleRStore(DATA_FOLDER + string("MATLAB����궨���\\Calib_Results_right.yml"), FileStorage::READ);
	singleRStore["cameraMatrix"] >> cameraMatrix_R;
	singleRStore["distCoeffs"] >> distCoeffs_R;
	singleRStore["rotationVector"] >> rotationVector_R;
	singleRStore["translationVector"] >> translationVector_R;
	singleRStore.release();
	Rodrigues(rotationVector_R, rotationMatrix_R);
	hconcat(rotationMatrix_R,translationVector_R, extrinsicMatrix_R);
	projectionMatrix_R = cameraMatrix_R*extrinsicMatrix_R;


	ui.textBrowser->append(QString::fromLocal8Bit("�Ѷ�ȡMATLAB����궨����!"));
	QCoreApplication::processEvents();
}

void chuixianyi::on_rayPlaneParamsBtn_clicked()
{
	FileStorage rayLineLStore(DATA_FOLDER + string(rayPlaneCali_result_L), FileStorage::READ);
	rayLineLStore["rayPlaneParams"] >> rayPlaneParams_L;
	rayLineLStore.release();

	FileStorage rayLineRStore(DATA_FOLDER + string(rayPlaneCali_result_R), FileStorage::READ);
	rayLineRStore["rayPlaneParams"] >> rayPlaneParams_R;
	rayLineRStore.release();

	ui.textBrowser->append(QString::fromLocal8Bit("�Ѷ�ȡ����ƽ��궨����!"));
	QCoreApplication::processEvents();
}

void chuixianyi::mousePressEvent(QMouseEvent *event)
{
	if (!flagIsMeasuring && !flagIsMeasuring1)
		return;

	//�������� ��ȡ�������
	if (flagIsMeasuring)
		if (event->button() == Qt::LeftButton)
		{
			//QLabel���������QLbael�����ƫ����
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

				ui.textBrowser->append(QString::fromLocal8Bit("��ͼ��ͼ�����꣺") + QString::number(pt.x()*5.12) + QString("  ") + QString::number(pt.y()*5.12));
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

				ui.textBrowser->append(QString::fromLocal8Bit("��ͼ��ͼ�����꣺") + QString::number(pt.x()*5.12) + QString("  ") + QString::number(pt.y()*5.12));
				QCoreApplication::processEvents();
			}

			//ѡ�������Ե��ʼ�����������
			if (labelPoints.size() == 4)
			{
				Point3f Pw1 = uv2xwywzw(Point2f(labelPoints[0].x()*5.12, labelPoints[0].y()*5.12), Point2f(labelPoints[1].x()*5.12, labelPoints[1].y()*5.12));
				Point3f Pw2 = uv2xwywzw(Point2f(labelPoints[2].x()*5.12, labelPoints[2].y()*5.12), Point2f(labelPoints[3].x()*5.12, labelPoints[3].y()*5.12));
				double distance = sqrt((Pw1.x - Pw2.x)*(Pw1.x - Pw2.x) + (Pw1.z - Pw2.z)*(Pw1.y - Pw2.y) + (Pw1.z - Pw2.z)*(Pw1.z - Pw2.z));

				ui.textBrowser->append(QString::fromLocal8Bit("��һ��������꣺"));
				ui.textBrowser->append(QString::number(Pw1.x) + QString("  ") + QString::number(Pw1.y) + QString("  ") + QString::number(Pw1.z));
				ui.textBrowser->append(QString::fromLocal8Bit("�ڶ���������꣺"));
				ui.textBrowser->append(QString::number(Pw2.x) + QString("  ") + QString::number(Pw2.y) + QString("  ") + QString::number(Pw2.z));
				ui.textBrowser->append(QString::fromLocal8Bit("����֮��ľ���Ϊ��") + QString::number(distance));

				labelPoints.clear();
			}
		}

	//�������� ��ȡ�������
	//�������Ƿ���ռ�����
	if (flagIsMeasuring1)
		if (event->button() == Qt::LeftButton)
		{
			//QLabel���������QLbael�����ƫ����
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

				ui.textBrowser->append(QString::fromLocal8Bit("��ͼ��ͼ�����꣺") + QString::number(pt.x()*5.12) + QString("  ") + QString::number(pt.y()*5.12));
				QCoreApplication::processEvents();
			}
			if (labelPoints.size() == 2)
			{
				Point3f Pw1 = uv2xwywzw(Point2f(labelPoints[0].x()*5.12, labelPoints[0].y()*5.12), projectionMatrix_L);
				Point3f Pw2 = uv2xwywzw(Point2f(labelPoints[1].x()*5.12, labelPoints[1].y()*5.12), projectionMatrix_L);
				double distance = sqrt((Pw1.x - Pw2.x)*(Pw1.x - Pw2.x) + (Pw1.y - Pw2.y)*(Pw1.y - Pw2.y) + (Pw1.z - Pw2.z)*(Pw1.z - Pw2.z));

				ui.textBrowser->append(QString::fromLocal8Bit("��һ��������꣺"));
				ui.textBrowser->append(QString::number(Pw1.x) + QString("  ") + QString::number(Pw1.y) + QString("  ") + QString::number(Pw1.z));
				ui.textBrowser->append(QString::fromLocal8Bit("�ڶ���������꣺"));
				ui.textBrowser->append(QString::number(Pw2.x) + QString("  ") + QString::number(Pw2.y) + QString("  ") + QString::number(Pw2.z));
				ui.textBrowser->append(QString::fromLocal8Bit("����֮��ľ���Ϊ��") + QString::number(distance));

				labelPoints.clear();
			}
		}
}



//******************************************************************
//
//
//					�����
//
//
//******************************************************************
void chuixianyi::on_startCameraBtn_clicked()
{
	timer->start();
	startCamera();
	flagStartCamera = true;
	flagEndCamera = false;
	ui.textBrowser->append(QString::fromLocal8Bit("�Ѵ�����ͷ!"));
	QCoreApplication::processEvents();
}

void chuixianyi::on_endCameraBtn_clicked()
{
	timer->stop();
	endCamera();
	flagStartCamera = false;
	flagEndCamera = true;
	ui.textBrowser->append(QString::fromLocal8Bit("�ѹر�����ͷ!"));
	QCoreApplication::processEvents();
}

void chuixianyi::on_saveFrameBtn_clicked()
{
	if (!flagStartCamera)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���û�д�"));
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

	ui.textBrowser->append(QString::fromLocal8Bit("�ѱ��浱ǰ֡����!"));
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
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("��û��������޷��ر����"));
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
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�Ҳ������"));
		return false;
	}


	// Read device selection, optional new IP address.
	PV_DISABLE_SIGNAL_HANDLER();
	PV_ENABLE_SIGNAL_HANDLER();

	//�����������
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
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("Unable to connect to device."));
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
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("Unable to stream from device."));
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







//******************************************************************
//
//
//					˫Ŀ����ı궨�Ͳ���
//
// ��Ŀ�궨��˫Ŀ�궨��������У��ͼƬ���ֶ�ѡ�������uv2xyz()����
//
//
//******************************************************************
void chuixianyi::on_calibrateBtn_clicked()
{
	if (flagStartCamera && !flagEndCamera)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ȹر������"));
		return;
	}

	singleCameraCalibrate(LEFT_FOLDER, imageList_L, singleCalibrate_result_L, objectPoints_L, corners_seq_L, cameraMatrix_L,
		distCoeffs_L, imageSize, patternSize, chessboardSize);
	ui.textBrowser->append(QString::fromLocal8Bit("�����������ı궨!"));
	QCoreApplication::processEvents();
	singleCameraCalibrate(RIGHT_FOLDER, imageList_R, singleCalibrate_result_R, objectPoints_R, corners_seq_R, cameraMatrix_R,
		distCoeffs_R, imageSize, patternSize, chessboardSize);
	ui.textBrowser->append(QString::fromLocal8Bit("�����������ı궨!"));
	QCoreApplication::processEvents();
	stereoCalibrate(stereoCalibrate_result_L, objectPoints_L, corners_seq_L, corners_seq_R, cameraMatrix_L, distCoeffs_L,
		cameraMatrix_R, distCoeffs_R, imageSize, R, T, E, F);
	ui.textBrowser->append(QString::fromLocal8Bit("�������궨���!"));
	QCoreApplication::processEvents();
	//stereoCalibrate(stereoCalibrate_result_R, objectPoints_R, corners_seq_L, corners_seq_R, cameraMatrix_L, distCoeffs_L,
	//	cameraMatrix_R, distCoeffs_R, imageSize, R2, T2, E2, F2);
	//cout << "���������궨��ɣ�" << endl;
	validRoi[0], validRoi[1] = stereoRectification(stereoRectifyParams, cameraMatrix_L, distCoeffs_L, cameraMatrix_R, distCoeffs_R,
		imageSize, R, T, R1, R2, P1, P2, Q, mapl1, mapl2, mapr1, mapr2);
	ui.textBrowser->append(QString::fromLocal8Bit("�Ѵ���ͼ����ͶӰӳ���!"));
	QCoreApplication::processEvents();
}

void chuixianyi::on_rectifyBtn_clicked()
{
	if (flagStartCamera && !flagEndCamera)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ȹر������"));
		return;
	}


	QStringList file_name_list;
	QStringList file_path_list;
	QStringList path_list = QFileDialog::getOpenFileNames(this,
		QString::fromLocal8Bit("��ѡ��ҪУ�����������ͼƬ"),
		QString::fromLocal8Bit(DATA_FOLDER),
		QString::fromLocal8Bit("Image Files(*.jpg *.png *.bmp *.pgm *.pbm);;All(*.*)"));
	for (int i = 0; i < path_list.size(); i++)
	{
		//�����ļ�·��
		QString path = path_list[i];
		QFileInfo file = QFileInfo(path);
		//����ļ���
		QString file_name = file.fileName();

		file_path_list.append(path);
		file_name_list.append(file_name);
	}

	if (path_list.isEmpty())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("δѡ��ͼƬ������ȡĬ��·����ͼƬ"));
		computeDisparityImage(imageName_L, imageName_R, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	}
	//��ѡ������ͼ��ѡ����ͼҲֻȡǰ����
	if (path_list.size() >= 2)
	{
		//QString·�����˫б��\\����תconst char*·��
		imageNameLeft = file_path_list[0].toStdString();
		imageNameRight = file_path_list[1].toStdString();
		string::size_type pos0 = 0, pos1 = 0;
		while ((pos0 = imageNameLeft.find('/', pos0)) != string::npos)
		{
			imageNameLeft.replace(pos0, 1, "\\", 0, 2);//�滻
		}
		while ((pos1 = imageNameRight.find('/', pos1)) != string::npos)
		{
			imageNameRight.replace(pos1, 1, "\\", 0, 2);//�滻
		}

		computeDisparityImage(imageNameLeft.c_str(), imageNameRight.c_str(), img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	}
	//��ֻѡ��һ�ţ�����ѡһ��
	if (path_list.size() == 1)
	{
		QString another_file_path = QFileDialog::getOpenFileName(this,
			QString::fromLocal8Bit("��ѡ��ҪУ���������ͼƬ"),
			QString::fromLocal8Bit(DATA_FOLDER),
			QString::fromLocal8Bit("Image Files(*.jpg *.png *.bmp *.pgm *.pbm);;All(*.*)"));

		if (another_file_path != "")
		{
			//QString·�����˫б��\\����תconst char*·��
			imageNameLeft = file_path_list[1].toStdString();
			imageNameRight = another_file_path.toStdString();
			string::size_type pos0 = 0, pos1 = 0;
			while ((pos0 = imageNameLeft.find('/', pos0)) != string::npos)
			{
				imageNameLeft.replace(pos0, 1, "\\", 0, 2);//�滻
			}
			while ((pos1 = imageNameRight.find('/', pos1)) != string::npos)
			{
				imageNameRight.replace(pos1, 1, "\\", 0, 2);//�滻
			}

			computeDisparityImage(imageNameLeft.c_str(), imageNameRight.c_str(), img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
		}
		else
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("δѡ�������ͼƬ������ȡĬ��·����ͼƬ"));
			computeDisparityImage(imageName_L, imageName_R, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
		}
	}
	ui.textBrowser->append(QString::fromLocal8Bit("���������ͼ��У��!"));
	QCoreApplication::processEvents();

	// ����άͶӰ������ӳ��
	reprojectImageTo3D(disparity, result3DImage, Q);
	imwrite(DATA_FOLDER + string("�Ӳ�ͼ.jpg"), disparity);
	//namedWindow("�Ӳ�ͼ", WINDOW_NORMAL);
	//imshow("�Ӳ�ͼ", disparity);
	//setMouseCallback("�Ӳ�ͼ", onMouse);
	//waitKey(0);
	//destroyAllWindows();
}

void chuixianyi::on_measureBtn_clicked()
{
	//������ť���һ�ο�ʼ�������ٴε�����˳�����
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

	//����ͽ�У�����frameLeft��frameRight��ʾ��label����
	computeDisparityImage(frameLeft, frameRight, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	ui.textBrowser->append(QString::fromLocal8Bit("�ѽ�����ͼ��У��!"));
	ui.textBrowser->append(QString::fromLocal8Bit("���ֶ�ѡ�����Ե�!"));
	QCoreApplication::processEvents();
	//while (labelPoints.size() != 4)
	//{

	//}
}




/*
��Ŀ�궨
������
imageList		��ű궨ͼƬ���Ƶ�txt
singleCalibrateResult	��ű궨�����txt
objectPoints	��������ϵ�е������
corners_seq		���ͼ���еĽǵ�,��������궨
cameraMatrix	������ڲ�������
distCoeffs		����Ļ���ϵ��
imageSize		����ͼ��ĳߴ磨���أ�
patternSize		�궨��ÿ�еĽǵ����, �궨��ÿ�еĽǵ���� (15, 9)
chessboardSize	������ÿ������ı߳���mm��
ע�⣺�����ؾ�ȷ��ʱ���������뵥ͨ����8λ���߸�����ͼ����������ͼ������Ͳ�ͬ�����������궨�����������ڲ�������ͻ���ϵ�������ڳ�ʼ��ʱҲҪ����ע�����͡�
*/
bool chuixianyi::singleCameraCalibrate(const char* imageFolder, const char* imageList, const char* singleCalibrateResult, vector<vector<Point3f>>& objectPoints,
	vector<vector<Point2f>>& corners_seq, Mat& cameraMatrix, Mat& distCoeffs, Size& imageSize, Size patternSize, Size chessboardSize)

{
	int n_boards = 0;
	ifstream imageStore(DATA_FOLDER + string(imageList)); // �򿪴�ű궨ͼƬ���Ƶ�txt
	FileStorage resultStore(DATA_FOLDER + string(singleCalibrateResult), FileStorage::WRITE); // ����궨�����txt
	// ��ʼ��ȡ�ǵ�����
	vector<Point2f> corners; // ���һ��ͼƬ�Ľǵ����� 
	string imageName; // ��ȡ�ı궨ͼƬ������
	while (getline(imageStore, imageName)) // ��ȡtxt��ÿһ�У�ÿһ�д����һ�ű궨ͼƬ�����ƣ�
	{
		n_boards++;
		Mat imageInput = imread(imageFolder + imageName);
		cvtColor(imageInput, imageInput, COLOR_RGB2GRAY);
		imageSize.width = imageInput.cols; // ��ȡͼƬ�Ŀ��
		imageSize.height = imageInput.rows; // ��ȡͼƬ�ĸ߶�
											// ���ұ궨��Ľǵ�
		bool found = findChessboardCorners(imageInput, patternSize, corners); // ���һ������int flags��ȱʡֵΪ��CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE
																			  // �����ؾ�ȷ������findChessboardCorners���Զ�������cornerSubPix��Ϊ�˸��Ӿ�ϸ���������Լ��ٵ���һ�Ρ�
		if (found) // �����еĽǵ㶼���ҵ�
		{
			TermCriteria criteria = TermCriteria(TermCriteria::Type::EPS + TermCriteria::Type::MAX_ITER, 40, 0.001); // ��ֹ��׼������40�λ��ߴﵽ0.001�����ؾ���
			cornerSubPix(imageInput, corners, Size(11, 11), Size(-1, -1), criteria);// �������ǵ�ͼ��ֻ��ϴ󣬽��������ڵ���һЩ����11�� 11��Ϊ��ʵ���ڵ�һ�룬��ʵ��СΪ��11*2+1�� 11*2+1��--��23�� 23��
			corners_seq.push_back(corners); // ����ǵ�����
											// ���ƽǵ�
											//drawChessboardCorners(imageInput, patternSize, corners, true);
											//imshow("cornersframe", imageInput);
											//waitKey(500); // ��ͣ0.5s

			Mat imageCorners = imageInput.clone();
			cvtColor(imageCorners, imageCorners, COLOR_GRAY2RGB);
			drawChessboardCorners(imageCorners, patternSize, corners, true);
			string cornersName = "corners_" + imageName;
			imwrite(DATA_FOLDER + string("corners\\") + cornersName, imageCorners);
		}
	}
	
	// ��������궨
	int pic, i, j;
	for (pic = 0; pic < n_boards; pic++)
	{
		vector<Point3f> realPointSet;
		for (i = 0; i < patternSize.height; i++)
		{
			for (j = 0; j < patternSize.width; j++)
			{
				Point3f realPoint;
				// ����궨��λ����������ϵZ=0��ƽ��
				realPoint.x = j * chessboardSize.width;
				realPoint.y = i * chessboardSize.height;
				realPoint.z = 0;
				realPointSet.push_back(realPoint);
			}
		}
		objectPoints.push_back(realPointSet);
	}
	// ִ�б궨����
	vector<Mat> rvec; // ��ת����
	vector<Mat> tvec; // ƽ������
	cv::calibrateCamera(objectPoints, corners_seq, imageSize, cameraMatrix, distCoeffs, rvec, tvec, 0);
	// ����궨���
	resultStore << "cameraMatrix" << cameraMatrix; // ����ڲ�������
	resultStore << "distCoeffs" << distCoeffs; // �������ϵ��
	// �����һ��ͼƬ��������
	Mat rotationVector, rotationMatrix, translationVector, extrinsicMatrix, projectionMatrix;
	rotationVector = rvec[0];
	resultStore << "rotationVector" << rotationVector; // �����ת����
	Rodrigues(rotationVector, rotationMatrix);
	resultStore << "rotationMatrix" << rotationMatrix; // �����ת����
	translationVector = tvec[0];
	resultStore << "translationVector" << translationVector; // ���ƽ������
	hconcat(rotationMatrix, translationVector, extrinsicMatrix);
	resultStore << "extrinsicMatrix" << extrinsicMatrix; // ������������
	projectionMatrix = cameraMatrix*extrinsicMatrix;
	resultStore << "projectionMatrix" << projectionMatrix; // ���ͶӰ����
	// ������ͶӰ�㣬��ԭͼ�ǵ�Ƚϣ��õ����
	double errPerImage = 0.; // ÿ��ͼ������
	double errAverage = 0.; // ����ͼ���ƽ�����
	double totalErr = 0.; // ����ܺ�
	vector<Point2f> projectImagePoints; // ��ͶӰ��
	fstream fs_imageErr(DATA_FOLDER + string("errPerImage.txt"), ios::out);
	for (i = 0; i < n_boards; i++)
	{
		vector<Point3f> tempObjectPoints = objectPoints[i]; // ��ʱ��ά��
		// ������ͶӰ��
		projectPoints(tempObjectPoints, rvec[i], tvec[i], cameraMatrix, distCoeffs, projectImagePoints);
		// �����µ�ͶӰ����ɵ�ͶӰ��֮������
		vector<Point2f> tempCornersPoints = corners_seq[i];// ��ʱ��ž�ͶӰ��
		Mat tempCornersPointsMat = Mat(1, tempCornersPoints.size(), CV_32FC2); // ���������ͨ����Mat��Ϊ�˼������
		Mat projectImagePointsMat = Mat(1, projectImagePoints.size(), CV_32FC2);
		// ��ֵ
		for (int j = 0; j < tempCornersPoints.size(); j++)
		{
			projectImagePointsMat.at<Vec2f>(0, j) = Vec2f(projectImagePoints[j].x, projectImagePoints[j].y);
			tempCornersPointsMat.at<Vec2f>(0, j) = Vec2f(tempCornersPoints[j].x, tempCornersPoints[j].y);
		}
		// opencv���norm������ʵ�����������ͨ���ֱ�ֿ��������(X1-X2)^2��ֵ��Ȼ��ͳһ��ͣ������и���
		errPerImage = norm(tempCornersPointsMat, projectImagePointsMat, NORM_L2) / (patternSize.width * patternSize.height);
		totalErr += errPerImage;
		fs_imageErr << "No." + to_string(i + 1) + "errPerImage" << errPerImage << endl;
	}
	fs_imageErr << "totalErr/n_boards" << totalErr / n_boards;
	fs_imageErr.close();
	imageStore.close();
	resultStore.release();
	return true;
}

/*
˫Ŀ�궨:����������������ת���� R,ƽ������ T, ��������E, ��������F
������
stereoCalibrateResult	�������궨�����txt
objectPoints			��ά��
imagePoints				��άͼ���ϵĵ�
cameraMatrix			����ڲ���
distCoeffs				�������ϵ��
imageSize				ͼ��ߴ�
R		���������Ե���ת����
T		���������Ե�ƽ������
E		��������
F		��������
*/
bool chuixianyi::stereoCalibrate(const char* stereoCalibrateResult, vector<vector<Point3f>> objectPoints, vector<vector<Point2f>> imagePoints1, vector<vector<Point2f>> imagePoints2,
	Mat& cameraMatrix1, Mat& distCoeffs1, Mat& cameraMatrix2, Mat& distCoeffs2, Size& imageSize, Mat& R, Mat& T, Mat& E, Mat& F)

{
	FileStorage stereoStore(DATA_FOLDER + string(stereoCalibrateResult), FileStorage::WRITE);
	TermCriteria criteria = TermCriteria(TermCriteria::COUNT | TermCriteria::EPS, 30, 1e-6); // ��ֹ����
	cv::stereoCalibrate(objectPoints, imagePoints1, imagePoints2, cameraMatrix1, distCoeffs1,
		cameraMatrix2, distCoeffs2, imageSize, R, T, E, F, CALIB_FIX_INTRINSIC, criteria); // ע�����˳�򣬿��Ե�������ļ��в鿴�����ⷵ��ʱ����
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
����У��
������
stereoRectifyParams		�������У�������txt
cameraMatrix			����ڲ���
distCoeffs				�������ϵ��
imageSize				ͼ��ߴ�
R						���������Ե���ת����
T						���������Ե�ƽ������
R1, R2					�ж�����תУ��
P1, P2					����ͶӰ����
Q						��ͶӰ����
map1, map2				��ͶӰӳ���
*/
Rect chuixianyi::stereoRectification(const char* stereoRectifyParams, Mat& cameraMatrix1, Mat& distCoeffs1, Mat& cameraMatrix2, Mat& distCoeffs2,
	Size& imageSize, Mat& R, Mat& T, Mat& R1, Mat& R2, Mat& P1, Mat& P2, Mat& Q, Mat& mapl1, Mat& mapl2, Mat& mapr1, Mat& mapr2)

{
	Rect validRoi[2];
	FileStorage stereoStore(DATA_FOLDER + string(stereoRectifyParams), FileStorage::WRITE);
	cv::stereoRectify(cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2, imageSize,
		R, T, R1, R2, P1, P2, Q, 0, -1, imageSize, &validRoi[0], &validRoi[1]);
	// ��������ͼ�����ͶӰӳ���
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
�����Ӳ�ͼ
������
imageName1	����������ͼ��
imageName2	����������ͼ��
img1_rectified	��ӳ����������ͼ��
img2_rectified	��ӳ�����Ҳ����ͼ��
map	��ͶӰӳ���
*/
bool chuixianyi::computeDisparityImage(const char* imageName1, const char* imageName2, Mat& img1_rectified,
	Mat& img2_rectified, Mat& mapl1, Mat& mapl2, Mat& mapr1, Mat& mapr2, Rect validRoi[2], Mat& disparity)

{
	// ���ȣ����������������ͼƬ�����ع�
	Mat img1 = imread(string(imageName1));
	Mat img2 = imread(string(imageName2));
	frameLeft = img1.clone();
	frameRight = img2.clone();
	if (img1.empty() | img2.empty())
	{
		cout << "ͼ��Ϊ��" << endl;
	}
	Mat gray_img1, gray_img2;
	cvtColor(img1, gray_img1, COLOR_BGR2GRAY);
	cvtColor(img2, gray_img2, COLOR_BGR2GRAY);
	Mat canvas(imageSize.height, imageSize.width * 2, CV_8UC1); // ע����������
	Mat canLeft = canvas(Rect(0, 0, imageSize.width, imageSize.height));
	Mat canRight = canvas(Rect(imageSize.width, 0, imageSize.width, imageSize.height));
	gray_img1.copyTo(canLeft);
	gray_img2.copyTo(canRight);
	imwrite(DATA_FOLDER + string("У��ǰ�������ͼ��.jpg"), canvas);
	remap(gray_img1, img1_rectified, mapl1, mapl2, INTER_LINEAR);
	remap(gray_img2, img2_rectified, mapr1, mapr2, INTER_LINEAR);
	imwrite(DATA_FOLDER + string("�����У��ͼ��.jpg"), img1_rectified);
	imwrite(DATA_FOLDER + string("�����У��ͼ��.jpg"), img2_rectified);

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
	imwrite(DATA_FOLDER + string("У�����������ͼ��.jpg"), canvas);
	// ��������ƥ��
	Ptr<StereoBM> bm = StereoBM::create(16, 9); // Ptr<>��һ������ָ��
	bm->compute(img1_rectified, img2_rectified, disparity); // �����Ӳ�ͼ
	disparity.convertTo(disparity, CV_32F, 1.0 / 16);
	// ��һ���Ӳ�ӳ��
	normalize(disparity, disparity, 0, 256, NORM_MINMAX, -1);
	return true;
}
bool chuixianyi::computeDisparityImage(Mat& img1, Mat& img2, Mat& img1_rectified,
	Mat& img2_rectified, Mat& mapl1, Mat& mapl2, Mat& mapr1, Mat& mapr2, Rect validRoi[2], Mat& disparity)

{
	// ���ȣ����������������ͼƬ�����ع�
	if (img1.empty() | img2.empty())
	{
		cout << "ͼ��Ϊ��" << endl;
	}
	Mat gray_img1, gray_img2;
	cvtColor(img1, gray_img1, COLOR_BGR2GRAY);
	cvtColor(img2, gray_img2, COLOR_BGR2GRAY);
	Mat canvas(imageSize.height, imageSize.width * 2, CV_8UC1); // ע����������
	Mat canLeft = canvas(Rect(0, 0, imageSize.width, imageSize.height));
	Mat canRight = canvas(Rect(imageSize.width, 0, imageSize.width, imageSize.height));
	gray_img1.copyTo(canLeft);
	gray_img2.copyTo(canRight);
	imwrite(DATA_FOLDER + string("У��ǰ�������ͼ��.jpg"), canvas);
	remap(gray_img1, img1_rectified, mapl1, mapl2, INTER_LINEAR);
	remap(gray_img2, img2_rectified, mapr1, mapr2, INTER_LINEAR);
	imwrite(DATA_FOLDER + string("�����У��ͼ��.jpg"), img1_rectified);
	imwrite(DATA_FOLDER + string("�����У��ͼ��.jpg"), img2_rectified);

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
	imwrite(DATA_FOLDER + string("У�����������ͼ��.jpg"), canvas);
	// ��������ƥ��
	Ptr<StereoBM> bm = StereoBM::create(16, 9); // Ptr<>��һ������ָ��
	bm->compute(img1_rectified, img2_rectified, disparity); // �����Ӳ�ͼ
	disparity.convertTo(disparity, CV_32F, 1.0 / 16);
	// ��һ���Ӳ�ӳ��
	normalize(disparity, disparity, 0, 256, NORM_MINMAX, -1);
	return true;
}

/*
˫Ŀ�Ӿ�������������
������
uvLeft	�����ͼ������
uvRight	�����ͼ������
P1		���������У�����ͶӰ����
P2		���������У�����ͶӰ����
�����
Point3f	��������	��ʱ��֪������ϵ����
*/
Point3f chuixianyi::uv2xwywzw(Point2f uvLeft, Point2f uvRight)
{
	//     [u1]      [xw]                      [u2]      [xw]
	//zc1 *|v1| = P1*[yw]                  zc2*|v2| = P2*[yw]
	//     [ 1]      [zw]                      [ 1]      [zw]
	//               [1 ]                                [1 ]
	
	Mat mLeftP = P1;
	Mat mRightP = P2;

	// ��С���˷�A����
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

	// ��С���˷�B����
	Mat B = Mat(4, 1, CV_32F);
	B.at<float>(0, 0) = mLeftP.at<double>(0, 3) - uvLeft.x * mLeftP.at<double>(2, 3);
	B.at<float>(1, 0) = mLeftP.at<double>(1, 3) - uvLeft.y * mLeftP.at<double>(2, 3);
	B.at<float>(2, 0) = mRightP.at<double>(0, 3) - uvRight.x * mRightP.at<double>(2, 3);
	B.at<float>(3, 0) = mRightP.at<double>(1, 3) - uvRight.y * mRightP.at<double>(2, 3);

	Mat XYZ = Mat(3, 1, CV_32F);
	// ����SVD��С���˷����XYZ
	solve(A, B, XYZ, DECOMP_SVD);

	// ��������ϵ������
	Point3f world;
	world.x = XYZ.at<float>(0, 0);
	world.y = XYZ.at<float>(1, 0);
	world.z = XYZ.at<float>(2, 0);

	return world;
}





//******************************************************************
//
//
//					����ƽ��궨�Ͳ���
//
// ��ȡ����MATLAB����궨������
// ��ȡ�궨��ͼƬ->��ȡ����������ĵ��ͼ������->��ȡ�궨���Zwֵ
// ->��궨���ϼ����������XwYw����->��С���˷���ϼ���ƽ�����
//
// �궨��ͼƬ������Ҫ�з��⣬�ᵼ���ûҶ����ķ�������ĵ�ƫ��̫��
// �����ñ궨�嵲ס����������
//
//
//******************************************************************
void chuixianyi::on_rayPlaneCalibrateBtn_clicked()
{
	// ���ȶ�ȡ����궨���� OPENCV��MATLAB

	rayPlaneCalibrate(LEFT_FOLDER, rayLineimageList_L, uvPoints_L, realPoints_L, caliPoints_L, caliErr_L, rayPlaneCali_result_L, 
		rayPlaneParams_L, projectionMatrix_L, cameraMatrix_L, distCoeffs_L);
	ui.textBrowser->append(QString::fromLocal8Bit("��������������ƽ��ı궨!"));
	QCoreApplication::processEvents();
	//rayPlaneCalibrate(RIGHT_FOLDER, rayLineimageList_R, uvPoints_R, realPoints_R, caliPoints_R, caliErr_R, rayPlaneCali_result_R,
	//	rayPlaneParams_R, projectionMatrix_R, cameraMatrix_R, distCoeffs_R);
	//ui.textBrowser->append(QString::fromLocal8Bit("��������������ƽ��ı궨!"));
	//QCoreApplication::processEvents();
}

/*
����ƽ��궨
������
imageList		��ű궨ͼƬ���Ƶ�txt
rayPlaneCalibrateResult		��ű궨�����txt
rayPlaneParams	����ƽ��궨���
projectionMatrix�����ͶӰ����
cameraMatrix	������ڲ�������
distCoeffs		����Ļ���ϵ��
*/
bool chuixianyi::rayPlaneCalibrate(const char* imageFolder, const char* imageList, const char* uvPoints, const char* realPoints, const char* caliPoints,
	const char* caliErr, const char*rayPlaneCali_result, Mat& rayPlaneParams, Mat& projectionMatrix, Mat& cameraMatrix, Mat& distCoeffs)
{
	ifstream imageStore(DATA_FOLDER + string(imageList)); // �򿪴�ű궨ͼƬ���Ƶ�txt
	fstream fs_uvPoints(DATA_FOLDER + string(uvPoints), ios::out);
	fstream fs_realPoints(DATA_FOLDER + string(realPoints), ios::out);
	fstream fs_caliPoints(DATA_FOLDER + string(caliPoints), ios::out);
	fstream fs_caliErr(DATA_FOLDER + string(caliErr), ios::out);
	FileStorage rayLineStore(DATA_FOLDER + string(rayPlaneCali_result), FileStorage::WRITE);

	vector<Point> rayLinePoints;
	vector<Point> tempPoints;
	vector<int> zws;
	string imageName; // ��ȡ�ı궨ͼƬ������
	while (getline(imageStore, imageName)) // ��ȡtxt��ÿһ�У�ÿһ�д����һ�ű궨ͼƬ�����ƣ�
	{
		srcImage = imread(imageFolder + imageName);
		Mat srcImage1;
		undistort(srcImage, srcImage1, cameraMatrix, distCoeffs); // ��������
		//remap(srcImage, srcImage1, map1, map2, INTER_LINEAR);
		srcImage = srcImage1;
		imwrite(DATA_FOLDER + string("rectifyImage.jpg"), srcImage);

		string zw_str = "";	// ͨ��ͼƬ�ļ�������ȡzw����		left01_10.jpg	right01_10.jpg
		char c = imageName[0];
		if (c == 'l')
			for (int i = 7; i < imageName.size(); ++i)
			{
				c = imageName[i];
				if (c == '.')
					break;
				zw_str += c;
			}
		else if (c == 'r')
			for (int i = 8; i < imageName.size(); ++i)
			{
				c = imageName[i];
				if (c == '.')
					break;
				zw_str += c;
			}
		int zw = atoi(zw_str.c_str());

		getRayLinePoints(srcImage, tempPoints); // ��ȡ�������������uv����
		for (int i = 0; i < tempPoints.size(); ++i)
		{
			rayLinePoints.push_back(tempPoints[i]);
			zws.push_back(zw);

			fs_uvPoints << tempPoints[i].x << " " << tempPoints[i].y << " " << zw << endl;
		}
		tempPoints.clear();
	}

	int m = rayLinePoints.size();
	Mat L = Mat::ones(m, 1, CV_32FC1);
	Mat realPointsMat = Mat(m, 3, CV_32FC1);
	for (int i = 0; i < m; ++i)
	{
		Point2f xwyw = uvzw2xwyw(rayLinePoints[i], zws[i], projectionMatrix);
		realPointsMat.at<float>(i, 0) = xwyw.x;
		realPointsMat.at<float>(i, 1) = xwyw.y;
		realPointsMat.at<float>(i, 2) = zws[i];

		fs_realPoints << xwyw.x << " " << xwyw.y << " " << zws[i] << endl;
	}
	solve(realPointsMat, L, rayPlaneParams, DECOMP_SVD);
	rayLineStore << "rayPlaneParams" << rayPlaneParams;

	// ����ÿ��������Ŀռ�����
	// ������� ���ݲ����������ֵ-ʵ��ֵ
	for (int i = 0; i < m; ++i)
	{
		Point3f caliPoint;
		uv2xwywzw(rayLinePoints[i], caliPoint, projectionMatrix);
		fs_caliPoints << caliPoint.x << " " << caliPoint.y << " " << caliPoint.z << endl;
		fs_caliErr << caliPoint.x - realPointsMat.at<float>(i, 0) << " " << caliPoint.y - realPointsMat.at<float>(i, 1) << " " << caliPoint.z - realPointsMat.at<float>(i, 2) << endl;
	}


	fs_uvPoints.close();
	fs_realPoints.close();
	fs_caliPoints.close();
	fs_caliErr.close();
	rayLineStore.release();
	return true;
}

/*
��ȡ����������ĵ�����
������
srcImage	����ͼ����ǰ��������
T_rayLine	��ֵ����ֵ
redtolerant
graytolerant
linePoints	��������ĵ�����
*/
void chuixianyi::getRayLinePoints(Mat& srcImage, vector<Point>& linePoints)
{
	cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);

	//midImage = grayImage.clone();
	//threshold(midImage, midImage, T_rayLine, 255, THRESH_BINARY);
	//maskImage = Mat::ones(midImage.size(), CV_8UC1); // ȥ��С��ͨ�����ͼ��mask
	//findSmallRegions(midImage, maskImage);
	//imwrite(DATA_FOLDER + string("maskImage_gray.jpg"), maskImage);
	//bitwise_not(maskImage, maskImage);
	//bitwise_and(grayImage, maskImage, grayImage);
	imwrite(DATA_FOLDER + string("grayImage.jpg"), grayImage);


	vector<Mat> BGR;
	split(srcImage, BGR); // opencv��ȡͼ����BGR��
	redImage = BGR.at(2).clone(); // ȡ��BGR�е�R����

	midImage = redImage.clone();
	threshold(midImage, midImage, T_rayLine, 255, THRESH_BINARY);
	maskImage = Mat::zeros(midImage.size(), CV_8UC1); // ȥ��С��ͨ�����ͼ��mask
	deleteSmallRegions(midImage, maskImage);
	imwrite(DATA_FOLDER + string("maskImage_red.jpg"), maskImage);
	for (int i = 0; i < redImage.rows; ++i)
		for (int j = 0; j < redImage.cols; ++j)
		{
			int v = redImage.at<uchar>(i, j);
			redImage.at<uchar>(i, j) = v > T_rayLine ? v : 0; // redImage��ֵ��
		}
	bitwise_and(redImage, maskImage, redImage);
	imwrite(DATA_FOLDER + string("redImage.jpg"), redImage);

	//�������Ǻ��ŵģ�����ɨ����
	int num = 9;
	int max_red, max_gray;
	cv::Point ij_red, ij_gray;
	vector<int> maxs_red, maxs_gray;
	vector<cv::Point> ijs_red, ijs_gray;
	for (int j = 0; j < redImage.cols; ++j)
	{
		//�ҵ�redImage��һ���лҶ����ĵ�
		max_red = redImage.at<uchar>(0, j);
		ij_red.x = 0;
		ij_red.y = j;
		for (int i = 0; i < redImage.rows; ++i)
		{
			if (redImage.at<uchar>(i, j) > max_red)
			{
				max_red = redImage.at<uchar>(i, j);
				ij_red.x = i;
				ij_red.y = j;
			}
		}
		if (max_red == 0)
			continue;
		//�ҵ�redImage��һ�еĺ�ѡ��
		for (int i = 0; i < redImage.rows; ++i)
		{
			if (redImage.at<uchar>(i, j) > max_red - redtolerant)
			{
				maxs_red.push_back(redImage.at<uchar>(i, j));
				ijs_red.push_back(cv::Point(i, j));
			}
		}


		//��redImage�к�ѡ���Ӧ��grayImage���ҵ���ѡ���лҶ�ֵ���ĵ�
		max_gray = grayImage.at<uchar>(ij_red.x, ij_red.y);
		ij_gray = ij_red;
		for (int i = 0; i < ijs_red.size(); ++i)
		{
			if (grayImage.at<uchar>(ij_red.x, ij_red.y) > max_gray)
			{
				max_gray = grayImage.at<uchar>(ij_red.x, ij_red.y);
				ij_gray.x = i;
				ij_gray.y = j;
			}
		}

		//�ҵ�grayImage��һ�еĺ�ѡ��
		//�����ƺ�ѡ�㲻����9��ò��Ч������
		for (int i = 0; i < grayImage.rows; ++i)
		{
			if (grayImage.at<uchar>(i, j) > max_gray - graytolerant)
			{
				maxs_gray.push_back(grayImage.at<uchar>(i, j));
				ijs_gray.push_back(cv::Point(i, j));
			}
		}

		////�ҵ�grayImage��һ�еĺ�ѡ��
		////��max_gray��λ�ÿ�ʼ�Һ�ѡ��
		//int i = ij_gray.x;
		//maxs_gray.push_back(max_gray);
		//ijs_gray.push_back(ij_gray);
		//for (int n = 1; n < min(i + 1, grayImage.rows - i); ++n)
		//{
		//	if (grayImage.at<uchar>(i - n, j) > max_gray - graytolerant)
		//	{
		//		maxs_gray.push_back(grayImage.at<uchar>(i - n, j));
		//		ijs_gray.push_back(cv::Point(i - n, j));
		//	}
		//	if (grayImage.at<uchar>(i + n, j) > max_gray - graytolerant)
		//	{
		//		maxs_gray.push_back(grayImage.at<uchar>(i + n, j));
		//		ijs_gray.push_back(cv::Point(i + n, j));
		//	}

		//	//���ƺ�ѡ�����������9��
		//	if (maxs_gray.size() > num)
		//		break;
		//}



		//���ѡ��ĻҶ����ģ���Ϊ�������ĵ������
		int u, v;
		int fenzi1 = 0, fenzi2 = 0, fenmu = 0;
		for (int i = 0; i < ijs_gray.size(); ++i)
		{
			fenzi1 += ijs_gray[i].x*grayImage.at<uchar>(ij_red.x, ij_red.y);
			//fenzi2 += ijs_gray[i].y*grayImage.at<uchar>(ij_red.x, ij_red.y);
			fenmu += grayImage.at<uchar>(ij_red.x, ij_red.y);
		}
		u = fenzi1 / fenmu;
		//v = fenzi2 / fenmu;
		v = j;
		//����uv��ָ�к��У�����ת��Ϊuv����
		double temp = v;
		v = u;
		u = temp;
		linePoints.push_back(cv::Point(u, v));

		maxs_red.clear();
		maxs_gray.clear();
		ijs_red.clear();
		ijs_gray.clear();
	}

	//ֱ�߷���
	// pt2((rho - image.rows * sin(theta))/cos(theta), image.rows)
	// pt2(image.cols, (rho - image.cols * cos(theta))/sin(theta))
	Vec4f line;
	fitLine(linePoints, line, DIST_HUBER, 0, 0.01, 0.01);
	double cos_theta = line[0];
	double sin_theta = line[1];
	double u0 = line[2], v0 = line[3];
	double phi = atan2(sin_theta, cos_theta) + CV_PI / 2.0;
	double rho = v0 * cos_theta - u0 * sin_theta;

	//(rho - image.rows * sin(theta)) / cos(theta)
	for (int i = 0; i < linePoints.size(); ++i)
	{
		linePoints[i].y = (rho - linePoints[i].x * cos(phi)) / sin(phi);
	}
	




	rayLineImage = srcImage.clone();
	for (int i = 0; i < linePoints.size(); ++i)
	{
		rayLineImage.at<Vec3b>(linePoints[i])[0] = 0;
		rayLineImage.at<Vec3b>(linePoints[i])[1] = 0;
		rayLineImage.at<Vec3b>(linePoints[i])[2] = 255;
	}
	imwrite(DATA_FOLDER + string("rayLineImage.jpg"), rayLineImage);
}

void chuixianyi::deleteSmallRegions(Mat & pSrc, Mat & pDst)
{
	// ��ȡ��ͨ���򣬲��޳�С�����ͨ����
	vector<vector<Point>> contours;           //��ֵͼ������������
	vector<Vec4i> hierarchy;                  //4��int�������ֱ��ʾ��ǰ�������ӵ��������
	findContours(pSrc, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);             //�����������

	vector<vector<Point>>::iterator k;                    //��������������������

	for (k = contours.begin(); k != contours.end();)      //��������,�����������
	{
		if (contourArea(*k, false) < msize)
		{//ɾ��ָ��Ԫ�أ�����ָ��ɾ��Ԫ����һ��Ԫ��λ�õĵ�����
			k = contours.erase(k);
		}
		else
			++k;
	}

	//contours[i]�����i��������contours[i].size()�����i�����������е����ص�
	for (int i = 0; i < contours.size(); i++)
	{
		for (int j = 0; j < contours[i].size(); j++)
		{
			//��ȡ�����ϵ������
			Point P = Point(contours[i][j].x, contours[i][j].y);
		}
		drawContours(pDst, contours, i, Scalar(255), -1, 8);
	}
}

void chuixianyi::findSmallRegions(Mat &pSrc, Mat &pDst)
{
	// ��ȡ��ͨ���򣬲��޳�С�����ͨ����
	vector<vector<Point>> contours;           //��ֵͼ������������
	vector<Vec4i> hierarchy;                  //4��int�������ֱ��ʾ��ǰ�������ӵ��������
	findContours(pSrc, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);             //�����������

	vector<vector<Point>>::iterator k;                    //��������������������

	for (k = contours.begin(); k != contours.end();)      //��������,�����������
	{
		if (contourArea(*k, false) > msize)
		{//ɾ��ָ��Ԫ�أ�����ָ��ɾ��Ԫ����һ��Ԫ��λ�õĵ�����
			k = contours.erase(k);
		}
		else
			++k;
	}

	//contours[i]�����i��������contours[i].size()�����i�����������е����ص�
	for (int i = 0; i < contours.size(); i++)
	{
		for (int j = 0; j < contours[i].size(); j++)
		{
			//��ȡ�����ϵ������
			Point P = Point(contours[i][j].x, contours[i][j].y);
		}
		drawContours(pDst, contours, i, Scalar(255), -1, 8);
	}
}

/*
���ù�դ�߱궨����ƽ��
�󼤹�ƽ�����������������
������
uv		ͼ������
zw		��դ�߶���
xwyw	�����������
M		���ͶӰ����
*/
void chuixianyi::uvzw2xwyw(Point &uv, float zw, Point2f &xwyw, Mat &M)
{
	//    [u]      [xw]
	// s *[v] = M *[yw]
	//    [1]      [zw]
	//             [ 1]
	//
	// ��ȥs�õ�������
	// [um31-m11 um32-m22] [xw] = [m14-um34-(um33-m13)zw]
	// [vm31-m11 vm32-m22] [yw]   [m24-vm34-(vm33-m23)zw]
	//
	// A * xwyw = B
	// xwyw = A_ * B

	float u = uv.x;
	float v = uv.y;
	float m11 = M.at<double>(0, 0);
	float m12 = M.at<double>(0, 1);
	float m13 = M.at<double>(0, 2);
	float m14 = M.at<double>(0, 3);
	float m21 = M.at<double>(1, 0);
	float m22 = M.at<double>(1, 1);
	float m23 = M.at<double>(1, 2);
	float m24 = M.at<double>(1, 3);
	float m31 = M.at<double>(2, 0);
	float m32 = M.at<double>(2, 1);
	float m33 = M.at<double>(2, 2);
	float m34 = M.at<double>(2, 3);

	Mat A = Mat(2, 2, CV_32FC1);
	A.at<float>(0, 0) = u*m31 - m11;
	A.at<float>(0, 1) = u*m32 - m12;
	A.at<float>(1, 0) = v*m31 - m21;
	A.at<float>(1, 1) = v*m32 - m22;
	Mat A_ = Mat(2, 2, CV_32FC1);
	A_ = A.inv();
	Mat B = Mat(2, 1, CV_32FC1);
	B.at<float>(0, 0) = m14 - u*m34 - (u*m33 - m13)*zw;
	B.at<float>(1, 0) = m24 - v*m34 - (v*m33 - m23)*zw;

	Mat world = Mat(2, 1, CV_32FC1);
	world = A_*B;

	xwyw.x = world.at<float>(0, 0);
	xwyw.y = world.at<float>(1, 0);
}

Point2f chuixianyi::uvzw2xwyw(Point uv, float zw, Mat &M)
{
	//    [u]      [xw]
	// s *[v] = M *[yw]
	//    [1]      [zw]
	//             [ 1]
	//
	// ��ȥs�õ�������
	// [um31-m11 um32-m22] [xw] = [m14-um34-(um33-m13)zw]
	// [vm31-m11 vm32-m22] [yw]   [m24-vm34-(vm33-m23)zw]
	//
	// A * xwyw = B
	// xwyw = A_ * B


	float u = uv.x;
	float v = uv.y;
	float m11 = M.at<double>(0, 0);
	float m12 = M.at<double>(0, 1);
	float m13 = M.at<double>(0, 2);
	float m14 = M.at<double>(0, 3);
	float m21 = M.at<double>(1, 0);
	float m22 = M.at<double>(1, 1);
	float m23 = M.at<double>(1, 2);
	float m24 = M.at<double>(1, 3);
	float m31 = M.at<double>(2, 0);
	float m32 = M.at<double>(2, 1);
	float m33 = M.at<double>(2, 2);
	float m34 = M.at<double>(2, 3);
	//float u = 562;
	//float v = 192;
	//zw = 0;
	//float m11 = -2066.92975377865;
	//float m12 = -7.05114282200192;
	//float m13 = 1223.09869029927;
	//float m14 = 505787.625081854;
	//float m21 = 125.089188682421;
	//float m22 = -2324.59339213365;
	//float m23 = 455.412657983516;
	//float m24 = 486641.305942175;
	//float m31 = 0.266947611747578;
	//float m32 = -0.0128936038434843;
	//float m33 = 0.963624785672406;
	//float m34 = 560.583950766622;



	Mat A = Mat(2, 2, CV_32FC1);
	A.at<float>(0, 0) = u*m31 - m11;
	A.at<float>(0, 1) = u*m32 - m12;
	A.at<float>(1, 0) = v*m31 - m21;
	A.at<float>(1, 1) = v*m32 - m22;
	Mat A_ = Mat(2, 2, CV_32FC1);
	A_ = A.inv();
	Mat B = Mat(2, 1, CV_32FC1);
	B.at<float>(0, 0) = m14 - u*m34 - (u*m33 - m13)*zw;
	B.at<float>(1, 0) = m24 - v*m34 - (v*m33 - m23)*zw;

	Mat world = Mat(2, 1, CV_32FC1);
	world = A_*B;

	Point2f xwyw;
	xwyw.x = world.at<float>(0, 0);
	xwyw.y = world.at<float>(1, 0);
	return xwyw;
}

/*
�������Ƿ�������������
��������ϵ�ɱ궨�ĵ�һ��ͼƬȷ��
������
uv		ͼ������
xwywzw	�����������
M		���ͶӰ����
*/
Point3f chuixianyi::uv2xwywzw(Point uv, Mat& M)
{
	//     [u]      [xw]
	// zw *[v] = M *[yw]
	//     [1]      [zw]
	//              [ 1]
	// a*xw + b*yw + c*zw*d = 0
	//
	// [um31-m11 um32-m12 um33-m13] [xw]   [m14-um34]
	// [vm31-m21 vm32-m22 vm33-m23] [yw] = [m24-vm34]
	// [    a        b        c   ] [zw]   [   -d   ]




	float u = uv.x;
	float v = uv.y;
	float a = rayPlaneParams_L.at<float>(0, 0);
	float b = rayPlaneParams_L.at<float>(1, 0);
	float c = rayPlaneParams_L.at<float>(2, 0);
	float d = -1;
	float m11 = M.at<double>(0, 0);
	float m12 = M.at<double>(0, 1);
	float m13 = M.at<double>(0, 2);
	float m14 = M.at<double>(0, 3);
	float m21 = M.at<double>(1, 0);
	float m22 = M.at<double>(1, 1);
	float m23 = M.at<double>(1, 2);
	float m24 = M.at<double>(1, 3);
	float m31 = M.at<double>(2, 0);
	float m32 = M.at<double>(2, 1);
	float m33 = M.at<double>(2, 2);
	float m34 = M.at<double>(2, 3);


	// [um31-m11 um32-m12 um33-m13] [xw]   [m14-um34]
	// [vm31-m21 vm32-m22 vm33-m23] [yw] = [m24-vm34]
	// [    a        b        c   ] [zw]   [   -d   ]
	// xyzw = A_ * B
	Mat A = Mat_<float>(3, 3);
	A.at<float>(0, 0) = u*m31 - m11;
	A.at<float>(0, 1) = u*m32 - m12;
	A.at<float>(0, 2) = u*m33 - m13;
	A.at<float>(1, 0) = v*m31 - m21;
	A.at<float>(1, 1) = v*m32 - m22;
	A.at<float>(1, 2) = v*m33 - m23;
	A.at<float>(2, 0) = a;
	A.at<float>(2, 1) = b;
	A.at<float>(2, 2) = c;
	Mat A_ = Mat_<float>(3, 3);
	A_ = A.inv();

	Mat B = Mat(3, 1, CV_32FC1);
	B.at<float>(0, 0) = m14 - u*m34;
	B.at<float>(1, 0) = m24 - v*m34;
	B.at<float>(2, 0) = -d;

	Mat world = Mat(3, 1, CV_32FC1);
	world = A_*B;

	Point3f xwywzw;
	xwywzw.x = world.at<float>(0, 0);
	xwywzw.y = world.at<float>(1, 0);
	xwywzw.z = world.at<float>(2, 0);

	return xwywzw;
}

void chuixianyi::uv2xwywzw(Point &uv, Point3f &xwywzw, Mat& M)
{
	//     [u]      [xw]
	// zw *[v] = M *[yw]
	//     [1]      [zw]
	//              [ 1]
	// a*xw + b*yw + c*zw*d = 0
	//
	// [um31-m11 um32-m12 um33-m13] [xw]   [m14-um34]
	// [vm31-m21 vm32-m22 vm33-m23] [yw] = [m24-vm34]
	// [    a        b        c   ] [zw]   [   -d   ]




	float u = uv.x;
	float v = uv.y;
	float a = rayPlaneParams_L.at<float>(0, 0);
	float b = rayPlaneParams_L.at<float>(1, 0);
	float c = rayPlaneParams_L.at<float>(2, 0);
	float d = -1;
	float m11 = M.at<double>(0, 0);
	float m12 = M.at<double>(0, 1);
	float m13 = M.at<double>(0, 2);
	float m14 = M.at<double>(0, 3);
	float m21 = M.at<double>(1, 0);
	float m22 = M.at<double>(1, 1);
	float m23 = M.at<double>(1, 2);
	float m24 = M.at<double>(1, 3);
	float m31 = M.at<double>(2, 0);
	float m32 = M.at<double>(2, 1);
	float m33 = M.at<double>(2, 2);
	float m34 = M.at<double>(2, 3);


	// [um31-m11 um32-m12 um33-m13] [xw]   [m14-um34]
	// [vm31-m21 vm32-m22 vm33-m23] [yw] = [m24-vm34]
	// [    a        b        c   ] [zw]   [   -d   ]
	// xyzw = A_ * B
	Mat A = Mat_<float>(3, 3);
	A.at<float>(0, 0) = u*m31 - m11;
	A.at<float>(0, 1) = u*m32 - m12;
	A.at<float>(0, 2) = u*m33 - m13;
	A.at<float>(1, 0) = v*m31 - m21;
	A.at<float>(1, 1) = v*m32 - m22;
	A.at<float>(1, 2) = v*m33 - m23;
	A.at<float>(2, 0) = a;
	A.at<float>(2, 1) = b;
	A.at<float>(2, 2) = c;
	Mat A_ = Mat_<float>(3, 3);
	A_ = A.inv();

	Mat B = Mat(3, 1, CV_32FC1);
	B.at<float>(0, 0) = m14 - u*m34;
	B.at<float>(1, 0) = m24 - v*m34;
	B.at<float>(2, 0) = -d;

	Mat world = Mat(3, 1, CV_32FC1);
	world = A_*B;

	xwywzw.x = world.at<float>(0, 0);
	xwywzw.y = world.at<float>(1, 0);
	xwywzw.z = world.at<float>(2, 0);
}



void chuixianyi::on_measure1Btn_clicked()
{
	//������ť���һ�ο�ʼ�������ٴε�����˳�����
	if (flagStartCamera && !flagEndCamera)
	{
		if (flagIsMeasuring1)
		{
			timer->start();
			flagIsMeasuring1 = false;
			return;
		}
	}
	flagIsMeasuring1 = true;

	prepareFrameLR();


	Mat test = imread(imageName_L);
	//Mat test1;
	//undistort(test, test1, cameraMatrix_L, distCoeffs_L); // ��������
	////remap(test, test1, maplx, maply, INTER_LINEAR);
	//test = test1;


	QImage QImage1 = CvMat2QImage(test);
	QPixmap QPixmap1 = QPixmap::fromImage(QImage1);
	QPixmap1 = QPixmap1.scaled(LABEL_WIDTH, LABEL_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.labelLeft->setPixmap(QPixmap1);

	ui.textBrowser->append(QString::fromLocal8Bit("��ʼ�������Ƿ�ѡ�����!"));
	ui.textBrowser->append(QString::fromLocal8Bit("���ֶ�ѡ�񼤹����ϵ�����!"));
	QCoreApplication::processEvents();
}




//******************************************************************
//
//
//					ͼ������
//
// Ԥ����->��ȡ������->��ȡ����->�ཻ�����㽻���ͼ������->���㴹��XY����
// ->����ƥ��->˫Ŀ�Ӿ����㴹��Z����
//
//******************************************************************
void chuixianyi::on_autoMatchBtn_clicked()
{
	//������ť���һ�ο�ʼ�������ٴε�����˳�����
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

	//����ͽ�У�����frameLeft��frameRight��ʾ��label����
	computeDisparityImage(frameLeft, frameRight, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	ui.textBrowser->append(QString::fromLocal8Bit("�ѽ�����ͼ��У��!"));

	//-- ��ȡͼ��
	Mat img1 = img1_rectified;
	Mat img2 = img2_rectified;
	//Mat img1 = imread(DATA_FOLDER + string("�����У��ͼ��.jpg"));
	//Mat img2 = imread(DATA_FOLDER + string("�����У��ͼ��.jpg"));




	//SURF����	KNN����ƥ��
	vector<KeyPoint> keypoints_1, keypoints_2;
	Mat descriptor_1, descriptor_2;
	int minHessian = 400;
	Ptr<Feature2D> surf = xfeatures2d::SURF::create(minHessian);

	surf->detectAndCompute(img1, noArray(), keypoints_1, descriptor_1);
	surf->detectAndCompute(img2, noArray(), keypoints_2, descriptor_2);
	cout << keypoints_1.size() << " " << keypoints_2.size() << endl;
	Mat outimg1;
	drawKeypoints(img1, keypoints_1, outimg1, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
	imwrite(DATA_FOLDER + string("SURF������.jpg"), outimg1);
	ui.textBrowser->append(QString::fromLocal8Bit("����ȡSURF������!"));
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
	imwrite(DATA_FOLDER + string("SURF_KNN����ƥ��.jpg"), img_goodmatch);
	ui.textBrowser->append(QString::fromLocal8Bit("�����KNN����ƥ��!"));
	QCoreApplication::processEvents();


	//https://blog.csdn.net/m0_37598482/article/details/78782252
	//����Լ����RANSAC�㷨�޳���ƥ��
	int ptCount = (int)matches.size();
	Mat p1(ptCount, 2, CV_32F);
	Mat p2(ptCount, 2, CV_32F);

	// ��Keypointת��ΪMat  
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
	// ��RANSAC��������F  
	// Mat m_Fundamental;  
	// ������������ǻ�������  
	vector<uchar> m_RANSACStatus;
	// ������������Ѿ�����������ڴ洢RANSAC��ÿ�����״̬  
	//m_Fundamental = findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);  
	Mat m_Fundamental = findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);
	cout << m_Fundamental << endl;
	// ����Ұ�����  
	int OutlinerCount = 0;
	for (int i = 0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] == 0) // ״̬Ϊ0��ʾҰ��  
		{
			OutlinerCount++;
		}
	}

	// �����ڵ�  
	vector<Point2f> m_LeftInlier;
	vector<Point2f> m_RightInlier;
	vector<DMatch> m_InlierMatches;
	// ���������������ڱ����ڵ��ƥ���ϵ  
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

	// ���ڵ�ת��ΪdrawMatches����ʹ�õĸ�ʽ  
	vector<KeyPoint> key1(InlinerCount);
	vector<KeyPoint> key2(InlinerCount);
	KeyPoint::convert(m_LeftInlier, key1);
	KeyPoint::convert(m_RightInlier, key2);
	//for (int i = 0; i<10; i++)
	//	cout << key1[i].pt << " " << key2[i].pt << " " << key1[i].pt.x - key2[i].pt.x << endl;

	Mat img_inliermatches;
	drawMatches(img1, key1, img2, key2, m_InlierMatches, img_inliermatches);
	imwrite(DATA_FOLDER + string("SURF_KNN_RANSAC����ƥ��.jpg"), img_inliermatches);
	ui.textBrowser->append(QString::fromLocal8Bit("����ɼ���Լ��RANSAC�޳���ƥ��!"));
	QCoreApplication::processEvents();



	//��ȡKeyPoint�еĶ�Ӧ��ϵ	�������Ӧ��Ŀռ�����
	int inlier_ptCount = (int)m_InlierMatches.size();
	Mat p3(ptCount, 2, CV_32F);
	Mat p4(ptCount, 2, CV_32F);
	Point2f pt_l, pt_r;
	Point3f real_pt;
	fstream fs(DATA_FOLDER + string("ƥ��������.txt"), ios::out);
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

			real_pt = uv2xwywzw(pt_l, pt_r);
			fs << real_pt.x << " " << real_pt.y << " " << real_pt.z << endl;
		}
	}
	fs.close();
	ui.textBrowser->append(QString::fromLocal8Bit("��д��ƥ������ά����!"));
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













	//ORB����	BF����ƥ��

	////��ʼ��
	//vector<KeyPoint> keypoints_1, keypoints_2;   //�ؼ���/�ǵ�
	//											 /**
	//											 opencv��keypoint���Ĭ�Ϲ��캯��Ϊ��
	//											 CV_WRAP KeyPoint() : pt(0,0), size(0), angle(-1), response(0), octave(0), class_id(-1) {}
	//											 pt(x,y):�ؼ���ĵ����ꣻ // size():�ùؼ�������ֱ����С�� // angle:�Ƕȣ���ʾ�ؼ���ķ���ֵΪ[0,360)����ֵ��ʾ��ʹ�á�
	//											 response:��Ӧǿ�ȣ�ѡ����Ӧ��ǿ�Ĺؼ���;   octacv:����һ��������õ��Ĵ˹ؼ��㡣
	//											 class_id:��Ҫ��ͼƬ���з���ʱ����class_id��ÿ���ؼ���������֣�Ĭ��Ϊ-1��
	//											 **/
	//Mat descriptors_1, descriptors_2;      //������
	//									   //����ORB���󣬲���ΪĬ��ֵ
	//Ptr<FeatureDetector> detector = ORB::create();
	//Ptr<DescriptorExtractor> descriptor = ORB::create();
	//Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
	///**
	//��Ptr<FeatureDetector> detector = ���ȼ��� ��FeatureDetector * detector =��
	//Ptr��OpenCV��ʹ�õ�����ָ��ģ���࣬�������ɹ���������͵�ָ�롣
	//���������FeatureDetetor�����࣬ͨ������FeatureDetector�Ķ������ʹ�ö���������⼰ƥ�䷽����ͨ��create()�������á�
	//��������ȡ��DescriptorExtractor����ȡ�ؼ�������������������ࡣ
	//������ƥ����DescriptorMatcher��������ƥ�䣬"Brute-force-Hamming"��ʾʹ�ú����������ƥ�䡣
	//**/

	////��һ�������Oriented Fast�ǵ�λ��
	//chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
	//detector->detect(img_1, keypoints_1);     //�Բ���1ͼ�������������ȡ������������2��������
	//detector->detect(img_2, keypoints_2);

	////�ڶ��������ݽǵ����BREIF������
	//descriptor->compute(img_1, keypoints_1, descriptors_1);   //computer()����ؼ����������������ע��˼���������õĺ����ԣ�
	//descriptor->compute(img_2, keypoints_2, descriptors_2);
	//chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
	//chrono::duration<double> time_used = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
	//cout << "extract ORB cost = " << time_used.count() << " seconds. " << endl;
	//Mat outimg1;
	//drawKeypoints(img_1, keypoints_1, outimg1, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
	////imshow("ORB features", outimg1);
	//imwrite(DATA_FOLDER + string("ORB������.jpg"), outimg1);

	////�������� ������ͼ���е������ӽ���ƥ�䣬ʹ��hamming����
	//vector<DMatch> matches;    //DMatch��ƥ��ؼ��������� ��, matches���ڴ��ƥ����
	//t1 = chrono::steady_clock::now();
	//matcher->match(descriptors_1, descriptors_2, matches); //�Բ���1 2�������ӽ���ƥ�䣬����ƥ��������matches��
	//t2 = chrono::steady_clock::now();
	//time_used = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
	//cout << "match the ORB cost: " << time_used.count() << "seconds. " << endl;

	////���Ĳ���ƥ����ɸѡ
	////������С�����������
	//auto min_max = minmax_element(matches.begin(), matches.end(),
	//	[](const DMatch &m1, const DMatch &m2) { return m1.distance < m2.distance; });
	//// auto ����������������ʱ����ݱ�����ʼֵ�������Զ�Ϊ�˱���ѡ��ƥ�������
	//// minmax_element()����ָ��Χ����С�����Ԫ�ص�һ�Ե�����������1 2Ϊ��ֹ��������Χ
	//// ����3�Ƕ����ƺ������ú������ܷ�Χ�ڵ�����Ԫ����Ϊ�����������ؿ�ת��Ϊbool��ֵ��
	//// ���ص�ֵָʾ��Ϊ��һ���������ݵ�Ԫ���Ƿ�С�ڵڶ������ú��������޸����κβ�����
	//double min_dist = min_max.first->distance;  // min_max�洢��һ�ѵ�������firstָ����СԪ��
	//double max_dist = min_max.second->distance; // secondָ�����Ԫ��

	//printf("-- Max dist : %f \n", max_dist);
	//printf("-- Min dist : %f \n", min_dist);

	////��������֮��ľ������������С����ʱ������Ϊƥ�����󡣵���ʱ��С�����ǳ�С������Ҫ����һ������ֵ30��Ϊ���ޡ�
	//vector<DMatch> good_matches;  //������õ�ƥ����
	//for (int i = 0; i < descriptors_1.rows; ++i)
	//{
	//	if (matches[i].distance <= max(2 * min_dist, 30.0))
	//	{
	//		good_matches.push_back(matches[i]);
	//	}
	//}

	////���岽������ƥ����
	//Mat img_match;         //�������ƥ���
	//Mat img_goodmatch;     //��źõ�ƥ���
	//					   // drawMatches���ڻ�������ͼ���ƥ��ؼ��㡣
	//					   // ����1�ǵ�һ��Դͼ�񣬲���2����ؼ������飻����3�ǵڶ���ԭͼ�񣬲���4����ؼ�������
	//					   // ����5������ͼ���ƥ��ؼ�������,����6���ڴ�ź����Ļ��ƽ��
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_match);
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, good_matches, img_goodmatch);
	//imwrite(DATA_FOLDER + string("����ƥ����.jpg"), img_match);
	//imwrite(DATA_FOLDER + string("�Ż���ƥ����.jpg"), img_goodmatch);
	//waitKey(0);


}

void chuixianyi::on_detectCrossBtn_clicked()
{
	//������ť���һ�ο�ʼ�������ٴε�����˳�����
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

	//����ͽ�У�����frameLeft��frameRight��ʾ��label����
	computeDisparityImage(frameLeft, frameRight, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	ui.textBrowser->append(QString::fromLocal8Bit("�ѽ�����ͼ��У��!"));

	//����ԭʼͼ
	g_srcImage = img1_rectified.clone();
	cvtColor(g_srcImage, g_srcImage, COLOR_GRAY2RGB);

	GetRedComponetBySplit(g_srcImage, g_redImage);	//��ȡ��ɫ����
	cvtColor(g_redImage, g_grayImage, COLOR_RGB2GRAY);	//��ʾ�Ҷ�ͼ 
	imwrite(DATA_FOLDER + string("Grey_imageLeft.jpg"), g_grayImage);

	//��ֵ��
	threshold(g_grayImage, imgHSVMask, threshold_value, 255, THRESH_BINARY);
	g_midImage = Mat::zeros(imgHSVMask.size(), CV_8UC1);  //����

	Delete_smallregions(imgHSVMask, g_midImage);		//ȥ��С�������
	imwrite(DATA_FOLDER + string("Target_imageLeft.jpg"), g_midImage);

	//normalizeLetter��ʾЧ��ͼ  
	normalizeLetter(g_midImage, g_dstImage);

	//����ӳ�䵽ԭͼ
	/*	threshold(g_grayImage, g_midImage, threshold_value, 255, CV_THRESH_BINARY);	*/
	/*	imshow("����ֵ��ͼ��", g_midImage);											*/
	Line_reflect(g_dstImage, g_midImage);
	imwrite(DATA_FOLDER + string("Reflect_imageLeft.jpg"), g_midImage);

	//ת�����ͣ�����skeletonͼ��
	normalize(g_dstImage, g_midImage, 0, 255, NORM_MINMAX, CV_8U);
	imwrite(DATA_FOLDER + string("Thinning_imageLeft.jpg"), g_midImage);




	//����ԭʼͼ
	g_srcImage = img2_rectified.clone();
	cvtColor(g_srcImage, g_srcImage, COLOR_GRAY2RGB);

	GetRedComponetBySplit(g_srcImage, g_redImage);	//��ȡ��ɫ����
	cvtColor(g_redImage, g_grayImage, COLOR_RGB2GRAY);	//��ʾ�Ҷ�ͼ 
	imwrite(DATA_FOLDER + string("Grey_imageRight.jpg"), g_grayImage);

	//��ֵ��
	threshold(g_grayImage, imgHSVMask, threshold_value, 255, THRESH_BINARY);
	g_midImage = Mat::zeros(imgHSVMask.size(), CV_8UC1);  //����

	Delete_smallregions(imgHSVMask, g_midImage);		//ȥ��С�������
	imwrite(DATA_FOLDER + string("Target_imageRight.jpg"), g_midImage);

	//normalizeLetter��ʾЧ��ͼ  
	normalizeLetter(g_midImage, g_dstImage);

	//����ӳ�䵽ԭͼ
	/*	threshold(g_grayImage, g_midImage, threshold_value, 255, CV_THRESH_BINARY);	*/
	/*	imshow("����ֵ��ͼ��", g_midImage);											*/
	Line_reflect(g_dstImage, g_midImage);
	imwrite(DATA_FOLDER + string("Reflect_imageRight.jpg"), g_midImage);

	//ת�����ͣ�����skeletonͼ��
	normalize(g_dstImage, g_midImage, 0, 255, NORM_MINMAX, CV_8U);
	imwrite(DATA_FOLDER + string("Thinning_imageRight.jpg"), g_midImage);

	ui.textBrowser->append(QString::fromLocal8Bit("�Ѽ�⼤���������!"));
	QCoreApplication::processEvents();
}


void chuixianyi::GetRedComponetBySplit(Mat srcImg, Mat &red)
{
	//������ͼ�еĺ�ɫ��������ȡ����
	Mat imgROI;
	vector<Mat>channels;
	split(srcImg, channels);
	Mat blueComponet = channels.at(0);
	Mat greenComponet = channels.at(1);
	blueComponet = Mat::zeros(srcImg.size(), CV_8UC1);//Mat�൱��ָ�룬���chnnels.at(0)���¸�ֵ
	greenComponet = Mat::zeros(srcImg.size(), CV_8UC1);
	merge(channels, imgROI);//����������ɫ����������������ֵΪ0
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
			{			//����
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
		compare(p_enlarged_src, p_thinMat2, p_cmp, CMP_EQ);	//�Ƚ������src1��src2�е�Ԫ�أ���Ϊ255������Ϊ0
															/// check
		int num_non_zero = countNonZero(p_cmp);					//���ػҶ�ֵ��Ϊ0��������
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
	// ��ȡ��ͨ���򣬲��޳�С�����ͨ����
	vector<vector<Point>> contours;           //��ֵͼ������������
	vector<Vec4i> hierarchy;                  //4��int�������ֱ��ʾ��ǰ�������ӵ��������
	findContours(pSrc, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);             //�����������

	vector<vector<Point>>::iterator k;                    //��������������������

	for (k = contours.begin(); k != contours.end();)      //��������,�����������
	{
		if (contourArea(*k, false) < msize)
		{//ɾ��ָ��Ԫ�أ�����ָ��ɾ��Ԫ����һ��Ԫ��λ�õĵ�����
			k = contours.erase(k);
		}
		else
			++k;
	}

	//contours[i]�����i��������contours[i].size()�����i�����������е����ص�
	for (int i = 0; i < contours.size(); i++)
	{
		for (int j = 0; j < contours[i].size(); j++)
		{
			//��ȡ�����ϵ������
			Point P = Point(contours[i][j].x, contours[i][j].y);
		}
		drawContours(pDst, contours, i, Scalar(255), -1, 8);
	}
}





void chuixianyi::on_liveShowCoordinateBtn_clicked()
{

}
