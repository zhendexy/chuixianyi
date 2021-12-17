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


void chuixianyi::on_startCameraBtn_clicked()
{
	timer->start();
	startCamera();
	flagStartCamera = true;
	flagEndCamera = false;
}

void chuixianyi::on_endCameraBtn_clicked()
{
	timer->stop();
	endCamera();
	flagStartCamera = false;
	flagEndCamera = true;
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

	readRectifyParams();
	validRoi[0], validRoi[1] = stereoRectification(stereoRectifyParams, cameraMatrix_L, distCoeffs_L, cameraMatrix_R, distCoeffs_R,
		imageSize, R, T, R1, R2, P1, P2, Q, mapl1, mapl2, mapr1, mapr2);
	ui.textBrowser->append(QString::fromLocal8Bit("�Ѵ���ͼ����ͶӰӳ���!"));
	QCoreApplication::processEvents();

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
	ui.textBrowser->append(QString::fromLocal8Bit("�Ӳ�ͼ�������!"));
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
	}

	readRectifyParams();
	validRoi[0], validRoi[1] = stereoRectification(stereoRectifyParams, cameraMatrix_L, distCoeffs_L, cameraMatrix_R, distCoeffs_R,
		imageSize, R, T, R1, R2, P1, P2, Q, mapl1, mapl2, mapr1, mapr2);
	ui.textBrowser->append(QString::fromLocal8Bit("�Ѷ�ȡ����궨����!"));
	QCoreApplication::processEvents();

	//����ͽ�У�����frameLeft��frameRight��ʾ��label����
	computeDisparityImage(frameLeft, frameRight, img1_rectified, img2_rectified, mapl1, mapl2, mapr1, mapr2, validRoi, disparity);
	ui.textBrowser->append(QString::fromLocal8Bit("�ѽ�����ͼ��У��!"));
	ui.textBrowser->append(QString::fromLocal8Bit("���ֶ�ѡ�����Ե�!"));
	QCoreApplication::processEvents();
	//while (labelPoints.size() != 4)
	//{

	//}


}

void chuixianyi::mousePressEvent(QMouseEvent *event)
{
	if (!flagIsMeasuring)
		return;

	//�������� ��ȡ�������
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
			Point3f Pw1 = uv2xyz(Point2f(labelPoints[0].x()*5.12, labelPoints[0].y()*5.12), Point2f(labelPoints[1].x()*5.12, labelPoints[1].y()*5.12));
			Point3f Pw2 = uv2xyz(Point2f(labelPoints[2].x()*5.12, labelPoints[2].y()*5.12), Point2f(labelPoints[3].x()*5.12, labelPoints[3].y()*5.12));
			double distance = sqrt((Pw1.x - Pw2.x)*(Pw1.x - Pw2.x) + (Pw1.z - Pw2.z)*(Pw1.y - Pw2.y) + (Pw1.z - Pw2.z)*(Pw1.z - Pw2.z));
			
			ui.textBrowser->append(QString::fromLocal8Bit("��һ��������꣺"));
			ui.textBrowser->append(QString::number(Pw1.x) + QString("  ") + QString::number(Pw1.y) + QString("  ") + QString::number(Pw1.z));
			ui.textBrowser->append(QString::fromLocal8Bit("�ڶ���������꣺"));
			ui.textBrowser->append(QString::number(Pw2.x) + QString("  ") + QString::number(Pw2.y) + QString("  ") + QString::number(Pw2.z));
			ui.textBrowser->append(QString::fromLocal8Bit("����֮��ľ���Ϊ��") + QString::number(distance));

			labelPoints.clear();
		}
	}
}


//************************************
// Description: ������������������������ռ�����
// Method:    uv2xyz
// FullName:  uv2xyz
// Access:    public 
// Parameter: Point2f uvLeft
// Parameter: Point2f uvRight
// Returns:   cv::Point3f
// Author:    С��
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
	//Mat mLeftRT = Mat(3, 4, CV_32F);//�����RT����
	//hconcat(mLeftRotation, mLeftTranslation, mLeftRT);
	//Mat mLeftIntrinsic = Mat(3, 3, CV_32F, leftIntrinsic);
	//Mat mLeftP = mLeftIntrinsic * mLeftRT;
	Mat mLeftP = P1;
	//cout<<"�����P���� = "<<endl<<mLeftP<<endl;

	//Mat mRightRotation = Mat(3, 3, CV_32F, rightRotation);
	//Mat mRightTranslation = Mat(3, 1, CV_32F, rightTranslation);
	//Mat mRightRT = Mat(3, 4, CV_32F);//�����RT����
	//hconcat(mRightRotation, mRightTranslation, mRightRT);
	//Mat mRightIntrinsic = Mat(3, 3, CV_32F, rightIntrinsic);
	//Mat mRightP = mRightIntrinsic * mRightRT;
	Mat mRightP = P2;
	//cout<<"�����P���� = "<<endl<<mRightP<<endl;

	//��С���˷�A����
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

	//��С���˷�B����
	Mat B = Mat(4, 1, CV_32F);
	B.at<float>(0, 0) = mLeftP.at<double>(0, 3) - uvLeft.x * mLeftP.at<double>(2, 3);
	B.at<float>(1, 0) = mLeftP.at<double>(1, 3) - uvLeft.y * mLeftP.at<double>(2, 3);
	B.at<float>(2, 0) = mRightP.at<double>(0, 3) - uvRight.x * mRightP.at<double>(2, 3);
	B.at<float>(3, 0) = mRightP.at<double>(1, 3) - uvRight.y * mRightP.at<double>(2, 3);

	Mat XYZ = Mat(3, 1, CV_32F);
	//����SVD��С���˷����XYZ
	solve(A, B, XYZ, DECOMP_SVD);

	//cout<<"�ռ�����Ϊ = "<<endl<<XYZ<<endl;

	//��������ϵ������
	Point3f world;
	world.x = XYZ.at<float>(0, 0);
	world.y = XYZ.at<float>(1, 0);
	world.z = XYZ.at<float>(2, 0);

	return world;
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
	FileStorage resultStore(DATA_FOLDER + string(singleCalibrateResult), FileStorage::WRITE);
	//ofstream resultStore(DATA_FOLDER + string(singleCalibrateResult)); // ����궨�����txt
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
	//destroyWindow("cornersframe");
	// ��������궨
	// ����ǵ��Ӧ����ά����
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
	resultStore << "cameraMatrix" << cameraMatrix;//����ڲ�������
	resultStore << "distCoeffs" << distCoeffs;//�������ϵ��
											  // ������ͶӰ�㣬��ԭͼ�ǵ�Ƚϣ��õ����
	double errPerImage = 0.; // ÿ��ͼ������
	double errAverage = 0.; // ����ͼ���ƽ�����
	double totalErr = 0.; // ����ܺ�
	vector<Point2f> projectImagePoints; // ��ͶӰ��
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
		//resultStore << "No." + to_string(i + 1) + "errPerImage" << errPerImage;
	}
	//resultStore << "totalErr/n_boards" << totalErr / n_boards;
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
	//ofstream stereoStore(DATA_FOLDER + string(stereoCalibrateResult));
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
stereoRectifyParams	�������У�������txt
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
	//ofstream stereoStore(DATA_FOLDER + string(stereoRectifyParams));
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
}
