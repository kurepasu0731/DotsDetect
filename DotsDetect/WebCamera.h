#ifndef WEBCAMERA_H
#define WEBCAMERA_H

#include <opencv2\opencv.hpp>
#include <boost/thread/thread.hpp>

class WebCamera
{
public:
	//解像度
	int width;
	int height;
	//VidepCapture
    cv::VideoCapture cap;
	//最新画像
	cv::Mat fc2Mat;
	//gray画像
	cv::Mat gray;

	WebCamera(){
		width = 0;
		height = 0;
		cap = cv::VideoCapture(0);
	};

	WebCamera(int device, int _width, int _height)
	{
		width = _width;
		height = _height;
		cap = cv::VideoCapture(device);
	};

	void init()
	{
		cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
		cap.set(CV_CAP_PROP_FPS, 30);

		// スレッド間共有クラス
		critical_section = boost::shared_ptr<criticalSection> (new criticalSection);
	}

	void start()
	{
		quit = false;
		running = true;

		//スレッド生成
		thread = boost::thread( &WebCamera::threadFunction, this);
		
		fc2Mat.create(height, width, CV_8UC3);
	}

	void stop()
	{
		boost::unique_lock<boost::mutex> lock(mutex);
		quit = true;
		running = false;
		lock.unlock();
	}

	void release()
	{
		boost::unique_lock<boost::mutex> lock(mutex);
		fc2Mat.release();
		lock.unlock();
	}

	void queryFrame()
	{
		cap >> fc2Mat;
	}

	cv::Mat getImage()
	{
		return fc2Mat;
	}

	cv::Mat getImageGray()
	{
		return gray;
	}


	~WebCamera(){};

protected:
	boost::thread thread;
	mutable boost::mutex mutex;

	//スレッド処理
	void threadFunction()
	{
		while(!quit)
		{
			boost::shared_ptr<imgSrc> imgsrc = boost::shared_ptr<imgSrc>(new imgSrc);
			boost::unique_lock<boost::mutex> lock(mutex);
			queryFrame();
			//cv::cvtColor(fc2Mat, gray, CV_BGR2GRAY);
			lock.unlock();
			//critical_section->setImage(fc2Mat);
			imgsrc->image = fc2Mat;
			critical_section->setImageSource(imgsrc);
		}
	}

	bool quit;
	bool running;

	//! スレッド間の共有クラス
	boost::shared_ptr<criticalSection> critical_section;


};
#endif