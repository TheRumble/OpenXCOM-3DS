#include "IndexedBlit3DS.h"

#include <SDL.h>

#include <cstring>

namespace OpenXcom
{
namespace IndexedBlit3DS
{

namespace
{

struct PaletteCache
{
        bool valid = false;
        int colorCount = 0;
        int destinationBytes = 0;
        Uint32 redMask = 0;
        Uint32 greenMask = 0;
        Uint32 blueMask = 0;
        Uint32 alphaMask = 0;
        SDL_Color colors[256] = {};
        Uint32 mappedColors[256] = {};
};

PaletteCache paletteCache;

const Uint32 *getMappedColors(
        const SDL_Palette *palette,
        const SDL_PixelFormat *destinationFormat)
{
        const int paletteColorCount =
                palette ?
                        palette->ncolors :
                        0;

        const int colorCount =
                paletteColorCount < 256 ?
                        paletteColorCount :
                        256;

        const int destinationBytes =
                destinationFormat ?
                        destinationFormat->BytesPerPixel :
                        0;

        const bool formatMatches =
                paletteCache.valid &&
                paletteCache.destinationBytes ==
                        destinationBytes &&
                paletteCache.redMask ==
                        destinationFormat->Rmask &&
                paletteCache.greenMask ==
                        destinationFormat->Gmask &&
                paletteCache.blueMask ==
                        destinationFormat->Bmask &&
                paletteCache.alphaMask ==
                        destinationFormat->Amask;

        const bool paletteMatches =
                formatMatches &&
                paletteCache.colorCount ==
                        colorCount &&
                (colorCount == 0 ||
                        std::memcmp(
                                paletteCache.colors,
                                palette->colors,
                                static_cast<std::size_t>(
                                        colorCount) *
                                        sizeof(SDL_Color)) == 0);

        if (paletteMatches)
        {
                return paletteCache.mappedColors;
        }

        paletteCache.valid = true;
        paletteCache.colorCount = colorCount;
        paletteCache.destinationBytes =
                destinationBytes;
        paletteCache.redMask =
                destinationFormat->Rmask;
        paletteCache.greenMask =
                destinationFormat->Gmask;
        paletteCache.blueMask =
                destinationFormat->Bmask;
        paletteCache.alphaMask =
                destinationFormat->Amask;

        std::memset(
                paletteCache.colors,
                0,
                sizeof(paletteCache.colors));

        std::memset(
                paletteCache.mappedColors,
                0,
                sizeof(paletteCache.mappedColors));

        if (colorCount > 0)
        {
                std::memcpy(
                        paletteCache.colors,
                        palette->colors,
                        static_cast<std::size_t>(
                                colorCount) *
                                sizeof(SDL_Color));
        }

        for (int index = 0;
                index < colorCount;
                ++index)
        {
                const SDL_Color &color =
                        palette->colors[index];

                paletteCache.mappedColors[index] =
                        SDL_MapRGB(
                                destinationFormat,
                                color.r,
                                color.g,
                                color.b);
        }

        return paletteCache.mappedColors;
}

struct FixedTopScaleCache
{
        int destinationWidth = 0;
        bool verticalMapReady = false;
        int sourceX[400] = {};
        int sourceY[240] = {};
};

FixedTopScaleCache fixedTopScaleCache;

bool getFixedTopScaleMaps(
        SDL_Surface *source,
        const SDL_Rect &sourceRect,
        const SDL_Rect &destinationRect,
        const int *&sourceX,
        const int *&sourceY)
{
        /*
         * The normal 3DS top screen always scales the complete
         * 320x200 indexed frame to a 240-line true-color rectangle.
         * Cache those nearest-neighbor coordinates instead of doing
         * fixed-point accumulation in every output pixel.
         */
        if (source->w != 320 ||
                source->h != 200 ||
                sourceRect.x != 0 ||
                sourceRect.y != 0 ||
                sourceRect.w != 320 ||
                sourceRect.h != 200 ||
                destinationRect.w <= 0 ||
                destinationRect.w > 400 ||
                destinationRect.h != 240)
        {
                sourceX = nullptr;
                sourceY = nullptr;
                return false;
        }

        if (fixedTopScaleCache.destinationWidth !=
                destinationRect.w)
        {
                fixedTopScaleCache.destinationWidth =
                        destinationRect.w;

                const Uint32 xStep =
                        (static_cast<Uint32>(320) << 16) /
                        destinationRect.w;

                Uint32 sourceXFixed = 0;

                for (int x = 0;
                        x < destinationRect.w;
                        ++x)
                {
                        fixedTopScaleCache.sourceX[x] =
                                static_cast<int>(
                                        sourceXFixed >> 16);

                        sourceXFixed += xStep;
                }
        }

        if (!fixedTopScaleCache.verticalMapReady)
        {
                const Uint32 yStep =
                        (static_cast<Uint32>(200) << 16) /
                        240;

                Uint32 sourceYFixed = 0;

                for (int y = 0; y < 240; ++y)
                {
                        fixedTopScaleCache.sourceY[y] =
                                static_cast<int>(
                                        sourceYFixed >> 16);

                        sourceYFixed += yStep;
                }

                fixedTopScaleCache.verticalMapReady = true;
        }

        sourceX = fixedTopScaleCache.sourceX;
        sourceY = fixedTopScaleCache.sourceY;
        return true;
}

}

bool blitNearest(
        SDL_Surface *source,
        SDL_Surface *destination,
        const SDL_Rect &sourceRect,
        const SDL_Rect &destinationRect)
{
        if (!source ||
                !destination ||
                !source->format ||
                !destination->format ||
                sourceRect.w == 0 ||
                sourceRect.h == 0 ||
                destinationRect.w == 0 ||
                destinationRect.h == 0)
        {
                SDL_SetError(
                        "IndexedBlit3DS received an invalid surface or rectangle");

                return false;
        }

        if (source->format->BitsPerPixel != 8 ||
                !source->format->palette)
        {
                SDL_SetError(
                        "IndexedBlit3DS requires an 8-bit paletted source");

                return false;
        }

        const int destinationBytes =
                destination->format->BytesPerPixel;

        if (destinationBytes != 2 &&
                destinationBytes != 4)
        {
                SDL_SetError(
                        "IndexedBlit3DS requires a 16-bit or 32-bit destination");

                return false;
        }

        if (sourceRect.x < 0 ||
                sourceRect.y < 0 ||
                sourceRect.x + sourceRect.w > source->w ||
                sourceRect.y + sourceRect.h > source->h ||
                destinationRect.x < 0 ||
                destinationRect.y < 0 ||
                destinationRect.x + destinationRect.w >
                        destination->w ||
                destinationRect.y + destinationRect.h >
                        destination->h)
        {
                SDL_SetError(
                        "IndexedBlit3DS rectangle lies outside a surface");

                return false;
        }

        const Uint32 *mappedColors =
                getMappedColors(
                        source->format->palette,
                        destination->format);

        bool sourceLocked = false;
        bool destinationLocked = false;

        if (SDL_MUSTLOCK(source))
        {
                if (SDL_LockSurface(source) != 0)
                {
                        return false;
                }

                sourceLocked = true;
        }

        if (SDL_MUSTLOCK(destination))
        {
                if (SDL_LockSurface(destination) != 0)
                {
                        if (sourceLocked)
                        {
                                SDL_UnlockSurface(source);
                        }

                        return false;
                }

                destinationLocked = true;
        }

        const int *fixedSourceX = nullptr;
        const int *fixedSourceY = nullptr;

        const bool fixedTopScale =
                getFixedTopScaleMaps(
                        source,
                        sourceRect,
                        destinationRect,
                        fixedSourceX,
                        fixedSourceY);

        const Uint32 xStep =
                fixedTopScale ?
                        0 :
                        (static_cast<Uint32>(
                                sourceRect.w) << 16) /
                                destinationRect.w;

        const Uint32 yStep =
                fixedTopScale ?
                        0 :
                        (static_cast<Uint32>(
                                sourceRect.h) << 16) /
                                destinationRect.h;

        Uint32 sourceYFixed = 0;
        int previousSourceY = -1;
        Uint8 *previousDestinationRow = nullptr;

        const std::size_t destinationRowBytes =
                static_cast<std::size_t>(
                        destinationRect.w) *
                static_cast<std::size_t>(
                        destinationBytes);

        for (int destinationY = 0;
                destinationY < destinationRect.h;
                ++destinationY)
        {
                const int sourceY =
                        fixedTopScale ?
                                fixedSourceY[destinationY] :
                                sourceRect.y +
                                static_cast<int>(
                                        sourceYFixed >> 16);

                Uint8 *destinationRow =
                        static_cast<Uint8 *>(
                                destination->pixels) +
                        (destinationRect.y +
                                destinationY) *
                                destination->pitch +
                        destinationRect.x *
                                destinationBytes;

                /*
                 * Vertical nearest-neighbor upscaling often repeats a
                 * source row. Copy the converted output row instead of
                 * converting every pixel again.
                 */
                if (sourceY == previousSourceY &&
                        previousDestinationRow)
                {
                        std::memcpy(
                                destinationRow,
                                previousDestinationRow,
                                destinationRowBytes);

                        if (!fixedTopScale)
                        {
                                sourceYFixed += yStep;
                        }

                        continue;
                }

                const Uint8 *sourceRow =
                        static_cast<const Uint8 *>(
                                source->pixels) +
                        sourceY * source->pitch;

                Uint32 sourceXFixed = 0;

                if (destinationBytes == 2)
                {
                        Uint16 *output =
                                reinterpret_cast<Uint16 *>(
                                        destinationRow);

                        for (int destinationX = 0;
                                destinationX <
                                        destinationRect.w;
                                ++destinationX)
                        {
                                const int sourceX =
                                        fixedTopScale ?
                                                fixedSourceX[
                                                        destinationX] :
                                                sourceRect.x +
                                                static_cast<int>(
                                                        sourceXFixed >>
                                                                16);

                                output[destinationX] =
                                        static_cast<Uint16>(
                                                mappedColors[
                                                        sourceRow[
                                                                sourceX]]);

                                if (!fixedTopScale)
                                {
                                        sourceXFixed += xStep;
                                }
                        }
                }
                else
                {
                        Uint32 *output =
                                reinterpret_cast<Uint32 *>(
                                        destinationRow);

                        for (int destinationX = 0;
                                destinationX <
                                        destinationRect.w;
                                ++destinationX)
                        {
                                const int sourceX =
                                        fixedTopScale ?
                                                fixedSourceX[
                                                        destinationX] :
                                                sourceRect.x +
                                                static_cast<int>(
                                                        sourceXFixed >>
                                                                16);

                                output[destinationX] =
                                        mappedColors[
                                                sourceRow[sourceX]];

                                if (!fixedTopScale)
                                {
                                        sourceXFixed += xStep;
                                }
                        }
                }

                previousSourceY = sourceY;
                previousDestinationRow =
                        destinationRow;

                if (!fixedTopScale)
                {
                        sourceYFixed += yStep;
                }
        }

        if (destinationLocked)
        {
                SDL_UnlockSurface(destination);
        }

        if (sourceLocked)
        {
                SDL_UnlockSurface(source);
        }

        return true;
}

}
}
