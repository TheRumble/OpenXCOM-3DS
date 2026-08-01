#include "System3DS.h"
#include "GlobeShadowWorker3DS.h"

#include <3ds.h>
#include <SDL.h>
#include <SDL_mixer.h>

namespace OpenXcom
{
namespace System3DS
{

void enableNew3DSSpeedup()
{
	/*
	 * On New Nintendo 3DS hardware, request the higher CPU clock
	 * and enable the L2 cache before OXCE begins loading data.
	 */
	osSetSpeedupEnable(true);
}

void shutdownGlobeShadowWorker()
{
	OpenXcom::shutdownGlobeShadowWorker3DS();
}

void shutdownAudio()
{
        /*
         * The 3DS fast-exit path skips Game's full destructor.
         * Stop SDL audio and wait for the NDSP worker before
         * libctru begins releasing process memory.
         */
        Mix_CloseAudio();
        SDL_CloseAudio();
}


}
}
