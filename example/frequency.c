
#include <stdint.h>

#ifdef _WIN32

#include <windows.h>
#include <profileapi.h>

typedef struct NVGtimer {
    uint64_t frequency;
    uint64_t offset;
} NVGtimer;


static NVGtimer _nvgTimer;


static void _nvgInitTimer(void)
{
    QueryPerformanceFrequency((LARGE_INTEGER*) &_nvgTimer.frequency);
}

static uint64_t _nvgGetTimerValue(void)
{
    uint64_t value;
    QueryPerformanceCounter((LARGE_INTEGER*) &value);
    return value;
}
#elif defined(__APPLE__)

#include <mach/mach_time.h>

typedef struct NVGtimer {
    uint64_t frequency;
    uint64_t offset;
} NVGtimer;

static NVGtimer _nvgTimer;

static void _nvgInitTimer(void)
{
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    _nvgTimer.frequency = (info.denom * 1e9) / info.numer;
}

static uint64_t _nvgGetTimerValue(void)
{
    return mach_absolute_time();
}

#else
#include <unistd.h>
#include <sys/time.h>

typedef struct NVGtimer {
    uint64_t frequency;
    uint64_t clock;
    uint64_t offset;
} NVGtimer;


static NVGtimer _nvgTimer;

static void _nvgInitTimer()
{
    _nvgTimer.clock = CLOCK_REALTIME;
    _nvgTimer.frequency = 1000000000;
#if defined(_POSIX_MONOTONIC_CLOCK)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
        _nvgTimer.clock = CLOCK_MONOTONIC;
#endif
}

static uint64_t _nvgGetTimerValue()
{
    struct timespec ts;
    clock_gettime(_nvgTimer.clock, &ts);
    return (uint64_t) ts.tv_sec * _nvgTimer.frequency + (uint64_t) ts.tv_nsec;
}

#endif

void nvgFrequencyInitTimer() {
    _nvgInitTimer();
    _nvgTimer.offset = _nvgGetTimerValue();
}

double nvgFrequencyGetTime()
{
    return (double) (_nvgGetTimerValue() - _nvgTimer.offset) / _nvgTimer.frequency;
}
