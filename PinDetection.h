#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <tchar.h> // #include "stdafx.h"
using namespace std;
//using namespace cv;
#include"GalaxyIncludes.h"
#include"windows.h"
#pragma once
#ifndef _PINDETECTION_H
#define _PINDETECTION_H

#define START startTime = cv::getTickCount()
#define END endTime = cv::getTickCount()
#define OUTPUT cout << "The running time: " << double(endTime - startTime)*1000 / cv::getTickFrequency() << "ms." << endl

const int THRESH_OFFSET = 20;
const int roiOffset = 20;

class CSampleCaptureEventHandler;
class CSampleFeatureEventHandler;
class CSampleDeviceOfflineEventHandler;

class PinDetection
{
private:
	//string input_image_path_;
	cv::Mat original_image_;
	cv::Mat object_roi_;
	cv::Mat binary_object_image;
	cv::Mat pin1Bound_;
	cv::Mat pin2Bound_;
	double pinArea1_, pinArea2_;
	vector<cv::Point> object_contour1;
	vector<cv::Point> object_contour2;

	IDeviceOfflineEventHandler* pDeviceOfflineEventHandler;
	IFeatureEventHandler* pFeatureEventHandler;
	ICaptureEventHandler* pCaptureEventHandler;

	gxdeviceinfo_vector vectorDeviceInfo;

	CGXDevicePointer ObjDevicePtr;

	GX_DEVICE_OFFLINE_CALLBACK_HANDLE hDeviceOffline;
	
	CGXFeatureControlPointer ObjFeatureControlPtr;

	// OTSU binarization
	void otsu(cv::Mat& input_image, uchar& thresh_value, uchar low_bound, uchar up_bound);

	// Horizontal projecting to locate upper and bottom bounds of object
	void horizontal_projecting(cv::Mat& input_image, int& upper_bound, int& bottom_bound);

	// Vertical projecting to locate left and right bounds of object
	void vertical_projecting(cv::Mat& input_image, int& left_bound, int& right_bound);

	// Ratio
	double ratio();

public:
	friend CSampleDeviceOfflineEventHandler;
	friend CSampleFeatureEventHandler;
	friend CSampleCaptureEventHandler;
	// Default constructor
	PinDetection()
	{
		cout << "The image have not been loaded!" << endl
			<< "Please call the member funcion \'image_load()\' of PinDetection class." << endl;
	}

	// Deconstructor
	~PinDetection()
	{
	
	};

	// Load image
	void image_load(cv::Mat &image)
	{
		cv::cvtColor(image, original_image_, cv::COLOR_BGR2GRAY);
		if (!original_image_.empty())
		{
			cout << "The image have been loaded successfully!" << endl;
			cout << "The size of input image: [" << original_image_.rows << ", " << original_image_.cols << "]" << endl;
			// showing image
			/*cv::namedWindow("original image", cv::WINDOW_GUI_EXPANDED);
			cv::imshow("original image", original_image_);*/
		}
		else
		{
			cout << "ERROR 1: Failed to load image, Please check the input path!!!" << endl;
		}
	}

	// Device Detection
	void DeviceDetection(CGXStreamPointer& ObjStreamPtr)
	{
		pDeviceOfflineEventHandler = NULL; //<掉线事件回调对象 
		pFeatureEventHandler = NULL;//<远端设备事件回调对象 
		pCaptureEventHandler = NULL;//<采集回调对象

		//初始化 
		IGXFactory::GetInstance().Init();

		//枚举设备 	
		IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);
		if (0 == vectorDeviceInfo.size())
		{
			cout << "无可用设备!" << endl;
		}

		//打开第一台设备以及设备下面第一个流 
		ObjDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(vectorDeviceInfo[0].GetSN(), GX_ACCESS_EXCLUSIVE);
		ObjStreamPtr = ObjDevicePtr->OpenStream(0);

