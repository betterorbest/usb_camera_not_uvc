#include "imageprocess.h"

#include "qdebug.h"
#include <Windows.h> 
#include <QtConcurrent>
#include "imagefifo.h"
//#include "qtextstream.h"

ImageProcess::ImageProcess()
	:m_imageProcessingFlag(true),
	m_pauseFlag(false),
	m_path("."),
	m_isTakingImage(false)
{

}

ImageProcess::~ImageProcess()
{
	if (m_imageData != nullptr)
	{
		delete[] m_imageData;
		m_imageData = nullptr;
	}
}

void ImageProcess::initialize(int width, int height, bool isColor)
{
	m_imageData = new uchar[width * height];
	m_isColor = isColor;
}

void ImageProcess::dataToImage(unsigned char *data, int bitsPerPixel, int width, int height)
{
	if (bitsPerPixel <= 8)
	{
		if (m_isColor)
		{
			cv::Mat image8bits = cv::Mat(height, width, CV_8UC1, data);
			cv::Mat image;
			cv::cvtColor(image8bits, image, CV_BayerRG2RGB);
			//cv::cvtColor(image8bits, image, CV_BayerGR2RGB);
			//DWORD start = GetTickCount();
			//cv::medianBlur(image, image, 3);                        //��ֵ�˲� 64-80ms

			//DWORD end = GetTickCount();
			//qDebug() << end - start;
			//autoWhiteBalance(image, image);
			QImage qImage = QImage(image.data, width, height, image.step, QImage::Format_RGB888);

			//QImage qImage = QImage(data, m_imageWidth, m_imageHeight, QImage::Format_Grayscale8);

			if (qImage.isNull()) return;

			m_imageShow = QPixmap::fromImage(qImage);
		}
		else
		{

			QImage qImage = QImage(data, width, height, QImage::Format_Grayscale8);
			if (qImage.isNull()) return;
			m_imageShow = QPixmap::fromImage(qImage);
		}

		emit showImage(m_imageShow);
		if (m_isTakingImage)
		{
			DWORD start = GetTickCount();
			takeShowingImage(m_imageShow);
			DWORD end = GetTickCount();
			qDebug() << end - start << "this is the time";
		}
	}
	else
	{
		unsigned short* data16bits = (unsigned short *)data;
		if (m_isColor)
		{

			int size = height * width;
			//unsigned short temp;
			//DWORD start = GetTickCount();
			for (int i = 0; i < size; ++i)
			{
				//m_imageData[i] = pow((data[2 * i + 1] << 8) + data[2 * i], 0.66);
				//m_imageData[i] = pow(data16bits[i], 0.66);
				//m_imageData[i] = data16bits[i] >> 4;
				m_imageData[i] = data16bits[i];
			}

			cv::Mat image8bits = cv::Mat(height, width, CV_8UC1, m_imageData);
			cv::Mat image;
			//image16bits.convertTo(image, CV_8UC1);
			cv::cvtColor(image8bits, image, CV_BayerRG2RGB);
			//cv::cvtColor(image8bits, image, CV_BayerGR2RGB);

			autoWhiteBalance(image, image);

			QImage qImage = QImage(image.data, width, height, image.step, QImage::Format_RGB888);
			//DWORD end = GetTickCount();
			//qDebug() << end - start;
			//cv::Mat image16bits = cv::Mat(m_imageHeight, m_imageWidth, CV_16UC1, data);
			//image16bits.convertTo(image16bits, CV_8UC1, 0.0625);
			//cv::Mat image;
			//cv::cvtColor(image16bits, image, CV_BayerRG2RGB);

			if (qImage.isNull()) return;
			m_imageShow = QPixmap::fromImage(qImage);
		}
		else
		{
			int size = height * width;
			//unsignd short temp;
			//DWORD start = GetTickCount();
			for (int i = 0; i < size; ++i)
			{
				//m_imageData[i] = pow((data[2 * i + 1] << 8) + data[2 * i], 0.66);
				//m_imageData[i] = pow(data16bits[i], 0.66);
				//m_imageData[i] = data16bits[i] >> 4;
				m_imageData[i] = data16bits[i];

			}

			//DWORD end = GetTickCount();
			//qDebug() << end - start;
			QImage qImage = QImage(m_imageData, width, height, QImage::Format_Grayscale8);
			if (qImage.isNull()) return;
			m_imageShow = QPixmap::fromImage(qImage);

		}
		emit showImage(m_imageShow);
		if (m_isTakingImage)
		{
			DWORD start = GetTickCount();
			takeOriginalImage(cv::Mat(height, width, CV_16UC1, data16bits), m_imageShow);
			DWORD end = GetTickCount();
			qDebug() << end - start << "this is the time";
		}
	}
	/*********���ݽ��ս���ʱʹ��*******************/
	/*m_image = QImage(m_imageData, m_imageWidth, m_imageHeight, QImage::Format_Grayscale8);
	QtConcurrent::run(this, &ImageProcess::imageToPixmap, m_image);*/

}


