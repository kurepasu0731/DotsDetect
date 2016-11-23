#include <boost/thread/thread.hpp>
#include <opencv2\opencv.hpp>


class criticalSection
{
public:
	criticalSection()
	{
		//image = boost::shared_ptr<cv::Mat>(new cv::Mat);
		counter = 0;
	}

	~criticalSection(){};

	inline cv::Mat getImage()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(image_mutex);
		return image;
	}

	inline cv::Mat getImage_data()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(image_mutex);
		return cv::Mat(1280, 720, CV_8UC3, data);
	}

	inline void setImage(const cv::Mat &img)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(image_mutex);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		image = img;
		//data = img.data;
	}

	inline void setTest()
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(image_mutex);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		counter++;
		testStr = std::to_string(counter);
	}

	inline std::string getTest()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(image_mutex);
		return testStr;
	}


private:
	boost::shared_mutex image_mutex;

	cv::Mat image;
	unsigned char* data;

	std::string testStr;
	int counter;

};
