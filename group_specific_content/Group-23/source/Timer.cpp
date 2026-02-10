/**
 * @file Timer.cpp
 * @author Lauren Phillips
 *
 * @brief util class for timing sections of code and outputting stats
 */

#include "Timer.hpp"

Timer::Timer() {}
Timer::~Timer() {}

void Timer::Start(const std::string& name) {
  // starting the timer
  (void)name; //placeholder to get rid of warnings

}

void Timer::Stop(const std::string& name){
  //stop timing, compute duration, and update stats (last/count/avg/min/max)
  (void)name; //placeholder to get rid of warnings
}

void Timer::Reset(const std::string&name) {
  // clear stored results for this timer name aka remove reset entry
  (void)name; //placeholder to get rid of warnings
}

void Timer::ResetAll() {
  // clear all timers and all recorded measurements.
}

bool Timer::HasData(const std::string& name)const{
  // if timer has one measurment recorded
  (void)name; //placeholder to get rid of warnings
  return false;
}

double Timer::Last(const std::string& name) const {
  //return the most recent duration for the timer name
  (void)name; //placeholder to get rid of warnings
  return 0.0;
}

double Timer::Min(const std::string& name) const {
  //  will return the fastest recorded run
  (void)name; //placeholder to get rid of warnings
  return 0.0;
}

double Timer::Max(const std::string& name) const {
  //will return the slowest recorded run
  (void)name; //placeholder to get rid of warnings
  return 0.0;
}

double Timer::Average(const std::string& name) const {
  // return the avg duration across all recorded runs
  (void)name; //placeholder to get rid of warnings
  return 0.0;
}

std::size_t Timer::Count(const std::string& name) const {
  //  return the number of completed runs recorded
  (void)name; //placeholder to get rid of warnings
  return 0;
}