void ImageProcess::imageToPixmap(QImage &image)
{
	//DWORD start = GetTickCount();
	QPixmap pixmap = QPixmap::fromImage(image);
	//DWORD end = GetTickCount();
	//qDebug() << end - start << "this is thread " << QThread::currentThreadId();
	emit showImage(pixmap);
}

void ImageProcess::saveOriginalData(unsigned short *data)
{
	int size = m_imageHeight * m_imageWidth;
	unsigned short *tempData = new unsigned short[size];
	for (int i = 0; i < size; ++i)
	{
		tempData[i] = data[i];
	}


	QTime time = QTime::currentTime();
	QString str = time.toString("hhmmsszzz");
	QString path = "./" + str + ".txt";
	QFile file(path);

	if (!file.open(QIODevice::WriteOnly))
		return;
	
	QTextStream out(&file);
	for (int i = 0; i < m_imageHeight; ++i)
	{
		int k = i * m_imageWidth;
		for (int j = 0; j < m_imageWidth; ++j)
		{
			out << tempData[k + j] << " ";
		}
		out << "\r\n";
	}
	delete tempData;
	tempData = nullptr;

}


void ImageProcess::enableSaveData()
{
	m_saveOriginalDataFlag = true;
}

void ImageProcess::setImageColorOrBlack(bool isColor)
{
	m_isColor = isColor;
}

void ImageProcess::dataToImage()
{
	ImageData* imageData;
	while (m_imageProcessingFlag)
	{
		imageData = ImageFifo::getFifoHead();
		if (imageData == nullptr) continue;
		if (!m_pauseFlag)
		{
			dataToImage((*imageData).m_data, (*imageData).m_bitsPerPixel, (*imageData).m_imageWidth, (*imageData).m_imageHeight);
		}
		ImageFifo::outFifo();
	}

	ImageFifo::emptyFifo();
	
}

void ImageProcess::disableImageProcess()
{
	m_imageProcessingFlag = false;
}

void ImageProcess::enableImageProcess()
{
	m_imageProcessingFlag = true;
}

void ImageProcess::setPauseFlag(bool flag)
{
	m_pauseFlag = flag;
}

void ImageProcess::setSavingPath(QString path)
{
	m_path = path;
}

void ImageProcess::takeOriginalImage(const cv::Mat& image, const QPixmap& pixmap)
{
	QTime time = QTime::currentTime();
	QString str = time.toString("hhmmsszzz");
	QString path = m_path + "\\" + str;
	cv::imwrite((path + ".png").toLocal8Bit().toStdString(), image);
	pixmap.save(path + ".bmp");
	m_isTakingImage = false;
}

void ImageProcess::takeShowingImage(const QPixmap& pixmap)
{
	QTime time = QTime::currentTime();
	QString str = time.toString("hhmmsszzz");
	QString path = m_path + "\\" + str + ".bmp";
	pixmap.save(path);
	m_isTakingImage = false;
}

void ImageProcess::setTakingImageFlag(bool flag)
{
	if (m_isTakingImage)
		return;
	m_isTakingImage = flag;
}

