//
//  CameraManager.cpp
//  youme_voice_engine
//
//  Created by bhb on 2017/10/18.
//  Copyright © 2017年 Youme. All rights reserved.
//

#include "CameraManager.h"
#include "../interface/IYouMeVoiceEngine.h"
#include "NgnTalkManager.hpp"
#include "../interface/YouMeEngineManagerForQiniu.hpp"
#include "YouMeEngineVideoCodec.hpp"
#include "YouMeVideoMixerAdapter.h"
#include "opencv2/opencv.hpp"

#define CLAMP(i)   (((i) < 0) ? 0 : (((i) > 255) ? 255 : (i)))

static void AMSampleGrabberCallBack(BYTE * pBuffer, long lBufferSize, int width, int height, int fps, double dblSampleTime, void* context)
{
    CameraManager * pThis = (CameraManager*)context;
    if (pThis)
    {
		uint64_t pts = tsk_time_now();
		pThis->videoDataOutput(pBuffer, lBufferSize, width, height, VIDEO_FMT_RGB24, 180, 4, pts, 1);
    }
}

CameraManager *CameraManager::getInstance()
{
    static CameraManager* sInstance = NULL;
    if (NULL == sInstance)
    {
        sInstance = new CameraManager ();
    }
    return sInstance;
}

CameraManager::CameraManager()
{
    m_width = 640;
    m_height = 480;
    m_fps = 30;
    m_cameraId = 0;
    m_cameraCount = -1;
	m_bBeautifyOpened = false;
	m_beautifyLevel = 0;

    this->cameraPreviewCallback = NULL;
}

YouMeErrorCode CameraManager::registerCameraPreviewCallback(CameraPreviewCallback *cb)
{
    std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    this->cameraPreviewCallback = cb;
    return YOUME_SUCCESS;
}

YouMeErrorCode CameraManager::unregisterCameraPreviewCallback()
{
    std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    this->cameraPreviewCallback = NULL;
    return YOUME_SUCCESS;
}

YouMeErrorCode CameraManager::registerYoumeEventCallback(IYouMeEventCallback *cb)
{
    std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    this->youmeEventCallback = cb;
    return YOUME_SUCCESS;
}

YouMeErrorCode CameraManager::unregisterYoumeEventCallback()
{
    std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    this->youmeEventCallback = NULL;
    return YOUME_SUCCESS;
}

YouMeErrorCode CameraManager::startCapture()
{
    if(this->open) {
        TSK_DEBUG_WARN("unexpected repeat calling");
        return YOUME_SUCCESS;
    }
	std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    m_amcapdshow.SetCallBack(AMSampleGrabberCallBack, this);
    if (m_amcapdshow.OpenCamera(m_cameraId, false, m_width, m_height, m_fps)){
        ICameraManager::startCapture();
        this->open = true;
        return YOUME_SUCCESS;
    }
    else  {
        return YOUME_ERROR_START_FAILED;
    }
    return YOUME_ERROR_START_FAILED;
}

YouMeErrorCode CameraManager::stopCapture()
{
    if(!this->open) {
        TSK_DEBUG_WARN("unexpected repeat calling");
        return YOUME_SUCCESS;
    }
	this->open = false;
	std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    m_amcapdshow.CloseCamera();
    ICameraManager::stopCapture();
    return YOUME_SUCCESS;
}

YouMeErrorCode CameraManager::setCaptureProperty(float fFps, int nWidth, int nHeight)
{
    m_width = nWidth;
    m_height = nHeight;
    m_fps = (fFps > 30 ? fFps : 30);

    TSK_DEBUG_INFO("setCaptureProperty mfps:%d, fps:%d, w:%d, h:%d", m_fps, fFps, nWidth, nHeight);
    //std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    //[[CameraController getInstance]setCapturePropertyWithFps:fFps Width:nWidth Height:nHeight];
	//YouMeEngineManagerForQiniu::getInstance()->setMixVideoSize(nWidth, nHeight);
    return YOUME_SUCCESS;
}

YouMeErrorCode CameraManager::setCaptureFrontCameraEnable(bool enable)
{
    std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    //[[CameraController getInstance] setCaptureFrontCameraEnable:enable];
    return YOUME_SUCCESS;
}

bool CameraManager::isCaptureFrontCameraEnable()
{
    std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    //return [[CameraController getInstance] isCaptureFrontCameraEnable];
    return false;
}

YouMeErrorCode CameraManager::switchCamera()
{
    std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    
    return YOUME_ERROR_UNKNOWN;
}

cv::Mat changeRGB24DataToMat(void* data, int len, int width, int height)
{
	cv::Mat input(height, width, CV_8UC(3), data);
	static cv::Mat output;

	//cv::cvtColor(input, output, cv::COLOR_RGB2BGR);
	return input;
}

