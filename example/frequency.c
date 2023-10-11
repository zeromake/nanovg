
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

static uint64_t _nvgGetTimerFrequency(void)
{
    return _nvgTimer.frequency;
}

#else
#include <unistd.h>
#include <sys/time.h>

typedef struct NVGtimer {
    uint64_t frequency;
    uint64_t clock;
    uint64_t offset;
} NVGtimer;


static const NVGtimer _nvgTimer;

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

static uint64_t _nvgGetTimerFrequency()
{
    return _nvgTimer.frequency;
}

#endif


void nvgFrequencyInitTimer() {
    _nvgInitTimer();
    _nvgTimer.offset = _nvgGetTimerValue();
}

double nvgFrequencyGetTime()
{
    return (double) (_nvgGetTimerValue() - _nvgTimer.offset) / _nvgGetTimerFrequency();
}
