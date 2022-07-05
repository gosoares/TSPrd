#ifndef TSPRD_TIMER_H
#define TSPRD_TIMER_H

#include <stdexcept>

template <class TimeT = std::chrono::milliseconds, class ClockT = std::chrono::steady_clock>
class Timer {
    using timep_t = typename ClockT::time_point;
    timep_t _start = ClockT::now(), _end = _start;

   public:
    void start() { _start = ClockT::now(); }

    void stop() { _end = ClockT::now(); }

    TimeT elapsedTime() const { return std::chrono::duration_cast<TimeT>(ClockT::now() - _start); }

    TimeT duration() const {
        if (_end == _start) throw runtime_error("called duration before stop");
        return std::chrono::duration_cast<TimeT>(_end - _start);
    }
};

#endif  // TSPRD_TIMER_H
