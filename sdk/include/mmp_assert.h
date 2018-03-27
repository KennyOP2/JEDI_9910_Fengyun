
#ifndef MMP_ASSERT_H
#define MMP_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#define MMP_ASSERT(x) \
  do {if (!(x)) while(1);} while(0)
#else
#include <assert.h>
#define MMP_ASSERT assert
#endif

#ifdef __cplusplus
}
#endif

#endif
