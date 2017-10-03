#ifdef WIN32
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#include "gettimeofday.h"

#ifdef WIN32
// emulation of *nix gettimeofday
int gettimeofday(struct timeval *tp, void *tzp) {
    struct _timeb timebuffer;
    
    _ftime(&timebuffer);
    tp->tv_sec = (long)timebuffer.time;
    tp->tv_usec = timebuffer.millitm * 1000;
    
    return 0;
}
#endif

struct timeval tick_otime, tick_ntime, tick_diff; // last tick, current tick, diff between ticks

// calculating difference between two timeval structures
void getdifftime(struct timeval t1, struct timeval t2, struct timeval *diff) {
	diff->tv_sec = t1.tv_sec - t2.tv_sec;
	diff->tv_usec = t1.tv_usec - t2.tv_usec;

	if (diff->tv_usec < 0) {
		diff->tv_sec--;
		diff->tv_usec += 1000000;
	}
	if (diff->tv_sec < 0)
		diff->tv_sec += 60*60*24;
}

// Calculates sum of two times T1 and T2
void getaddtime(struct timeval t1, struct timeval t2, struct timeval *res) {
	res->tv_sec = t1.tv_sec + t2.tv_sec;
	res->tv_usec = t1.tv_usec + t2.tv_usec;

	if (res->tv_usec >= 1000000) {
		res->tv_sec++;
		res->tv_usec -= 1000000;
	}
	if (res->tv_sec >= 60*60*24)
		res->tv_sec -= 60*60*24;
}
