#include "Timer.h"
#include "Util.h"

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/resource.h>
#include <sys/time.h>
#endif



namespace {

#if !defined(_WIN32) && !defined(_WIN64)
uint64_t GetMicroSeconds(const struct timeval& tv) {
  return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

uint64_t GetTimeOfDayMicroSeconds() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

#else
#include <time.h>
#include <Windows.h>
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
 
struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};
 
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;
 
  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
 
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tmpres /= 10;  /*convert into microseconds*/
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }
 
  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }
 
  return 0;
}

uint64_t GetMicroSeconds(const struct timeval& tv) {
  return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

uint64_t GetTimeOfDayMicroSeconds() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}
#endif

} // namespace

namespace MosesTuning
{
  

void Timer::GetCPUTimeMicroSeconds(Timer::CPUTime* cpu_time) const {
#if !defined(_WIN32) && !defined(_WIN64)
  struct rusage usage;
  if (getrusage(RUSAGE_SELF, &usage)) {
    TRACE_ERR("Error occurred: getrusage().\n");
    exit(1);
  }
  cpu_time->user_time = GetMicroSeconds(usage.ru_utime);
  cpu_time->sys_time = GetMicroSeconds(usage.ru_stime);
#else  // Windows
  // Not implemented yet.
  // TODO: implement the Windows version using native APIs.
#endif
}

double Timer::get_elapsed_cpu_time() const {
  return static_cast<double>(get_elapsed_cpu_time_microseconds()) * 1e-6;
}

uint64_t Timer::get_elapsed_cpu_time_microseconds() const {
  CPUTime e;
  GetCPUTimeMicroSeconds(&e);
  return (e.user_time - m_start_time.user_time) +
      (e.sys_time - m_start_time.sys_time);
}

double Timer::get_elapsed_wall_time() const {
  return static_cast<double>(get_elapsed_wall_time_microseconds()) * 1e-6;
}

uint64_t Timer::get_elapsed_wall_time_microseconds() const {
  return GetTimeOfDayMicroSeconds() - m_wall;
}

void Timer::start(const char* msg)
{
  // Print an optional message, something like "Starting timer t";
  if (msg) TRACE_ERR( msg << std::endl);
  if (m_is_running) return;
  m_is_running = true;
  m_wall = GetTimeOfDayMicroSeconds();
  GetCPUTimeMicroSeconds(&m_start_time);
}

void Timer::restart(const char* msg)
{
  if (msg) {
    TRACE_ERR(msg << std::endl);
  }
  m_wall = GetTimeOfDayMicroSeconds();
  GetCPUTimeMicroSeconds(&m_start_time);
}

void Timer::check(const char* msg)
{
  // Print an optional message, something like "Checking timer t";
  if (msg) TRACE_ERR( msg << " : ");

  if (m_is_running) {
    TRACE_ERR("[Wall " << get_elapsed_wall_time()
              << " CPU " << get_elapsed_cpu_time() << "] seconds.\n");
  } else {
    TRACE_ERR("WARNING: the timer is not running.\n");
  }
}

std::string Timer::ToString() const {
  std::string res;
  const double wall = get_elapsed_wall_time();
  CPUTime e;
  GetCPUTimeMicroSeconds(&e);
  const double utime = (e.user_time - m_start_time.user_time) * 1e-6;
  const double stime = (e.sys_time - m_start_time.sys_time) * 1e-6;
  std::stringstream ss;
  ss << "wall "  << wall << " sec. user " << utime << " sec. sys " << stime
     << " sec. total " << utime + stime << " sec.";
  res.append(ss.str());

  return res;
}

}
