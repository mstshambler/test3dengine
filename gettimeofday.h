#ifdef WIN32
struct timeval {
    long tv_sec;
    long tv_usec;
};

int gettimeofday(struct timeval *tp, void *tzp);
#endif

void getdifftime(struct timeval t1, struct timeval t2, struct timeval *diff);
void getaddtime(struct timeval t1, struct timeval t2, struct timeval *res);

extern struct timeval tick_otime, tick_ntime, tick_diff; // last tick, current tick, diff between ticks