#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sys/time.h>
#include <thread>
#include <utilityPivot.h>

using namespace std;

TEST(TestUtilityPivot, convertTimestamp) 
{
    struct timeval timestamp;
    gettimeofday(&timestamp, NULL);
    long tsMs = timestamp.tv_sec * 1000;

    int testTime = 3000;

    for (int i = 0; i <= testTime ; i++ ) {
        long ts1 = tsMs + i ;
        std::pair<long, long> p = UtilityPivot::fromTimestamp(ts1);
        long ts2 = UtilityPivot::toTimestamp(p.first, p.second);

        ASSERT_EQ(ts1, ts2);
    }
}

TEST(TestUtilityPivot, getTimestamp) 
{
    long t1 = (long)(UtilityPivot::getCurrentTimestampMs() / 1000L);

    // sleep
    this_thread::sleep_for(chrono::milliseconds(1000));

    long t2 = (long)(UtilityPivot::getCurrentTimestampMs() / 1000L);

    ASSERT_EQ(t1 + 1, t2);   
}