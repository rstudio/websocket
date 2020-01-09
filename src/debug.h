#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

// See the Makevars file to see how to compile with various debugging settings.

#ifdef DEBUG_THREAD

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #define __UNDEF_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #ifdef __UNDEF_LEAN_AND_MEAN
    #undef WIN32_LEAN_AND_MEAN
    #undef __UNDEF_LEAN_AND_MEAN
  #endif
#else
  #include <pthread.h>
#endif


#ifdef _WIN32
typedef HANDLE wsdebug_thrd_t;
#else
typedef pthread_t wsdebug_thrd_t;
#endif

wsdebug_thrd_t wsdebug_thrd_current(void);


extern wsdebug_thrd_t __main_thread__;
extern wsdebug_thrd_t __background_thread__;

// This must be called from the main thread so that thread assertions can be
// tested later.
#define REGISTER_MAIN_THREAD()       __main_thread__ = wsdebug_thrd_current();
#define REGISTER_BACKGROUND_THREAD() __background_thread__ = wsdebug_thrd_current();
#define ASSERT_MAIN_THREAD()         assert(wsdebug_thrd_current() == __main_thread__);
#define ASSERT_BACKGROUND_THREAD()   assert(wsdebug_thrd_current() == __background_thread__);

#else // ifdef DEBUG_THREAD

#define REGISTER_MAIN_THREAD()
#define REGISTER_BACKGROUND_THREAD()
#define ASSERT_MAIN_THREAD()
#define ASSERT_BACKGROUND_THREAD()

#endif // ifdef DEBUG_THREAD


#ifdef __cplusplus
}      // extern "C"
#endif

#endif
