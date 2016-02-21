#pragma once

#include <stdint.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#if defined(__linux__)
#include <time.h>
#endif

#include <fpplus.h>

/**
 * @brief Returns time in nanoseconds
 */
FPPLUS_STATIC_INLINE double high_precision_time() {
#if defined(__APPLE__) && defined(__MACH__)
    uint64_t ticks = mach_absolute_time();
    mach_timebase_info_data_t timebase_info;
    mach_timebase_info(&timebase_info);
    return ((double) ticks) * (((double) timebase_info.numer) / ((double) timebase_info.denom));
#elif defined(__linux__)
    struct timespec timespec;
    clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);
    return (double) (timespec.tv_sec * 1000000000ll + timespec.tv_nsec);
#else
    #error Not implemented
#endif
}

/**
 * @brief Returns the CPU timestamp counter value
 */
FPPLUS_STATIC_INLINE uint64_t cpu_ticks() {
    register uint32_t counter_lo, counter_hi;
#ifndef __KNC__
    asm volatile (
        "RDTSCP;"
        : "=a" (counter_lo), "=d" (counter_hi)
        :
        : "rcx"
    );
#else
    asm volatile (
        "RDTSC;"
        : "=a" (counter_lo), "=d" (counter_hi)
        :
        : "rcx"
    );
#endif
    return (((uint64_t) counter_hi) << 32) | counter_lo;
}

double median_double(double array[], size_t length);
uint64_t median_uint64(uint64_t array[], size_t length);
