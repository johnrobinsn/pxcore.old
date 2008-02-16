// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxCore.h

#ifndef PX_CORE_H
#define PX_CORE_H

#include "pxConfig.h"

typedef int pxError;

#define PX_OK                   0
#define PX_FAIL                 1           // General Failure
#define PX_NOTINITILIZED        2           // Object requires initialization before use

// Utility Functions

template <typename t> 
inline t pxMin(t a1, t a2)
{
    return (a1 < a2)?a1:a2;
}

template <typename t> 
inline t pxMax(t a1, t a2)
{
    return (a1 > a2)?a1:a2;
}

template <typename t> 
inline t pxClamp(t v, t min, t max)
{
    return pxMin<t>(max, pxMax<t>(min, v));
}

template <typename t> 
inline t pxClamp(t v, t max)
{
    return pxMin<t>(max, pxMax<t>(0, v));
}

template <typename t>
inline t pxAbs(t i)
{
    if (i < 0) return -i;
    else return i;
}

#if defined(PX_PLATFORM_WIN)
#include "win/pxConfigNative.h"
#elif defined(PX_PLATFORM_MAC)
#include "mac/pxConfigNative.h"
#elif defined(PX_PLATFORM_X11)
#include "x11/pxConfigNative.h"
#else
#error "PX_PLATFORM NOT HANDLED"
#endif

#include "pxPixels.h"


#endif

