#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
// #include "OpenCV/cv.h"
#include "CinderOpenCv.h"

namespace ix {

class Fingertips
{
public:
    Fingertips();
    void unproject( unsigned short* depth, float* x, float* y, float* z );
    // std::vector<cv::Point2i> detectFingertips( cv::Mat1f z, float zMin=0.0f, float zMax=0.75f );
    std::vector<cv::Point2i> detectFingertips( cv::Mat z, int zMin=0, int zMax=1000 );
    void drawContours();
    void drawFingertips();
    void drawField();

    bool initialized;

    cv::Mat field;
    cv::Mat handmask;
    std::vector< cv::Point2i > fingertips;
    std::vector< std::vector< cv::Point > > contours;
    std::vector< std::vector< cv::Point > > curves;
    std::vector< std::vector< int > > hulls;

    ci::Surface surface;
    float cutoff;
};

} // namespace ix

