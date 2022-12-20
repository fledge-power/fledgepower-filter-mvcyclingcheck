/*
 * Utility.
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Yannick Marchetaux
 * 
 */
#include <cmath>
#include <sys/time.h>
#include <utilityPivot.h>

/**
 * Convert secondSinceEpoch and secondSinceEpoch to timestamp
 * @param secondSinceEpoch : interval in seconds continuously counted from the epoch 1970-01-01 00:00:00 UTC
 * @param fractionOfSecond : represents the fraction of the current second when the value of the TimeStamp has been determined.
 * @return timestamp (ms)
*/
long UtilityPivot::toTimestamp(long secondSinceEpoch, long fractionOfSecond) {
    long timestamp = 0;
    long msPart = round((double)(fractionOfSecond * 1000) / 16777216.0);
    timestamp = (secondSinceEpoch * 1000L) + msPart;
    return timestamp;
}

/**
 * Convert timestamp (ms) in pair of secondSinceEpoch and fractionOfSecond
 * @param timestamp : timestamp (ms) 
 * @return pair of secondSinceEpoch and fractionOfSecond
*/
std::pair<long, long> UtilityPivot::fromTimestamp(long timestamp) {
    long remainder = (timestamp % 1000L);
    long fractionOfSecond = remainder * 16777 + ((remainder * 216) / 1000);
    return std::make_pair(timestamp / 1000L, fractionOfSecond);
}

/**
 * Get current timestamp in milisecond
 * @return timestamp in ms
*/
long UtilityPivot::getCurrentTimestampMs() {
    struct timeval timestamp;
    gettimeofday(&timestamp, nullptr);
    return timestamp.tv_sec * 1000 + (timestamp.tv_usec / 1000L);
}