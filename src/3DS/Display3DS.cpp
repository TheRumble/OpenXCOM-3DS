#include "Display3DS.h"

#include "../Engine/Options.h"
#include "../Engine/Screen.h"

#include <SDL.h>

namespace OpenXcom
{
namespace Display3DS
{

void applyFixedDisplayOptions()
{
	// Native New Nintendo 3DS top-screen dimensions.
	Options::displayWidth = 400;
	Options::displayHeight = 240;
	Options::newDisplayWidth = 400;
	Options::newDisplayHeight = 240;

	// Desktop window modes do not apply on the 3DS.
	Options::fullscreen = false;
	Options::newFullscreen = false;
	Options::allowResize = false;
	Options::newAllowResize = false;
	Options::borderless = false;
	Options::newBorderless = false;
	Options::rootWindowedMode = false;
	Options::newRootWindowedMode = false;
	Options::captureMouse = SDL_GRAB_OFF;

	// The 3DS port currently uses the software renderer.
	Options::useOpenGL = false;
	Options::newOpenGL = false;
	Options::useScaleFilter = false;
	Options::newScaleFilter = false;
	Options::useHQXFilter = false;
	Options::newHQXFilter = false;
	Options::useXBRZFilter = false;
	Options::newXBRZFilter = false;

	// Preserve the original 320:200 interface without cropping.
	Options::keepAspectRatio = true;
	Options::nonSquarePixelRatio = false;
	Options::cursorInBlackBandsInWindow = true;

	Options::geoscapeScale = SCALE_ORIGINAL;
	Options::battlescapeScale = SCALE_ORIGINAL;
	Options::newGeoscapeScale = SCALE_ORIGINAL;
	Options::newBattlescapeScale = SCALE_ORIGINAL;
}

void applyBaseResolution()
{
	Options::baseXResolution = Screen::ORIGINAL_WIDTH;
	Options::baseYResolution = Screen::ORIGINAL_HEIGHT;

	Options::baseXGeoscape = Screen::ORIGINAL_WIDTH;
	Options::baseYGeoscape = Screen::ORIGINAL_HEIGHT;

	Options::baseXBattlescape = Screen::ORIGINAL_WIDTH;
	Options::baseYBattlescape = Screen::ORIGINAL_HEIGHT;
}

}
}
