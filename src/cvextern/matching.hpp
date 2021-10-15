#include <opencv2/opencv.hpp> 
#include <iostream> 
#include <opencv2/imgproc.hpp>
#include <chrono> 
#include <list> 
#include <iterator> 
using namespace std::chrono;
using namespace concurrency;
using namespace cv;
using namespace std;

namespace cvextern{
    void clip(Rect* imageroi,int width,int height) {
        if ((*imageroi).width + (*imageroi).x > width) {
            (*imageroi).width = width - (*imageroi).x;
        }
        if ((*imageroi).height + (*imageroi).y >height) {
            (*imageroi).height = height - (*imageroi).y;
        }
    }
    std::vector<cv::Rect>  findROI(Mat image) {
        std::vector<cv::Rect> results= std::vector<cv::Rect>();
        Mat labels;
        Mat stats;
        Mat centroids;
        connectedComponentsWithStats(image, labels, stats, centroids);
        for (int i = 1; i < stats.rows; i++)
        {
            int x = stats.at<int>(Point(0, i));
            int y = stats.at<int>(Point(1, i));
            int w = stats.at<int>(Point(2, i));
            int h = stats.at<int>(Point(3, i));
            cv::Rect rect(x * 2, y * 2, w* 2,h * 2);
            results.push_back(rect);
            
        }
        
        
        return results;
    }
    std::vector<cv::Rect>  findROIContours(Mat image) {
        std::vector<cv::Rect> results = std::vector<cv::Rect>();
        std::vector<std::vector<cv::Point> > contours;
        cv::findContours(image, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

        // Use the contours to define region of interest and 
        // perform template matching on the areas
        for (int i = 0; i < contours.size(); i++)
        {
            cv::Rect r = cv::boundingRect(contours[i]);
            cv::Rect rect(r.x * 2, r.y * 2, r.width * 2, r.height * 2);
            results.push_back(rect);
        }
        return results;
    }
    class TemplateMatching{
        
        public:
        int level=4;
        std::vector<Mat> templates;
        Mat templateImg;
        void CreateModel(int level, Mat templateImg)
        {
            this->templateImg=templateImg;
            this->level=level;
            this->templates = std::vector<Mat>(level);
            templates[0] = templateImg;
            for (int i = 1; i < level; i++)
            {
                templates[i] = Mat();
                pyrDown(templates[i - 1], templates[i]);
            }
        }
        void FindTemplate(Mat image,float threshold){
            Mat images[level];
            images[0] = image;
            for (int i = 1; i <level; i++)
            {
                images[i] = Mat();
                pyrDown(images[i - 1], images[i]);
            }
            
            Mat mask;
            std::vector<cv::Rect> ROI;
            Mat result;
            for (int i = level-1; i >= 0; i--)
            {
                result = cv::Mat::zeros(images[i].size() + cv::Size(1, 1) - templates[i].size(), CV_32FC1);
                
                if (i!=(level-1)) {
                    auto start_inner = high_resolution_clock::now();
                    for(int j=0; j<(int)ROI.size(); j++)
                    {
                        auto imageroi = ROI[j] + templates[i].size();
                        auto templateroi = ROI[j] + cv::Size(1, 1);
                        clip(&imageroi, images[i].cols, images[i].rows);
                        clip(&templateroi, result.cols, result.rows);
                        
                        matchTemplate(images[i](imageroi), templates[i], result(templateroi), TM_CCORR_NORMED);
                        
                        
                    };
                    
                    auto stop_inner = duration_cast<milliseconds> (high_resolution_clock::now() - start_inner);
                    cout << "Fast template pyramid " << i << ": " << stop_inner.count() << "ms" << endl;
                    if (i == 0) {
                        double min, max;
                        Point minloc, maxloc;
                        minMaxLoc(result, &min, &max, &minloc, &maxloc);
                        rectangle(image, Rect(maxloc.x, maxloc.y, templateImg.cols, templateImg.rows),255);
                        break;
                    }
                    
                }			
                else {
                    auto start_inner = high_resolution_clock::now();
                    matchTemplate(images[i], templates[i], result, TM_CCORR_NORMED);
                    auto stop_inner = duration_cast<milliseconds> (high_resolution_clock::now() - start_inner);
                    cout << "Fast template pyramid " << i << ": " << stop_inner.count() << "ms" << endl;
                    
                }
                result = result > threshold;
                ROI = findROIContours(result);
                
                
            }
        };

    };
}