cv::Mat doBeauty(cv::Mat img, float beautyLevel)
{
	//int value1 = 3;
	//int value2 = 1;

	//int dx = 5;// *5;
	//double fc = value1 * 12.5;

	//static cv::Mat dst;
	//static cv::Mat temp1, temp2;
	//cv::bilateralFilter(img, temp1, dx, fc, fc);
	//cv::subtract(temp1, img, temp2);
	//cv::add(temp2, cv::Scalar(10, 10, 10, 128), temp2);
	//cv::GaussianBlur(temp2, temp1, cv::Size(2 * value2 - 1, 2 * value2 - 1), 0);
	//cv::add(img, temp1, dst);

	////cv::addWeighted(img, p, temp4, 1 - p, 0, dst);
	////cv::add(dst, cv::Scalar(10, 10, 10, 255), dst);

	////通过融合原始图像和美颜图像，来处理beautyLevel
	//cv::addWeighted(img, (1 - beautyLevel), dst, beautyLevel, 0, dst);

	static cv::UMat src;
	img.copyTo(src);

	int value1 = 3;
	int value2 = 1;

	int dx = 15;// *5;
	double fc = value1 * 12.5;

	static cv::Mat dst;
	static cv::UMat temp1, temp2;
	cv::bilateralFilter(src, temp1, dx, fc, fc);
	cv::subtract(temp1, src, temp2);
	cv::add(temp2, cv::Scalar(10, 10, 10, 128), temp2);
	cv::GaussianBlur(temp2, temp1, cv::Size(2 * value2 - 1, 2 * value2 - 1), 0);
	cv::add(src, temp1, temp2);

	//cv::addWeighted(img, p, temp4, 1 - p, 0, dst);
	//cv::add(dst, cv::Scalar(10, 10, 10, 255), dst);

	//通过融合原始图像和美颜图像，来处理beautyLevel
	cv::addWeighted(src, (1 - beautyLevel), temp2, beautyLevel, 0, temp1);

	temp1.copyTo(dst);

	return dst;
}

void changeMatToRGB24Data(cv::Mat img, void* data, int len, int width, int height)
{
	//cv::Mat imgRGB;
	//cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
	memcpy(data, img.data, len);

	return;
}

void beautyFilter(void* data, int len, int width, int height, int fmt, bool isbeautyOpen,  float beautyLevel )
{
	//美颜不起效就不处理了
	if (!isbeautyOpen || beautyLevel <= 0.01)
	{
		return;
	}

	if (fmt != VIDEO_FMT_RGB24)
	{
		return;
	}

	cv::Mat  img = changeRGB24DataToMat( data, len, width, height );
	cv::Mat  imgBeauty = doBeauty(img, beautyLevel );

	changeMatToRGB24Data(imgBeauty, data, len, width, height );

	return;
}

void  CameraManager::openBeautify(bool open)
{
	m_bBeautifyOpened = open;
}
void  CameraManager::beautifyChanged(float param)
{
	m_beautifyLevel = param;
	if (m_beautifyLevel > 1)
	{
		m_beautifyLevel = 1;
	}
	else if( m_beautifyLevel < 0 ){
		m_beautifyLevel = 0;
	}
}

YouMeErrorCode CameraManager::videoDataOutput(void* data, int len, int width, int height, int fmt, int rotation, int mirror, uint64_t timestamp, int renderflag){
	if (!data)
        return YOUME_ERROR_INVALID_PARAM;
	if (!this->open){
		return YOUME_ERROR_WRONG_STATE;
	}

	beautyFilter(data, len, width, height, fmt, m_bBeautifyOpened, m_beautifyLevel);
    std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    FrameImage* frameImage = new FrameImage(width, height, data, len, mirror, timestamp);
    len = ICameraManager::format_transfer(frameImage, fmt);
    if (fmt != VIDEO_FMT_H264) {
        frameImage->len = len;
        frameImage->fmt = VIDEO_FMT_YUV420P;
    }

    if(rotation)
        ICameraManager::rotation_and_mirror(frameImage, rotation, tsk_false);

    if (renderflag) {
        YouMeVideoMixerAdapter::getInstance()->pushVideoFrameLocal(CNgnTalkManager::getInstance()->m_strUserID, frameImage->data, len, frameImage->width, frameImage->height, fmt, rotation, mirror, timestamp);
        YouMeEngineVideoCodec::getInstance()->pushFrame(frameImage, true);
    } else {
        frameImage->videoid = 2;    // share video stream type
        frameImage->double_stream = true; 
        YouMeEngineVideoCodec::getInstance()->pushFrameNew(frameImage);
    }
    
    
    return YOUME_SUCCESS;
}

int  CameraManager::getCameraCount()
{
    std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    m_cameraCount = CCameraCaptureDShow::CameraCount();
    return m_cameraCount;
}
std::string CameraManager::getCameraName(int cameraId)
{
    return CCameraCaptureDShow::CameraName(cameraId);
}
YouMeErrorCode CameraManager::setOpenCameraId(int cameraId)
{
    std::lock_guard<std::recursive_mutex> stateLock(*mutex);
    if (m_cameraCount == -1) {
        m_cameraCount = CCameraCaptureDShow::CameraCount();
    }
    if(cameraId >= m_cameraCount) 
    {
        return YOUME_ERROR_INVALID_PARAM;
    }
    m_cameraId = cameraId;
    return YOUME_SUCCESS;
}
