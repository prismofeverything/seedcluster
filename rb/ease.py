from pylab import *

class Ease:
    def __init__(self, a, b, dur):
        self.a = a
        self.b = b
        self.dur = dur
        self.now = 0

    def done(self):
        return self.now >= self.dur

    def easeIn(self):
        slice = self.a
        if self.now > 0:
            slice = (self.b - self.a) * pow(2, 10 * ((float(self.now) / self.dur) - 1)) + self.a
        self.now += 1
        return slice

    def easeOut(self):
        slice = self.a
        if self.now > 0:
            slice = (self.b - self.a) * (-pow(2, -10 * (float(self.now) / self.dur)) + 1) + self.a
        self.now += 1
        return slice

    def plot(self):
        series = array(map(lambda x: self.easeIn(), range(self.dur)))
        plot(series)
            
