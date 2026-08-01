#pragma once

struct SDL_Surface;

namespace OpenXcom
{

class Game;

namespace BottomScreen3DS
{

/*
 * Synthetic mouse events carrying this device value originate from
 * the lower-screen geoscape controls.
 */
constexpr unsigned char BOTTOM_CONTROL_EVENT_WHICH = 0x3d;


enum Mode
{
        MODE_MENU,
        MODE_GEOSCAPE,
        MODE_GEOSCAPE_FROZEN,
        MODE_BATTLESCAPE,
        MODE_INVENTORY
};

void setGame(Game *game);

void captureGameFrame(SDL_Surface *gameSurface);
void requestPanelRefresh();

void setMode(Mode mode);
Mode getMode();

void setBottomCursorFocused(bool focused);
bool isBottomCursorFocused();
void getBottomCursorPosition(int &x, int &y);
void toggleCursorFocus();

void moveBottomCursor(int dx, int dy);
bool snapBottomCursor(int horizontal, int vertical);

bool getControlTargetAt(
        int x,
        int y,
        int &logicalX,
        int &logicalY,
        bool &opensTopPanel);

bool getBottomCursorControlTarget(
        int &logicalX,
        int &logicalY,
        bool &opensTopPanel);

bool isTrackpadPoint(int x, int y);

void renderPanel(
        SDL_Surface *screen,
        SDL_Surface *gameSurface);

}
}
