#include "PinDetection.h"
void PinDetection::otsu(cv::Mat& inputImage, uchar& threshValue, uchar lowBound, uchar upBound)
{
	cv::Mat dstHist;
	int size = (upBound - lowBound);
	const float intensityRanges[] = { lowBound, upBound };
	const float* ranges[] = { intensityRanges };
	const int channels[] = { 0 };

	// Call function: calcHist()
	cv::calcHist(&inputImage, 1, channels, cv::Mat(), dstHist, 1, &size, ranges, 1, 0);

	// Calculate the adaptive threshValue
	double w0 = 0, w1 = 0;
	double u0 = 0, u1 = 0, uk = 0, uT = 0;
	double bVariances = 0;
	double max_bVariances = 0;
	int totalNumber = 0;
	for (int i = 1; i < dstHist.rows; i++)
	{
		totalNumber = totalNumber + dstHist.at<float>(i, 0);
	}
	for (int i = 1; i < dstHist.rows; i++)
	{
		uT = uT + i * dstHist.at<float>(i, 0) / totalNumber;
	}
	for (int i = 1; i < dstHist.rows; i++)
	{
		w0 = w0 + dstHist.at<float>(i, 0) / totalNumber;
		w1 = 1 - w0;
		uk = uk + i * dstHist.at<float>(i, 0) / totalNumber;
		u0 = uk / w0;
		u1 = (uT - uk) / w1;
		bVariances = w0 * w1 * (u1 - u0) * (u1 - u0);
		if (bVariances > max_bVariances)
		{
			max_bVariances = bVariances;
			threshValue = i;
		}
	}
}

