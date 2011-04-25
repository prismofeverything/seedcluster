#pragma once

class Ease
{
 public:
    Ease() : now(0), duration(0), from(0), to(0) {};
    Ease( float _from, float _to, int _duration );
    bool done();
    float in();
    float out();
    void realign( float _from, float _to, int _duration );

    int now;
    int duration;
    float from;
    float to;
};

