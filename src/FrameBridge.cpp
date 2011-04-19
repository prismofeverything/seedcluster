#include <vector>
#include <map>
#include "FrameBridge.h"

namespace ix {

bool compareLink( const FrameLink & a, const FrameLink & b) {
    return a.power < b.power;
}

FramePriority::FramePriority( std::vector<FrameLink> priority )
{
    sort( priority.begin(), priority.end(), compareLink );
    for ( std::vector<FrameLink>::iterator it = priority.begin(); it < priority.end(); it++ ) {
        ordering.push_back[it->b];
        links[it->b] = *it;
    }
}

template <class T>
std::vector<int> FrameBridge<T>::bridge( std::vector<T> a, std::vector<T> b ) 
{

}

}
