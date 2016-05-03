#include <vector>
#include "gtest/gtest.h"
#include "../src/chronoimpl.h"

using namespace std::chrono;
using namespace eprosima::fastrtps::rtps;

// Time_t helpers

// Not all of these are actually used (in the end), but it seems like a full
// set somewhere would be useful

// Time_t "tick" is the smallest increment, = 1/2^32 seconds (~0.233 nsec)
inline int64_t toTicks(const Time_t &t)
{
    int64_t ticks = t.seconds;
    ticks = (ticks << 32) | t.fraction;
    return ticks;
}

inline Time_t toTime(int64_t ticks)
{
    return Time_t(ticks >> 32, ticks);
}

// Note: A Time_t is used as both a point-in-time and a delta-time
// Normally, one would only add a point-in-time and a delta-time; it does not
// really make sense to add point-in-time + point-in-time

inline Time_t operator+(const Time_t &lhs, const Time_t &rhs)
{
    return toTime(toTicks(lhs) + toTicks(rhs));
}

inline Time_t operator-(const Time_t &lhs, const Time_t &rhs)
{
    return toTime(toTicks(lhs) - toTicks(rhs));
}

inline Time_t operator+(const Time_t &lhs, int64_t ticks)
{
    return toTime(toTicks(lhs) + ticks);
}

inline Time_t &operator++(Time_t &t1)
{
    if (t1.fraction == UINT32_MAX)
        t1.seconds++;
    t1.fraction++;
    return t1;
}

// ++time_point - it is not clear that this is useful outside testing
inline commkit::clock::time_point &operator++(commkit::clock::time_point &t1)
{
    t1 += commkit::clock::duration(1);
    return t1;
}

// Time Tests
//
// Exhaustive tests are prohibitive; instead, test at "interesting" times:
// near zero, one sec, 10 min, 30 min, now, where the absolute ones are
// intended to cover monotonic times for a system up that long, and "now" is
// ~45 years as a sanity check, or in case this is changed to system clock
// some day.
//
// time_point -> Time_t -> time_point -> Time_t
//   * time_points identical; convertion to Time_t and back should be lossless
//     if rounding is correctly implemented
//   * Time_t's identical - the point here is if we start with a time_point
//     (which we usually will) we should be able to convert to Time_t and back
//     without losing information or getting mysterious compare errors
//
// Time_t -> time_point -> Time_t:
//   * Result should be max 2 ticks off. Converting from Time_t to time_point
//     can have a max error of 0.5 time_point ticks. Converting from
//     time_point to Time_t can have a max error of 0.5 Time_t ticks. Summing
//     those in units of Time_t, the max error is
//         (0.5 tp)(2^32 t/10^9 tp) + 0.5 t = ~2.65 t
//     It is not obvious (to me) whether that means we might get errors of 3
//     ticks; in testing, it appears 2 ticks is the worst, but in the code we
//     call it an error at more than 3.
//
// Running with t_delta and tp_delta set to ~1M ticks, the test takes < 1s on
// a 2015 macbook pro.

// Invalid conversions
//
// invalid in should always yield invalid out
TEST(ChronoimplTest, Invalid)
{
    EXPECT_EQ(commkit::NSEC_INVALID, commkit::toInt64(c_TimeInvalid));
    EXPECT_EQ(commkit::NSEC_INVALID, commkit::toInt64(commkit::TIME_POINT_INVALID));
    EXPECT_EQ(commkit::TIME_POINT_INVALID, commkit::toTimePoint(commkit::NSEC_INVALID));
    EXPECT_EQ(commkit::TIME_POINT_INVALID, commkit::toTimePoint(c_TimeInvalid));
    EXPECT_EQ(c_TimeInvalid, commkit::toRtpsTime(commkit::NSEC_INVALID));
    EXPECT_EQ(c_TimeInvalid, commkit::toRtpsTime(commkit::TIME_POINT_INVALID));
}

// Constant time values, about which we will run tests, both Time_t and time_point

const Time_t t_zero(0, 0);
const Time_t t_1_s(1, 0);
const Time_t t_1_m(60, 0);
const Time_t t_10_m(10 * 60, 0);
const Time_t t_30_m(30 * 60, 0);
const Time_t t_45_y(45 * 365 * 24 * 60 * 60, 0);
const Time_t t_delta(0, 0x100000000ull / 4000); // 0.25 msec is ~1M ticks

const commkit::clock::time_point tp_zero(commkit::clock::duration::zero());
const commkit::clock::time_point tp_1_s(seconds(1));
const commkit::clock::time_point tp_1_m(minutes(1));
const commkit::clock::time_point tp_10_m(minutes(10));
const commkit::clock::time_point tp_30_m(minutes(30));
const commkit::clock::time_point tp_45_y(hours(45 * 365 * 24));
const commkit::clock::duration tp_delta = milliseconds(1); // 1 msec is 1M ticks

