#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "CinderOpenCv.h"

namespace ix {

class Fingertips
{
public:
    Fingertips();
    std::vector<cv::Point2i> detectFingertips( cv::Mat z, int zMin=100, int zMax=255 );
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

    float cutoff;
    ci::Surface surface;
    cv::Scalar fieldMean;
};

} // namespace ix

