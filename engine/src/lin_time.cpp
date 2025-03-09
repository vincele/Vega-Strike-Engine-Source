/*
 * lin_time.cpp
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "vegastrike.h"
#include "in_kb.h"
#include "vs_random.h"
#include "vs_logging.h"

static double firsttime;
static double newtime;
static double lasttime;
static double elapsedtime = .1;
static double timecompression = 1;

int timecount;

VSRandom vsrandom(time(NULL));

#if !defined WIN32 && !defined _POSIX_MONOTONIC_CLOCK && !defined HAVE_GETTIMEOFDAY && !defined HAVE_SDL
# error "We have no way to determine the time on this system."
#endif

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif //tells VCC not to generate min/max macros
#include <windows.h>
static LARGE_INTEGER li_ttime{};
static LARGE_INTEGER li_newtime{};
static LARGE_INTEGER li_freq{};
#else // WIN32
#if defined (HAVE_SDL)
#   include <SDL2/SDL.h>
#endif /* defined( HAVE_SDL ) */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif // WIN32

double getNewTime() {
    return newtime - firsttime;
}

void inc_time_compression(const KBData &, KBSTATE a) {
    if (a == PRESS) {
        timecompression *= 1.5;
        timecount++;
    }
}

void dec_time_compression(const KBData &, KBSTATE a) {
    if (a == PRESS) {
        timecompression /= 1.5;
        timecount--;
    }
}

void reset_time_compression(const KBData &, KBSTATE a) {
    if (a == PRESS) {
        timecompression = 1;
        timecount = 0;
    }
}

float getTimeCompression() {
    return timecompression;
}

void setTimeCompression(float tc) {
    timecompression = tc;
    timecount = 0;     //to avoid any problems with time compression sounds... use getTimeCompression() instead
}

bool toggle_pause() {
    static bool paused = false;
    if (paused) {
        VS_LOG(debug, "toggle_pause() in lin_time.cpp: Resuming (unpausing)");
        setTimeCompression(1);
        paused = false;
    } else {
        VS_LOG(debug, "toggle_pause() in lin_time.cpp: Pausing");

        // If you make this value too small, then when the user presses the
        // Pause key again to resume, the game will take too long to respond.
        // It will effectively stay frozen.
        setTimeCompression(.00001);
        paused = true;
    }
    return paused;
}

#ifdef _WIN32

void micro_sleep( unsigned int n )
{
    Sleep( n/1000 );
}

#elif defined (IRIX)

void micro_sleep( unsigned int n )
{
    usleep(static_cast<useconds_t>(n));
}

#elif defined (__APPLE__) && defined (__MACH__)

void micro_sleep(unsigned int n) {
    usleep(static_cast<useconds_t>(n));
}

#else

void micro_sleep(unsigned int n) {
    struct timeval tv = {
            0, 0
    };

    tv.tv_usec = n % 1000000;
    tv.tv_sec = n / 1000000;
    select(0, NULL, NULL, NULL, &tv);
}

#endif

#ifdef _WIN32

static double get_time() {
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    double time = static_cast<double>(ticks.QuadPart);
    if (li_freq.QuadPart > 0) {
        time /= static_cast<double>(li_freq.QuadPart);
    }
    return time;
}

#else // _WIN32

static double get_time() {
    double time;
#if defined (_POSIX_MONOTONIC_CLOCK)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    time = static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) * 1.e-9;
    // VS_LOG(trace, (boost::format("lin_time.cpp: get_time(): lasttime is %1%; newtime is %2%; elapsedtime before time compression is %3%") % lasttime % newtime % elapsedtime));
#elif defined (HAVE_GETTIMEOFDAY)
    struct timeval tv;
    (void) gettimeofday(&tv, NULL);
    time = (double) tv.tv_sec + (double) tv.tv_usec * 1.e-6;
#elif defined (HAVE_SDL)
    time = SDL_GetTicks() * 1.e-3;
#endif
    return time;
}

#endif // _WIN32

void InitTime() {
    VS_LOG(trace, "InitTime() called");
#ifdef WIN32
    QueryPerformanceFrequency(&li_freq);
    if (li_freq.QuadPart == 0) {
        VS_LOG(serious_warning, "InitTime(): li_freq is zero!");
    }
    QueryPerformanceCounter(&li_ttime);
    if (freq.QuadPart == 0) {
        newtime = static_cast<double>(li_ttime.QuadPart);
    } else {
        newtime = static_cast<double>(li_ttime.QuadPart) / static_cast<double>(li_freq.QuadPart);
    }
#else
    newtime  = get_time();
#endif
    lasttime = newtime - .0001;
    elapsedtime = .0001;
}

double GetElapsedTime() {
    return elapsedtime;
}

double queryTime() {
    return get_time() - firsttime;
}

double realTime() {
    double tmpnewtime = get_time();
#ifdef WIN32
    if (tmpnewtime == INFINITY) {
        tmpnewtime = 0;
    }
#endif

    static double reallyfirsttime = tmpnewtime;
    return tmpnewtime - reallyfirsttime;
}

void UpdateTime() {
    static bool first = true;
    lasttime = newtime;
    newtime = get_time();
    elapsedtime = newtime - lasttime;
#ifdef WIN32
    li_ttime = li_newtime;
#endif
    if (first)
        firsttime = newtime;
    elapsedtime *= timecompression;
    // VS_LOG(trace, (boost::format("lin_time.cpp: UpdateTime():                                  elapsedtime after  time compression is %1%") % elapsedtime));
    first = false;
}

void setNewTime(double newnewtime) {
    firsttime -= newnewtime - queryTime();
    UpdateTime();
}

