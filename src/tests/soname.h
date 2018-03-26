#include "openssl/opensslv.h"

#define XSTR(x) STR(x)
#define STR(x) #x
echo XSTR(SHLIB_VERSION_NUMBER)