void ImageProcess::autoWhiteBalance(cv::Mat &src, cv::Mat &dst)
{
	/*********ԭʼ�ĻҶ������㷨**************/
	cv::Scalar averagePerChannel = cv::mean(src);
	double ratio = (averagePerChannel[0] + averagePerChannel[1] + averagePerChannel[2]) / 3;
	averagePerChannel[0] = ratio / averagePerChannel[0];
	averagePerChannel[1] = ratio / averagePerChannel[1];
	averagePerChannel[2] = ratio / averagePerChannel[2];
	cv::multiply(src, averagePerChannel, dst);

	/**********���������㷨*****************/
	//int histRGBSum[766] = { 0 };
	//short *RGBSum = new short[src.rows * src.cols];
	//ZeroMemory(RGBSum, src.rows * src.cols * sizeof(short));
	//
	////�������ص�׼��
	//int nr = src.rows;
	//int nc = src.cols;
	//if (src.isContinuous())    //��߱�����Ч��
	//{
	//	nc = nc * nr;
	//	nr = 1;
	//}

	////ͳ�Ƹ�������R+G+B��ֵ
	//short sum;
	//short *ptrRGBSum = RGBSum;
	//for (int i = 0; i < nr; ++i)
	//{
	//	uchar *data = src.ptr<uchar>(i);
	//	for (int j = 0; j < nc; ++j)
	//	{
	//		sum = (short)(*data + *(data + 1) + *(data + 2));
	//		++histRGBSum[sum];
	//		*ptrRGBSum = sum;
	//		data += 3;
	//		++ptrRGBSum;
	//	}
	//}

	////ȷ��������ֵ
	//float thresholdRatio = 0.02;
	//int threshold;
	//int thresholdSum = 0;
	//for (int i = 765; i > 0; --i)
	//{
	//	thresholdSum += histRGBSum[i];
	//	if (thresholdSum >= thresholdRatio * src.total())
	//	{
	//		threshold = i;
	//		break;
	//	}
	//}

	////����ͼ�񣬼��������ֵ�����ص��R\G\B�������ۻ��͵�ƽ��ֵ
	//cv::Scalar averageRGB(0);
	//int amount = 0;
	//ptrRGBSum = RGBSum;
	//for (int i = 0; i < nr; ++i)
	//{
	//	uchar *data = src.ptr<uchar>(i);
	//	for (int j = 0; j < nc; ++j)
	//	{
	//		if (*ptrRGBSum > threshold)
	//		{
	//			averageRGB[0] += *data;
	//			averageRGB[1] += *(data + 1);
	//			averageRGB[2] += *(data + 2);
	//			++amount;
	//		}
	//		data += 3;
	//		++ptrRGBSum;
 //		}
	//}
	//averageRGB /= amount;
	//
	////��ԭͼ��Ӧ�ø�ƽ��ֵ���б�������
	//averageRGB[0] = 255 / averageRGB[0];
	//averageRGB[1] = 255 / averageRGB[1];
	//averageRGB[2] = 255 / averageRGB[2];
	//cv::multiply(src, averageRGB, dst);

	//delete[] RGBSum;

}







//cv::Vec3d m_last_avg_bgr = cv::Vec3d(-1, -1, -1);
//double m_smooth_ratio = 0.8;
//std::vector<int> m_white_hist;
//m_white_hist.resize(766, 0);
//std::fill(m_white_hist.begin(), m_white_hist.end(), 0);
//std::vector<cv::Mat> bgr;
//split(src, bgr);
//cv::Mat sum = cv::Mat::zeros(src.size(), CV_32FC1);
//sum = cv::Mat::zeros(src.size(), CV_32FC1);
//cv::accumulate(bgr[0], sum);
//cv::accumulate(bgr[1], sum);
//cv::accumulate(bgr[2], sum);
//double min_value, max_value;
//cv::minMaxLoc(sum, &min_value, &max_value);
//int t = int(max_value*(1 - 0.1));

//float* ptr_sum = sum.ptr<float>();
//ptr_sum = sum.ptr<float>();
//for (int i = 0; i < sum.total(); ++i)
//{
//	m_white_hist[ptr_sum[i]]++;
//}

////t = 255;
//int hist_sum = 0;
//for (int i = 765; i >= 0; --i)
//{
//	hist_sum += m_white_hist[i];
//	if (hist_sum > sum.total() * 0.05)
//	{
//		t = i;
//		break;
//	}
//}

//ptr_sum = sum.ptr<float>();
//cv::Vec3b* ptr_img = src.ptr<cv::Vec3b>();
//ptr_img = src.ptr<cv::Vec3b>();
//int count = 0;
//cv::Vec3d avg_bgr(0, 0, 0);
//for (int i = 0; i < sum.total(); ++i)
//{
//	if (ptr_sum[i] > t)
//	{
//		count++;
//		avg_bgr += ptr_img[i];
//	}
//}
//avg_bgr /= count;
//if (count > 0)
//{
//	if (m_last_avg_bgr[0] < 0)
//	{
//		m_last_avg_bgr = avg_bgr;
//	}
//	avg_bgr = m_smooth_ratio * m_last_avg_bgr + (1 - m_smooth_ratio) * avg_bgr;
//	m_last_avg_bgr = avg_bgr;

//	cv::Scalar gain(255 / avg_bgr[0], 255 / avg_bgr[1], 255 / avg_bgr[2]);
//	cv::multiply(src, gain, dst);
//}
