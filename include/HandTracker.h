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
    ~Hand() {};
    void drawFingertips();

    cv::Point center;
    bool isHand;
    double area;
    float radius;
    float hue;
    std::vector<cv::Point> fingertips;
    std::vector<cv::Point> contour;
    std::vector<cv::Point> approx;
    std::vector<int> hull;
};

class HandTracker
{
 public:
    HandTracker();
    std::vector<Hand> detectHands( cv::Mat z, int zMin=100, int zMax=255 );
    void drawField();

    bool initialized;

    cv::Mat field;
    cv::Mat handmask;
    std::vector<Hand> before;
    std::vector<Hand> hands;
    std::vector<std::vector<cv::Point> > contours;
    ci::gl::Texture texture;
};

} // namespace ix

