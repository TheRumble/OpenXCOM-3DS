#include "VideoFilter3DS.h"

#include <citro3d.h>

namespace
{

C3D_Tex *displayTexture = nullptr;
bool smoothFiltering = false;
bool filterDirty = true;

}

/*
 * C3D_TexInit() is inline and ultimately calls this function.
 * The linker wrapper lets the application observe SDL-3DS creating
 * its private presentation texture without modifying the system SDL
 * installation.
 */
extern "C" bool __real_C3D_TexInitWithParams(
        C3D_Tex *texture,
        C3D_TexCube *cube,
        C3D_TexInitParams params);

extern "C" bool __wrap_C3D_TexInitWithParams(
        C3D_Tex *texture,
        C3D_TexCube *cube,
        C3D_TexInitParams params)
{
        const bool initialized =
                __real_C3D_TexInitWithParams(
                        texture,
                        cube,
                        params);

        /*
         * OXCE's fixed 400x480 dual-screen SDL surface is stored in
         * a 512x512 power-of-two texture by SDL-3DS.
         */
        if (initialized &&
                params.type == GPU_TEX_2D &&
                params.width == 512 &&
                params.height == 512)
        {
                displayTexture = texture;
                filterDirty = true;
        }

        return initialized;
}

namespace OpenXcom
{
namespace VideoFilter3DS
{

void setSmooth(bool smooth)
{
        if (smoothFiltering != smooth)
        {
                smoothFiltering = smooth;
                filterDirty = true;
        }
}

void apply()
{
        if (!displayTexture || !filterDirty)
        {
                return;
        }

        const GPU_TEXTURE_FILTER_PARAM filter =
                smoothFiltering ?
                        GPU_LINEAR :
                        GPU_NEAREST;

        C3D_TexSetFilter(
                displayTexture,
                filter,
                filter);

        filterDirty = false;
}

}
}