		//注册设备掉线事件【目前只有千兆网系列相机支持此事件通知】 
		hDeviceOffline = NULL;
		pDeviceOfflineEventHandler = new CSampleDeviceOfflineEventHandler();
		hDeviceOffline = ObjDevicePtr->RegisterDeviceOfflineCallback(pDeviceOfflineEventHandler, NULL);

		//获取远端设备属性控制器 
		ObjFeatureControlPtr = ObjDevicePtr->GetRemoteFeatureControl();

		//注册远端设备事件:曝光结束事件【目前只有千兆网系列相机支持曝光结束事件】 
		//选择事件源 
		//ObjFeatureControlPtr->GetEnumFeature("EventSelector")->SetValue("ExposureEnd");

		////使能事件 
		//ObjFeatureControlPtr->GetEnumFeature("EventNotification")->SetValue("On");
		//GX_FEATURE_CALLBACK_HANDLE hFeatureEvent = NULL;
		//pFeatureEventHandler = new CSampleFeatureEventHandler();
		//hFeatureEvent = ObjFeatureControlPtr->RegisterFeatureCallback("EventExposureEnd", pFeatureEventHandler, NULL);

		////注册回调采集 
		//pCaptureEventHandler = new CSampleCaptureEventHandler();
		//ObjStreamPtr->RegisterCaptureCallback(pCaptureEventHandler, NULL);

		//发送开采命令 
		ObjStreamPtr->StartGrab();
		ObjFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();
	}

	// Release_Device Detection
	void Release_Detection(CGXStreamPointer& ObjStreamPtr)
	{
		//发送停采命令 
		ObjFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
		ObjStreamPtr->StopGrab();

		//注销采集回调 
		//ObjStreamPtr->UnregisterCaptureCallback();

		//注销远端设备事件 
		//ObjFeatureControlPtr->UnregisterFeatureCallback(hFeatureEvent);

		//注销设备掉线事件 
		ObjDevicePtr->UnregisterDeviceOfflineCallback(hDeviceOffline);

		//释放资源 
		ObjStreamPtr->Close();
		ObjDevicePtr->Close();

		//反初始化库 
		IGXFactory::GetInstance().Uninit();

		//销毁事件回调指针 
		if (NULL != pCaptureEventHandler)
		{
			delete pCaptureEventHandler;
			pCaptureEventHandler = NULL;
		}
		if (NULL != pDeviceOfflineEventHandler)
		{
			delete pDeviceOfflineEventHandler;
			pDeviceOfflineEventHandler = NULL;
		}
		if (NULL != pFeatureEventHandler)
		{
			delete pFeatureEventHandler;
			pFeatureEventHandler = NULL;
		}
	}

	// Locate object
	void object_locate();

	// Determine whether pins are connected
	bool is_connected();

	// Calculate the area of each pin
	void area_calculate();

	// Draw the contour of pins
	void object_drawing();

};
//用户继承掉线事件处理类 
class CSampleDeviceOfflineEventHandler : public IDeviceOfflineEventHandler
{
public:
	void DoOnDeviceOfflineEvent(void* pUserParam)
	{
		cout << "收到设备掉线事件!" << endl;
	}
};
//用户继承属性更新事件处理类 
class CSampleFeatureEventHandler : public IFeatureEventHandler
{
public:
	void DoOnFeatureEvent(const GxIAPICPP::gxstring& strFeatureName, void* pUserParam)
	{
		cout << "收到曝光结束事件!" << endl;
	}
};
//用户继承采集事件处理类 
class CSampleCaptureEventHandler : public ICaptureEventHandler
{
public:
	void DoOnImageCaptured(CImageDataPointer& objImageDataPointer, void* pUserParam)
	{
		cout << "收到一帧图像!" << endl;
		cout << "ImageInfo: " << objImageDataPointer->GetStatus() << endl;
		cout << "ImageInfo: " << objImageDataPointer->GetWidth() << endl;
		cout << "ImageInfo: " << objImageDataPointer->GetHeight() << endl;
		cout << "ImageInfo: " << objImageDataPointer->GetPayloadSize() << endl;
	}
};

#endif

