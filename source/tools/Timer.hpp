
//note:implementation may change based on stats preferred by company.

#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <cstddef>
#include <cassert>
namespace cse498
{
  class Timer {
  private:
    using Clock = std::chrono::steady_clock;
    struct TimerEntry {
      bool running = false; //tracks if start() called w/o matching stop()
      Clock::time_point start{};

      //stats for completed runs
      std::size_t count=0;
      double totalSeconds=0.0;
      double lastSeconds=0.0;
      double minSeconds=0.0;
      double maxSeconds=0.0;
    };

    std::unordered_map<std::string,TimerEntry> mTimers; ///timers keyed by name

  public:
    Timer() = default;
    ~Timer() = default;

    //peer review fix; define copy and move behavior
    Timer(const Timer&) = delete; 
    Timer& operator=(const Timer&) = delete;
    Timer(Timer&&) noexcept = default;
    Timer& operator=(Timer&&) noexcept = default;

    void Start(const std::string& name)  
    { //begin timing in named section; const& avoids copying name
      auto& entry= mTimers[name]; 
      assert(!entry.running &&"Timer::Start called while timer is already running");
      entry.running =true;
      entry.start = Clock::now();
    }

    void Stop(const std::string& name)
    { 
      //stop timing and record duration
      auto it= mTimers.find(name);
      assert(it !=mTimers.end()&& "Timer::Stop called for unknown timer name");
      auto& entry =it->second; //ref to timerentry for the name
      assert(entry.running&& "Timer::Stop called while timer is not running"); 

      const auto end= Clock::now();
      const std::chrono::duration<double> elapsed= end-entry.start;
      const double seconds =elapsed.count();

      entry.running = false; //no longer running, then record stats. (most recent dur,avg)
      entry.lastSeconds =seconds;
      entry.totalSeconds+= seconds; 
      entry.count +=1;

      if (entry.count==1)
      { 
        //if this was first run
        entry.minSeconds =seconds;
        entry.maxSeconds =seconds;
      } 

      else 
      { 
        //if not first run, update min and max as needed
        if (seconds < entry.minSeconds)entry.minSeconds = seconds;
        if (seconds> entry.maxSeconds) entry.maxSeconds =seconds;
      }
    }

    void Reset(const std::string& name) 
    {
       //clear stored result for timer
      mTimers.erase(name);
    }

    void ResetAll()
    { 
      //clear ALL timers and stats
      mTimers.clear();
    }

    bool HasData(const std::string& name)const 
    { 
      //true if timer has 1 measurement
      auto it = mTimers.find(name);
      return (it != mTimers.end())&& (it->second.count > 0);
    }

    double Last(const std::string& name) const
    { 
      //most recent recorded dur; returns 0.0 if no data.
      auto it = mTimers.find(name);
      if (it == mTimers.end()|| it->second.count== 0) return 0.0;
      return it->second.lastSeconds;
    }

    double Min(const std::string& name) const
    { 
      //fastest run in sec; returns 0 if no data
      auto it = mTimers.find(name);
      if (it == mTimers.end() ||it->second.count == 0) return 0.0;
      return it->second.minSeconds;
    }

    double Max(const std::string& name)const 
    { 
      //slowest run in sec; returns 0 if no data
      auto it =mTimers.find(name);
      if(it ==mTimers.end() ||it->second.count == 0) return 0.0;
      return it->second.maxSeconds;
    }

    double Average(const std::string& name)const 
    { 
      //avg dur across all runs in sec; returns 0 if no data
      auto it = mTimers.find(name);
      if (it == mTimers.end()|| it->second.count == 0) return 0.0;
      return it->second.totalSeconds/static_cast<double>(it->second.count);
    }

    std::size_t Count(const std::string& name)const
    { 
      //# of completed runs; returns 0 if no data
      auto it= mTimers.find(name);
      if (it ==mTimers.end()) return 0;
      return it->second.count;
    }
  };
}
