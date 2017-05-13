#ifndef CORAL_CORALSTUBS_CPPTYPES_H
#define CORAL_CORALSTUBS_CPPTYPES_H

#include "CoralServerBase/wintypes.h"

#ifdef _WIN32

namespace coral {

  typedef __int16 uint16_t;
  typedef __int64 uint64_t;
  typedef __int16 int16_t;
  typedef __int32 int32_t;
  typedef __int64 int64_t;

  typedef uint64_t uint128_t[2];

}

#else

#include <stdint.h>

namespace coral {

  typedef uint64_t uint128_t[2];

}

#endif

#include <limits.h>

#if ULONG_MAX == 0xFFFFFFFF
#define __SIZEOF_LONG__ 4
#define __TYPEOF_LONG__ uint32_t
#elif ULONG_MAX == 0xFFFFFFFFFFFFFFFF
#define __SIZEOF_LONG__ 8
#define __TYPEOF_LONG__ uint64_t
#else
#error unknown long max
#endif

#if UINT_MAX == 0xFFFF
#define __SIZEOF_INT__ 2
#define __TYPEOF_INT__ uint16_t
#elif UINT_MAX == 0xFFFFFFFF
#define __SIZEOF_INT__ 4
#define __TYPEOF_INT__ uint32_t
#elif UINT_MAX == 0xFFFFFFFFFFFFFFFF
#define __SIZEOF_INT__ 8
#define __TYPEOF_INT__ uint64_t
#else
#error unknown int max
#endif

#define __SIZEOF_LONGLONG__ 8
#define __TYPEOF_LONGLONG__ uint64_t


#endif
