#pragma once

struct SDL_Rect;
struct SDL_Surface;

namespace OpenXcom
{
namespace IndexedBlit3DS
{

bool blitNearest(
        SDL_Surface *source,
        SDL_Surface *destination,
        const SDL_Rect &sourceRect,
        const SDL_Rect &destinationRect);

}
}
