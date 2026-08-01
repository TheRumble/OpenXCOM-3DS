#pragma once
#include <SDL_version.h>
#include <SDL_rwops.h>

/*
 * Bits of SDL2 API that are missing from SDL1
 * Implementations are in FileMap.cpp
 * To be removed with transition to SDL2
 */
# if !SDL_VERSION_ATLEAST(2,0,0)
extern "C"
{
Uint8 SDL_ReadU8(SDL_RWops *src);
Sint64 SDL_RWsize(SDL_RWops *src);
}
# endif
#if !SDL_VERSION_ATLEAST(2,0,6)
extern "C"
{
void *SDL_LoadFile_RW(SDL_RWops *src, size_t *datasize, int freesrc);
}
#endif
