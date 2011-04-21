#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include "FrameBridge.h"

namespace ix {

bool compareLink( const FrameLink & a, const FrameLink & b ) {
    return a.power < b.power;
}

FramePath::FramePath( std::vector<FrameLink> path )
{
    sort( path.begin(), path.end(), compareLink );
    for ( std::vector<FrameLink>::iterator it = path.begin(); it < path.end(); it++ ) {
        ordering.push_back(it->b);
        links[it->b] = *it;
    }
}

int FramePath::target()
{
    return ordering[0];
}

void FramePath::reassign( int newTarget )
{
    std::vector<int>::iterator found = std::find( ordering.begin(), ordering.end(), newTarget );
    std::iter_swap( ordering.begin(), found );
}

} // namespace ix
