#pragma once

#include <chrono>
#include <cstdint>
#include <sys/resource.h>

class Resources
{

public:
    Resources() : _cpuLoad(0), _mtimePrev(), _utimePrev(0), _stimePrev(0)
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
        std::chrono::nanoseconds utime = toDuration(r.ru_utime);
        std::chrono::nanoseconds stime = toDuration(r.ru_stime);

        if (mtime == _mtimePrev) {
            _cpuLoad = 1.0;
        } else {
            _cpuLoad = toDouble((utime - _utimePrev) + (stime - _stimePrev)) /
                       toDouble(mtime - _mtimePrev);
        }

        _mtimePrev = mtime;
        _utimePrev = utime;
        _stimePrev = stime;

        return true;
    }

    double cpuLoad() const
    {
        return _cpuLoad;
    }

private:
    std::chrono::nanoseconds toDuration(const struct timeval &tv)
    {
        return std::chrono::nanoseconds(tv.tv_sec * 1000000000ull + tv.tv_usec * 1000ull);
    }

    double toDouble(std::chrono::nanoseconds d)
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(d).count();
    }

    double _cpuLoad;

    // time of last measurement
    std::chrono::steady_clock::time_point _mtimePrev;

    // elapsed user and system times as of last measurement
    std::chrono::nanoseconds _utimePrev;
    std::chrono::nanoseconds _stimePrev;
};
