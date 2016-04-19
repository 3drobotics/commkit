#pragma once

#include <cstdint>
#include <sys/resource.h>

#include "time_util.h"

class Resources
{

public:
    Resources() : _cpu_load(0), _mtime_prev_us(0), _utime_prev_us(0), _stime_prev_us(0)
    {
        sample();
    }

    bool sample()
    {
        struct rusage r;
        if (getrusage(RUSAGE_SELF, &r) != 0) {
            return false;
        }
        std::uint64_t mtime_us = clock_gettime_ns(CLOCK_MONOTONIC) / 1000;
        std::uint64_t utime_us = to_us(r.ru_utime);
        std::uint64_t stime_us = to_us(r.ru_stime);

        if (mtime_us == _mtime_prev_us) {
            _cpu_load = 1.0;
        } else {
            _cpu_load = (double)((utime_us - _utime_prev_us) + (stime_us - _stime_prev_us)) /
                        (double)(mtime_us - _mtime_prev_us);
        }

        _mtime_prev_us = mtime_us;
        _utime_prev_us = utime_us;
        _stime_prev_us = stime_us;

        return true;
    }

    double cpu_load() const
    {
        return _cpu_load;
    }

private:
    std::uint64_t to_us(const struct timeval &tv)
    {
        return tv.tv_sec * 1000000ULL + tv.tv_usec;
    }

    double _cpu_load;

    // CLOCK_MONOTONIC of last measurement
    std::uint64_t _mtime_prev_us;

    // user and system times as of last measurement
    std::uint64_t _utime_prev_us;
    std::uint64_t _stime_prev_us;
};
