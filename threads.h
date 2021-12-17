#pragma once

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
#include <threads.h>
#else

#ifdef __GNUC__

#ifndef __has_warning
#define __has_warning(x) 0
#endif

#pragma GCC diagnostic push
#if __has_warning("-Wreserved-macro-identifier")
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#endif
#if __has_warning("-Wpadded")
#pragma GCC diagnostic ignored "-Wpadded"
#endif
#if __has_warning("-Wmissing-noreturn")
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#endif
#include "3rd/threads/threads.h"
#pragma GCC diagnostic pop

#else

#include "3rd/threads/threads.h"

#endif // __GNUC__

#endif // __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
