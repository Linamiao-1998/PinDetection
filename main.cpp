#define _CRT_SECURE_NO_WARNINGS
#include "PinDetection.h"
cv::Mat img;
int _tmain(int argc, _TCHAR* argv[])
{
	PinDetection pin_detect;
	CGXStreamPointer ObjStreamPtr;
	pin_detect.DeviceDetection(ObjStreamPtr);
	//声明事件回调对象指针 
	//IDeviceOfflineEventHandler* pDeviceOfflineEventHandler = NULL; //<掉线事件回调对象 
	//IFeatureEventHandler* pFeatureEventHandler = NULL;//<远端设备事件回调对象 
	//ICaptureEventHandler* pCaptureEventHandler = NULL;//<采集回调对象

	//初始化 
	//IGXFactory::GetInstance().Init();

	//枚举设备 
	//gxdeviceinfo_vector vectorDeviceInfo;
	//IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);
	//if (0 == vectorDeviceInfo.size())
	//{
	//	cout << "无可用设备!" << endl;
	//}
	//打开第一台设备以及设备下面第一个流 
	//CGXDevicePointer ObjDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(vectorDeviceInfo[0].GetSN(), GX_ACCESS_EXCLUSIVE);
	//CGXStreamPointer ObjStreamPtr = ObjDevicePtr->OpenStream(0);

	//注册设备掉线事件【目前只有千兆网系列相机支持此事件通知】 
	//GX_DEVICE_OFFLINE_CALLBACK_HANDLE hDeviceOffline = NULL;
	//pDeviceOfflineEventHandler = new CSampleDeviceOfflineEventHandler();
	//hDeviceOffline = ObjDevicePtr->RegisterDeviceOfflineCallback(pDeviceOfflineEventHandler, NULL);

	//获取远端设备属性控制器 
	//CGXFeatureControlPointer ObjFeatureControlPtr = ObjDevicePtr->GetRemoteFeatureControl();

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
	//ObjStreamPtr->StartGrab();
	//ObjFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();

    while (cv::waitKey() != 27)
    {
       //采单帧
	   CImageDataPointer objImageDataPtr;
	   objImageDataPtr = ObjStreamPtr->GetImage(500);//超时时间使用500ms，用户可以自行设定
	   if (objImageDataPtr->GetStatus() == GX_FRAME_STATUS_SUCCESS)				//采图成功而且是完整帧，可以进行图像处理...
	    {
	      img.create(objImageDataPtr->GetHeight(), objImageDataPtr->GetWidth(), CV_8UC3);
		  void* pRGB24Buffer = NULL;
		  //假设原始数据是BayerRG8图像
		  pRGB24Buffer = objImageDataPtr->ConvertToRGB24(GX_BIT_0_7, GX_RAW2RGB_NEIGHBOUR, true);
		  memcpy(img.data, pRGB24Buffer, (objImageDataPtr->GetHeight()) * (objImageDataPtr->GetWidth()) * 3);
		  cv::flip(img, img, 0);
		  cv::namedWindow("sss",cv::WINDOW_GUI_EXPANDED);
		  cv::imshow("sss", img);
		  //cv::waitKey(1);
		  //cv::imwrite("0001.bmp", img);
		  cout << "帧数：" << objImageDataPtr->GetFrameID() << endl;
	    }

	   //此时开采成功,进行图像处理
	   double startTime;
	   double endTime;
	   // Setting dataset root
	   //string dataset_root = "D:\\ProgramDebug\\PinDetection\\cap1\\";
	   //int frameNum = 3;
	   //for (int frame = 1; frame <= frameNum; frame++)
	   //{
		 //cout << "frame:" << frame << endl;
		 START;
		 ///char* image_name = new char[20];
		 ///sprintf(image_name, "%04d.bmp");
		 
		 // loading original image
		 pin_detect.image_load(img);
		 pin_detect.object_locate();
		 if (!pin_detect.is_connected())
			{
			 cout << "ERROR 2: The pins of this chip are connected!!!" << endl;
			}
		 else
			{
			 cout << "There are no problem with chip pins." << endl;
			 pin_detect.area_calculate();
			 }
		 END;
		 OUTPUT;	
    }	
	pin_detect.Release_Detection(ObjStreamPtr);
	return 0;
}

