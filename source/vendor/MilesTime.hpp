#ifndef SOURCE_VENDOR_MILES_TIME_HPP
#define SOURCE_VENDOR_MILES_TIME_HPP

extern "C" unsigned long long rrGetTime();
extern "C" unsigned long long rrTimeToMillis(unsigned long long uTime);
extern "C" unsigned long long rrTimeToMicros(unsigned long long uTime);
extern "C" unsigned long long rrMillisToTime(unsigned long long uMilliseconds);
extern "C" unsigned long long rrMicrosToTime(unsigned long long uMicroseconds);

#endif
