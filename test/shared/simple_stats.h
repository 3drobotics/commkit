#pragma once

#include <cmath>
#include <limits>

// accumulate samples and provide min, max, avg, and stdev
template <typename T>
class SimpleStats
{
public:
    SimpleStats()
    {
        reset();
    }

    virtual ~SimpleStats()
    {
        reset();
    }

    void reset()
    {
        _count = 0;
        _sum = 0;
        _sumSquares = 0;
        _min = std::numeric_limits<T>::max();
        _max = std::numeric_limits<T>::min();
    }

    void accumulate(T x)
    {
        _count++;
        _sum += x;
        _sumSquares += (x * x);
        if (_min > x)
            _min = x;
        if (_max < x)
            _max = x;
    }

    unsigned count()
    {
        return _count;
    }

    T min()
    {
        return _min;
    }

    T max()
    {
        return _max;
    }

    double average()
    {
        if (_count == 0)
            return 0;
        return _sum / _count;
    }

    double stdev()
    {
        if (_count == 0)
            return 0;
        double avg = average();
        return std::sqrt(double(_sumSquares) / double(_count) - avg * avg);
    }

private:
    unsigned _count;
    T _sum;
    T _sumSquares;
    T _min;
    T _max;
};
