#define _CRT_SECURE_NO_WARNINGS
#include "PinDetection.h"
cv::Mat img;
int _tmain(int argc, _TCHAR* argv[])
{
	PinDetection pin_detect;
	CGXStreamPointer ObjStreamPtr;
	pin_detect.DeviceDetection(ObjStreamPtr);
	//�����¼��ص�����ָ�� 
	//IDeviceOfflineEventHandler* pDeviceOfflineEventHandler = NULL; //<�����¼��ص����� 
	//IFeatureEventHandler* pFeatureEventHandler = NULL;//<Զ���豸�¼��ص����� 
	//ICaptureEventHandler* pCaptureEventHandler = NULL;//<�ɼ��ص�����

	//��ʼ�� 
	//IGXFactory::GetInstance().Init();

	//ö���豸 
	//gxdeviceinfo_vector vectorDeviceInfo;
	//IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);
	//if (0 == vectorDeviceInfo.size())
	//{
	//	cout << "�޿����豸!" << endl;
	//}
	//�򿪵�һ̨�豸�Լ��豸�����һ���� 
	//CGXDevicePointer ObjDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(vectorDeviceInfo[0].GetSN(), GX_ACCESS_EXCLUSIVE);
	//CGXStreamPointer ObjStreamPtr = ObjDevicePtr->OpenStream(0);

	//ע���豸�����¼���Ŀǰֻ��ǧ����ϵ�����֧�ִ��¼�֪ͨ�� 
	//GX_DEVICE_OFFLINE_CALLBACK_HANDLE hDeviceOffline = NULL;
	//pDeviceOfflineEventHandler = new CSampleDeviceOfflineEventHandler();
	//hDeviceOffline = ObjDevicePtr->RegisterDeviceOfflineCallback(pDeviceOfflineEventHandler, NULL);

	//��ȡԶ���豸���Կ����� 
	//CGXFeatureControlPointer ObjFeatureControlPtr = ObjDevicePtr->GetRemoteFeatureControl();

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
	//ObjStreamPtr->StartGrab();
	//ObjFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();

    while (cv::waitKey() != 27)
    {
       //�ɵ�֡
	   CImageDataPointer objImageDataPtr;
	   objImageDataPtr = ObjStreamPtr->GetImage(500);//��ʱʱ��ʹ��500ms���û����������趨
	   if (objImageDataPtr->GetStatus() == GX_FRAME_STATUS_SUCCESS)				//��ͼ�ɹ�����������֡�����Խ���ͼ����...
	    {
	      img.create(objImageDataPtr->GetHeight(), objImageDataPtr->GetWidth(), CV_8UC3);
		  void* pRGB24Buffer = NULL;
		  //����ԭʼ������BayerRG8ͼ��
		  pRGB24Buffer = objImageDataPtr->ConvertToRGB24(GX_BIT_0_7, GX_RAW2RGB_NEIGHBOUR, true);
		  memcpy(img.data, pRGB24Buffer, (objImageDataPtr->GetHeight()) * (objImageDataPtr->GetWidth()) * 3);
		  cv::flip(img, img, 0);
		  cv::namedWindow("sss",cv::WINDOW_GUI_EXPANDED);
		  cv::imshow("sss", img);
		  //cv::waitKey(1);
		  //cv::imwrite("0001.bmp", img);
		  cout << "֡����" << objImageDataPtr->GetFrameID() << endl;
	    }

	   //��ʱ���ɳɹ�,����ͼ����
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

