#pragma once

#include <chrono>
#include <cstdint>
#include <sys/resource.h>

class Resources
{

public:
    Resources() : _cpu_load(0), _mtime_prev(), _utime_prev(0), _stime_prev(0)
    {
        sample();
    }

    bool sample()
    {
        struct rusage r;
        if (getrusage(RUSAGE_SELF, &r) != 0) {
            return false;
        }
        std::chrono::steady_clock::time_point mtime = std::chrono::steady_clock::now();
        std::chrono::nanoseconds utime = to_duration(r.ru_utime);
        std::chrono::nanoseconds stime = to_duration(r.ru_stime);

        if (mtime == _mtime_prev) {
            _cpu_load = 1.0;
        } else {
            _cpu_load = to_double((utime - _utime_prev) + (stime - _stime_prev)) /
                        to_double(mtime - _mtime_prev);
        }

        _mtime_prev = mtime;
        _utime_prev = utime;
        _stime_prev = stime;

        return true;
    }

    double cpu_load() const
    {
        return _cpu_load;
    }

private:
    std::chrono::nanoseconds to_duration(const struct timeval &tv)
    {
        return std::chrono::nanoseconds(tv.tv_sec * 1000000000ull + tv.tv_usec * 1000ull);
    }

    double to_double(std::chrono::nanoseconds d)
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(d).count();
    }

    double _cpu_load;

    // time of last measurement
    std::chrono::steady_clock::time_point _mtime_prev;

    // elapsed user and system times as of last measurement
    std::chrono::nanoseconds _utime_prev;
    std::chrono::nanoseconds _stime_prev;
};
