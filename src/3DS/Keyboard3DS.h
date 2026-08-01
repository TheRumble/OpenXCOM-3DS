#pragma once

#include <SDL.h>

namespace OpenXcom
{
namespace Keyboard3DS
{

/* OXCE_3DS_KEYBOARD_MODES */
enum Mode
{
        MODE_TEXT,
        MODE_BINDING,
        MODE_FULL
};

void setMode(Mode mode);
Mode getMode();

void setVisible(bool visible);
bool isVisible();

bool getKeyAt(
        int x,
        int y,
        int renderedWidth,
        SDLKey &key,
        Uint16 &unicode);

/* OXCE_3DS_KEYBOARD_CURSOR_CONTROL */
bool snapCursor(
        int &physicalX,
        int &physicalY,
        int horizontal,
        int vertical);

void setPressedKey(SDLKey key);
void clearPressedKey();

void render(SDL_Surface *screen);

}
}