void PinDetection::horizontal_projecting(cv::Mat& input_image, int& upper_bound, int& bottom_bound)
{
	vector<int> pointCount(input_image.rows); // pointer to hold the number of rows
	for (int i = 0; i < input_image.rows; i++) //遍历列数 
	{
		uchar* p_inputImage = input_image.ptr<uchar>(i);
		for (int j = 0; j < input_image.cols; j++)
		{
			if (p_inputImage[j] > 0)
				pointCount[i]++;
		}
	}
	cv::normalize(pointCount, pointCount, 100, 0, cv::NORM_MINMAX);
	//int scale = 4;
	//cv::Mat histImage = cv::Mat::zeros(Size(150, input_image.rows * scale), CV_8UC3);
	// Drawing Histogram
	int TH = 5;
	for (int k = 0; k < pointCount.size(); k++)
	{
		int binValue = pointCount[k];
		if (binValue <= TH && pointCount[k + 3] > TH && pointCount[k + 5] > TH)
		{
			upper_bound = k;
			//rectangle(histImage, Rect(0, k * scale, binValue, scale), Scalar(255, 255, 0), 2);
			continue;
		}
		if (binValue > TH && pointCount[k + 3] < TH && pointCount[k + 5] < TH)
		{
			bottom_bound = k;
			//rectangle(histImage, Rect(0, k * scale, binValue, scale), Scalar(0, 255, 255), 2);
			continue;
		}
		//rectangle(histImage, Rect( 0, k * scale, binValue, scale), Scalar(255, 255, 255), 2);

	}
	//namedWindow("horizontalProjecting", WINDOW_GUI_EXPANDED);
	//imshow("horizontalProjecting", histImage);
}
void PinDetection::vertical_projecting(cv::Mat& input_image, int& left_bound, int& right_bound)
{
	vector<int> pointCount(input_image.cols);

	for (int i = 0; i < input_image.cols; i++)
	{
		for (int j = 0; j < input_image.rows; j++)
		{
			if (input_image.at<uchar>(j, i) > 0)
				pointCount[i]++;
		}
	}

	cv::normalize(pointCount, pointCount, 100, 0, cv::NORM_MINMAX);
	//cout << cv::Mat(pointCount) << endl;
	//int scale = 4;
	//cv::Mat histImage = cv::Mat::zeros(Size(input_image.cols * scale, 150), CV_8UC3);
	// Drawing Vertical
	int TH = 10;
	int flag = 0;
	for (int k = 0; k < pointCount.size(); k++)
	{
		int binValue = pointCount[k];
		if (flag == 0 && binValue <= TH && pointCount[k + 3] > TH) //k<=1 k+1>1 k+3>1 定义为上升沿
		{
			left_bound = k;
			flag = 1;
			//cout << "left bound: " << left_bound << endl;
			//rectangle(histImage, Rect(k * scale, 0, scale, binValue), Scalar(255, 255, 0), 2);
			continue;
		}
		if (flag == 1 && binValue > TH && pointCount[k + 3] <= TH) // k>1, k+1<=1, k+3<=1 定义为下降沿
		{
			right_bound = k;
			//cout << "right bound: " << right_bound << endl;
			//rectangle(histImage, Rect(k * scale, 0, scale, binValue), Scalar(0, 255, 255), 2);
			continue;
		}
		//rectangle(histImage, Rect(k * scale, 0, scale, binValue), Scalar(255, 255, 255), 2);

	}
	//namedWindow("verticalProjecting", WINDOW_GUI_EXPANDED);
	//imshow("verticalProjecting", histImage);
}
void PinDetection::object_locate()
{
	cv::Mat* original_resize_image = new cv::Mat;
	cv::Mat* binaryImage = new cv::Mat;
	cv::Mat* roi = new cv::Mat;
	// Filter
	cv::GaussianBlur(original_image_, original_image_, cv::Size(3, 3), 1, 0.5);

	// Resize original image
	cv::resize(original_image_, *original_resize_image, cv::Size(original_image_.cols >> 3, original_image_.rows >> 3), cv::INTER_CUBIC);

	// OTSU binariztion
	uchar thresh_value;
	otsu(*original_resize_image, thresh_value, 0, 255);
	cout << "Global thresh value: " << int(thresh_value) << endl;
	thresh_value = thresh_value - THRESH_OFFSET;
	cv::threshold(*original_resize_image, *binaryImage, thresh_value, 255, cv::THRESH_BINARY);
	//showing image
	//cv::namedWindow("binaryImage", cv::WINDOW_GUI_EXPANDED);
	//cv::imshow("binaryImage", *binaryImage);

	int upper_bound = 0, bottom_bound = 0, left_bound = 0, right_bound = 0;
	horizontal_projecting(*binaryImage, upper_bound, bottom_bound);
	vertical_projecting(*binaryImage, left_bound, right_bound);
	//std::cout << "bound in image:" << endl;
	//std::cout << "upper: " << (upper_bound << 3) << "\t bottom: " << (bottom_bound << 3) << endl;
	//std::cout << "left: " << (left_bound << 3) << "\t right: " << (right_bound << 3) << endl;
	*roi = original_image_(cv::Range(upper_bound << 3, bottom_bound << 3), cv::Range(left_bound << 3, right_bound << 3));

	object_roi_ = cv::Mat::zeros(cv::Size(roi->cols + 2 * roiOffset, roi->rows + 2 * roiOffset), CV_8UC1);
	copyMakeBorder(*roi, object_roi_, roiOffset, roiOffset, roiOffset, roiOffset, cv::BORDER_CONSTANT, cv::Scalar::all(0));
	// showing image
	cv::namedWindow("objectImage", cv::WINDOW_GUI_EXPANDED);
	cv::imshow("objectImage", object_roi_);

	/*
	// Locate pins
	vector<vector<Point>> contours;
	vector<cv::Point> object_contour;
	vector<Vec4i> hierarchy;
	findContours(*binaryImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
	std::cout << "contour: " << contours.size() << endl;
	// Find the max contour
	int cmin = 100;
	vector<vector<Point>>::const_iterator itc = contours.begin();
	while (itc != contours.end())
	{
		if (itc->size() < cmin)
			itc = contours.erase(itc);
		else
			++itc;
	}
	for (size_t i = 0; i < contours.size(); i++)
	{
		// Approximate the contour of object to a rotated rectangle
		cv::RotatedRect rotate_rect_object = minAreaRect(contours[i]);
		// Recover to original scale
		cv::RotatedRect rotate_rect_object_;
		Point2f object_vertices_[4];
		int roi_offset = 10;
		rotate_rect_object_.angle = rotate_rect_object.angle;
		rotate_rect_object_.center = cv::Point2f(rotate_rect_object.center.x * 8, rotate_rect_object.center.y * 8);
		rotate_rect_object_.size = cv::Size(rotate_rect_object.size.width * 8 + roi_offset, rotate_rect_object.size.height * 8 + roi_offset);
		rotate_rect_object_.points(object_vertices_);
		cv::cvtColor(original_image_, object_roi_, cv::COLOR_GRAY2BGR);
		for (int j = 0; j <= 3; j++)
		{
			line(object_roi_, object_vertices_[j], object_vertices_[(j + 1) % 4], Scalar(0, 0, 255), 1);
		}
		// showing image
		cv::namedWindow("objectImage", cv::WINDOW_GUI_EXPANDED);
		cv::imshow("objectImage", object_roi_);
	}*/
	delete original_resize_image, binaryImage, roi;
}
bool PinDetection::is_connected()
{
	cv::Mat* resize_object_roi = new cv::Mat;
	cv::Mat* rectImage = new cv::Mat;

	// Resize original image
	cv::resize(object_roi_, *resize_object_roi, cv::Size(object_roi_.cols >> 2, object_roi_.rows >> 2), cv::INTER_CUBIC);

	// OTSU binariztion
	uchar thresh_value;
	otsu(*resize_object_roi, thresh_value, 0, 255);
	thresh_value = thresh_value + 55;

	cv::threshold(*resize_object_roi, binary_object_image, thresh_value, 255, cv::THRESH_BINARY);
	// showing image
	//cv::namedWindow("binary object image", cv::WINDOW_GUI_EXPANDED);
	//cv::imshow("binary object image", binary_object_image);

	// Find contours
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
	//vector<cv::Point> object_contour1;
	//vector<cv::Point> object_contour2;
	findContours(binary_object_image, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	/*for (int i = 0; i < contours.size(); i++)
	{
		std::cout << "contours: " << contours[i].size() << endl;
	}*/
	//std::cout << "contours: " << contours.size() << endl;
	int max_len = 0;
	int second_len = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > max_len)
		{
			object_contour1 = contours[i];
			max_len = contours[i].size();
		}
	}
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > second_len && contours[i].size() < max_len && max_len > second_len)
		{
			object_contour2 = contours[i];
			second_len = contours[i].size();
		}
	}
	//std::cout << "max_len: ,second_len: " << max_len << "   " << second_len << endl;
	/*
	vector<vector<Point>>::const_iterator itc = contours.begin();
	while (itc != contours.end())
	{
		if (itc->size() !=max_len && itc->size() != second_len)
			itc = contours.erase(itc);
		else
			++itc;
	}*/
	double area1 = contourArea(object_contour1);
	double area2 = contourArea(object_contour2);
	int flag;
	//std::cout << "area1: area2: " << area1 << "   " << area2 << endl;
	double div_area;
	if (area1 >= area2)
	{
		div_area = area1 / area2;
	}
	else  div_area = area2 / area1;
	//std::cout << "div_area:  " << div_area << endl;
	if (contours.size() == 1)
	{
		flag = 0;
		std::cout << "The cap's pins are connected." << endl;
	}
	else
	{
		if (div_area >= 1 && div_area <= 1.3)
		{
			flag = 1;
			std::cout << "The cap's pins are not connected." << endl;
		}
		else
		{
			flag = 0;
			std::cout << "The cap's pins are connected." << endl;
		}
	}
	/*cvtColor(binary_object_image, *rectImage, cv::COLOR_GRAY2BGR);
	RotatedRect rect_pin1 = minAreaRect(object_contour1);
	RotatedRect rect_pin2 = minAreaRect(object_contour2);
	Point2f P1[4];
	Point2f P2[4];
	rect_pin1.points(P1);
	rect_pin2.points(P2);
	for (int j = 0; j <= 3; j++)
	{
		line(*rectImage, P1[j], P1[(j + 1) % 4], Scalar(0, 0, 255), 1);
		line(*rectImage, P2[j], P2[(j + 1) % 4], Scalar(255, 0, 0), 1);
		// showing image
		cv::namedWindow("rectImage", cv::WINDOW_GUI_EXPANDED);
		cv::imshow("rectImage", *rectImage);
	}
	int area1 = rect_pin1.size.width * rect_pin1.size.height;
	int area2 = rect_pin2.size.width * rect_pin2.size.height;
	std::cout << "area1: area2: " << area1<<"   "<<area2 << endl;
	double div_area;
	if (area1 >= area2)
	{
		div_area = area1 / area2;
	}
	else  div_area = area2 / area1;
	//std::cout << "div_area:  " << div_area << endl;
	if (contours.size() == 1)
	{
		std::cout << "The cap's pins are connected." << endl;
	}
	else
	{
		if (div_area >= 1 && div_area <= 1.1)
		{
			std::cout << "The cap's pins are not connected." << endl;
		}
		else
			std::cout << "The cap's pins are connected." << endl;
	}*/

	delete resize_object_roi, rectImage;
	return flag;
}
double PinDetection::ratio()
{
	double width_len = 1.25;
	double height_len = 2;

	// Resize original image
	cv::Mat resize_ori_ratio;
	cv::resize(original_image_, resize_ori_ratio, cv::Size(object_roi_.cols >> 2, object_roi_.rows >> 2), cv::INTER_CUBIC);

	// OTSU binariztion
	cv::Mat* ratio_otsu = new cv::Mat;
	cv::Mat* ratio_otsu_image = new cv::Mat;
	uchar thresh_value;
	otsu(resize_ori_ratio, thresh_value, 0, 255);
	//cout << "Global thresh value: " << int(thresh_value) << endl;
	thresh_value = thresh_value - 2 * THRESH_OFFSET;
	cv::threshold(resize_ori_ratio, *ratio_otsu, thresh_value, 255, cv::THRESH_BINARY);
	//cv::namedWindow("*ratio_otsu", cv::WINDOW_GUI_EXPANDED);
	//cv::imshow("*ratio_otsu", *ratio_otsu);

	//cvtColor(*ratio_otsu, *ratio_otsu_image, cv::COLOR_GRAY2BGR);
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
	findContours(*ratio_otsu, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	int maxlen = 0;
	vector<cv::Point> object_contour;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > maxlen)
		{
			object_contour = contours[i];
			maxlen = contours[i].size();
		}
	}
	// 绘制轮廓的最小外接矩形
	cv::RotatedRect rect = minAreaRect(object_contour);
	double frame_area = 16 * rect.size.width * rect.size.height;
	//std::cout << "frame_area: " << frame_area << endl;
	double k = (width_len * height_len) / frame_area;
	std::cout << "k: " << k << endl;
	//Point2f P[4];
	//rect.points(P);
	//for (int j = 0; j <= 3; j++)
	//{
	//	line(*ratio_otsu_image, P[j], P[(j + 1) % 4], Scalar(0,0,255), 1);
		//std::cout << " :" << P[0] << P[1] << P[2]<<P[3]<<endl;
	//}
	//cv::namedWindow("*ratio_otsu_image", cv::WINDOW_GUI_EXPANDED);
	//cv::imshow("*ratio_otsu_image", *ratio_otsu_image)
	delete  ratio_otsu, ratio_otsu_image;
	return k;
}
void PinDetection::area_calculate()
{
	cv::Mat* rectImage = new cv::Mat;
	cv::Mat* pin1_op_rotImage = new cv::Mat;
	cv::Mat* pin2_op_rotImage = new cv::Mat;
	//cvtColor(binary_object_image, *rectImage, cv::COLOR_GRAY2BGR);
	cv::RotatedRect rect_pin1 = minAreaRect(object_contour1);
	cv::RotatedRect rect_pin2 = minAreaRect(object_contour2);

	double k1 = PinDetection::ratio();
	pinArea1_ = rect_pin1.size.width * rect_pin1.size.height * k1;
	pinArea2_ = rect_pin2.size.width * rect_pin2.size.height * k1;
	std::cout << "pinArea1_: " << pinArea1_ << "mm*mm" << endl;
	std::cout << "pinArea2_: " << pinArea2_ << "mm*mm" << endl;
	/*
	Point2f P1[4];
	Point2f P2[4];
	rect_pin1.points(P1);
	rect_pin2.points(P2);
	//std::cout << "area:" << rect_pin1.size << endl;
	Rect boundRect1;
	Rect boundRect2;
	boundRect1 = boundingRect(Mat(object_contour1));//由轮廓（点集）确定出正外接矩形
	boundRect2 = boundingRect(Mat(object_contour2));
	//获得正外接矩形的左上角坐标及宽高
	int width1 = boundRect1.width;
	int height1 = boundRect1.height;
	int x1= boundRect1.x;
	int y1 = boundRect1.y;
	//std::cout << " x,y:" << x  << " " <<y  << endl;
	int width2 = boundRect2.width;
	int height2 = boundRect2.height;
	int x2 = boundRect2.x;
	int y2 = boundRect2.y;
	//用画矩形方法绘制正外接矩形
	//rectangle(*rectImage , Rect(x1, y1,width1,height1), Scalar(255, 0, 255), 1, 8);
	//rectangle(*rectImage , Rect(x2, y2, width2, height2), Scalar(255, 0, 255), 1, 8);
	// showing image
	//cv::namedWindow("rectImage", cv::WINDOW_GUI_EXPANDED);
	//cv::imshow("rectImage", *rectImage);

	cv::Mat* bound_rectImage1 = new cv::Mat;
	cv::Mat* bound_rectImage2 = new cv::Mat;
	*bound_rectImage1 = object_roi_(cv::Range(y1 * 4, (y1 + height1) * 4), cv::Range(x1 * 4, (x1 + width1) * 4));
	*bound_rectImage2 = object_roi_(cv::Range(y2 * 4, (y2 + height2) * 4), cv::Range(x2 * 4, (x2 + width2) * 4));
	pin1Bound_ = Mat::zeros(Size(bound_rectImage1->cols +  roiOffset, bound_rectImage1->rows +  roiOffset), CV_8UC1);
	copyMakeBorder(*bound_rectImage1, pin1Bound_,  0.5 * roiOffset, 0.5 * roiOffset, 0.5 * roiOffset, 0.5 * roiOffset, BORDER_CONSTANT, Scalar::all(0));
	pin2Bound_ = Mat::zeros(Size(bound_rectImage2->cols + roiOffset, bound_rectImage2->rows + roiOffset), CV_8UC1);
	copyMakeBorder(*bound_rectImage2, pin2Bound_, 0.5 * roiOffset, 0.5 * roiOffset, 0.5 * roiOffset, 0.5 * roiOffset, BORDER_CONSTANT, Scalar::all(0));
	//std::cout << "x1<<2,y1<<2,(x1 + width1) << 2,(y1 + height1) << 2:" << (x1 * 4- 2 * roiOffset) << " " << (y1 * 4 - 2 * roiOffset) << " " << ((x1 + width1) * 4 + 2 * roiOffset) << " " << ((y1 + height1) * 4 + 2 * roiOffset) << " " << endl;
	// showing image
	cv::namedWindow("pin1Bound", cv::WINDOW_GUI_EXPANDED);
	cv::imshow("pin1Bound", pin1Bound_);
	cv::namedWindow("pin2Bound", cv::WINDOW_GUI_EXPANDED);
	cv::imshow("pin2Bound", pin2Bound_);

	cv::Mat* pin1_binary = new cv::Mat;
	cv::Mat* pin2_binary = new cv::Mat;
	cv::Mat* mask1 = new cv::Mat;
	*mask1 = Mat::zeros(pin1Bound_.size(), CV_8UC1);
	cv::Mat* mask2 = new cv::Mat;
	*mask2 = Mat::zeros(pin2Bound_.size(), CV_8UC1);
	//pin1Bound_.copyTo(*mask1);

	//bound_rectImage2.copyTo(*mask2);
	Point root_points1[1][4];
	int x11 = ((int)P1[0].x - x1 + 0.2 * roiOffset) * 4;
	int y11 = ((int)P1[0].y - y1 + 0.2 * roiOffset) * 4;
	int x12 = ((int)P1[1].x - x1 + 0.1 * roiOffset) * 4;
	int y12 = ((int)P1[1].y - y1 + 0.1 * roiOffset) * 4;
	int x13 = ((int)P1[2].x - x1 + 0.1 * roiOffset) * 4;
	int y13 = ((int)P1[2].y - y1 + 0.1 * roiOffset) * 4;
	int x14 = ((int)P1[3].x - x1 + 0.2 * roiOffset) * 4;
	int y14 = ((int)P1[3].y - y1 + 0.2 * roiOffset) * 4;
	root_points1[0][0] = Point(x11 , y11);
	root_points1[0][1] = Point(x12 , y12);
	root_points1[0][2] = Point(x13 , y13);
	root_points1[0][3] = Point(x14 , y14);
	//cout << "root_points1[0][0]" << root_points1[0][0] << " " << root_points1[0][1] << " " << root_points1[0][2] << " " << root_points1[0][3] << " " <<endl;
	const Point* ppt1[1] = { root_points1[0] };
	int npt1[] = { 4 };
	fillPoly(*mask1, ppt1, npt1, 1, Scalar(255, 255, 255));

	Point root_points2[1][4];
	int x21 = ((int)P2[0].x - x2 + 0.2 * roiOffset) * 4;
	int y21 = ((int)P2[0].y - y2 + 0.2 * roiOffset) * 4;
	int x22 = ((int)P2[1].x - x2 + 0.1 * roiOffset) * 4;
	int y22 = ((int)P2[1].y - y2 + 0.1 * roiOffset) * 4;
	int x23 = ((int)P2[2].x - x2 + 0.1 * roiOffset) * 4;
	int y23 = ((int)P2[2].y - y2 + 0.1 * roiOffset) * 4;
	int x24 = ((int)P2[3].x - x2 + 0.2 * roiOffset) * 4;
	int y24 = ((int)P2[3].y - y2 + 0.2 * roiOffset) * 4;
	root_points2[0][0] = Point(x21, y21);
	root_points2[0][1] = Point(x22, y22);
	root_points2[0][2] = Point(x23, y23);
	root_points2[0][3] = Point(x24, y24);
	const Point* ppt2[1] = { root_points2[0] };
	int npt2[] = { 4 };
	fillPoly(*mask2, ppt2, npt2, 1, Scalar(255, 255, 255));

	// showing image
	cv::namedWindow("mask1", cv::WINDOW_GUI_EXPANDED);
	cv::imshow("mask1", *mask1);
	copyTo(pin1Bound_, *pin1_op_rotImage, *mask1);

	cv::namedWindow("mask2", cv::WINDOW_GUI_EXPANDED);
	cv::imshow("mask2", *mask2);
	copyTo(pin2Bound_, *pin2_op_rotImage, *mask2);
	// showing image
	cv::namedWindow("pin1_op_rotImage", cv::WINDOW_GUI_EXPANDED);
	cv::imshow("pin1_op_rotImage", *pin1_op_rotImage);

	cv::namedWindow("pin2_op_rotImage", cv::WINDOW_GUI_EXPANDED);
	cv::imshow("pin2_op_rotImage", *pin2_op_rotImage);

	//OTSU
	uchar thresh_value1;
	otsu(*pin1_op_rotImage, thresh_value1, 0, 255);
	//cout << "Global thresh value: " << int(thresh_value1) << endl;
	thresh_value1 = thresh_value1 - THRESH_OFFSET;
	cv::threshold(*pin1_op_rotImage, *pin1_binary, thresh_value1, 255, THRESH_BINARY);

	uchar thresh_value2;
	otsu(*pin2_op_rotImage, thresh_value2, 0, 255);
	//cout << "Global thresh value: " << int(thresh_value2) << endl;
	thresh_value2 = thresh_value2 - THRESH_OFFSET;
	cv::threshold(*pin2_op_rotImage, *pin2_binary, thresh_value2, 255, THRESH_BINARY);
	//showing image
	cv::namedWindow("pin1_binary", cv::WINDOW_GUI_EXPANDED);
	cv::imshow("pin1_binary", * pin1_binary);

	cv::namedWindow("pin2_binary", cv::WINDOW_GUI_EXPANDED);
	cv::imshow("pin2_binary", *pin2_binary);*/

	delete rectImage, pin1_op_rotImage, pin2_op_rotImage;

}

