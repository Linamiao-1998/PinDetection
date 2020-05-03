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
		pDeviceOfflineEventHandler = NULL; //<�����¼��ص����� 
		pFeatureEventHandler = NULL;//<Զ���豸�¼��ص����� 
		pCaptureEventHandler = NULL;//<�ɼ��ص�����

		//��ʼ�� 
		IGXFactory::GetInstance().Init();

		//ö���豸 	
		IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);
		if (0 == vectorDeviceInfo.size())
		{
			cout << "�޿����豸!" << endl;
		}

		//�򿪵�һ̨�豸�Լ��豸�����һ���� 
		ObjDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(vectorDeviceInfo[0].GetSN(), GX_ACCESS_EXCLUSIVE);
		ObjStreamPtr = ObjDevicePtr->OpenStream(0);

		//ע���豸�����¼���Ŀǰֻ��ǧ����ϵ�����֧�ִ��¼�֪ͨ�� 
		hDeviceOffline = NULL;
		pDeviceOfflineEventHandler = new CSampleDeviceOfflineEventHandler();
		hDeviceOffline = ObjDevicePtr->RegisterDeviceOfflineCallback(pDeviceOfflineEventHandler, NULL);

		//��ȡԶ���豸���Կ����� 
		ObjFeatureControlPtr = ObjDevicePtr->GetRemoteFeatureControl();

		//ע��Զ���豸�¼�:�ع�����¼���Ŀǰֻ��ǧ����ϵ�����֧���ع�����¼��� 
		//ѡ���¼�Դ 
		//ObjFeatureControlPtr->GetEnumFeature("EventSelector")->SetValue("ExposureEnd");

		////ʹ���¼� 
		//ObjFeatureControlPtr->GetEnumFeature("EventNotification")->SetValue("On");
		//GX_FEATURE_CALLBACK_HANDLE hFeatureEvent = NULL;
		//pFeatureEventHandler = new CSampleFeatureEventHandler();
		//hFeatureEvent = ObjFeatureControlPtr->RegisterFeatureCallback("EventExposureEnd", pFeatureEventHandler, NULL);

		////ע��ص��ɼ� 
		//pCaptureEventHandler = new CSampleCaptureEventHandler();
		//ObjStreamPtr->RegisterCaptureCallback(pCaptureEventHandler, NULL);

		//���Ϳ������� 
		ObjStreamPtr->StartGrab();
		ObjFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();
	}

	// Release_Device Detection
	void Release_Detection(CGXStreamPointer& ObjStreamPtr)
	{
		//����ͣ������ 
		ObjFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
		ObjStreamPtr->StopGrab();

		//ע���ɼ��ص� 
		//ObjStreamPtr->UnregisterCaptureCallback();

		//ע��Զ���豸�¼� 
		//ObjFeatureControlPtr->UnregisterFeatureCallback(hFeatureEvent);

		//ע���豸�����¼� 
		ObjDevicePtr->UnregisterDeviceOfflineCallback(hDeviceOffline);

		//�ͷ���Դ 
		ObjStreamPtr->Close();
		ObjDevicePtr->Close();

		//����ʼ���� 
		IGXFactory::GetInstance().Uninit();

		//�����¼��ص�ָ�� 
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
//�û��̳е����¼������� 
class CSampleDeviceOfflineEventHandler : public IDeviceOfflineEventHandler
{
public:
	void DoOnDeviceOfflineEvent(void* pUserParam)
	{
		cout << "�յ��豸�����¼�!" << endl;
	}
};
//�û��̳����Ը����¼������� 
class CSampleFeatureEventHandler : public IFeatureEventHandler
{
public:
	void DoOnFeatureEvent(const GxIAPICPP::gxstring& strFeatureName, void* pUserParam)
	{
		cout << "�յ��ع�����¼�!" << endl;
	}
};
//�û��̳вɼ��¼������� 
class CSampleCaptureEventHandler : public ICaptureEventHandler
{
public:
	void DoOnImageCaptured(CImageDataPointer& objImageDataPointer, void* pUserParam)
	{
		cout << "�յ�һ֡ͼ��!" << endl;
		cout << "ImageInfo: " << objImageDataPointer->GetStatus() << endl;
		cout << "ImageInfo: " << objImageDataPointer->GetWidth() << endl;
		cout << "ImageInfo: " << objImageDataPointer->GetHeight() << endl;
		cout << "ImageInfo: " << objImageDataPointer->GetPayloadSize() << endl;
	}
};

#endif

