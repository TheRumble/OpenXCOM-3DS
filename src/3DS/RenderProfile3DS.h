#pragma once

/*
 * Public-release no-op renderer profiling interface.
 *
 * The private development branch contains the sampled profiler.
 * Public builds retain the tested call sites without collecting
 * measurements or writing reports to the SD card.
 */

#ifdef NINTENDO_3DS

#include <3ds.h>

namespace OpenXcom
{
namespace RenderProfile3DS
{

inline u64 now()
{
    return 0;
}

inline void flushPending()
{
}

inline void markSelectorRedraw()
{
}

inline bool consumeSelectorRedraw()
{
    return false;
}

inline void beginMapFrame(bool)
{
}

inline void setTerrainTicks(u64)
{
}

inline void endMapFrame()
{
}

inline bool isRawPixelCountingActive()
{
    return false;
}

inline void recordRawShader(
    u64,
    u64,
    bool)
{
}

inline void recordEmptyRawShader(bool)
{
}

class ScopedRawBlit
{
public:
    ScopedRawBlit()
    {
    }

    ~ScopedRawBlit()
    {
    }
};

class ScopedUnitDraw
{
public:
    ScopedUnitDraw()
    {
    }

    ~ScopedUnitDraw()
    {
    }
};

}
}

#endif
