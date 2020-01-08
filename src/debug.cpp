#include "debug.h"

#if defined(DEBUG_THREAD)

wsdebug_thrd_t __main_thread__;
wsdebug_thrd_t __background_thread__;

wsdebug_thrd_t wsdebug_thrd_current(void) {
#if defined(_TTHREAD_WIN32_)
  return GetCurrentThread();
#else
  return pthread_self();
#endif
}

#endif // defined(DEBUG_THREAD)
