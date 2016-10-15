#pragma once

#define NOMINMAX
#include <Windows.h>
#include <opencv2\opencv.hpp>

#include <atltime.h>

#include<fstream>
#include<iostream>
#include<string>
#include<sstream> //文字ストリーム



#define CAMERA_WIDTH 1920
#define CAMERA_HEIGHT 1080

#define DOT_SIZE 150
#define A_THRESH_VAL -5
#define DOT_THRESH_VAL_MIN 100  // ドットノイズ弾き
#define DOT_THRESH_VAL_MAX 500 // エッジノイズ弾き

void calCoG_dot_v0(cv::Mat &src, cv::Point& sum, int& cnt, cv::Point& min, cv::Point& max, cv::Point p) 
{
	if (src.at<uchar>(p)) {
		sum += p; cnt++;
		src.at<uchar>(p) = 0;
		if (p.x<min.x) min.x = p.x;
		if (p.x>max.x) max.x = p.x;
		if (p.y<min.y) min.y = p.y;
		if (p.y>max.y) max.y = p.y;

		if (p.x - 1 >= 0) calCoG_dot_v0(src, sum, cnt, min, max, cv::Point(p.x-1, p.y));
		if (p.x + 1 < CAMERA_WIDTH) calCoG_dot_v0(src, sum, cnt, min, max, cv::Point(p.x + 1, p.y));
		if (p.y - 1 >= 0) calCoG_dot_v0(src, sum, cnt, min, max, cv::Point(p.x, p.y - 1));
		if (p.y + 1 < CAMERA_HEIGHT) calCoG_dot_v0(src, sum, cnt, min, max, cv::Point(p.x, p.y + 1));
	}
}

bool init_v0(cv::Mat &src) 
{
	cv::Mat origSrc = src.clone();
	//適応的閾値処理
	cv::adaptiveThreshold(src, src, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 7, A_THRESH_VAL);
	//普通の二値化
	//cv::threshold(src, src, 150, 255, cv::THRESH_BINARY);
	cv::Mat ptsImg = cv::Mat::zeros(CAMERA_HEIGHT, CAMERA_WIDTH, CV_8UC3);
	cv::cvtColor(src, ptsImg, CV_GRAY2BGR);

	cv::Point sum, min, max, p;
	int cnt;
	std::vector<cv::Point> dots;
	for (int i = 0; i < CAMERA_HEIGHT; i++) {
		for (int j = 0; j < CAMERA_WIDTH; j++) {
			if (src.at<uchar>(i, j)) {
				sum = cv::Point(0, 0); cnt = 0; min = cv::Point(j, i); max = cv::Point(j, i);
				calCoG_dot_v0(src, sum, cnt, min, max, cv::Point(j, i));
				if (cnt>DOT_THRESH_VAL_MIN && max.x - min.x < DOT_THRESH_VAL_MAX && max.y - min.y < DOT_THRESH_VAL_MAX) {
					dots.push_back(cv::Point(sum.x / cnt, sum.y / cnt));
				}
			}
		}
	}


	//cv::rectangle(ptsImg, cv::Point(CamWidth / 4, CamHeight / 4), cv::Point(CamWidth * 3 / 4, CamHeight * 3 / 4), cv::Scalar(255, 0, 0), 5, 4);
	// OpenGL用に予めRGB用のデータ作成
	//cv::rectangle(ptsImg, cv::Point(CamWidth / 4, CamHeight / 4), cv::Point(CamWidth * 3 / 4, CamHeight * 3 / 4), cv::Scalar(0, 0, 255), 5, 4);
	std::vector<cv::Point>::iterator it = dots.begin();
	
	bool k = (dots.size()==DOT_SIZE);
	//for (int i=0; it != dots.end(); i++,++it) {
	//	if (i && i%MarkersWidth == 0) {
	//		if ((*it).y <= (*(it - 1)).y) k = false;
	//	} else {
	//		if (i && (*it).x <= (*(it - 1)).x) k = false;
	//	}
	//}
	//cv::Scalar co = k ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
	// OpenGL用に予めRGB用のデータ作成
	cv::Scalar color = k ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 255, 0);
	for (it = dots.begin(); it != dots.end(); ++it) {
		cv::circle(ptsImg, *it, 3, color, 2);
	}

	//if (k) {
	//	it = dots.begin();
	//	for (int i = 0; it != dots.end(); ++it) {
	//		marker_u[i / MarkersWidth][i%MarkersWidth] = *it;
	//		marker_s[i / MarkersWidth][i%MarkersWidth] = true;
	//		i++;
	//	}
	//	
	//	flag = 1;
	//	std::cout << "init complete!" << std::endl;
	//	
	//}

	cv::Mat resize_src, resize_pts;
	cv::resize(origSrc, resize_src, cv::Size(), 0.5, 0.5);
	cv::resize(ptsImg, resize_pts, cv::Size(), 0.5, 0.5);

	cv::imshow("src", resize_src);
	cv::imshow("result", resize_pts);

	return k;
}

bool loadDots(std::vector<cv::Point2f> &corners)
{
	std::string filename = "dots.csv";

    //ファイルの読み込み
    std::ifstream ifs(filename);
    if(!ifs){
        return 0;
    }

    //csvファイルを1行ずつ読み込む
    std::string str;
    while(std::getline(ifs,str)){
        std::string token;
        std::istringstream stream(str);

		//x座標
		std::getline(stream,token,',');
		int x = std::stoi(token);
		//y座標
		std::getline(stream,token,',');
		int y = std::stoi(token);

		corners.emplace_back(cv::Point2f(x, y));

	}
	return true;
}


int main(int argc, char** argv)
{
    cv::VideoCapture cap(0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT);
	cap.set(CV_CAP_PROP_FPS, 30);

    const int cycle = 30;

	bool refresh = true;

    CFileTime cTimeStart, cTimeEnd;
    CFileTimeSpan cTimeSpan;

    cv::waitKey(cycle);

	std::vector<cv::Point2f> dots;
	loadDots(dots);

	for(int i = 0; i < dots.size(); i++)
	{
		std::cout << "[" << i << "]: " << dots[i] << std::endl;
	}

    while (1) {

        cv::Mat frame;
        cap >> frame;

		cv::Mat currFrameGray;
        cv::cvtColor(frame, currFrameGray, CV_BGR2GRAY);

		cTimeStart = CFileTime::GetCurrentTime();
		init_v0(currFrameGray);
		cTimeEnd = CFileTime::GetCurrentTime();           // 現在時刻
		cTimeSpan = cTimeEnd - cTimeStart;
		std::cout << cTimeSpan.GetTimeSpan()/10000 << "[ms]" << std::endl;

		if(cv::waitKey(32) == 27) break;

	}
    return 0;
}