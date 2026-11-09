#ifndef SOURCE_VENDOR_MILES_CORE_API_HPP
#define SOURCE_VENDOR_MILES_CORE_API_HPP

#include <stdio.h>

struct _DIG_DRIVER;

typedef void (*AIL_LOG_CALLBACK)(const char* pText, int iDepth);
typedef void (*AIL_TIMER_CALLBACK)(unsigned int uUser);

extern int AIL_log_start_ms;
extern AIL_LOG_CALLBACK AIL_log_callback;
extern FILE* AIL_log_file;

extern "C" void AIL_API_shutdown();
extern "C" const char* AIL_API_last_error();
extern "C" void AIL_API_sleep(int iMilliseconds);
extern "C" int AIL_API_register_timer(AIL_TIMER_CALLBACK pCallback);
extern "C" unsigned int AIL_API_set_timer_user(int iTimer, unsigned int uUser);
extern "C" void AIL_API_set_timer_period(int iTimer, unsigned int uMicroseconds);
extern "C" void AIL_API_set_timer_frequency(int iTimer, unsigned int uHertz);
extern "C" void AIL_API_start_timer(int iTimer);
extern "C" void AIL_API_release_timer_handle(int iTimer);
extern "C" void AIL_API_release_all_timers();

extern "C" AIL_LOG_CALLBACK AIL_configure_logging(const char* pFileName, AIL_LOG_CALLBACK pCallback,
                                                  unsigned int uMode);
extern "C" int AIL_startup();
extern "C" void AIL_shutdown();
extern "C" const char* AIL_last_error();
extern "C" void AIL_sleep(int iMilliseconds);
extern "C" int AIL_register_timer(AIL_TIMER_CALLBACK pCallback);
extern "C" unsigned int AIL_set_timer_user(int iTimer, unsigned int uUser);
extern "C" void AIL_set_timer_period(int iTimer, unsigned int uMicroseconds);
extern "C" void AIL_set_timer_frequency(int iTimer, unsigned int uHertz);
extern "C" void AIL_start_timer(int iTimer);
extern "C" void AIL_release_timer_handle(int iTimer);
extern "C" void AIL_release_all_timers();
extern "C" int MC_open_output_filter(const char* pName, _DIG_DRIVER* pDriver, int iMatrix);

#endif
