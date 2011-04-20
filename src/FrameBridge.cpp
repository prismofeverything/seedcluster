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

template <class T>
std::vector<FrameLink> FrameBridge<T>::bridge( std::vector<T> a, std::vector<T> b, std::binary_function<T, T, float> distance ) 
{
    int ax = 0;
    int bx = 0;
    for ( ax = 0; ax < a.size(); a++ ) { // std::vector<T>::iterator at = a.begin(); at < a.end(); at++ ) {
        std::vector<FrameLink> links;
        for ( bx = 0; bx < b.size(); b++ ) { // std::vector<T>::iterator bt = b.begin(); bt < b.end(); bt++ ) {
            FrameLink link( ax, bx, distance( a[ ax ], b[ bx ] ) );
            links.push_back( link );
        }

        FramePath path( links );
        int target = path.target();
        powermatrix[ target ].push_back( path );
    }

    std::vector<int> matched;
    for ( bx = 0; bx < b.size(); bx++ ) {
        if ( powermatrix.find( bx ) == powermatrix.end() ) {
            bUnmatched.push_back( bx );
        } else {
            matched.push_back( bx );
        }
    }

    for ( std::vector<int>::iterator mit = matched.begin(); mit < matched.end(); mit++ ) {
        while ( powermatrix[ *mit ].size() > 1 ) {
            if ( !bUnmatched.empty() ) {
                std::vector<int>::iterator alone = bUnmatched.front();
                std::vector<FramePath>::iterator affine = std::min_element( powermatrix[ *mit ].begin(), powermatrix[ *mit ].end(), FramePathCompare( *alone ) );
                
                FramePath path(*affine);
                powermatrix[ *mit ].erase( affine );
                path.reassign( *alone );
                bUnmatched.erase( bUnmatched.begin() );
            } else {
                std::vector<FramePath>::iterator weakest = std::max_element( powermatrix[ *mit ].begin(), powermatrix[ *mit ].end(), FramePathCompare( *mit ) );
                int unm = weakest->target();
                powermatrix[ *mit ].erase( weakest );
                aUnmatched.push_back( unm );
            }
        }
    }

    std::vector<FrameLink> links;
    for ( std::map<int, std::vector<FramePath> >::iterator it = powermatrix.begin();
          it != powermatrix.end(); it++ ) {
        int target = it->second.front().target();
        links.push_back( it->second.front().links[ target ] );
    }

    return links;
}

} // namespace ix
