#include "imagemodel.h"
#include "imagefifo.h"
#define FIFO_SIZE 2  //����ֵ������ڵ���2

ImageModel::ImageModel(QWidget *mainWindow)
	:m_mainWindow(mainWindow)
{
	m_camera.moveToThread(&m_receiveThread);

	m_imageProcess.moveToThread(&m_imageProcessThread);

	connect(&m_receiveThread, SIGNAL(started()), &m_camera, SLOT(receiveData()));
	connect(&m_imageProcessThread, SIGNAL(started()), &m_imageProcess, SLOT(dataToImage()));

	connect(&m_imageProcess, SIGNAL(showImage(QPixmap)), m_mainWindow, SLOT(updateImage(QPixmap)));
	//connect(&m_imageProcess, SIGNAL(showImage(QImage)), m_mainWindow, SLOT(updateImage(QImage)));
	connect(&m_camera, SIGNAL(completeFrameTransmission()), m_mainWindow, SLOT(countReceiveFrames()));
	
	connect(this, &ImageModel::resolutionChanged, &m_camera, static_cast<void (CyDevice::*)(int, int, int, LONG, int ,int)>(&CyDevice::changeResolution));
	//connect(&m_camera, SIGNAL(completeFrameTransmission(unsigned char *, bool)), &m_imageProcess, SLOT(dataToImage(unsigned char *, bool)), Qt::DirectConnection);
	//connect(&m_camera, SIGNAL(completeFrameTransmission(unsigned char *, bool)), &m_imageProcess, SLOT(dataToImage(unsigned char *, bool)));

}

ImageModel::~ImageModel()
{
	if (m_receiveThread.isRunning())
	{

		m_camera.disableReceving();
		m_receiveThread.quit();
		m_receiveThread.wait();

		//m_camera.closeDevice();    device������ʱ���жϲ�delete CyDevice
	}
	if (m_imageProcessThread.isRunning())
	{
		m_imageProcess.disableImageProcess();

		m_imageProcessThread.quit();
		m_imageProcessThread.wait();
	}

	QThreadPool::globalInstance()->waitForDone();
}

void ImageModel::initialize(int width, int height, int pixelWidth, int bufferNum, bool isColor)
{
	m_camera.initialize(width, height, pixelWidth, bufferNum);
	m_imageProcess.initialize(width, height, isColor);
}

void ImageModel::initializeTransfer(int packetNum, int xferQueSize, int timeOut)
{
	m_camera.initializeTransfer(packetNum, xferQueSize, timeOut);
}

bool ImageModel::openUSBCamera()
{
	if (m_camera.openDevice((HANDLE)(m_mainWindow->winId())))
	{
		
		return true;
	}
	else
	{
		return false;
	}
}

void ImageModel::closeUSBCamera()
{
	if (m_receiveThread.isRunning()){
		m_camera.disableReceving();
		m_imageProcess.disableImageProcess();

		m_receiveThread.quit();
		m_receiveThread.wait();
		m_camera.closeDevice();

		m_imageProcessThread.quit();
		m_imageProcessThread.wait();
	}
}

void ImageModel::readData()
{
	if (!m_imageProcessThread.isRunning())
	{
		m_imageProcess.enableImageProcess();
		m_imageProcessThread.start();
	}
	m_camera.enableReceving();
	m_receiveThread.start();
}

void ImageModel::setResolution(int height, int width)
{
	m_camera.setHeight(height);
	m_camera.setWidth(width);
	m_imageProcess.setHeight(height);
	m_imageProcess.setWidth(width);
}

void ImageModel::saveData()
{
	m_imageProcess.enableSaveData();
}

void ImageModel::whetherPausingUSBCamera(bool trueOrFalse)
{
	//m_camera.setPauseFlag(trueOrFalse);
	m_imageProcess.setPauseFlag(trueOrFalse);
}

void ImageModel::changeWidthTo8bitsPerPixel(bool flag)
{
	if (flag)
	{
		m_camera.changeWidthTo8bitsPerPixel();
	}
	else
	{
		m_camera.changeWidthTo16bitsPerPixel();
	}
}

void ImageModel::changeImageToColor(bool flag)
{
	m_imageProcess.setImageColorOrBlack(flag);
}

void ImageModel::changeResolution(int width, int height, int req, long sizePerXfer, int xferQueueSize, int timeOut)
{
	//m_camera.changeResolution(width, height, req);
	//���Զ��л��޷����е��ڽ��洦���д���
	if (m_camera.isReceving())
	{
		m_camera.disableReceving();
		emit resolutionChanged(width, height, req, sizePerXfer, xferQueueSize, timeOut);
	}
}

void ImageModel::sendSettingCommand(uchar u1, uchar u2, uchar u3, uchar u4)
{
	uchar buf[4] = { u1, u2, u3, u4 };

	m_camera.configRegister(buf, 4);

}

void ImageModel::setSavingPath(QString path)
{
	m_imageProcess.setSavingPath(path);
}

void ImageModel::takeImage()
{
	m_imageProcess.setTakingImageFlag(true);
}