#include <math.h>
#include "Ease.h"

Ease::Ease( float _from, float _to, int _duration )
    : now(0),
      duration(_duration),
      from(_from),
      to(_to)
{
    
}

void Ease::realign( float _from, float _to, int _duration )
{
    now = 0;
    from = _from;
    to = _to;
    duration = _duration;
}

float Ease::in()
{
    float slice = ( now == 0 ) ? from : ( to - from ) * pow( 2, 10 * (((float) now) / duration - 1) ) + from;
    now++;
    return slice;
}

float Ease::out()
{
    float slice = ( now == 0 ) ? from : ( to - from ) * ( -pow( 2, -10 * ((float) now) / duration ) + 1 ) + from;
    now++;
    return slice;
}

bool Ease::done()
{
    return now >= duration;
}
