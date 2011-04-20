#include <vector>
#include <map>
#include <functional>

namespace ix {

class FrameLink
{
 public:
    FrameLink() {};
    FrameLink( int _a, int _b, float _power ) : a(_a), b(_b), power(_power) {};

    int a;
    int b; 
    float power;
};

bool compareLink( const FrameLink & a, const FrameLink & b );

class FramePath
{
 public:
    FramePath( std::vector<FrameLink> path );
    int target();
    void reassign( int newTarget );

    std::vector<int> ordering;
    std::map<int, FrameLink> links;
};

class FramePathCompare : public std::binary_function<FramePath, FramePath, bool>
{
 public:
    FramePathCompare( int _target ) : target( _target ) {};
    inline bool operator() ( const FramePath & a, const FramePath & b ) {
        return a.links.find( target )->second.power < b.links.find( target )->second.power;
    }

    int target;
};

template <class T>
class FrameBridge
{
 public:
    FrameBridge() {};
    std::vector<FrameLink> bridge( std::vector<T> a, std::vector<T> b, std::binary_function<T, T, float> distance );

    std::map<int, std::vector<FramePath> > powermatrix;
    std::vector<int> aUnmatched;
    std::vector<int> bUnmatched;
};

} // namespace ix
