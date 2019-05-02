#ifndef __res__h__ 
#define __res__h__

namespace mad {

constexpr static const int& N = 50;

struct Res {
    int minimum_duration;
    int workers;
    int critical_workers;
    int min_workers;
    int start_dates[N];
};

}; // namespace mad;

#endif
