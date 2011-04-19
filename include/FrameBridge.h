#include <vector>
#include <map>

namespace ix {

class FrameLink
{
 public:
    FrameLink( int _a, int _b, float _power ) : a(_a), b(_b), power(_power) {};

    int a;
    int b; 
    float power;
};

bool compareLink( const FrameLink & a, const FrameLink & b );

class FramePriority
{
 public:
    FramePriority( std::vector<FrameLink> priority );

    std::vector<int> ordering;
    std::map<int, FrameLink> links;
};

template <class T>
class FrameBridge
{
 public:
    FrameBridge() {};
    std::vector<int> bridge( std::vector<T> a, std::vector<T> b );

    std::map<int, std::vector<FramePriority> > powermatrix;
    std::vector<FrameLink> unmatchedPre;
    std::vector<FrameLink> unmatchedPost;
};

}
