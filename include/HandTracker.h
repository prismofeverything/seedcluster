#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "CinderOpenCv.h"

namespace ix {

class Hand
{
 public:
    Hand();
    void drawFingertips();

    cv::Point center;
    double area;
    std::vector<cv::Point> fingertips;
    std::vector<cv::Point> contour;
    std::vector<cv::Point> approx;
    std::vector<int> hull;
};

class HandTracker
{
 public:
    HandTracker();
    std::vector<cv::Point2i> detectFingertips( cv::Mat z, int zMin=100, int zMax=255 );
    std::vector<Hand> detectHands( cv::Mat z, int zMin=100, int zMax=255 );
    void drawContours();
    void drawFingertips();
    void drawField();

    bool initialized;

    cv::Mat field;
    cv::Mat handmask;
    std::vector<Hand> hands;
    std::vector<cv::Point> fingertips;
    std::vector<std::vector<cv::Point> > contours;
    std::vector<std::vector<cv::Point> > curves;
    std::vector<std::vector<int> > hulls;

    float cutoff;
    ci::gl::Texture texture;
};

} // namespace ix

