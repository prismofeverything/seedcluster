#include <math.h>
#include "Ease.h"

Ease::Ease( float _from, float _to, int _duration )
    : now(0),
      duration(_duration),
      from(_from),
      to(_to)
{
    
}

float Ease::step()
{
    float slice = ( now == 0 ) ? from : ( to - from ) * pow( 2, 10 * (((float) now) / duration - 1) ) + from;
    now++;
    return slice;
}

bool Ease::done()
{
    return now >= duration;
}
