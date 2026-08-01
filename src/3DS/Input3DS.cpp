#include "Input3DS.h"
#include "BottomScreen3DS.h"
#include "Keyboard3DS.h"
#include "MenuNavigation3DS.h"
#include "../Geoscape/GeoscapeState.h"
#include "../Geoscape/BuildNewBaseState.h"
#include "../Battlescape/InventoryState.h"
#include "../Battlescape/UnitInfoState.h"
#include "../Battlescape/BattlescapeState.h"

/* OXCE_3DS_CONTROLS_BINDING_INCLUDE */
#include "../Menu/OptionsControlsState.h"

#include "../Engine/Game.h"
#include "../Engine/Screen.h"
#include "../Engine/Options.h"
#include "../Geoscape/Globe.h"
#include "../Savegame/Base.h"
#include "../Savegame/Craft.h"
#include "../Savegame/MovingTarget.h"
#include "../Savegame/Ufo.h"
#include "../Savegame/SavedGame.h"

#include <3ds.h>
#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace OpenXcom
{
namespace Input3DS
{

namespace
{

int cursorX = 0;
int cursorY = 0;
bool cursorInitialized = false;

MovingTarget *followCraft3DS = nullptr;
double followLongitude3DS = 0.0;
double followLatitude3DS = 0.0;
bool followPositionValid3DS = false;

u32 previousHeld = 0;
bool buttonStateInitialized = false;

bool trackpadDragActive = false;
int previousTouchX = 0;
int previousTouchY = 0;

int trackpadStartX = 0;
int trackpadStartY = 0;
bool trackpadMoved = false;
u64 trackpadStartedAt = 0;
bool trackpadButtonDown = false;
Uint8 trackpadButton = SDL_BUTTON_LEFT;

bool physicalRightMouseDown = false;
bool bottomZeroTUsYConsumed = false;
BattlescapeState *bottomZeroTUsPressedState = nullptr;
bool physicalMiddleMouseDown = false;
bool physicalInventoryStartDown = false;

Globe *cStickGlobe = nullptr;
int cStickLongitudeDirection = 0;
int cStickLatitudeDirection = 0;




bool eventFilterInstalled = false;
bool menuNavigationActive = false;
bool textEditingActive = false;
/* OXCE_3DS_BINDING_ACTIVE_STATE */
bool keyBindingActive = false;

/*
 * The complete desktop-style keyboard can also be opened manually
 * outside text fields and the Controls binding screen.
 */
bool fullKeyboardVisible = false;
bool fullKeyboardPreviousBottomFocus = false;

TextEdit *trackedTextEdit = nullptr;
bool keyboardHiddenByUser = false;

bool backConsumedByComboBox = false;
bool actionConsumedBySlider = false;
bool actionConsumedByKeyboard = false;
bool actionConsumedByBottomCursor = false;

bool bottomCursorControlActive = false;
int bottomCursorControlX = 0;
int bottomCursorControlY = 0;

bool bottomTouchControlActive = false;
int bottomTouchControlX = 0;
int bottomTouchControlY = 0;

/*
 * A main geoscape command temporarily hands focus to the top screen.
 * Once its menu closes and the real geoscape returns, bottom focus is
 * restored automatically.
 */
bool topPanelHandoffPending = false;
bool topPanelHandoffSawMenu = false;
bool topPanelHandoffPreviousBottomFocus = false;
u64 topPanelHandoffStartedAt = 0;

BottomScreen3DS::Mode topPanelHandoffOriginMode =
        BottomScreen3DS::MODE_MENU;

bool keyboardTouchActive = false;
SDLKey keyboardTouchKey = SDLK_UNKNOWN;
Uint16 keyboardTouchUnicode = 0;

struct RepeatState
{
        u32 direction = 0;
        u64 startedAt = 0;
        u64 lastRepeatedAt = 0;

        void reset()
        {
                direction = 0;
                startedAt = 0;
                lastRepeatedAt = 0;
        }

        bool setDirection(
                u32 requestedDirection,
                u64 now)
        {
                if (requestedDirection == direction)
                {
                        return false;
                }

                direction = requestedDirection;
                startedAt = now;
                lastRepeatedAt = now;
                return true;
        }

        u64 heldFor(u64 now) const
        {
                return now - startedAt;
        }

        bool ready(
                u64 now,
                u64 initialDelayMs,
                u64 repeatIntervalMs)
        {
                if (direction == 0 ||
                        heldFor(now) < initialDelayMs ||
                        now - lastRepeatedAt <
                                repeatIntervalMs)
                {
                        return false;
                }

                lastRepeatedAt = now;
                return true;
        }
};

RepeatState sliderRepeat;
RepeatState menuRepeat;
RepeatState bottomCursorRepeat;

int mouseTopHeight = 240;

int filter3DSEvent(const SDL_Event *event)
{
        /* OXCE_3DS_KEYBOARD_MOUSE_FILTER */
        if (Keyboard3DS::isVisible() &&
                (event->type == SDL_MOUSEMOTION ||
                 event->type == SDL_MOUSEBUTTONDOWN ||
                 event->type == SDL_MOUSEBUTTONUP))
        {
                return 0;
        }

        /*
         * SDL-3DS automatically converts touchscreen input into
         * absolute mouse events in the lower half of the combined
         * dual-screen surface. OXCE's mouse uses only the top half.
         */
        if (event->type == SDL_MOUSEMOTION &&
                event->motion.y >= mouseTopHeight)
        {
                return 0;
        }

        if ((event->type == SDL_MOUSEBUTTONDOWN ||
                event->type == SDL_MOUSEBUTTONUP) &&
                event->button.y >= mouseTopHeight)
        {
                return 0;
        }

        /*
         * SDL-3DS emits keyboard arrow events for the D-pad.
         * Suppress those while it is selecting menu controls.
         */
        /* OXCE_3DS_BINDING_DPAD_FILTER */
        if ((menuNavigationActive ||
                textEditingActive ||
                keyBindingActive ||
                BottomScreen3DS::isBottomCursorFocused()) &&
                (event->type == SDL_KEYDOWN ||
                        event->type == SDL_KEYUP))
        {
                const SDLKey key = event->key.keysym.sym;

                if (key == SDLK_UP ||
                        key == SDLK_DOWN ||
                        key == SDLK_LEFT ||
                        key == SDLK_RIGHT)
                {
                        return 0;
                }
        }

        return 1;
}

int getCursorHeight(SDL_Surface *video)
{
        return video->h > 240 ? video->h / 2 : video->h;
}

int clampSpeedPercent3DS(int value)
{
        return std::max(
                50,
                std::min(200, value));
}

int getCursorSpeedPercent3DS(Game *game)
{
        /*
         * These checks inspect the actual top state. A popup or Options
         * screen opened over gameplay therefore uses Menu speed.
         */
        if (MenuNavigation3DS::isInventory(game))
        {
                return clampSpeedPercent3DS(
                        Options::
                                threeDSInventoryCursorSpeed);
        }

        if (MenuNavigation3DS::isBattlescape(game))
        {
                return clampSpeedPercent3DS(
                        Options::
                                threeDSBattlescapeCursorSpeed);
        }

        if (MenuNavigation3DS::isGeoscape(game))
        {
                return clampSpeedPercent3DS(
                        Options::
                                threeDSGeoscapeCursorSpeed);
        }

        return clampSpeedPercent3DS(
                Options::threeDSMenuCursorSpeed);
}

void initializeCursor(SDL_Surface *video)
{
        if (cursorInitialized)
        {
                return;
        }

        cursorX = video->w / 2;
        cursorY = getCursorHeight(video) / 2;

        SDL_WarpMouse(
                static_cast<Uint16>(cursorX),
                static_cast<Uint16>(cursorY));

        cursorInitialized = true;
}

void setCursor(SDL_Surface *video, int x, int y)
{
        initializeCursor(video);

        cursorX = std::max(
                0,
                std::min(video->w - 1, x));

        cursorY = std::max(
                0,
                std::min(
                        getCursorHeight(video) - 1,
                        y));

        SDL_WarpMouse(
                static_cast<Uint16>(cursorX),
                static_cast<Uint16>(cursorY));
}

void cancelCraftFollow3DS()
{
        followCraft3DS = nullptr;
        followLongitude3DS = 0.0;
        followLatitude3DS = 0.0;
        followPositionValid3DS = false;
}

void beginCraftFollow3DS(Target *target)
{
        MovingTarget *movingTarget =
                dynamic_cast<MovingTarget *>(target);

        bool valid =
                movingTarget &&
                movingTarget->getDestination() &&
                movingTarget->getSpeed() > 0 &&
                movingTarget->getMarker() != -1;

        if (valid)
        {
                Craft *craft =
                        dynamic_cast<Craft *>(
                                movingTarget);

                Ufo *ufo =
                        dynamic_cast<Ufo *>(
                                movingTarget);

                if (craft)
                {
                        valid =
                                craft->getStatus() ==
                                        "STR_OUT";
                }
                else if (ufo)
                {
                        valid =
                                ufo->getDetected() &&
                                ufo->getStatus() ==
                                        Ufo::FLYING;
                }
                else
                {
                        valid = false;
                }
        }

        if (!valid)
        {
                cancelCraftFollow3DS();
                return;
        }

        followCraft3DS = movingTarget;
        followPositionValid3DS = false;
}

bool updateCraftFollow3DS(
        SDL_Surface *video,
        Game *game)
{
        if (!followCraft3DS ||
                !game ||
                !game->getSavedGame())
        {
                return false;
        }

        GeoscapeState *geoscape =
                game->getGeoscapeState();

        if (!geoscape ||
                !game->isState(geoscape) ||
                !geoscape->
                        canNavigateGlobeTargets3DS())
        {
                cancelCraftFollow3DS();
                return false;
        }

        /*
         * Confirm that the followed pointer still belongs to the
         * current saved game before dereferencing it.
         */
        MovingTarget *liveTarget = nullptr;

        for (Base *base :
                *game->getSavedGame()->getBases())
        {
                for (Craft *craft : *base->getCrafts())
                {
                        if (craft == followCraft3DS)
                        {
                                liveTarget = craft;
                                break;
                        }
                }

                if (liveTarget)
                {
                        break;
                }
        }

        if (!liveTarget)
        {
                for (Ufo *ufo :
                        *game->getSavedGame()->getUfos())
                {
                        if (ufo == followCraft3DS)
                        {
                                liveTarget = ufo;
                                break;
                        }
                }
        }

        if (!liveTarget)
        {
                cancelCraftFollow3DS();
                return false;
        }

        bool valid =
                liveTarget->getDestination() &&
                liveTarget->getSpeed() > 0 &&
                liveTarget->getMarker() != -1;

        Craft *craft =
                dynamic_cast<Craft *>(liveTarget);

        Ufo *ufo =
                dynamic_cast<Ufo *>(liveTarget);

        if (craft)
        {
                valid =
                        valid &&
                        craft->getStatus() ==
                                "STR_OUT";
        }
        else if (ufo)
        {
                valid =
                        valid &&
                        ufo->getDetected() &&
                        ufo->getStatus() ==
                                Ufo::FLYING;
        }
        else
        {
                valid = false;
        }

        if (!valid)
        {
                cancelCraftFollow3DS();
                return false;
        }

        Globe *globe = geoscape->getGlobe();
        Screen *screen = game->getScreen();

        if (!globe || !screen)
        {
                cancelCraftFollow3DS();
                return false;
        }

        const double longitude =
                liveTarget->getLongitude();

        const double latitude =
                liveTarget->getLatitude();

        if (!followPositionValid3DS ||
                longitude != followLongitude3DS ||
                latitude != followLatitude3DS)
        {
                globe->center(
                        longitude,
                        latitude);

                followLongitude3DS = longitude;
                followLatitude3DS = latitude;
                followPositionValid3DS = true;
        }

        Sint16 logicalX = 0;
        Sint16 logicalY = 0;

        globe->polarToCart(
                longitude,
                latitude,
                &logicalX,
                &logicalY);

        const double scaleX =
                screen->getXScale();

        const double scaleY =
                screen->getYScale();

        if (scaleX <= 0.0 || scaleY <= 0.0)
        {
                cancelCraftFollow3DS();
                return false;
        }

        const int physicalX =
                screen->getCursorLeftBlackBand() +
                static_cast<int>(
                        std::lround(
                                logicalX * scaleX));

        const int physicalY =
                screen->getCursorTopBlackBand() +
                static_cast<int>(
                        std::lround(
                                logicalY * scaleY));

        setCursor(
                video,
                physicalX,
                physicalY);

        return true;
}


void moveCursor(SDL_Surface *video, int dx, int dy)
{
        initializeCursor(video);

        if (dx == 0 && dy == 0)
        {
                return;
        }

        setCursor(
                video,
                cursorX + dx,
                cursorY + dy);
}

void pushKeyEvent(
        Uint8 type,
        SDLKey key,
        Uint16 unicode = 0)
{
        SDL_Event event = {};
        event.type = type;
        event.key.type = type;
        event.key.which = 0;
        event.key.state =
                type == SDL_KEYDOWN ?
                        SDL_PRESSED : SDL_RELEASED;
        event.key.keysym.scancode = 0;
        event.key.keysym.sym = key;
        event.key.keysym.mod = KMOD_NONE;
        event.key.keysym.unicode = unicode;

        SDL_PushEvent(&event);
}

void pushMouseButtonEvent(Uint8 type, Uint8 button)
{
        SDL_Event event = {};
        event.type = type;
        event.button.type = type;
        event.button.which = 0;
        event.button.button = button;
        event.button.state =
                type == SDL_MOUSEBUTTONDOWN ?
                        SDL_PRESSED : SDL_RELEASED;
        event.button.x = static_cast<Uint16>(cursorX);
        event.button.y = static_cast<Uint16>(cursorY);

        SDL_PushEvent(&event);
}

void pushMouseButtonEventAt(
        Uint8 type,
        Uint8 button,
        int x,
        int y)
{
        SDL_Event event = {};
        event.type = type;
        event.button.type = type;
        event.button.which =
                BottomScreen3DS::
                        BOTTOM_CONTROL_EVENT_WHICH;
        event.button.button = button;
        event.button.state =
                type == SDL_MOUSEBUTTONDOWN ?
                        SDL_PRESSED :
                        SDL_RELEASED;

        event.button.x =
                static_cast<Uint16>(x);

        event.button.y =
                static_cast<Uint16>(y);

        SDL_PushEvent(&event);
}

bool logicalTargetToPhysical(
        Game *game,
        int logicalX,
        int logicalY,
        int &physicalX,
        int &physicalY)
{
        if (!game ||
                !game->getScreen())
        {
                return false;
        }

        Screen *screen =
                game->getScreen();

        physicalX =
                screen->
                        getCursorLeftBlackBand() +
                static_cast<int>(
                        logicalX *
                                screen->getXScale() +
                        0.5);

        physicalY =
                screen->
                        getCursorTopBlackBand() +
                static_cast<int>(
                        logicalY *
                                screen->getYScale() +
                        0.5);

        physicalX =
                std::max(
                        0,
                        std::min(
                                screen->getWidth() - 1,
                                physicalX));

        physicalY =
                std::max(
                        0,
                        std::min(
                                screen->getHeight() - 1,
                                physicalY));

        return true;
}

bool physicalCursorToLogical3DS(
        Game *game,
        int physicalX,
        int physicalY,
        int &logicalX,
        int &logicalY)
{
        if (!game ||
                !game->getScreen())
        {
                return false;
        }

        Screen *screen =
                game->getScreen();

        const double scaleX =
                screen->getXScale();

        const double scaleY =
                screen->getYScale();

        if (scaleX <= 0.0 || scaleY <= 0.0)
        {
                return false;
        }

        logicalX =
                static_cast<int>(
                        std::lround(
                                (physicalX -
                                 screen->getCursorLeftBlackBand()) /
                                scaleX));

        logicalY =
                static_cast<int>(
                        std::lround(
                                (physicalY -
                                 screen->getCursorTopBlackBand()) /
                                scaleY));

        return true;
}


void beginTopPanelHandoff()
{
        topPanelHandoffPreviousBottomFocus =
                BottomScreen3DS::
                        isBottomCursorFocused();

        topPanelHandoffOriginMode =
                BottomScreen3DS::getMode();

        BottomScreen3DS::
                setBottomCursorFocused(false);

        topPanelHandoffPending = true;
        topPanelHandoffSawMenu = false;
        topPanelHandoffStartedAt = osGetTime();
}


void clearTopPanelHandoff()
{
        topPanelHandoffPending = false;
        topPanelHandoffSawMenu = false;

        topPanelHandoffOriginMode =
                BottomScreen3DS::MODE_MENU;
}

void updateTopPanelHandoff(Game *game)
{
        if (!topPanelHandoffPending)
        {
                return;
        }

        const BottomScreen3DS::Mode mode =
                BottomScreen3DS::getMode();

        bool originStillAvailable = false;

        if (topPanelHandoffOriginMode ==
                BottomScreen3DS::MODE_GEOSCAPE)
        {
                originStillAvailable =
                        game &&
                        game->getGeoscapeState();
        }
        else if (topPanelHandoffOriginMode ==
                BottomScreen3DS::MODE_BATTLESCAPE)
        {
                originStillAvailable =
                        game &&
                        game->getSavedGame() &&
                        game->getSavedGame()->
                                getSavedBattle();
        }

        /*
         * Cancel safely if the player left the originating game flow
         * instead of merely closing the opened menu.
         */
        if (!originStillAvailable)
        {
                clearTopPanelHandoff();
                return;
        }

        const bool showingOpenedPanel =
                (topPanelHandoffOriginMode ==
                        BottomScreen3DS::
                                MODE_GEOSCAPE &&
                 mode ==
                        BottomScreen3DS::
                                MODE_GEOSCAPE_FROZEN) ||
                (topPanelHandoffOriginMode ==
                        BottomScreen3DS::
                                MODE_BATTLESCAPE &&
                 mode ==
                        BottomScreen3DS::
                                MODE_MENU);

        if (showingOpenedPanel)
        {
                topPanelHandoffSawMenu = true;
                return;
        }

        if (mode == topPanelHandoffOriginMode &&
                topPanelHandoffSawMenu)
        {
                /*
                 * The opened menu has closed. Restore whichever screen
                 * owned cursor focus before the lower button was pressed.
                 */
                BottomScreen3DS::
                        setBottomCursorFocused(
                                topPanelHandoffPreviousBottomFocus);

                clearTopPanelHandoff();
                return;
        }

        /*
         * A transition to another unrelated gameplay mode means the
         * original panel will not be returning.
         */
        if (mode != topPanelHandoffOriginMode)
        {
                clearTopPanelHandoff();
                return;
        }

        /*
         * Avoid leaving focus stuck on top when a command fails to open
         * its expected menu.
         */
        constexpr u64 handoffTimeoutMs = 1500;

        if (!topPanelHandoffSawMenu &&
                osGetTime() -
                        topPanelHandoffStartedAt >=
                                handoffTimeoutMs)
        {
                BottomScreen3DS::
                        setBottomCursorFocused(
                                topPanelHandoffPreviousBottomFocus);

                clearTopPanelHandoff();
        }
}




void pushBottomCursorHoverEvent3DS(
        int x,
        int y)
{
        SDL_Event event = {};

        event.type = SDL_MOUSEMOTION;
        event.motion.type = SDL_MOUSEMOTION;
        event.motion.which =
                BottomScreen3DS::
                        BOTTOM_CONTROL_EVENT_WHICH;
        event.motion.state = 0;
        event.motion.x =
                static_cast<Uint16>(
                        x);
        event.motion.y =
                static_cast<Uint16>(
                        y);
        event.motion.xrel = 0;
        event.motion.yrel = 0;

        SDL_PushEvent(&event);
}

void updateBottomCursorHoverIfChanged3DS(Game *game)
{
        static bool lastValid = false;
        static int lastPhysicalX = -1;
        static int lastPhysicalY = -1;

        static BattlescapeState *
                zeroTUsHoverState = nullptr;

        BattlescapeState *battlescape =
                dynamic_cast<BattlescapeState *>(
                        game ?
                                game->getTopState() :
                                nullptr);

        bool onTftdZeroTUs = false;

        if (game &&
                battlescape &&
                !Keyboard3DS::isVisible() &&
                BottomScreen3DS::
                        isBottomCursorFocused() &&
                BottomScreen3DS::getMode() ==
                        BottomScreen3DS::
                                MODE_BATTLESCAPE &&
                Options::getActiveMaster() ==
                        "xcom2")
        {
                int bottomX = 0;
                int bottomY = 0;

                BottomScreen3DS::
                        getBottomCursorPosition(
                                bottomX,
                                bottomY);

                /*
                 * Exact displayed button is approximately
                 * x=12..27, y=47..79. Keep a few pixels of
                 * cursor-hotspot tolerance around it.
                 */
                onTftdZeroTUs =
                        bottomX >= 8 &&
                        bottomX < 31 &&
                        bottomY >= 44 &&
                        bottomY < 82;
        }

        if (onTftdZeroTUs)
        {
                if (zeroTUsHoverState &&
                        zeroTUsHoverState !=
                                battlescape)
                {
                        zeroTUsHoverState->
                                setZeroTUsHover3DS(
                                        false);
                }

                battlescape->
                        setZeroTUsHover3DS(true);

                zeroTUsHoverState =
                        battlescape;

                /*
                 * Do not send the old generalized hover event for
                 * this relocated control. Its original source-space
                 * hitbox does not correspond to its lower-screen
                 * location.
                 */
                lastValid = false;
                lastPhysicalX = -1;
                lastPhysicalY = -1;
                return;
        }

        if (zeroTUsHoverState)
        {
                zeroTUsHoverState->
                        setZeroTUsHover3DS(false);

                zeroTUsHoverState = nullptr;
        }

        if (!game ||
                Keyboard3DS::isVisible() ||
                !BottomScreen3DS::
                        isBottomCursorFocused())
        {
                lastValid = false;
                lastPhysicalX = -1;
                lastPhysicalY = -1;
                return;
        }

        int logicalX = 0;
        int logicalY = 0;
        bool opensTopPanel = false;

        int physicalX = 0;
        int physicalY = 0;

        if (!(BottomScreen3DS::
                getBottomCursorControlTarget(
                        logicalX,
                        logicalY,
                        opensTopPanel) &&
                logicalTargetToPhysical(
                        game,
                        logicalX,
                        logicalY,
                        physicalX,
                        physicalY)))
        {
                lastValid = false;
                lastPhysicalX = -1;
                lastPhysicalY = -1;
                return;
        }

        if (lastValid &&
                lastPhysicalX == physicalX &&
                lastPhysicalY == physicalY)
        {
                return;
        }

        lastValid = true;
        lastPhysicalX = physicalX;
        lastPhysicalY = physicalY;

        pushBottomCursorHoverEvent3DS(
                physicalX,
                physicalY);
}


void updateBottomControlTouch(
        Game *game,
        u32 down,
        u32 up)
{
        if (down & KEY_TOUCH)
        {
                /*
                 * A new touch always starts a new possible bottom
                 * control press.
                 */
                bottomTouchControlActive = false;

                if (!Keyboard3DS::isVisible())
                {
                        touchPosition touch = {};
                        hidTouchRead(&touch);

                        int logicalX = 0;
                        int logicalY = 0;
                        bool opensTopPanel = false;

                        if (BottomScreen3DS::
                                getControlTargetAt(
                                        touch.px,
                                        touch.py,
                                        logicalX,
                                        logicalY,
                                        opensTopPanel) &&
                                logicalTargetToPhysical(
                                        game,
                                        logicalX,
                                        logicalY,
                                        bottomTouchControlX,
                                        bottomTouchControlY))
                        {
                                bottomTouchControlActive =
                                        true;

                                pushMouseButtonEventAt(
                                        SDL_MOUSEBUTTONDOWN,
                                        SDL_BUTTON_LEFT,
                                        bottomTouchControlX,
                                        bottomTouchControlY);

                                if (opensTopPanel)
                                {
                                        beginTopPanelHandoff();
                                }
                        }
                }
        }

        if ((up & KEY_TOUCH) &&
                bottomTouchControlActive)
        {
                pushMouseButtonEventAt(
                        SDL_MOUSEBUTTONUP,
                        SDL_BUTTON_LEFT,
                        bottomTouchControlX,
                        bottomTouchControlY);

                bottomTouchControlActive = false;
        }
}


InventoryState *getActiveInventoryState3DS(Game *game)
{
        return dynamic_cast<InventoryState *>(
                game ? game->getTopState() : nullptr);
}


BattlescapeState *getActiveBattlescapeState3DS(Game *game)
{
        return dynamic_cast<BattlescapeState *>(
                game ? game->getTopState() : nullptr);
}


bool updateInventoryPhysicalButtons3DS(
        Game *game,
        u32 down,
        u32 up)
{
        InventoryState *inventory =
                getActiveInventoryState3DS(game);

        if (!inventory)
        {
                if (up & KEY_START)
                {
                        physicalInventoryStartDown = false;
                }

                return false;
        }

        bool consumed = false;

        if (down & KEY_L)
        {
                inventory->btnPrevClick(nullptr);
                consumed = true;
        }

        if (down & KEY_R)
        {
                inventory->btnNextClick(nullptr);
                consumed = true;
        }

        if (down & KEY_START)
        {
                inventory->btnOkClick(nullptr);
                physicalInventoryStartDown = true;
                consumed = true;
        }

        if ((up & KEY_START) &&
                physicalInventoryStartDown)
        {
                physicalInventoryStartDown = false;
                consumed = true;
        }

        return consumed;
}

bool updateBattlescapePhysicalButtons3DS(
        Game *game,
        u32 down)
{
        BattlescapeState *battlescape =
                getActiveBattlescapeState3DS(game);

        if (!battlescape)
        {
                return false;
        }

        bool consumed = false;

        /*
         * L/R are reserved for Battlescape view-level control on 3DS.
         * Unit cycling should use the bottom-screen controls instead.
         */
        return consumed;
}


bool updateBattlescapeCStickPan3DS(Game *game)
{
        BattlescapeState *battlescape =
                getActiveBattlescapeState3DS(game);

        static BattlescapeState *activeBattlescape = nullptr;

        if (activeBattlescape &&
                activeBattlescape != battlescape)
        {
                activeBattlescape->setCStickPan3DS(0, 0);
        }

        activeBattlescape = battlescape;

        if (!battlescape)
        {
                return false;
        }

        circlePosition cStick = {};
        hidCstickRead(&cStick);

        constexpr int deadzone = 20;

        /*
         * Lower than normal keyboard scroll speed so C-stick panning
         * is deliberate, but still timer-driven and smooth.
         */
        const int cStickBattlePanSpeed =
                std::max(
                        1,
                        (5 *
                         clampSpeedPercent3DS(
                                Options::
                                threeDSBattlescapeCameraSpeed) +
                         50) /
                                100);

        int scrollX = 0;
        int scrollY = 0;

        if (static_cast<int>(cStick.dx) < -deadzone)
        {
                scrollX = cStickBattlePanSpeed;
        }
        else if (static_cast<int>(cStick.dx) > deadzone)
        {
                scrollX = -cStickBattlePanSpeed;
        }

        if (static_cast<int>(cStick.dy) > deadzone)
        {
                scrollY = cStickBattlePanSpeed;
        }
        else if (static_cast<int>(cStick.dy) < -deadzone)
        {
                scrollY = -cStickBattlePanSpeed;
        }

        if (Options::threeDSInvertBattlescapeCameraX)
        {
                scrollX = -scrollX;
        }

        if (Options::threeDSInvertBattlescapeCameraY)
        {
                scrollY = -scrollY;
        }

        battlescape->setCStickPan3DS(scrollX, scrollY);

        return scrollX != 0 || scrollY != 0;
}


void updateGeoscapeViewControls(
        Game *game,
        u32 down,
        bool suppressShoulderActions)
{
        GeoscapeState *geoscape =
                game ?
                        game->getGeoscapeState() :
                        nullptr;

        BuildNewBaseState *buildNewBase =
                BuildNewBaseState::getActive3DS();

        /*
         * BuildNewBaseState is pushed over the normal Geoscape, but it
         * still operates the same Globe object. Confirm that it is the
         * current top state so controls do not leak into confirmation
         * or base-naming popups.
         */
        const bool placingNewBase =
                game &&
                buildNewBase &&
                game->isState(buildNewBase);

        Globe *globe =
                placingNewBase ?
                        buildNewBase->getGlobe3DS() :
                        (geoscape ?
                                geoscape->getGlobe() :
                                nullptr);

        const bool ordinaryGeoscape =
                geoscape &&
                game &&
                game->isState(geoscape) &&
                geoscape->
                        canNavigateGlobeTargets3DS();

        const bool active =
                ordinaryGeoscape ||
                placingNewBase;

        /*
         * Step through Geoscape time scales without wrapping.
         *
         * ZR = one step faster
         * ZL = one step slower
         */
        if (ordinaryGeoscape)
        {
                if (down & KEY_ZR)
                {
                        geoscape->
                                stepTimeSpeed3DS(1);
                }
                else if (down & KEY_ZL)
                {
                        geoscape->
                                stepTimeSpeed3DS(-1);
                }
        }

        if (!active || !globe)
        {
                if (cStickGlobe)
                {
                        cStickGlobe->rotateStop();
                }

                cStickGlobe = nullptr;
                cStickLongitudeDirection = 0;
                cStickLatitudeDirection = 0;
                return;
        }

        if (cStickGlobe != globe)
        {
                if (cStickGlobe)
                {
                        cStickGlobe->rotateStop();
                }

                cStickGlobe = globe;
                cStickLongitudeDirection = 0;
                cStickLatitudeDirection = 0;
        }

        circlePosition cStick = {};
        hidCstickRead(&cStick);

        constexpr int cStickDeadzone = 20;

        int requestedLongitude = 0;
        int requestedLatitude = 0;

        if (static_cast<int>(cStick.dx) <
                -cStickDeadzone)
        {
                requestedLongitude = -1;
        }
        else if (static_cast<int>(cStick.dx) >
                cStickDeadzone)
        {
                requestedLongitude = 1;
        }

        if (static_cast<int>(cStick.dy) >
                cStickDeadzone)
        {
                requestedLatitude = 1;
        }
        else if (static_cast<int>(cStick.dy) <
                -cStickDeadzone)
        {
                requestedLatitude = -1;
        }

        if (Options::threeDSInvertGeoscapeCameraX)
        {
                requestedLongitude =
                        -requestedLongitude;
        }

        if (Options::threeDSInvertGeoscapeCameraY)
        {
                requestedLatitude =
                        -requestedLatitude;
        }

        if (requestedLongitude !=
                cStickLongitudeDirection)
        {
                if (requestedLongitude < 0)
                {
                        globe->rotateLeft();
                }
                else if (requestedLongitude > 0)
                {
                        globe->rotateRight();
                }
                else
                {
                        globe->rotateStopLon();
                }

                cStickLongitudeDirection =
                        requestedLongitude;
        }

        if (requestedLatitude !=
                cStickLatitudeDirection)
        {
                if (requestedLatitude > 0)
                {
                        globe->rotateUp();
                }
                else if (requestedLatitude < 0)
                {
                        globe->rotateDown();
                }
                else
                {
                        globe->rotateStopLat();
                }

                cStickLatitudeDirection =
                        requestedLatitude;
        }

        if (requestedLongitude != 0 ||
                requestedLatitude != 0)
        {
                cancelCraftFollow3DS();
        }

        /*
         * Zoom only once per physical press. Holding a shoulder does
         * not race through every zoom level.
         */
        if (!suppressShoulderActions)
        {
                if (down & KEY_L)
                {
                        cancelCraftFollow3DS();
                        globe->zoomOut();
                }

                if (down & KEY_R)
                {
                        cancelCraftFollow3DS();
                        globe->zoomIn();
                }
        }
}

void updateCursor(
        SDL_Surface *video,
        Game *game,
        u32 held,
        u32 down)
{
        initializeCursor(video);

        circlePosition circle = {};
        hidCircleRead(&circle);

        int dx = 0;
        int dy = 0;

        constexpr int dpadSpeed = 6;
        constexpr int circleDeadzone = 20;

        /*
         * 100% preserves the original divisor of 24.
         * Higher percentages reduce the divisor and move faster.
         */
        const int circleDivisor =
                std::max(
                        1,
                        2400 /
                                getCursorSpeedPercent3DS(
                                        game));

        static int bottomCircleRemainderX = 0;
        static int bottomCircleRemainderY = 0;

        if (BottomScreen3DS::
                isBottomCursorFocused())
        {
                cancelCraftFollow3DS();
                const u32 directionMask =
                        KEY_DLEFT |
                        KEY_DRIGHT |
                        KEY_DUP |
                        KEY_DDOWN;

                const u32 directionDown =
                        down & directionMask;

                const u32 directionHeld =
                        held & directionMask;

                u32 requestedDirection = 0;

                if (directionDown & KEY_DLEFT)
                {
                        requestedDirection = KEY_DLEFT;
                }
                else if (directionDown & KEY_DRIGHT)
                {
                        requestedDirection = KEY_DRIGHT;
                }
                else if (directionDown & KEY_DUP)
                {
                        requestedDirection = KEY_DUP;
                }
                else if (directionDown & KEY_DDOWN)
                {
                        requestedDirection = KEY_DDOWN;
                }
                else if (bottomCursorRepeat.direction != 0 &&
                        (directionHeld &
                                bottomCursorRepeat.direction))
                {
                        requestedDirection =
                                bottomCursorRepeat.direction;
                }
                else if (directionHeld & KEY_DLEFT)
                {
                        requestedDirection = KEY_DLEFT;
                }
                else if (directionHeld & KEY_DRIGHT)
                {
                        requestedDirection = KEY_DRIGHT;
                }
                else if (directionHeld & KEY_DUP)
                {
                        requestedDirection = KEY_DUP;
                }
                else if (directionHeld & KEY_DDOWN)
                {
                        requestedDirection = KEY_DDOWN;
                }

                bool performSnap = false;

                if (requestedDirection == 0)
                {
                        bottomCursorRepeat.reset();
                }
                else
                {
                        const u64 now = osGetTime();

                        if (bottomCursorRepeat.setDirection(
                                requestedDirection,
                                now))
                        {
                                performSnap = true;
                        }
                        else
                        {
                                constexpr u64 initialDelayMs = 275;
                                constexpr u64 repeatIntervalMs = 90;

                                performSnap =
                                        bottomCursorRepeat.ready(
                                                now,
                                                initialDelayMs,
                                                repeatIntervalMs);
                        }
                }

                bool snapped = false;

                if (performSnap)
                {
                        if (requestedDirection == KEY_DLEFT)
                        {
                                snapped =
                                        BottomScreen3DS::
                                        snapBottomCursor(-1, 0);
                        }
                        else if (requestedDirection == KEY_DRIGHT)
                        {
                                snapped =
                                        BottomScreen3DS::
                                        snapBottomCursor(1, 0);
                        }
                        else if (requestedDirection == KEY_DUP)
                        {
                                snapped =
                                        BottomScreen3DS::
                                        snapBottomCursor(0, -1);
                        }
                        else if (requestedDirection == KEY_DDOWN)
                        {
                                snapped =
                                        BottomScreen3DS::
                                        snapBottomCursor(0, 1);
                        }
                }

                /*
                 * Circle Pad remains unrestricted for directly
                 * positioning the cursor anywhere on the lower screen.
                 */
                if (snapped)
                {
                        bottomCircleRemainderX = 0;
                        bottomCircleRemainderY = 0;
                }
                else
                {
                        const int circleX =
                                static_cast<int>(
                                        circle.dx);

                        const int circleY =
                                -static_cast<int>(
                                        circle.dy);

                        if (std::abs(circleX) >
                                circleDeadzone)
                        {
                                bottomCircleRemainderX +=
                                        circleX;

                                dx =
                                        bottomCircleRemainderX /
                                        circleDivisor;

                                bottomCircleRemainderX -=
                                        dx *
                                        circleDivisor;
                        }
                        else
                        {
                                bottomCircleRemainderX = 0;
                        }

                        if (std::abs(circleY) >
                                circleDeadzone)
                        {
                                bottomCircleRemainderY +=
                                        circleY;

                                dy =
                                        bottomCircleRemainderY /
                                        circleDivisor;

                                bottomCircleRemainderY -=
                                        dy *
                                        circleDivisor;
                        }
                        else
                        {
                                bottomCircleRemainderY = 0;
                        }

                        BottomScreen3DS::
                                moveBottomCursor(
                                        dx,
                                        dy);
                }

                sliderRepeat.reset();
                menuRepeat.reset();
                return;
        }

        /*
         * Do not carry a lower-screen repeat timer back into top-screen
         * cursor or ordinary menu navigation.
         */
        bottomCursorRepeat.reset();

        /*
         * On the active Geoscape, reserve the D-pad for directional
         * navigation between visible world markers. Circle Pad movement
         * remains unrestricted.
         */
        GeoscapeState *geoscape =
                game ?
                        game->getGeoscapeState() :
                        nullptr;

        const bool activeGeoscape =
                geoscape &&
                game->isState(geoscape);

        /*
         * Native 3DS version of middle-mouse globe dragging.
         *
         * The cursor remains stationary. Circle Pad movement is applied
         * directly to the globe, avoiding SDL's desktop mouse-warp loop.
         */
        const bool nativeGlobeDrag3DS =
                activeGeoscape &&
                geoscape->
                        canNavigateGlobeTargets3DS() &&
                (held & KEY_X) &&
                !((held & KEY_L) &&
                  (held & KEY_R));

        static int globeDragRemainderX3DS = 0;
        static int globeDragRemainderY3DS = 0;
        static bool globeDragWasActive3DS = false;

        if (nativeGlobeDrag3DS)
        {
                cancelCraftFollow3DS();

                const int circleX =
                        static_cast<int>(
                                circle.dx);

                const int circleY =
                        -static_cast<int>(
                                circle.dy);

                int dragX = 0;
                int dragY = 0;

                if (std::abs(circleX) >
                        circleDeadzone)
                {
                        globeDragRemainderX3DS +=
                                circleX;

                        dragX =
                                globeDragRemainderX3DS /
                                circleDivisor;

                        globeDragRemainderX3DS -=
                                dragX *
                                circleDivisor;
                }
                else
                {
                        globeDragRemainderX3DS = 0;
                }

                if (std::abs(circleY) >
                        circleDeadzone)
                {
                        globeDragRemainderY3DS +=
                                circleY;

                        dragY =
                                globeDragRemainderY3DS /
                                circleDivisor;

                        globeDragRemainderY3DS -=
                                dragY *
                                circleDivisor;
                }
                else
                {
                        globeDragRemainderY3DS = 0;
                }

                Globe *globe =
                        geoscape->getGlobe();

                if (globe)
                {
                        globe->
                                dragWithCirclePad3DS(
                                        dragX,
                                        dragY);
                }

                globeDragWasActive3DS = true;

                sliderRepeat.reset();
                menuRepeat.reset();
                return;
        }

        globeDragRemainderX3DS = 0;
        globeDragRemainderY3DS = 0;

        /*
         * Suppress one cursor-motion frame when X is released. This
         * keeps the pointer at its original location even if the Circle
         * Pad has not quite returned to center yet.
         */
        if (globeDragWasActive3DS)
        {
                globeDragWasActive3DS = false;

                sliderRepeat.reset();
                menuRepeat.reset();
                return;
        }

        const bool dogfightNavigationAvailable =
                activeGeoscape &&
                geoscape->
                        canNavigateDogfightControls3DS();

        const bool targetNavigationAvailable =
                activeGeoscape &&
                !dogfightNavigationAvailable &&
                geoscape->
                        canNavigateGlobeTargets3DS();

        const bool circleMoved =
                std::abs(static_cast<int>(circle.dx)) >
                        circleDeadzone ||
                std::abs(static_cast<int>(circle.dy)) >
                        circleDeadzone;


        if (dogfightNavigationAvailable)
        {
                /*
                 * An expanded dogfight is drawn and handled inside
                 * GeoscapeState, so it needs its own spatial snapping
                 * pass before ordinary globe-target navigation.
                 */
                cancelCraftFollow3DS();

                const u32 dogfightDirectionMask =
                        KEY_DLEFT |
                        KEY_DRIGHT |
                        KEY_DUP |
                        KEY_DDOWN;

                const u32 dogfightDirection =
                        down &
                        dogfightDirectionMask;

                int directionX = 0;
                int directionY = 0;

                if (dogfightDirection & KEY_DLEFT)
                {
                        directionX = -1;
                }
                else if (dogfightDirection & KEY_DRIGHT)
                {
                        directionX = 1;
                }
                else if (dogfightDirection & KEY_DUP)
                {
                        directionY = -1;
                }
                else if (dogfightDirection & KEY_DDOWN)
                {
                        directionY = 1;
                }

                if (directionX != 0 ||
                        directionY != 0)
                {
                        int snapX = cursorX;
                        int snapY = cursorY;

                        if (geoscape->
                                snapToDogfightControl3DS(
                                        directionX,
                                        directionY,
                                        cursorX,
                                        cursorY,
                                        snapX,
                                        snapY))
                        {
                                setCursor(
                                        video,
                                        snapX,
                                        snapY);
                        }
                }

                /*
                 * Do not turn a dogfight navigation press into free
                 * cursor movement when no candidate exists in that
                 * direction.
                 */
                if (held & dogfightDirectionMask)
                {
                        sliderRepeat.reset();
                        menuRepeat.reset();
                        return;
                }
        }

        if (!targetNavigationAvailable ||
                circleMoved)
        {
                cancelCraftFollow3DS();
        }

        if (targetNavigationAvailable)
        {
                const u32 targetDirectionMask =
                        KEY_DLEFT |
                        KEY_DRIGHT |
                        KEY_DUP |
                        KEY_DDOWN;

                const u32 targetDirection =
                        down & targetDirectionMask;

                int directionX = 0;
                int directionY = 0;

                if (targetDirection & KEY_DLEFT)
                {
                        directionX = -1;
                }
                else if (targetDirection & KEY_DRIGHT)
                {
                        directionX = 1;
                }
                else if (targetDirection & KEY_DUP)
                {
                        directionY = -1;
                }
                else if (targetDirection & KEY_DDOWN)
                {
                        directionY = 1;
                }

                if (directionX != 0 ||
                        directionY != 0)
                {
                        int snapX = cursorX;
                        int snapY = cursorY;

                        Target *selectedTarget =
                                geoscape->
                                snapToGlobeTarget3DS(
                                        directionX,
                                        directionY,
                                        cursorX,
                                        cursorY,
                                        snapX,
                                        snapY);

                        if (selectedTarget)
                        {
                                beginCraftFollow3DS(
                                        selectedTarget);

                                setCursor(
                                        video,
                                        snapX,
                                        snapY);
                        }
                }

                if (!circleMoved)
                {
                        updateCraftFollow3DS(
                                video,
                                game);
                }

                /*
                 * Do not fall through to ordinary free D-pad cursor
                 * movement while a Geoscape direction is held.
                 */
                if (held & targetDirectionMask)
                {
                        sliderRepeat.reset();
                        menuRepeat.reset();
                        return;
                }
        }

        bottomCircleRemainderX = 0;
        bottomCircleRemainderY = 0;

        /*
         * Battlescape D-pad snap phase 1:
         * snap only to meaningful unit targets, never tile-by-tile.
         */
        const u32 battlescapeSnapMask =
                KEY_DLEFT |
                KEY_DRIGHT |
                KEY_DUP |
                KEY_DDOWN;

        BattlescapeState *battlescape =
                getActiveBattlescapeState3DS(game);

        if (battlescape &&
                !menuNavigationActive &&
                !textEditingActive)
        {
                const u32 snapDirection =
                        down & battlescapeSnapMask;

                int directionX = 0;
                int directionY = 0;

                if (snapDirection & KEY_DLEFT)
                {
                        directionX = -1;
                }
                else if (snapDirection & KEY_DRIGHT)
                {
                        directionX = 1;
                }
                else if (snapDirection & KEY_DUP)
                {
                        directionY = -1;
                }
                else if (snapDirection & KEY_DDOWN)
                {
                        directionY = 1;
                }

                if (directionX != 0 || directionY != 0)
                {
                        int currentLogicalX = 0;
                        int currentLogicalY = 0;
                        int snapLogicalX = 0;
                        int snapLogicalY = 0;
                        int snapPhysicalX = cursorX;
                        int snapPhysicalY = cursorY;

                        if (physicalCursorToLogical3DS(
                                        game,
                                        cursorX,
                                        cursorY,
                                        currentLogicalX,
                                        currentLogicalY) &&
                                battlescape->snapVisibleUnit3DS(
                                        directionX,
                                        directionY,
                                        currentLogicalX,
                                        currentLogicalY,
                                        snapLogicalX,
                                        snapLogicalY) &&
                                logicalTargetToPhysical(
                                        game,
                                        snapLogicalX,
                                        snapLogicalY,
                                        snapPhysicalX,
                                        snapPhysicalY))
                        {
                                setCursor(
                                        video,
                                        snapPhysicalX,
                                        snapPhysicalY);
                        }
                }

                /*
                 * While in Battlescape, D-pad is snap-only. Do not
                 * fall through into pixel cursor drift while held.
                 */
                if (held & battlescapeSnapMask)
                {
                        sliderRepeat.reset();
                        menuRepeat.reset();
                        return;
                }
        }

        if (!menuNavigationActive && !textEditingActive)
        {
                if (held & KEY_DLEFT)
                {
                        dx -= dpadSpeed;
                }
                if (held & KEY_DRIGHT)
                {
                        dx += dpadSpeed;
                }
                if (held & KEY_DUP)
                {
                        dy -= dpadSpeed;
                }
                if (held & KEY_DDOWN)
                {
                        dy += dpadSpeed;
                }
        }

        int analogTargetX = cursorX;
        int analogTargetY = cursorY;

        const bool analogControlCaptured =
                menuNavigationActive &&
                MenuNavigation3DS::
                        adjustAnalogControlAtCursor(
                                game,
                                cursorX,
                                cursorY,
                                static_cast<int>(
                                        circle.dx),
                                static_cast<int>(
                                        circle.dy),
                                analogTargetX,
                                analogTargetY);

        if (analogControlCaptured)
        {
                setCursor(
                        video,
                        analogTargetX,
                        analogTargetY);
        }
        else
        {
                if (std::abs(
                        static_cast<int>(
                                circle.dx)) >
                        circleDeadzone)
                {
                        dx +=
                                static_cast<int>(
                                        circle.dx) /
                                circleDivisor;
                }

                if (std::abs(
                        static_cast<int>(
                                circle.dy)) >
                        circleDeadzone)
                {
                        /*
                         * 3DS positive Y is upward;
                         * SDL positive Y is downward.
                         */
                        dy -=
                                static_cast<int>(
                                        circle.dy) /
                                circleDivisor;
                }
        }

        moveCursor(video, dx, dy);

        if (textEditingActive)
        {
                sliderRepeat.reset();
                menuRepeat.reset();
                return;
        }

        if (!menuNavigationActive)
        {
                menuRepeat.reset();
                return;
        }

        MenuNavigation3DS::Direction direction =
                MenuNavigation3DS::UP;

        bool directionPressed = false;
        int repeatSteps = 1;
        u32 requestedDirection = 0;

        const bool sliderAdjusting =
                MenuNavigation3DS::isAdjustingSlider();

        const bool inventoryNavigationActive =
                MenuNavigation3DS::isInventory(game);

        /*
         * Inventory uses held repeat in all four directions. Ordinary
         * menus retain their existing one-shot Left/Right behavior.
         */
        if (inventoryNavigationActive)
        {
                sliderRepeat.reset();

                const u32 directionMask =
                        KEY_DLEFT |
                        KEY_DRIGHT |
                        KEY_DUP |
                        KEY_DDOWN;

                const u32 directionHeld =
                        held & directionMask;

                const u32 directionDown =
                        down & directionMask;

                if (directionDown & KEY_DLEFT)
                {
                        requestedDirection = KEY_DLEFT;
                }
                else if (directionDown & KEY_DRIGHT)
                {
                        requestedDirection = KEY_DRIGHT;
                }
                else if (directionDown & KEY_DUP)
                {
                        requestedDirection = KEY_DUP;
                }
                else if (directionDown & KEY_DDOWN)
                {
                        requestedDirection = KEY_DDOWN;
                }
                else if (menuRepeat.direction != 0 &&
                        (directionHeld &
                                menuRepeat.direction))
                {
                        requestedDirection =
                                menuRepeat.direction;
                }
                else if (directionHeld & KEY_DLEFT)
                {
                        requestedDirection = KEY_DLEFT;
                }
                else if (directionHeld & KEY_DRIGHT)
                {
                        requestedDirection = KEY_DRIGHT;
                }
                else if (directionHeld & KEY_DUP)
                {
                        requestedDirection = KEY_DUP;
                }
                else if (directionHeld & KEY_DDOWN)
                {
                        requestedDirection = KEY_DDOWN;
                }

                if (requestedDirection == 0)
                {
                        menuRepeat.reset();
                }
                else
                {
                        const u64 now = osGetTime();

                        if (menuRepeat.setDirection(
                                requestedDirection,
                                now))
                        {
                                directionPressed = true;
                        }
                        else
                        {
                                constexpr u64 initialDelayMs =
                                        275;

                                constexpr u64 repeatIntervalMs =
                                        90;

                                directionPressed =
                                        menuRepeat.ready(
                                                now,
                                                initialDelayMs,
                                                repeatIntervalMs);
                        }
                }
        }
        else if (sliderAdjusting)
        {
                menuRepeat.reset();

                /*
                 * Sliders support accelerated Left/Right hold-repeat.
                 * Up and Down remain inactive until B exits adjustment
                 * mode.
                 */
                const u32 sliderHeld =
                        held & (KEY_DLEFT | KEY_DRIGHT);

                const u32 sliderDown =
                        down & (KEY_DLEFT | KEY_DRIGHT);

                if (sliderDown & KEY_DLEFT)
                {
                        requestedDirection = KEY_DLEFT;
                }
                else if (sliderDown & KEY_DRIGHT)
                {
                        requestedDirection = KEY_DRIGHT;
                }
                else if (sliderHeld & sliderRepeat.direction)
                {
                        requestedDirection =
                                sliderRepeat.direction;
                }
                else if (sliderHeld & KEY_DLEFT)
                {
                        requestedDirection = KEY_DLEFT;
                }
                else if (sliderHeld & KEY_DRIGHT)
                {
                        requestedDirection = KEY_DRIGHT;
                }

                if (requestedDirection == 0)
                {
                        sliderRepeat.reset();
                }
                else
                {
                        const u64 now = osGetTime();

                        if (sliderRepeat.setDirection(
                                requestedDirection,
                                now))
                        {
                                /*
                                 * A newly pressed direction changes one
                                 * step immediately.
                                 */
                                directionPressed = true;
                        }
                        else
                        {
                                const u64 heldFor =
                                        sliderRepeat.heldFor(now);

                                constexpr u64 initialDelayMs =
                                        200;

                                u64 repeatIntervalMs = 55;

                                if (heldFor >= 1400)
                                {
                                        repeatIntervalMs = 25;
                                        repeatSteps = 6;
                                }
                                else if (heldFor >= 650)
                                {
                                        repeatIntervalMs = 35;
                                        repeatSteps = 3;
                                }

                                directionPressed =
                                        sliderRepeat.ready(
                                                now,
                                                initialDelayMs,
                                                repeatIntervalMs);
                        }
                }
        }
        else
        {
                /*
                 * Ordinary menu navigation repeats Up/Down while held.
                 * Left/Right remain one action per physical press.
                 */
                sliderRepeat.reset();

                const u32 verticalHeld =
                        held & (KEY_DUP | KEY_DDOWN);

                const u32 verticalDown =
                        down & (KEY_DUP | KEY_DDOWN);

                const u32 horizontalDown =
                        down & (KEY_DLEFT | KEY_DRIGHT);

                if (horizontalDown & KEY_DLEFT)
                {
                        requestedDirection = KEY_DLEFT;
                        directionPressed = true;

                        menuRepeat.reset();
                }
                else if (horizontalDown & KEY_DRIGHT)
                {
                        requestedDirection = KEY_DRIGHT;
                        directionPressed = true;

                        menuRepeat.reset();
                }
                else if (verticalDown & KEY_DUP)
                {
                        requestedDirection = KEY_DUP;
                        directionPressed = true;

                        menuRepeat.setDirection(
                                requestedDirection,
                                osGetTime());
                }
                else if (verticalDown & KEY_DDOWN)
                {
                        requestedDirection = KEY_DDOWN;
                        directionPressed = true;

                        menuRepeat.setDirection(
                                requestedDirection,
                                osGetTime());
                }
                else if (verticalHeld)
                {
                        requestedDirection =
                                verticalHeld & KEY_DUP ?
                                        KEY_DUP :
                                        KEY_DDOWN;

                        const u64 now = osGetTime();

                        if (menuRepeat.setDirection(
                                requestedDirection,
                                now))
                        {
                                /*
                                 * Changing direction while the D-pad
                                 * remains held moves immediately.
                                 */
                                directionPressed = true;
                        }
                        else
                        {
                                const u64 heldFor =
                                        menuRepeat.heldFor(now);

                                constexpr u64 initialDelayMs =
                                        275;

                                const u64 repeatIntervalMs =
                                        heldFor >= 1200 ?
                                                45 : 85;

                                directionPressed =
                                        menuRepeat.ready(
                                                now,
                                                initialDelayMs,
                                                repeatIntervalMs);
                        }
                }
                else
                {
                        menuRepeat.reset();
                }
        }

        if (!directionPressed)
        {
                return;
        }

        if (requestedDirection == KEY_DUP)
        {
                direction = MenuNavigation3DS::UP;
        }
        else if (requestedDirection == KEY_DDOWN)
        {
                direction = MenuNavigation3DS::DOWN;
        }
        else if (requestedDirection == KEY_DLEFT)
        {
                direction = MenuNavigation3DS::LEFT;
        }
        else if (requestedDirection == KEY_DRIGHT)
        {
                direction = MenuNavigation3DS::RIGHT;
        }
        else
        {
                return;
        }

        int targetX = cursorX;
        int targetY = cursorY;
        bool moved = false;

        /*
         * Faster slider stages perform several value changes during
         * each repeat. Normal controls always use repeatSteps == 1.
         */
        for (int step = 0; step < repeatSteps; ++step)
        {
                int nextX = targetX;
                int nextY = targetY;

                if (!MenuNavigation3DS::findTarget(
                        game,
                        direction,
                        targetX,
                        targetY,
                        nextX,
                        nextY))
                {
                        break;
                }

                targetX = nextX;
                targetY = nextY;
                moved = true;
        }

        if (moved)
        {
                setCursor(video, targetX, targetY);
        }
}

void updateTrackpad(
        SDL_Surface *video,
        u32 held,
        u32 down,
        u32 up)
{
        (void)down;
        auto releaseTrackpadButton = []()
        {
                if (!trackpadButtonDown)
                {
                        return;
                }

                pushMouseButtonEvent(
                        SDL_MOUSEBUTTONUP,
                        trackpadButton);

                trackpadButtonDown = false;
        };

        if (Keyboard3DS::isVisible() ||
                MenuNavigation3DS::isAdjustingSlider())
        {
                releaseTrackpadButton();

                trackpadDragActive = false;
                trackpadMoved = false;
                trackpadStartedAt = 0;
                return;
        }

        /*
         * A quick stationary touch is a normal click. Holding the
         * touch still sends mouse-down after a short delay and keeps
         * it down until the touchscreen is released.
         */
        if (!(held & KEY_TOUCH))
        {
                if ((up & KEY_TOUCH) &&
                        trackpadDragActive)
                {
                        if (trackpadButtonDown)
                        {
                                releaseTrackpadButton();
                        }
                        else if (!trackpadMoved)
                        {
                                pushMouseButtonEvent(
                                        SDL_MOUSEBUTTONDOWN,
                                        trackpadButton);

                                pushMouseButtonEvent(
                                        SDL_MOUSEBUTTONUP,
                                        trackpadButton);
                        }
                }
                else
                {
                        releaseTrackpadButton();
                }

                trackpadDragActive = false;
                trackpadMoved = false;
                trackpadStartedAt = 0;
                return;
        }

        touchPosition touch = {};
        hidTouchRead(&touch);

        if (!trackpadDragActive)
        {
                if (!BottomScreen3DS::isTrackpadPoint(
                        touch.px,
                        touch.py))
                {
                        return;
                }

                BottomScreen3DS::
                        setBottomCursorFocused(false);

                trackpadStartX = touch.px;
                trackpadStartY = touch.py;

                previousTouchX = touch.px;
                previousTouchY = touch.py;

                trackpadMoved = false;
                trackpadButtonDown = false;
                trackpadStartedAt = osGetTime();

                /*
                 * Shoulder buttons now control Geoscape zoom.
                 * Trackpad gestures remain ordinary left-button
                 * gestures; hold physical B for right-dragging.
                 */
                trackpadButton = SDL_BUTTON_LEFT;

                trackpadDragActive = true;
                return;
        }

        int dx =
                static_cast<int>(touch.px) -
                previousTouchX;

        int dy =
                static_cast<int>(touch.py) -
                previousTouchY;

        const int totalX =
                static_cast<int>(touch.px) -
                trackpadStartX;

        const int totalY =
                static_cast<int>(touch.py) -
                trackpadStartY;

        const int dragThreshold =
                std::max(
                        1,
                        std::min(
                                20,
                                Options::
                                        threeDSTouchpadDragThreshold));

        if (!trackpadMoved &&
                (std::abs(totalX) > dragThreshold ||
                 std::abs(totalY) > dragThreshold))
        {
                /*
                 * Include the distance accumulated before crossing the
                 * movement threshold.
                 */
                trackpadMoved = true;
                dx = totalX;
                dy = totalY;
        }

        previousTouchX = touch.px;
        previousTouchY = touch.py;

        if (!trackpadMoved)
        {
                const u64 holdDelayMs =
                        static_cast<u64>(
                                std::max(
                                        100,
                                        std::min(
                                                600,
                                                Options::
                                                threeDSTouchpadHoldDelay)));

                if (!trackpadButtonDown &&
                        osGetTime() -
                                trackpadStartedAt >=
                                holdDelayMs)
                {
                        pushMouseButtonEvent(
                                SDL_MOUSEBUTTONDOWN,
                                trackpadButton);

                        trackpadButtonDown = true;
                }

                return;
        }

        /*
         * Moving after the hold threshold drags with the mouse button
         * still down. Moving before the threshold remains ordinary
         * trackpad cursor movement.
         */
        /*
         * 100% preserves the original 5/4 movement scale.
         * The option scales that established baseline rather than
         * replacing it with raw touchscreen movement.
         */
        const int sensitivityPercent =
                std::max(
                        50,
                        std::min(
                                200,
                                Options::
                                        threeDSTouchpadSensitivity));

        constexpr int sensitivityNumerator = 5;
        constexpr int sensitivityDenominator = 400;

        dx =
                dx *
                sensitivityNumerator *
                sensitivityPercent /
                sensitivityDenominator;

        dy =
                dy *
                sensitivityNumerator *
                sensitivityPercent /
                sensitivityDenominator;

        moveCursor(video, dx, dy);
}

void releaseKeyboardTouch()
{
        if (!keyboardTouchActive)
        {
                return;
        }

        pushKeyEvent(
                SDL_KEYUP,
                keyboardTouchKey,
                keyboardTouchUnicode);

        keyboardTouchActive = false;
        keyboardTouchKey = SDLK_UNKNOWN;
        keyboardTouchUnicode = 0;

        Keyboard3DS::clearPressedKey();
}

/* OXCE_3DS_DIRECT_BINDING_TOUCH */
bool activateKeyboardKey3DS(
        SDLKey key,
        Uint16 unicode)
{
        if (key == SDLK_UNKNOWN)
        {
                return false;
        }

        if (Keyboard3DS::getMode() ==
                Keyboard3DS::MODE_BINDING)
        {
                Keyboard3DS::setPressedKey(key);

                const bool assigned =
                        OptionsControlsState::
                                assignKey3DS(key);

                Keyboard3DS::clearPressedKey();

                if (assigned)
                {
                        keyBindingActive = false;

                        Keyboard3DS::setVisible(false);
                        Keyboard3DS::setMode(
                                Keyboard3DS::MODE_TEXT);
                }

                /*
                 * The binding keyboard owns and consumes this activation.
                 */
                return true;
        }

        return false;
}


void updateKeyboardTouch(
        SDL_Surface *video,
        u32 down,
        u32 up)
{
        /*
         * One physical touchscreen press produces at most one key.
         * Moving the stylus or finger across other keys does not type
         * again until the screen is released and pressed anew.
         */
        if (down & KEY_TOUCH)
        {
                releaseKeyboardTouch();

                if (Keyboard3DS::isVisible())
                {
                        touchPosition touch = {};
                        hidTouchRead(&touch);

                        SDLKey key = SDLK_UNKNOWN;
                        Uint16 unicode = 0;

                        if (Keyboard3DS::getKeyAt(
                                touch.px,
                                touch.py,
                                video->w,
                                key,
                                unicode))
                        {
                                if (!activateKeyboardKey3DS(
                                        key,
                                        unicode))
                                {
                                        keyboardTouchActive = true;
                                        keyboardTouchKey = key;
                                        keyboardTouchUnicode = unicode;

                                        Keyboard3DS::setPressedKey(key);

                                        pushKeyEvent(
                                                SDL_KEYDOWN,
                                                key,
                                                unicode);
                                }
                        }
                }
        }

        if (up & KEY_TOUCH)
        {
                releaseKeyboardTouch();
        }
}

}

/**
 * Reports the physical X-button state used for synthetic
 * middle-mouse input.
 */
bool isMiddleMouseHeld()
{
        return physicalMiddleMouseDown;
}


static void pushKeyboardEvent3DS(
		SDLKey key,
		Uint8 type)
{
	if (key == SDLK_UNKNOWN)
	{
		return;
	}

	SDL_Event event = {};
	event.type = type;
	event.key.type = type;
	event.key.state =
		type == SDL_KEYDOWN ?
			SDL_PRESSED :
			SDL_RELEASED;
	event.key.keysym.sym = key;
	event.key.keysym.mod = KMOD_NONE;
	event.key.keysym.unicode = 0;

	SDL_PushEvent(&event);
}

static void tapKeyboardKey3DS(
		SDLKey key)
{
	pushKeyboardEvent3DS(key, SDL_KEYDOWN);
	pushKeyboardEvent3DS(key, SDL_KEYUP);
}

static void handleUnitInfoShoulderHotkeys3DS(
		Game *game,
		u32 down,
		bool textEditingActive)
{
	if (textEditingActive ||
		Keyboard3DS::isVisible() ||
		!game)
	{
		return;
	}

	UnitInfoState *unitInfo =
		dynamic_cast<UnitInfoState *>(
			game->getTopState());

	if (!unitInfo)
	{
		return;
	}

	/*
	 * Call UnitInfoState's existing handlers directly.
	 * This avoids any platform-specific key-binding conflicts.
	 */
	if (down & KEY_L)
	{
		unitInfo->btnPrevClick(nullptr);
	}

	if (down & KEY_R)
	{
		unitInfo->btnNextClick(nullptr);
	}
}


static void handleBattlescapeShoulderHotkeys3DS(
		Game *game,
		u32 down,
		bool textEditingActive)
{
	if (textEditingActive ||
		Keyboard3DS::isVisible() ||
		!MenuNavigation3DS::isBattlescape(game))
	{
		return;
	}

	if (down & KEY_L)
	{
		tapKeyboardKey3DS(
			static_cast<SDLKey>(
				Options::keyBattleLevelDown));
	}

	if (down & KEY_R)
	{
		tapKeyboardKey3DS(
			static_cast<SDLKey>(
				Options::keyBattleLevelUp));
	}

	if (down & KEY_ZL)
	{
		tapKeyboardKey3DS(
			static_cast<SDLKey>(
				Options::keyBattleUseLeftHand));
	}

	if (down & KEY_ZR)
	{
		tapKeyboardKey3DS(
			static_cast<SDLKey>(
				Options::keyBattleUseRightHand));
	}
}


void pump(Game *game)
{
        hidScanInput();

        const u32 held = hidKeysHeld();

        if (!buttonStateInitialized)
        {
                previousHeld = held;
                buttonStateInitialized = true;
        }

        const u32 down = held & ~previousHeld;
        const u32 up = previousHeld & ~held;
        previousHeld = held;

        SDL_Surface *video = SDL_GetVideoSurface();

        if (!video)
        {
                return;
        }

        /*
         * Press all four New 3DS shoulder buttons to toggle the complete
         * desktop-style keyboard. Requiring at least one newly pressed
         * button prevents the chord from toggling repeatedly while held.
         */
        constexpr u32 FULL_KEYBOARD_CHORD =
                KEY_ZL |
                KEY_L |
                KEY_R |
                KEY_ZR;

        const bool fullKeyboardChordPressed =
                (held & FULL_KEYBOARD_CHORD) ==
                        FULL_KEYBOARD_CHORD &&
                (down & FULL_KEYBOARD_CHORD) != 0;

        /*
         * Do not allow the final shoulder press that completes the chord
         * to trigger its ordinary gameplay action.
         */
        const u32 controlDown =
                fullKeyboardChordPressed ?
                        down & ~FULL_KEYBOARD_CHORD :
                        down;

        mouseTopHeight = getCursorHeight(video);

        BottomScreen3DS::setGame(game);

        if (MenuNavigation3DS::isGeoscape(game))
        {
                BottomScreen3DS::setMode(
                        BottomScreen3DS::MODE_GEOSCAPE);
        }
        else if (MenuNavigation3DS::
                isInventory(game))
        {
                BottomScreen3DS::setMode(
                        BottomScreen3DS::
                                MODE_INVENTORY);
        }
        else if (MenuNavigation3DS::isBattlescape(game))
        {
                BottomScreen3DS::setMode(
                        BottomScreen3DS::MODE_BATTLESCAPE);
        }
        else if (game &&
                game->getGeoscapeState() &&
                (!game->getSavedGame() ||
                 !game->getSavedGame()->getSavedBattle()))
        {
                BottomScreen3DS::setMode(
                        BottomScreen3DS::
                                MODE_GEOSCAPE_FROZEN);
        }
        else
        {
                BottomScreen3DS::setMode(
                        BottomScreen3DS::MODE_MENU);
        }

        updateTopPanelHandoff(game);

        TextEdit *focusedTextEdit =
                MenuNavigation3DS::
                        getFocusedTextEdit(game);

        textEditingActive =
                focusedTextEdit != nullptr;

        /* OXCE_3DS_BINDING_STATE_UPDATE */
        keyBindingActive =
                OptionsControlsState::
                        isWaitingForKey3DS();

        /*
         * A newly focused field opens the keyboard automatically.
         * Once the user hides it, keep it hidden until requested or
         * until focus moves to another TextEdit.
         */
        if (focusedTextEdit != trackedTextEdit)
        {
                trackedTextEdit = focusedTextEdit;
                keyboardHiddenByUser = false;
        }

        if (!textEditingActive)
        {
                keyboardHiddenByUser = false;
        }

        /* OXCE_3DS_BINDING_KEYBOARD_VISIBILITY */
        if (keyBindingActive)
        {
                /*
                 * A real Controls binding operation always takes priority
                 * over a manually opened keyboard.
                 */
                if (fullKeyboardVisible)
                {
                        releaseKeyboardTouch();
                        fullKeyboardVisible = false;
                }
        }
        else if (fullKeyboardChordPressed)
        {
                if (!fullKeyboardVisible)
                {
                        fullKeyboardPreviousBottomFocus =
                                BottomScreen3DS::
                                        isBottomCursorFocused();
                }
                else
                {
                        releaseKeyboardTouch();

                        /*
                         * Do not immediately reopen the smaller text
                         * keyboard when this was closed over a TextEdit.
                         */
                        keyboardHiddenByUser =
                                textEditingActive;
                }

                fullKeyboardVisible =
                        !fullKeyboardVisible;
        }

        Keyboard3DS::setMode(
                keyBindingActive ?
                        Keyboard3DS::MODE_BINDING :
                        (fullKeyboardVisible ?
                                Keyboard3DS::MODE_FULL :
                                Keyboard3DS::MODE_TEXT));

        Keyboard3DS::setVisible(
                keyBindingActive ||
                fullKeyboardVisible ||
                (textEditingActive &&
                 !keyboardHiddenByUser));

        /*
         * setBottomCursorFocused() permits menu-screen focus only while
         * the keyboard is already visible, so do this after setVisible().
         */
        if (fullKeyboardChordPressed &&
                !keyBindingActive)
        {
                BottomScreen3DS::
                        setBottomCursorFocused(
                                fullKeyboardVisible ?
                                        true :
                                        fullKeyboardPreviousBottomFocus);
        }

	handleUnitInfoShoulderHotkeys3DS(
		game,
		controlDown,
		textEditingActive);

	handleBattlescapeShoulderHotkeys3DS(
		game,
		controlDown,
		textEditingActive);


        GeoscapeState *activeGeoscape3DS =
                game ?
                        game->getGeoscapeState() :
                        nullptr;

        const bool dogfightNavigationActive =
                activeGeoscape3DS &&
                game->isState(activeGeoscape3DS) &&
                activeGeoscape3DS->
                        canNavigateDogfightControls3DS();

        /* OXCE_3DS_KEYBOARD_CURSOR_FOCUS */
        if (Keyboard3DS::isVisible())
        {
                if (down & KEY_SELECT)
                {
                        BottomScreen3DS::
                                toggleCursorFocus();
                }
        }
        else if (textEditingActive ||
                dogfightNavigationActive ||
                BottomScreen3DS::getMode() ==
                        BottomScreen3DS::MODE_MENU)
        {
                BottomScreen3DS::
                        setBottomCursorFocused(false);
        }
        else if (down & KEY_SELECT)
        {
                BottomScreen3DS::
                        toggleCursorFocus();
        }

        /* OXCE_3DS_BINDING_MENU_NAVIGATION */
        menuNavigationActive =
                !textEditingActive &&
                !keyBindingActive &&
                !Keyboard3DS::isVisible() &&
                MenuNavigation3DS::isActive(game);

        if (!eventFilterInstalled)
        {
                SDL_SetEventFilter(filter3DSEvent);
                eventFilterInstalled = true;
        }

        /*
         * When a dropdown has just opened, visibly place the cursor on
         * its current selection before processing further navigation.
         */
        int comboTargetX = cursorX;
        int comboTargetY = cursorY;

        if (MenuNavigation3DS::
                focusNewlyOpenedComboBox(
                        game,
                        comboTargetX,
                        comboTargetY))
        {
                setCursor(
                        video,
                        comboTargetX,
                        comboTargetY);
        }

        if (down & KEY_TOUCH)
        {
                cancelCraftFollow3DS();
        }

        const bool inventoryPhysicalButtonConsumed =
                updateInventoryPhysicalButtons3DS(
                        game,
                        controlDown,
                        up);

        const bool battlescapePhysicalButtonConsumed =
                updateBattlescapePhysicalButtons3DS(
                        game,
                        controlDown);

        updateBattlescapeCStickPan3DS(game);

        updateGeoscapeViewControls(
                game,
                controlDown,
                        fullKeyboardChordPressed ||
                        inventoryPhysicalButtonConsumed ||
                        battlescapePhysicalButtonConsumed);

        updateCursor(video, game, held, down);
        updateBottomCursorHoverIfChanged3DS(game);
        /* OXCE_3DS_KEYBOARD_TOUCH_ROUTING */
        if (Keyboard3DS::isVisible())
        {
                /*
                 * updateTrackpad() clears any gesture that began before
                 * the keyboard appeared, then immediately returns.
                 */
                updateTrackpad(
                        video,
                        held,
                        down,
                        up);

                updateKeyboardTouch(
                        video,
                        down,
                        up);
        }
        else
        {
                updateBottomControlTouch(
                        game,
                        down,
                        up);

                updateTrackpad(
                        video,
                        held,
                        down,
                        up);
        }

        if (down & KEY_A)
        {
                actionConsumedByKeyboard = false;
                actionConsumedByBottomCursor = false;

                if (BottomScreen3DS::
                        isBottomCursorFocused())
                {
                        /* OXCE_3DS_KEYBOARD_A_ACTIVATION */
                        if (Keyboard3DS::isVisible())
                        {
                                actionConsumedByBottomCursor = true;
                                bottomCursorControlActive = false;

                                int keyboardX = 0;
                                int keyboardY = 0;

                                BottomScreen3DS::
                                        getBottomCursorPosition(
                                                keyboardX,
                                                keyboardY);

                                SDLKey key = SDLK_UNKNOWN;
                                Uint16 unicode = 0;

                                if (Keyboard3DS::getKeyAt(
                                        keyboardX,
                                        keyboardY,
                                        video->w,
                                        key,
                                        unicode))
                                {
                                        if (!activateKeyboardKey3DS(
                                                key,
                                                unicode))
                                        {
                                                Keyboard3DS::setPressedKey(key);

                                                pushKeyEvent(
                                                        SDL_KEYDOWN,
                                                        key,
                                                        unicode);

                                                pushKeyEvent(
                                                        SDL_KEYUP,
                                                        key,
                                                        unicode);

                                                Keyboard3DS::clearPressedKey();
                                        }
                                }
                        }
                        else
                        {
                                /*
                                 * The bottom cursor's top-left tip is its
                                 * hotspot, matching the normal OXCE cursor.
                                 */
                                actionConsumedByBottomCursor = true;
                                bottomCursorControlActive = false;

                                int logicalX = 0;
                                int logicalY = 0;
                                bool opensTopPanel = false;

                                if (BottomScreen3DS::
                                        getBottomCursorControlTarget(
                                                logicalX,
                                                logicalY,
                                                opensTopPanel) &&
                                        logicalTargetToPhysical(
                                                game,
                                                logicalX,
                                                logicalY,
                                                bottomCursorControlX,
                                                bottomCursorControlY))
                                {
                                        bottomCursorControlActive =
                                                true;

                                        pushMouseButtonEventAt(
                                                SDL_MOUSEBUTTONDOWN,
                                                SDL_BUTTON_LEFT,
                                                bottomCursorControlX,
                                                bottomCursorControlY);

                                        if (opensTopPanel)
                                        {
                                                beginTopPanelHandoff();
                                        }
                                }
                        }
                }
                /* OXCE_3DS_BINDING_A_GUARD */
                else if (keyBindingActive &&
                        Keyboard3DS::isVisible())
                {
                        actionConsumedByKeyboard = true;
                }
                else if (textEditingActive &&
                        !Keyboard3DS::isVisible())
                {
                        keyboardHiddenByUser = false;
                        Keyboard3DS::setVisible(true);
                        actionConsumedByKeyboard = true;
                }
                else
                {
                        int targetX = cursorX;
                        int targetY = cursorY;

                        actionConsumedBySlider =
                                MenuNavigation3DS::
                                activateSliderAtCursor(
                                        game,
                                        cursorX,
                                        cursorY,
                                        targetX,
                                        targetY);

                        if (actionConsumedBySlider)
                        {
                                setCursor(
                                        video,
                                        targetX,
                                        targetY);
                        }
                        else
                        {
                                pushMouseButtonEvent(
                                        SDL_MOUSEBUTTONDOWN,
                                        SDL_BUTTON_LEFT);
                        }
                }
        }

        if (up & KEY_A)
        {
                if (bottomCursorControlActive)
                {
                        pushMouseButtonEventAt(
                                SDL_MOUSEBUTTONUP,
                                SDL_BUTTON_LEFT,
                                bottomCursorControlX,
                                bottomCursorControlY);

                        bottomCursorControlActive = false;
                }

                if (!actionConsumedBySlider &&
                        !actionConsumedByKeyboard &&
                        !actionConsumedByBottomCursor)
                {
                        pushMouseButtonEvent(
                                SDL_MOUSEBUTTONUP,
                                SDL_BUTTON_LEFT);
                }

                actionConsumedBySlider = false;
                actionConsumedByKeyboard = false;
                actionConsumedByBottomCursor = false;
        }


        /*
         * In inventory, X directly quick-moves the item under the
         * cursor. Outside inventory, preserve X as middle mouse.
         */
        InventoryState *inventoryForQuickMove =
                getActiveInventoryState3DS(game);

        if ((down & KEY_X) &&
                inventoryForQuickMove)
        {
                inventoryForQuickMove->
                        quickMoveItemAt3DS(cursorX, cursorY);
        }
        else if ((down & KEY_X) &&
                !(game &&
                  game->getGeoscapeState() &&
                  game->isState(
                        game->getGeoscapeState()) &&
                  game->getGeoscapeState()->
                        canNavigateGlobeTargets3DS()))
        {
                pushMouseButtonEvent(
                        SDL_MOUSEBUTTONDOWN,
                        SDL_BUTTON_MIDDLE);

                physicalMiddleMouseDown = true;
        }

        if ((up & KEY_X) &&
                physicalMiddleMouseDown)
        {
                pushMouseButtonEvent(
                        SDL_MOUSEBUTTONUP,
                        SDL_BUTTON_MIDDLE);

                physicalMiddleMouseDown = false;
        }

        /*
         * Y remains an ordinary right mouse button except while the
         * lower cursor targets the Battlescape Zero TUs control.
         *
         * Use the resolved logical control rather than master-specific
         * screen coordinates. This supports both UFO Defense and TFTD.
         */
        if (down & KEY_Y)
        {
                bottomZeroTUsYConsumed = false;
                bottomZeroTUsPressedState = nullptr;

                BattlescapeState *battlescape =
                        dynamic_cast<BattlescapeState *>(
                                game ?
                                        game->getTopState() :
                                        nullptr);

                bool onZeroTUs = false;

                if (battlescape &&
                        BottomScreen3DS::
                                isBottomCursorFocused() &&
                        BottomScreen3DS::getMode() ==
                                BottomScreen3DS::
                                        MODE_BATTLESCAPE)
                {
                        int logicalX = 0;
                        int logicalY = 0;
                        bool opensTopPanel = false;

                        if (BottomScreen3DS::
                                getBottomCursorControlTarget(
                                        logicalX,
                                        logicalY,
                                        opensTopPanel))
                        {
                                /*
                                 * Zero TUs is located at local position
                                 * 54,44 inside the original 320x56
                                 * Battlescape strip.
                                 */
                                const int expectedX =
                                        std::max(
                                                0,
                                                (Options::
                                                        baseXResolution -
                                                 320) /
                                                        2) +
                                        54;

                                const int expectedY =
                                        std::max(
                                                0,
                                                Options::
                                                        baseYResolution -
                                                        56) +
                                        44;

                                onZeroTUs =
                                        logicalX ==
                                                expectedX &&
                                        logicalY ==
                                                expectedY;
                        }
                }

                if (onZeroTUs)
                {
                        tapKeyboardKey3DS(
                                static_cast<SDLKey>(
                                        Options::
                                                keyBattleZeroTUs));

                        battlescape->
                                setZeroTUsPressed3DS(
                                        true);

                        bottomZeroTUsPressedState =
                                battlescape;

                        bottomZeroTUsYConsumed = true;
                }
                else
                {
                        pushMouseButtonEvent(
                                SDL_MOUSEBUTTONDOWN,
                                SDL_BUTTON_RIGHT);

                        physicalRightMouseDown = true;
                }
        }

        if (up & KEY_Y)
        {
                if (bottomZeroTUsYConsumed)
                {
                        if (bottomZeroTUsPressedState)
                        {
                                bottomZeroTUsPressedState->
                                        setZeroTUsPressed3DS(
                                                false);
                        }

                        bottomZeroTUsPressedState = nullptr;
                        bottomZeroTUsYConsumed = false;
                }
                else if (physicalRightMouseDown)
                {
                        pushMouseButtonEvent(
                                SDL_MOUSEBUTTONUP,
                                SDL_BUTTON_RIGHT);

                        physicalRightMouseDown = false;
                }
        }


        /*
         * Preserve B's original 3DS Back/Escape behavior.
         */
        if (down & KEY_B)
        {
                /* OXCE_3DS_BINDING_B_CANCEL */
                if (keyBindingActive &&
                        Keyboard3DS::isVisible())
                {
                        releaseKeyboardTouch();

                        OptionsControlsState::
                                cancelKeyBinding3DS();

                        Keyboard3DS::setVisible(false);
                        Keyboard3DS::setMode(
                                Keyboard3DS::MODE_TEXT);

                        keyBindingActive = false;
                        backConsumedByComboBox = true;
                }
                else if (fullKeyboardVisible &&
                        Keyboard3DS::isVisible())
                {
                        /*
                         * B closes a manually opened complete keyboard
                         * without sending Escape to the game underneath.
                         */
                        releaseKeyboardTouch();

                        fullKeyboardVisible = false;
                        keyboardHiddenByUser =
                                textEditingActive;

                        Keyboard3DS::setMode(
                                Keyboard3DS::MODE_TEXT);
                        Keyboard3DS::setVisible(false);

                        BottomScreen3DS::
                                setBottomCursorFocused(
                                        fullKeyboardPreviousBottomFocus);

                        backConsumedByComboBox = true;
                }
                else if (textEditingActive &&
                        Keyboard3DS::isVisible())
                {
                        /*
                         * The first B press hides the software keyboard
                         * without changing TextEdit focus.
                         */
                        releaseKeyboardTouch();

                        keyboardHiddenByUser = true;
                        Keyboard3DS::setVisible(false);
                        backConsumedByComboBox = true;
                }
                else
                {
                        /*
                         * With the keyboard already hidden, preserve the
                         * existing cancellation behavior. Ordinary fields
                         * unfocus; mandatory fields remain active.
                         */
                        backConsumedByComboBox =
                                MenuNavigation3DS::
                                cancelTextEditing(game);

                        if (!backConsumedByComboBox)
                        {
                                backConsumedByComboBox =
                                        MenuNavigation3DS::
                                        cancelInventoryCarry(
                                                game);
                        }

                        if (!backConsumedByComboBox)
                        {
                                backConsumedByComboBox =
                                        MenuNavigation3DS::
                                        closeOpenComboBox(game);
                        }

                        if (!backConsumedByComboBox)
                        {
                                backConsumedByComboBox =
                                        MenuNavigation3DS::
                                        leaveSliderMode();
                        }

                        if (!backConsumedByComboBox)
                        {
                                pushKeyEvent(
                                        SDL_KEYDOWN,
                                        SDLK_ESCAPE);
                        }
                }
        }

        if (up & KEY_B)
        {
                if (!backConsumedByComboBox)
                {
                        pushKeyEvent(
                                SDL_KEYUP,
                                SDLK_ESCAPE);
                }

                backConsumedByComboBox = false;
        }

        if ((down & KEY_START) &&
                !inventoryPhysicalButtonConsumed)
        {
                pushKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
        }

        if ((up & KEY_START) &&
                !inventoryPhysicalButtonConsumed)
        {
                pushKeyEvent(SDL_KEYUP, SDLK_RETURN);
        }
}

}
}