// Lossless conversions
//
// Do:   time_point_1 -> Time_t_1 -> time_point_2 -> Time_t_2
// Then: time_point_2 should always equal time_point_1
// And:  Time_2_2 should always equal Time_t_1
TEST(ChronoimplTest, Lossless)
{
    // zero...delta
    for (commkit::clock::time_point tp1 = tp_zero; tp1 < (tp_zero + tp_delta); ++tp1) {
        Time_t t1 = commkit::toRtpsTime(tp1);
        commkit::clock::time_point tp2 = commkit::toTimePoint(t1);
        EXPECT_EQ(tp2, tp1);
        Time_t t2 = commkit::toRtpsTime(tp2);
        EXPECT_EQ(t2, t1);
    }
    // time-delta...time+delta
    std::vector<commkit::clock::time_point> starts = {tp_1_s, tp_1_m, tp_10_m, tp_30_m, tp_45_y};
    for (auto it = starts.begin(); it != starts.end(); it++) {
        for (commkit::clock::time_point tp1 = *it - tp_delta; tp1 < (*it + tp_delta); ++tp1) {
            Time_t t1 = commkit::toRtpsTime(tp1);
            commkit::clock::time_point tp2 = commkit::toTimePoint(t1);
            EXPECT_EQ(tp2, tp1);
            Time_t t2 = commkit::toRtpsTime(tp2);
            EXPECT_EQ(t2, t1);
        }
    }
}

// Lossy conversions
//
// Do:   Time_t_1 -> time_point_1 -> Time_t_2
// Then: Time_t_2 - Time_t_1 should be 0, 1, or 2 ticks
TEST(ChronoimplTest, Lossy)
{
    // histogram error counts to make sure we see some of each, i.e. if we run
    // this test and all errors are zero, something is wrong
    std::vector<uint64_t> hist = {0, 0, 0, 0, 0};
    // test interval zero...delta
    for (Time_t t1 = t_zero; t1 < (t_zero + t_delta); ++t1) {
        commkit::clock::time_point tp1 = commkit::toTimePoint(t1);
        Time_t t2 = commkit::toRtpsTime(tp1);
        int err = std::abs(toTicks(t2 - t1));
        ASSERT_LE(err, hist.size());
        EXPECT_LE(err, 3);
        hist[err]++;
    }
    // Make sure we have a reasonable distribution in hist[]. The idea here is
    // not to figure out exactly what it should be, but to make sure we don't
    // have something like "all 0".
    uint64_t sum = hist[0] + hist[1] + hist[2];
    EXPECT_GE(100 * hist[0] / sum, 10); // at least 10%
    EXPECT_GE(100 * hist[1] / sum, 10);
    EXPECT_GE(100 * hist[2] / sum, 10);
    // clear histotram
    hist[0] = hist[1] = hist[2] = 0;
    // test intervals time-delta...time+delta
    std::vector<Time_t> starts = {t_1_s, t_1_m, t_10_m, t_30_m, t_45_y};
    for (auto it = starts.begin(); it != starts.end(); it++) {
        for (Time_t t1 = *it - t_delta; t1 < (*it + t_delta); ++t1) {
            commkit::clock::time_point tp1 = commkit::toTimePoint(t1);
            Time_t t2 = commkit::toRtpsTime(tp1);
            int err = std::abs(toTicks(t2 - t1));
            ASSERT_LE(err, hist.size());
            EXPECT_LE(err, 3);
            hist[err]++;
        }
        // check histogram
        uint64_t sum = hist[0] + hist[1] + hist[2];
        EXPECT_GE(100 * hist[0] / sum, 10);
        EXPECT_GE(100 * hist[1] / sum, 10);
        EXPECT_GE(100 * hist[2] / sum, 10);
        // clear histotram
        hist[0] = hist[1] = hist[2] = 0;
    }
}

TEST(ChronoimplTest, Duration)
{
    Duration_t d;

    d = commkit::toRtpsDuration(std::chrono::milliseconds(1));
    EXPECT_EQ(d.seconds, 0);
    EXPECT_EQ(d.fraction, (uint64_t(1) << 32) / 1000);

    d = commkit::toRtpsDuration(std::chrono::milliseconds(1000));
    EXPECT_EQ(d.seconds, 1);
    EXPECT_EQ(d.fraction, 0);

    d = commkit::toRtpsDuration(std::chrono::microseconds(52));
    EXPECT_EQ(d.seconds, 0);
    EXPECT_EQ(d.fraction, (uint64_t(1) << 32) * 52 / 1000000);

    d = commkit::toRtpsDuration(std::chrono::nanoseconds(17));
    EXPECT_EQ(d.seconds, 0);
    EXPECT_EQ(d.fraction, (uint64_t(1) << 32) *17 / 1000000000);

    d = commkit::toRtpsDuration(std::chrono::minutes(2));
    EXPECT_EQ(d.seconds, 120);
    EXPECT_EQ(d.fraction, 0);
}
