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

struct FramePathCompare : public std::binary_function<FramePath, FramePath, bool>
{
    FramePathCompare( int _target ) : target( _target ) {};
    inline bool operator() ( const FramePath & a, const FramePath & b ) {
        return a.links.find( target )->second.power < b.links.find( target )->second.power;
    }

    int target;
};

template <class T, class Distance>
class FrameBridge
{
 public:
    FrameBridge() {};
    std::vector<FrameLink> bridge( std::vector<T> a, std::vector<T> b, Distance distance );

    std::map<int, std::vector<FramePath> > powermatrix;
    std::vector<int> aUnmatched;
    std::vector<int> bUnmatched;
};

template <class T, class Distance>
std::vector<FrameLink> FrameBridge<T, Distance>::bridge( std::vector<T> a, std::vector<T> b, Distance distance )
{
    powermatrix.clear();
    aUnmatched.clear();
    bUnmatched.clear();

    if ( b.empty() ) {
        for ( int aun = 0; aun < a.size(); aun++ ) {
            aUnmatched.push_back( aun );
        }
        return std::vector<FrameLink>();
    } else {
        int ax = 0;
        int bx = 0;
        for ( ax = 0; ax < a.size(); ax++ ) { 
            std::vector<FrameLink> links;
            for ( bx = 0; bx < b.size(); bx++ ) {
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
                    int alone = bUnmatched.front();
                    std::vector<FramePath>::iterator affine = std::min_element( powermatrix[ *mit ].begin(), 
                                                                                powermatrix[ *mit ].end(), 
                                                                                FramePathCompare( alone ) );
                    FramePath path(*affine);
                    powermatrix[ *mit ].erase( affine );
                    path.reassign( alone );
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
}

} // namespace ix
