#include <vector>
#include <functional>
#include <math.h>
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "CinderOpenCv.h"
#include "FrameBridge.h"

namespace ix {

class Hand
{
 public:
    Hand();
    ~Hand() {};
    void sync( const Hand & other );
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

struct PointDistance : public std::binary_function<cv::Point, cv::Point, float>
{
    inline float operator() ( const cv::Point & a, const cv::Point & b ) {
        return (float) pow( a.x - b.x, 2 ) + pow( a.y - b.y, 2 );
    }
};

class HandTracker
{
 public:
    HandTracker();
    std::vector<Hand> detectHands( cv::Mat z, int zMin=100, int zMax=255 );
    void bridgeFrames();
    void drawField();

    bool initialized;

    ix::FrameBridge<cv::Point, PointDistance> bridge;
    cv::Mat field;
    cv::Mat handmask;
    std::vector<Hand> before;
    std::vector<Hand> hands;
    std::vector<std::vector<cv::Point> > contours;
    ci::gl::Texture texture;
};

} // namespace ix

