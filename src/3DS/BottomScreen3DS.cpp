#include "BottomScreen3DS.h"
#include "GenericMenuArtwork3DS.h"
#include "IndexedBlit3DS.h"
#include "Keyboard3DS.h"
#include "MenuNavigation3DS.h"

#include "../Engine/Game.h"
#include "../Battlescape/BattlescapeState.h"
#include "../Engine/Surface.h"
#include "../Engine/Options.h"
#include "../Geoscape/GeoscapeState.h"
#include "../Interface/Cursor.h"
#include "../Mod/Mod.h"

#include <SDL.h>

#include <algorithm>
#include <climits>
#include <utility>
#include <vector>

namespace OpenXcom
{
namespace BottomScreen3DS
{

namespace
{

/*
 * Physical bottom-screen dimensions.
 *
 * SDL_DUALSCR with SDL_FITWIDTH gives the combined surface a
 * 400-pixel logical width, while touch coordinates remain 320 wide.
 * Vertical coordinates remain native.
 */
constexpr int BOTTOM_NATIVE_WIDTH = 320;
constexpr int BOTTOM_NATIVE_HEIGHT = 240;

constexpr int GEOSCAPE_SIDEBAR_WIDTH = 64;
constexpr int GEOSCAPE_SIDEBAR_HEIGHT = 200;

constexpr int BATTLESCAPE_BAR_HEIGHT = 56;

/*
 * Battlescape bottom trackpad.
 *
 * Keep this out of the command strip and hand-slot rectangles.
 * Touching here behaves like the Geoscape trackpad: it controls
 * the top-screen cursor and exits lower-screen cursor focus.
 */
constexpr int BATTLESCAPE_TRACKPAD_LEFT = 4;
constexpr int BATTLESCAPE_TRACKPAD_TOP = 84;
constexpr int BATTLESCAPE_TRACKPAD_RIGHT = 247;
constexpr int BATTLESCAPE_TRACKPAD_BOTTOM = 236;


/*
 * The unit stats/health area is part of the scaled top strip:
 * original source rect 107..271, 177..200 inside the 320x200
 * Battlescape frame, visually mapped from source 48..272,
 * 144..200 into the 320x80 lower-screen strip.
 */
constexpr int BATTLESCAPE_STATS_PANEL_LEFT = 84;
constexpr int BATTLESCAPE_STATS_PANEL_TOP = 47;
constexpr int BATTLESCAPE_STATS_PANEL_RIGHT = 320;
constexpr int BATTLESCAPE_STATS_PANEL_BOTTOM = 80;
constexpr int BATTLESCAPE_STATS_LOGICAL_X = 189;
constexpr int BATTLESCAPE_STATS_LOGICAL_Y = 188;

constexpr int MENU_TRACKPAD_LEFT = 6;
constexpr int MENU_TRACKPAD_TOP = 64;
constexpr int MENU_TRACKPAD_RIGHT = 314;
constexpr int MENU_TRACKPAD_BOTTOM = 232;

/*
 * Generic menu lower screen: one nearly full-screen touchpad.
 */
constexpr int GENERIC_MENU_TRACKPAD_LEFT = 4;
constexpr int GENERIC_MENU_TRACKPAD_TOP = 4;
constexpr int GENERIC_MENU_TRACKPAD_RIGHT = 316;
constexpr int GENERIC_MENU_TRACKPAD_BOTTOM = 236;

/*
 * OXCE_3DS_INVENTORY_BOTTOM_PANEL
 *
 * The inventory keeps one large touchpad and a narrow action strip.
 */
/*
 * OXCE_3DS_INVENTORY_TOP_BUTTON_ROW
 */
constexpr int INVENTORY_TRACKPAD_LEFT = 4;
constexpr int INVENTORY_TRACKPAD_TOP = 40;
constexpr int INVENTORY_TRACKPAD_RIGHT = 316;
constexpr int INVENTORY_TRACKPAD_BOTTOM = 236;

constexpr int KEYBOARD_BUTTON_LEFT = 262;
constexpr int KEYBOARD_BUTTON_TOP = 184;
constexpr int KEYBOARD_BUTTON_RIGHT = 314;
constexpr int KEYBOARD_BUTTON_BOTTOM = 232;

Mode currentMode = MODE_MENU;
Game *currentGame = nullptr;

bool bottomCursorFocused = false;
int bottomCursorX = 262;
int bottomCursorY = 10;

/*
 * The first snapshot is refreshed every frame before OXCE draws its
 * cursor. The second stores the last completed geoscape frame and is
 * held unchanged while another geoscape-related state is open.
 */
Surface::UniqueBufferPtr cleanFrameBuffer;
Surface::UniqueSurfacePtr cleanFrame;

Surface::UniqueBufferPtr frozenGeoscapeBuffer;
Surface::UniqueSurfacePtr frozenGeoscapeFrame;

/*
 * Gameplay panels remain indexed while they are assembled. The
 * completed panel is then converted directly to the true-color
 * display using its own palette.
 */
Surface::UniqueBufferPtr panelBuffer;
Surface::UniqueSurfacePtr panel;

Surface::UniqueBufferPtr geoscapeSidebarBuffer;
Surface::UniqueSurfacePtr geoscapeSidebarFrame;

/*
 * Rebuild expensive gameplay panel artwork at a lower cadence, while
 * presenting the cached panel every frame so cursor movement remains
 * responsive and the previous cursor image is erased.
 */
constexpr unsigned PANEL_REFRESH_INTERVAL = 4;

bool panelDirty = true;
unsigned panelRefreshCountdown = 0;

bool previousBottomCursorVisible = false;
SDL_Rect previousBottomCursorRect = {};

void invalidatePanel()
{
        panelDirty = true;
        panelRefreshCountdown = 0;
}

bool shouldRebuildPanel()
{
        if (panelDirty ||
                !panel ||
                panelRefreshCountdown == 0)
        {
                return true;
        }

        --panelRefreshCountdown;
        return false;
}

void finishPanelRebuild(bool successful)
{
        panelDirty = !successful;

        panelRefreshCountdown =
                successful ?
                        PANEL_REFRESH_INTERVAL - 1 :
                        0;
}

/*
 * Aspect-preserving geoscape control layout.
 *
 * The source artwork dimensions are:
 *   command button: 63x11
 *   speed button:   31x13
 *   globe block:    63x44
 *
 * Integer destination sizes below differ from those ratios by less
 * than one percent and avoid the visibly wide stretching used before.
 */
constexpr int GEOSCAPE_COMMAND_LEFT = 210;
constexpr int GEOSCAPE_COMMAND_RIGHT = 313;

constexpr int GEOSCAPE_SPEED_LEFT_1 = 214;
constexpr int GEOSCAPE_SPEED_RIGHT_1 = 252;
constexpr int GEOSCAPE_SPEED_LEFT_2 = 271;
constexpr int GEOSCAPE_SPEED_RIGHT_2 = 309;

constexpr int GEOSCAPE_GLOBE_LEFT = 220;
constexpr int GEOSCAPE_GLOBE_TOP = 179;
constexpr int GEOSCAPE_GLOBE_RIGHT = 304;
constexpr int GEOSCAPE_GLOBE_BOTTOM = 238;

struct ControlTarget
{
        int left;
        int top;
        int right;
        int bottom;
        int logicalX;
        int logicalY;
        bool opensTopPanel;
};

struct NativeRect3DS
{
        int left;
        int top;
        int right;
        int bottom;
};

struct InventoryControlPlacement
{
        int drawLeft;
        int drawTop;
        int drawRight;
        int drawBottom;

        int hitLeft;
        int hitTop;
        int hitRight;
        int hitBottom;

        int logicalX;
        int logicalY;

        bool enabled;
        Surface *surface;

        MenuNavigation3DS::InventoryBottomControl control;
};

static const MenuNavigation3DS::InventoryBottomControl
        INVENTORY_CONTROL_ORDER[] =
{
        MenuNavigation3DS::INVENTORY_OK,
        MenuNavigation3DS::INVENTORY_PREVIOUS,
        MenuNavigation3DS::INVENTORY_NEXT,
        MenuNavigation3DS::INVENTORY_UNLOAD,
        MenuNavigation3DS::INVENTORY_SAVE_CONFIG,
        MenuNavigation3DS::INVENTORY_LOAD_CONFIG,
        MenuNavigation3DS::INVENTORY_LINKS,
        MenuNavigation3DS::INVENTORY_GROUND
};

constexpr int INVENTORY_CONTROL_COUNT =
        sizeof(INVENTORY_CONTROL_ORDER) /
        sizeof(INVENTORY_CONTROL_ORDER[0]);

/*
 * Build one native-resolution horizontal row from the actual button
 * surfaces. No pixels are sampled from the completed top-screen frame.
 */
void buildInventoryControlLayout(
        std::vector<InventoryControlPlacement> &layout)
{
        layout.clear();

        /*
         * OXCE_3DS_INVENTORY_EVEN_TOOLBAR
         *
         * First control begins at the left edge. The final Ground /
         * Scroll Right control ends at the right edge. All seven spaces
         * between the eight controls are distributed evenly.
         */
        constexpr int screenLeft = 2;
        constexpr int screenRight = 318;

        constexpr int rowTop = 0;
        constexpr int rowBottom = 37;

        constexpr int scaleNumerator = 5;
        constexpr int scaleDenominator = 4;

        struct PendingControl
        {
                Surface *surface;

                MenuNavigation3DS::InventoryBottomControl control;

                int logicalX;
                int logicalY;

                int sourceWidth;
                int sourceHeight;

                int width;
                int height;

                bool enabled;
        };

        std::vector<PendingControl> pending;

        for (int index = 0;
                index < INVENTORY_CONTROL_COUNT;
                ++index)
        {
                int logicalX = 0;
                int logicalY = 0;
                bool enabled = false;

                Surface *surface =
                        MenuNavigation3DS::
                        getInventoryBottomControlSurface(
                                currentGame,
                                INVENTORY_CONTROL_ORDER[index],
                                logicalX,
                                logicalY,
                                enabled);

                if (!surface ||
                        !surface->getSurface() ||
                        surface->getWidth() <= 0 ||
                        surface->getHeight() <= 0)
                {
                        continue;
                }

                PendingControl control = {};

                control.surface = surface;
                control.control =
                        INVENTORY_CONTROL_ORDER[index];

                control.logicalX = logicalX;
                control.logicalY = logicalY;

                control.sourceWidth =
                        surface->getWidth();

                control.sourceHeight =
                        surface->getHeight();

                control.width =
                        std::max(
                                1,
                                (control.sourceWidth *
                                        scaleNumerator +
                                        scaleDenominator / 2) /
                                        scaleDenominator);

                control.height =
                        std::max(
                                1,
                                (control.sourceHeight *
                                        scaleNumerator +
                                        scaleDenominator / 2) /
                                        scaleDenominator);

                const int maximumHeight =
                        rowBottom -
                        rowTop;

                if (control.height >
                        maximumHeight)
                {
                        control.height =
                                maximumHeight;

                        control.width =
                                std::max(
                                        1,
                                        control.sourceWidth *
                                                control.height /
                                                control.sourceHeight);
                }

                control.enabled = enabled;

                pending.push_back(control);
        }

        if (pending.empty())
        {
                return;
        }

        const int availableWidth =
                screenRight -
                screenLeft;

        const int gapCount =
                std::max(
                        0,
                        static_cast<int>(
                                pending.size()) -
                                1);

        int totalButtonWidth = 0;

        for (const PendingControl &control :
                pending)
        {
                totalButtonWidth +=
                        control.width;
        }

        /*
         * Safety fallback for mods that define unusually wide buttons.
         */
        if (totalButtonWidth >
                availableWidth)
        {
                const int originalTotal =
                        totalButtonWidth;

                for (PendingControl &control :
                        pending)
                {
                        control.width =
                                std::max(
                                        1,
                                        control.width *
                                                availableWidth /
                                                originalTotal);

                        control.height =
                                std::max(
                                        1,
                                        control.sourceHeight *
                                                control.width /
                                                control.sourceWidth);
                }

                totalButtonWidth = 0;

                for (const PendingControl &control :
                        pending)
                {
                        totalButtonWidth +=
                                control.width;
                }
        }

        const int leftover =
                std::max(
                        0,
                        availableWidth -
                                totalButtonWidth);

        const int regularGap =
                gapCount > 0
                        ? leftover / gapCount
                        : 0;

        const int remainder =
                gapCount > 0
                        ? leftover % gapCount
                        : 0;

        int currentX =
                screenLeft;

        for (std::size_t index = 0;
                index < pending.size();
                ++index)
        {
                const PendingControl &source =
                        pending[index];

                InventoryControlPlacement placement = {};

                placement.drawLeft =
                        currentX;

                placement.drawTop =
                        rowTop +
                        ((rowBottom - rowTop) -
                                source.height) /
                                2;

                placement.drawRight =
                        placement.drawLeft +
                        source.width;

                placement.drawBottom =
                        placement.drawTop +
                        source.height;

                placement.logicalX =
                        source.logicalX;

                placement.logicalY =
                        source.logicalY;

                placement.enabled =
                        source.enabled;

                placement.surface =
                        source.surface;

                placement.control =
                        source.control;

                layout.push_back(placement);

                currentX =
                        placement.drawRight;

                if (index + 1 <
                        pending.size())
                {
                        currentX +=
                                regularGap;

                        if (static_cast<int>(index) <
                                remainder)
                        {
                                ++currentX;
                        }
                }
        }

        /*
         * Split each empty gap between the neighboring touch targets.
         * This produces generous targets without overlapping them.
         */
        for (std::size_t index = 0;
                index < layout.size();
                ++index)
        {
                InventoryControlPlacement &control =
                        layout[index];

                control.hitTop = 0;
                control.hitBottom = 43;

                if (index == 0)
                {
                        control.hitLeft = 0;
                }
                else
                {
                        control.hitLeft =
                                (layout[index - 1].drawRight +
                                 control.drawLeft) /
                                2;
                }

                if (index + 1 ==
                        layout.size())
                {
                        control.hitRight = 320;
                }
                else
                {
                        control.hitRight =
                                (control.drawRight +
                                 layout[index + 1].drawLeft) /
                                2;
                }
        }
}


/*
 * OXCE_3DS_RESTORED_GEOSCAPE_CONTROL_TABLE
 *
 * These are the original lower-screen Geoscape control definitions.
 * They were accidentally removed while replacing the inventory-row
 * layout helpers.
 */


/*
 * Native 320x240 touchscreen rectangles. These match the actual
 * rendered button artwork rather than filling the surrounding gaps.
 */
static const ControlTarget BOTTOM_CONTROLS[] =
{
        /* Main commands. */
        {GEOSCAPE_COMMAND_LEFT,   1,
         GEOSCAPE_COMMAND_RIGHT, 19, 288,  5, true},

        {GEOSCAPE_COMMAND_LEFT,  21,
         GEOSCAPE_COMMAND_RIGHT, 39, 288, 17, true},

        {GEOSCAPE_COMMAND_LEFT,  41,
         GEOSCAPE_COMMAND_RIGHT, 59, 288, 29, true},

        {GEOSCAPE_COMMAND_LEFT,  61,
         GEOSCAPE_COMMAND_RIGHT, 79, 288, 41, true},

        {GEOSCAPE_COMMAND_LEFT,  81,
         GEOSCAPE_COMMAND_RIGHT, 99, 288, 53, true},

        {GEOSCAPE_COMMAND_LEFT, 101,
         GEOSCAPE_COMMAND_RIGHT,119, 288, 65, true},

        /* Time-speed grid. */
        {GEOSCAPE_SPEED_LEFT_1, 123,
         GEOSCAPE_SPEED_RIGHT_1,139, 272,118, false},

        {GEOSCAPE_SPEED_LEFT_2, 123,
         GEOSCAPE_SPEED_RIGHT_2,139, 304,118, false},

        {GEOSCAPE_SPEED_LEFT_1, 141,
         GEOSCAPE_SPEED_RIGHT_1,157, 272,132, false},

        {GEOSCAPE_SPEED_LEFT_2, 141,
         GEOSCAPE_SPEED_RIGHT_2,157, 304,132, false},

        {GEOSCAPE_SPEED_LEFT_1, 159,
         GEOSCAPE_SPEED_RIGHT_1,175, 272,146, false},

        {GEOSCAPE_SPEED_LEFT_2, 159,
         GEOSCAPE_SPEED_RIGHT_2,175, 304,146, false},

        /* Globe rotation and zoom. */
        {238, 187, 256, 203, 277,168, false},
        {221, 204, 239, 220, 265,181, false},
        {255, 204, 273, 220, 289,181, false},
        {238, 221, 256, 237, 277,193, false},

        {270, 179, 302, 210, 306,167, false},
        {277, 213, 295, 237, 306,190, false}
};

constexpr int BOTTOM_CONTROL_COUNT =
        sizeof(BOTTOM_CONTROLS) /
        sizeof(BOTTOM_CONTROLS[0]);

/*
 * 3DS Battlescape bottom-screen controls.
 *
 * left/top/right/bottom are native 320x240 touchscreen rectangles.
 * logicalX/logicalY are local coordinates inside the original
 * 320x56 Battlescape icon strip. They are converted back to the
 * active OXCE logical screen coordinates in getControlTargetAt().
 */
static const ControlTarget BATTLESCAPE_CONTROLS[] =
{
        /*
         * First two entries are direct native hand-slot hitboxes.
         * Remaining entries use original top-strip coordinates:
         *   x = 48..272  -> x = 0..320
         *   y = 0..56    -> y = 0..80
         */

        /* Full-size scaled hand slots below the bar. */
        {251,  80, 320, 160, 296, 28, false}, // Right hand
        {251, 160, 320, 240,  24, 28, false}, // Left hand under right

        /* Original command buttons. */
        { 48,   0,  80,  16,  64,  8, false}, // Unit up
        { 48,  16,  80,  32,  64, 24, false}, // Unit down

        { 80,   0, 112,  16,  96,  8, false}, // Map up
        { 80,  16, 112,  32,  96, 24, false}, // Map down

        {112,   0, 144,  16, 128,  8, false}, // Show map
        {112,  16, 144,  32, 128, 24, false}, // Kneel

        {144,   0, 176,  16, 160,  8, false}, // Inventory
        {144,  16, 176,  32, 160, 24, false}, // Center

        {176,   0, 208,  16, 192,  8, false}, // Next soldier
        {176,  16, 208,  32, 192, 24, false}, // Next stop

        {208,   0, 240,  16, 224,  8, false}, // Show layers
        {208,  16, 240,  32, 224, 24, true},  // Help / menu

        {240,   0, 272,  16, 256,  8, false}, // End turn
        {240,  16, 272,  32, 256, 24, false}, // Abort

        /* Reserve / zero-TU cluster. */
        { 60,  33,  77,  44,  68, 38, false}, // Reserve none
        { 78,  33,  95,  44,  86, 38, false}, // Reserve snap
        { 60,  45,  77,  56,  68, 50, false}, // Reserve aimed
        { 78,  45,  95,  56,  86, 50, false}, // Reserve auto
        { 96,  33, 106,  56, 101, 44, false}, // Reserve kneel
        { 49,  33,  59,  56,  54, 44, false}  // Zero TUs
};


constexpr int BATTLESCAPE_CONTROL_COUNT =
        sizeof(BATTLESCAPE_CONTROLS) /
        sizeof(BATTLESCAPE_CONTROLS[0]);

int battlescapeBarLogicalLeft()
{
        return std::max(
                0,
                (Options::baseXResolution -
                        BOTTOM_NATIVE_WIDTH) /
                        2);
}

int battlescapeBarLogicalTop()
{
        return std::max(
                0,
                Options::baseYResolution -
                        BATTLESCAPE_BAR_HEIGHT);
}

int battlescapeVisualToNativeX(int localX)
{
        /*
         * Uniformly scale the original top-strip content
         * (x = 48..272, width 224) to the full 320-pixel width.
         */
        constexpr int sourceLeft = 48;
        constexpr int sourceRight = 272;
        constexpr int sourceWidth = sourceRight - sourceLeft;

        const int clampedX =
                std::max(
                        sourceLeft,
                        std::min(
                                sourceRight,
                                localX));

        return (clampedX - sourceLeft) *
                BOTTOM_NATIVE_WIDTH /
                sourceWidth;
}

int battlescapeVisualToNativeY(int localY)
{
        /*
         * Keep aspect ratio with the same 10/7 scale:
         * 56 px source height -> 80 px destination height.
         */
        constexpr int sourceHeight = 56;
        constexpr int targetHeight = 80;

        const int clampedY =
                std::max(
                        0,
                        std::min(
                                sourceHeight,
                                localY));

        return clampedY *
                targetHeight /
                sourceHeight;
}


bool isTftdMaster3DS()
{
        return Options::getActiveMaster() == "xcom2";
}



static const int SNAP_NEIGHBORS[BOTTOM_CONTROL_COUNT][4] =
{
        {-1,  1, -1, -1},
        { 0,  2, -1, -1},
        { 1,  3, -1, -1},
        { 2,  4, -1, -1},
        { 3,  5, -1, -1},
        { 4,  6, -1, -1},

        { 5,  8, -1,  7},
        { 5,  9,  6, -1},
        { 6, 10, -1,  9},
        { 7, 11,  8, -1},
        { 8, 12, -1, 11},
        { 9, 16, 10, -1},

        {10, 15, 13, 14},
        {12, 15, -1, 12},
        {12, 15, 12, 17},
        {12, -1, 13, 14},

        {11, 17, 14, -1},
        {16, -1, 14, -1}
};

bool pointInRect(
        int x,
        int y,
        int left,
        int top,
        int right,
        int bottom)
{
        return x >= left &&
                x < right &&
                y >= top &&
                y < bottom;
}

int nativeToRenderedX(
        const SDL_Surface *screen,
        int nativeX)
{
        if (!screen)
        {
                return nativeX;
        }

        return nativeX *
                screen->w /
                BOTTOM_NATIVE_WIDTH;
}

void fillRect(
        SDL_Surface *surface,
        int x,
        int y,
        int width,
        int height,
        Uint8 red,
        Uint8 green,
        Uint8 blue)
{
        if (!surface ||
                width <= 0 ||
                height <= 0)
        {
                return;
        }

        SDL_Rect rect = {};
        rect.x = static_cast<Sint16>(x);
        rect.y = static_cast<Sint16>(y);
        rect.w = static_cast<Uint16>(width);
        rect.h = static_cast<Uint16>(height);

        SDL_FillRect(
                surface,
                &rect,
                SDL_MapRGB(
                        surface->format,
                        red,
                        green,
                        blue));
}

void writePixel(
        SDL_Surface *surface,
        int x,
        int y,
        Uint32 color)
{
        if (!surface ||
                x < 0 ||
                y < 0 ||
                x >= surface->w ||
                y >= surface->h)
        {
                return;
        }

        Uint8 *pixel =
                static_cast<Uint8 *>(
                        surface->pixels) +
                y * surface->pitch +
                x * surface->format->BytesPerPixel;

        switch (surface->format->BytesPerPixel)
        {
        case 1:
                *pixel = static_cast<Uint8>(color);
                break;

        case 2:
                *reinterpret_cast<Uint16 *>(pixel) =
                        static_cast<Uint16>(color);
                break;

        case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                pixel[0] =
                        static_cast<Uint8>(
                                (color >> 16) & 0xff);
                pixel[1] =
                        static_cast<Uint8>(
                                (color >> 8) & 0xff);
                pixel[2] =
                        static_cast<Uint8>(
                                color & 0xff);
#else
                pixel[0] =
                        static_cast<Uint8>(
                                color & 0xff);
                pixel[1] =
                        static_cast<Uint8>(
                                (color >> 8) & 0xff);
                pixel[2] =
                        static_cast<Uint8>(
                                (color >> 16) & 0xff);
#endif
                break;

        case 4:
                *reinterpret_cast<Uint32 *>(pixel) =
                        color;
                break;
        }
}

/*
 * Restore only the small region covered by the bottom cursor on the
 * preceding frame. The source is the already assembled cached panel.
 */
void restorePreviousBottomCursor(
        SDL_Surface *screen,
        int bottomY)
{
        if (!previousBottomCursorVisible ||
                !screen ||
                !panel)
        {
                return;
        }

        SDL_Rect sourceRect =
                previousBottomCursorRect;

        SDL_Rect destinationRect =
                previousBottomCursorRect;

        destinationRect.y =
                static_cast<Sint16>(
                        bottomY +
                        previousBottomCursorRect.y);

        IndexedBlit3DS::blitNearest(
                panel.get(),
                screen,
                sourceRect,
                destinationRect);

        previousBottomCursorVisible = false;
}

/*
 * Remember the rectangular area touched by drawBottomCursor(), using
 * coordinates relative to the 400x240 cached panel.
 */
void rememberBottomCursorRect(
        SDL_Surface *screen,
        int bottomY)
{
        previousBottomCursorVisible = false;

        if (!screen ||
                !bottomCursorFocused ||
                Keyboard3DS::isVisible() ||
                !currentGame)
        {
                return;
        }

        Cursor *cursor =
                currentGame->getCursor();

        if (!cursor ||
                !cursor->getVisible() ||
                !cursor->getPalette())
        {
                return;
        }

        constexpr int sourceWidth = 9;
        constexpr int sourceHeight = 13;

        const int renderedWidth =
                std::max(
                        1,
                        nativeToRenderedX(
                                screen,
                                sourceWidth));

        const int renderedHeight =
                std::max(
                        1,
                        (sourceHeight *
                                BOTTOM_NATIVE_HEIGHT +
                                199) /
                                200);

        const int left =
                std::max(
                        0,
                        nativeToRenderedX(
                                screen,
                                bottomCursorX));

        const int top =
                std::max(
                        0,
                        bottomCursorY);

        const int right =
                std::min(
                        screen->w,
                        left +
                        renderedWidth);

        const int bottom =
                std::min(
                        screen->h -
                                bottomY,
                        top +
                        renderedHeight);

        if (right <= left ||
                bottom <= top)
        {
                return;
        }

        previousBottomCursorRect.x =
                static_cast<Sint16>(left);

        previousBottomCursorRect.y =
                static_cast<Sint16>(top);

        previousBottomCursorRect.w =
                static_cast<Uint16>(
                        right -
                        left);

        previousBottomCursorRect.h =
                static_cast<Uint16>(
                        bottom -
                        top);

        previousBottomCursorVisible = true;
}

void drawBottomCursor(
        SDL_Surface *screen,
        int bottomY)
{
        if (!screen ||
                !bottomCursorFocused ||
                !currentGame)
        {
                return;
        }

        Cursor *cursor =
                currentGame->getCursor();

        if (!cursor ||
                !cursor->getVisible() ||
                !cursor->getPalette())
        {
                return;
        }

        /*
         * Cursor::draw() always builds this 9x13 layered pointer.
         * Generate those same indexed pixels, then map them through
         * the cursor's current palette into the RGB565 display.
         */
        constexpr int sourceWidth = 9;
        constexpr int sourceHeight = 13;

        int cursorPixels[sourceHeight][sourceWidth];

        for (int y = 0;
                y < sourceHeight;
                ++y)
        {
                for (int x = 0;
                        x < sourceWidth;
                        ++x)
                {
                        cursorPixels[y][x] = -1;
                }
        }

        Uint8 color =
                cursor->getColor();

        int x1 = 0;
        int y1 = 0;
        int x2 = sourceWidth - 1;
        int y2 = sourceHeight - 1;

        for (int layer = 0;
                layer < 4;
                ++layer)
        {
                for (int y = y1;
                        y <= y2;
                        ++y)
                {
                        cursorPixels[y][x1] =
                                color;
                }

                const int diagonalLength =
                        x2 - x1;

                for (int step = 0;
                        step <= diagonalLength;
                        ++step)
                {
                        cursorPixels[
                                y1 + step][
                                x1 + step] =
                                color;
                }

                ++x1;
                y1 += 2;
                --y2;
                --x2;
                ++color;
        }

        cursorPixels[8][4] =
                static_cast<Uint8>(
                        cursor->getColor() + 3);

        const int renderedWidth =
                std::max(
                        1,
                        nativeToRenderedX(
                                screen,
                                sourceWidth));

        /*
         * Match the vertical size of the top cursor, which is scaled
         * from OXCE's 200-pixel logical height to the 240-pixel top.
         */
        const int renderedHeight =
                std::max(
                        1,
                        (sourceHeight *
                                BOTTOM_NATIVE_HEIGHT +
                                199) /
                                200);

        const int renderedX =
                nativeToRenderedX(
                        screen,
                        bottomCursorX);

        const int renderedY =
                bottomY +
                bottomCursorY;

        SDL_Color *palette =
                cursor->getPalette();

        bool locked = false;

        if (SDL_MUSTLOCK(screen))
        {
                if (SDL_LockSurface(screen) != 0)
                {
                        return;
                }

                locked = true;
        }

        for (int outputY = 0;
                outputY < renderedHeight;
                ++outputY)
        {
                const int sourceY =
                        outputY *
                        sourceHeight /
                        renderedHeight;

                for (int outputX = 0;
                        outputX < renderedWidth;
                        ++outputX)
                {
                        const int sourceX =
                                outputX *
                                sourceWidth /
                                renderedWidth;

                        const int paletteIndex =
                                cursorPixels[
                                        sourceY][
                                        sourceX];

                        if (paletteIndex < 0)
                        {
                                continue;
                        }

                        const SDL_Color &pixelColor =
                                palette[
                                        paletteIndex];

                        writePixel(
                                screen,
                                renderedX + outputX,
                                renderedY + outputY,
                                SDL_MapRGB(
                                        screen->format,
                                        pixelColor.r,
                                        pixelColor.g,
                                        pixelColor.b));
                }
        }

        if (locked)
        {
                SDL_UnlockSurface(screen);
        }
}

void drawTrackpad(
        SDL_Surface *screen,
        int bottomY,
        int nativeLeft,
        int nativeTop,
        int nativeRight,
        int nativeBottom)
{
        const int left =
                nativeToRenderedX(
                        screen,
                        nativeLeft);

        const int right =
                nativeToRenderedX(
                        screen,
                        nativeRight);

        fillRect(
                screen,
                left,
                bottomY + nativeTop,
                right - left,
                nativeBottom - nativeTop,
                36,
                52,
                68);

        const int innerLeft =
                nativeToRenderedX(
                        screen,
                        nativeLeft + 4);

        const int innerRight =
                nativeToRenderedX(
                        screen,
                        nativeRight - 4);

        fillRect(
                screen,
                innerLeft,
                bottomY + nativeTop + 4,
                innerRight - innerLeft,
                nativeBottom - nativeTop - 8,
                52,
                72,
                92);
}

bool stretchRegion(
        SDL_Surface *source,
        SDL_Surface *destination,
        SDL_Rect sourceRect,
        SDL_Rect destinationRect)
{
        if (!source || !destination)
        {
                return false;
        }

        return SDL_SoftStretch(
                source,
                &sourceRect,
                destination,
                &destinationRect) == 0;
}

bool ensureSnapshot(
        SDL_Surface *source,
        Surface::UniqueBufferPtr &buffer,
        Surface::UniqueSurfacePtr &snapshot)
{
        if (!source ||
                source->w <= 0 ||
                source->h <= 0 ||
                source->format->BitsPerPixel != 8)
        {
                return false;
        }

        if (snapshot &&
                snapshot->w == source->w &&
                snapshot->h == source->h)
        {
                return true;
        }

        auto pair =
                Surface::NewPair8Bit(
                        source->w,
                        source->h);

        buffer = std::move(pair.first);
        snapshot = std::move(pair.second);

        return static_cast<bool>(snapshot);
}

bool copySnapshot(
        SDL_Surface *source,
        Surface::UniqueBufferPtr &buffer,
        Surface::UniqueSurfacePtr &snapshot)
{
        if (!ensureSnapshot(
                source,
                buffer,
                snapshot))
        {
                return false;
        }

        if (source->format->palette &&
                snapshot->format->palette)
        {
                SDL_SetColors(
                        snapshot.get(),
                        source->format->palette->colors,
                        0,
                        source->format->palette->ncolors);
        }

        SDL_Rect destination = {};
        destination.x = 0;
        destination.y = 0;

        return SDL_BlitSurface(
                source,
                nullptr,
                snapshot.get(),
                &destination) == 0;
}

bool ensurePanel(
        int width,
        int height)
{
        if (width <= 0 || height <= 0)
        {
                return false;
        }

        if (panel &&
                panel->w == width &&
                panel->h == height)
        {
                return true;
        }

        auto pair =
                Surface::NewPair8Bit(
                        width,
                        height);

        panelBuffer = std::move(pair.first);
        panel = std::move(pair.second);

        return static_cast<bool>(panel);
}

bool preparePanel(
        SDL_Surface *paletteSource,
        int width,
        int height)
{
        if (!paletteSource ||
                !paletteSource->format ||
                !paletteSource->format->palette ||
                !ensurePanel(width, height))
        {
                return false;
        }

        SDL_SetColors(
                panel.get(),
                paletteSource->format->palette->colors,
                0,
                paletteSource->format->palette->ncolors);

        Surface::CleanSdlSurface(panel.get());

        return true;
}

bool presentPanel(
        SDL_Surface *screen,
        int bottomY)
{
        if (!screen || !panel)
        {
                return false;
        }

        SDL_Rect sourceRect = {};
        sourceRect.x = 0;
        sourceRect.y = 0;
        sourceRect.w =
                static_cast<Uint16>(
                        panel->w);
        sourceRect.h =
                static_cast<Uint16>(
                        panel->h);

        SDL_Rect destinationRect = {};
        destinationRect.x = 0;
        destinationRect.y =
                static_cast<Sint16>(
                        bottomY);
        destinationRect.w =
                static_cast<Uint16>(
                        panel->w);
        destinationRect.h =
                static_cast<Uint16>(
                        panel->h);

        return IndexedBlit3DS::blitNearest(
                panel.get(),
                screen,
                sourceRect,
                destinationRect);
}

/*
 * Redraw the live funds, clock and date block every displayed frame.
 * The rest of the geoscape panel remains cached.
 */
void presentLiveGeoscapeInformation(
        SDL_Surface *screen,
        SDL_Surface *source,
        int bottomY)
{
        if (!screen ||
                !source ||
                source->w < 64 ||
                source->h < 200)
        {
                return;
        }

        SDL_Rect sourceRect = {};
        sourceRect.x =
                source->w >= 320 ?
                        256 :
                        0;
        sourceRect.y = 72;
        sourceRect.w = 64;
        sourceRect.h = 40;

        SDL_Rect destinationRect = {};

        destinationRect.x =
                static_cast<Sint16>(
                        nativeToRenderedX(
                                screen,
                                4));

        destinationRect.y =
                static_cast<Sint16>(
                        bottomY + 2);

        destinationRect.w =
                static_cast<Uint16>(
                        nativeToRenderedX(
                                screen,
                                132) -
                        nativeToRenderedX(
                                screen,
                                4));

        destinationRect.h = 80;

        IndexedBlit3DS::blitNearest(
                source,
                screen,
                sourceRect,
                destinationRect);
}

SDL_Surface *getResourceSurface(
        const char *resourceName)
{
        if (!currentGame ||
                !currentGame->getMod())
        {
                return nullptr;
        }

        Surface *resource =
                currentGame->getMod()->getSurface(
                        resourceName,
                        false);

        if (!resource ||
                !static_cast<bool>(*resource))
        {
                return nullptr;
        }

        return resource->getSurface();
}

void stretchNativeRegion(
        SDL_Surface *source,
        SDL_Surface *destination,
        SDL_Rect sourceRect,
        int bottomY,
        int nativeLeft,
        int nativeTop,
        int nativeRight,
        int nativeBottom)
{
        if (!source ||
                !destination ||
                nativeRight <= nativeLeft ||
                nativeBottom <= nativeTop)
        {
                return;
        }

        SDL_Rect destinationRect = {};

        destinationRect.x =
                static_cast<Sint16>(
                        nativeToRenderedX(
                                destination,
                                nativeLeft));

        destinationRect.y =
                static_cast<Sint16>(
                        bottomY + nativeTop);

        destinationRect.w =
                static_cast<Uint16>(
                        nativeToRenderedX(
                                destination,
                                nativeRight) -
                        nativeToRenderedX(
                                destination,
                                nativeLeft));

        destinationRect.h =
                static_cast<Uint16>(
                        nativeBottom -
                        nativeTop);

        stretchRegion(
                source,
                destination,
                sourceRect,
                destinationRect);
}

SDL_Surface *getEmbeddedGenericMenuArtwork3DS(
        SDL_Surface *screen)
{
        if (!screen ||
                !screen->format)
        {
                return nullptr;
        }

        static SDL_Surface *artwork = nullptr;

        const bool formatMatches =
                artwork &&
                artwork->format &&
                artwork->format->BitsPerPixel ==
                        screen->format->BitsPerPixel &&
                artwork->format->Rmask ==
                        screen->format->Rmask &&
                artwork->format->Gmask ==
                        screen->format->Gmask &&
                artwork->format->Bmask ==
                        screen->format->Bmask &&
                artwork->format->Amask ==
                        screen->format->Amask;

        if (formatMatches)
        {
                return artwork;
        }

        if (artwork)
        {
                SDL_FreeSurface(artwork);
                artwork = nullptr;
        }

        artwork =
                SDL_CreateRGBSurface(
                        SDL_SWSURFACE,
                        GenericMenuArtwork3DS::WIDTH,
                        GenericMenuArtwork3DS::HEIGHT,
                        screen->format->BitsPerPixel,
                        screen->format->Rmask,
                        screen->format->Gmask,
                        screen->format->Bmask,
                        screen->format->Amask);

        if (!artwork)
        {
                return nullptr;
        }

        bool locked = false;

        if (SDL_MUSTLOCK(artwork))
        {
                if (SDL_LockSurface(artwork) != 0)
                {
                        SDL_FreeSurface(artwork);
                        artwork = nullptr;
                        return nullptr;
                }

                locked = true;
        }

        for (int y = 0;
                y < GenericMenuArtwork3DS::HEIGHT;
                ++y)
        {
                for (int x = 0;
                        x < GenericMenuArtwork3DS::WIDTH;
                        ++x)
                {
                        const std::uint16_t packed =
                                GenericMenuArtwork3DS::PIXELS[
                                        y *
                                        GenericMenuArtwork3DS::WIDTH +
                                        x];

                        const Uint8 red =
                                static_cast<Uint8>(
                                        ((packed >> 11) & 0x1f) *
                                        255 /
                                        31);

                        const Uint8 green =
                                static_cast<Uint8>(
                                        ((packed >> 5) & 0x3f) *
                                        255 /
                                        63);

                        const Uint8 blue =
                                static_cast<Uint8>(
                                        (packed & 0x1f) *
                                        255 /
                                        31);

                        writePixel(
                                artwork,
                                x,
                                y,
                                SDL_MapRGB(
                                        artwork->format,
                                        red,
                                        green,
                                        blue));
                }
        }

        if (locked)
        {
                SDL_UnlockSurface(artwork);
        }

        return artwork;
}




void renderMenuPanel(
        SDL_Surface *screen,
        int bottomY,
        int bottomHeight)
{
        fillRect(
                screen,
                0,
                bottomY,
                screen->w,
                bottomHeight,
                8,
                14,
                24);

        drawTrackpad(
                screen,
                bottomY,
                GENERIC_MENU_TRACKPAD_LEFT,
                GENERIC_MENU_TRACKPAD_TOP,
                GENERIC_MENU_TRACKPAD_RIGHT,
                GENERIC_MENU_TRACKPAD_BOTTOM);

        SDL_Surface *background =
                getEmbeddedGenericMenuArtwork3DS(
                        screen);

        if (!background ||
                background->w <= 0 ||
                background->h <= 0)
        {
                return;
        }

        const int innerLeft =
                GENERIC_MENU_TRACKPAD_LEFT + 4;

        const int innerTop =
                GENERIC_MENU_TRACKPAD_TOP + 4;

        const int innerRight =
                GENERIC_MENU_TRACKPAD_RIGHT - 4;

        const int innerBottom =
                GENERIC_MENU_TRACKPAD_BOTTOM - 4;

        SDL_Rect sourceRect = {};

        sourceRect.x = 0;
        sourceRect.y = 0;

        sourceRect.w =
                static_cast<Uint16>(
                        background->w);

        sourceRect.h =
                static_cast<Uint16>(
                        background->h);

        /*
         * Deliberately stretch the complete artwork to fill the whole
         * touchpad interior. No cropping or aspect-ratio preservation.
         */
        stretchNativeRegion(
                background,
                screen,
                sourceRect,
                bottomY,
                innerLeft,
                innerTop,
                innerRight,
                innerBottom);
}


void renderInventoryPanel(
        SDL_Surface *screen,
        SDL_Surface *,
        int bottomY,
        int bottomHeight)
{
        /*
         * OXCE_3DS_INVENTORY_GREY_DISABLED_TEMPLATES
         */
        fillRect(
                screen,
                0,
                bottomY,
                screen->w,
                bottomHeight,
                8,
                14,
                24);

        drawTrackpad(
                screen,
                bottomY,
                INVENTORY_TRACKPAD_LEFT,
                INVENTORY_TRACKPAD_TOP,
                INVENTORY_TRACKPAD_RIGHT,
                INVENTORY_TRACKPAD_BOTTOM);

        /*
         * OXCE_3DS_INVENTORY_BACK13
         * Purchase/Hire and Sell/Sack background.
         */
        SDL_Surface *background =
                getResourceSurface(
                        "BACK13.SCR");

        if (background)
        {
                SDL_Rect sourceRect = {};

                sourceRect.x = 0;
                sourceRect.y = 0;
                sourceRect.w =
                        static_cast<Uint16>(
                                background->w);
                sourceRect.h =
                        static_cast<Uint16>(
                                background->h);

                stretchNativeRegion(
                        background,
                        screen,
                        sourceRect,
                        bottomY,
                        INVENTORY_TRACKPAD_LEFT + 4,
                        INVENTORY_TRACKPAD_TOP + 4,
                        INVENTORY_TRACKPAD_RIGHT - 4,
                        INVENTORY_TRACKPAD_BOTTOM - 4);

                /*
                 * OXCE_3DS_INVENTORY_BACK13_ALPHA_OVERLAY
                 *
                 * Apply one uniform translucent black layer over the
                 * artwork, rather than alternating dark scanlines.
                 */
                const int overlayLeft =
                        nativeToRenderedX(
                                screen,
                                INVENTORY_TRACKPAD_LEFT + 4);

                const int overlayRight =
                        nativeToRenderedX(
                                screen,
                                INVENTORY_TRACKPAD_RIGHT - 4);

                const int overlayWidth =
                        std::max(
                                1,
                                overlayRight -
                                        overlayLeft);

                const int overlayHeight =
                        std::max(
                                1,
                                INVENTORY_TRACKPAD_BOTTOM -
                                        INVENTORY_TRACKPAD_TOP -
                                        8);

                static SDL_Surface *darkOverlay = nullptr;
                static int darkOverlayWidth = 0;
                static int darkOverlayHeight = 0;

                if (!darkOverlay ||
                        darkOverlayWidth != overlayWidth ||
                        darkOverlayHeight != overlayHeight)
                {
                        if (darkOverlay)
                        {
                                SDL_FreeSurface(
                                        darkOverlay);
                        }

                        darkOverlay =
                                SDL_CreateRGBSurface(
                                        SDL_SWSURFACE,
                                        overlayWidth,
                                        overlayHeight,
                                        screen->format->BitsPerPixel,
                                        screen->format->Rmask,
                                        screen->format->Gmask,
                                        screen->format->Bmask,
                                        screen->format->Amask);

                        darkOverlayWidth =
                                overlayWidth;

                        darkOverlayHeight =
                                overlayHeight;

                        if (darkOverlay)
                        {
                                SDL_FillRect(
                                        darkOverlay,
                                        nullptr,
                                        SDL_MapRGB(
                                                darkOverlay->format,
                                                0,
                                                0,
                                                0));

                                SDL_SetAlpha(
                                        darkOverlay,
                                        SDL_SRCALPHA,
                                        48);
                        }
                }

                if (darkOverlay)
                {
                        SDL_Rect overlayDestination = {};

                        overlayDestination.x =
                                static_cast<Sint16>(
                                        overlayLeft);

                        overlayDestination.y =
                                static_cast<Sint16>(
                                        bottomY +
                                        INVENTORY_TRACKPAD_TOP +
                                        4);

                        SDL_BlitSurface(
                                darkOverlay,
                                nullptr,
                                screen,
                                &overlayDestination);
                }

        }

        std::vector<InventoryControlPlacement> layout;

        buildInventoryControlLayout(layout);

        for (const InventoryControlPlacement &control :
                layout)
        {
                if (!control.surface ||
                        !control.surface->getSurface())
                {
                        continue;
                }

                SDL_Surface *buttonSurface =
                        control.surface->getSurface();

                SDL_Rect sourceRect = {};

                sourceRect.x = 0;
                sourceRect.y = 0;

                sourceRect.w =
                        static_cast<Uint16>(
                                buttonSurface->w);

                sourceRect.h =
                        static_cast<Uint16>(
                                buttonSurface->h);

                stretchNativeRegion(
                        buttonSurface,
                        screen,
                        sourceRect,
                        bottomY,
                        control.drawLeft,
                        control.drawTop,
                        control.drawRight,
                        control.drawBottom);

                if (!control.enabled)
                {
                        const int renderedLeft =
                                nativeToRenderedX(
                                        screen,
                                        control.drawLeft);

                        const int renderedRight =
                                nativeToRenderedX(
                                        screen,
                                        control.drawRight);

                        /*
                         * Cover alternating rows with dark gray. This
                         * keeps the icon recognizable while making its
                         * disabled state clear.
                         */
                        for (int y =
                                        control.drawTop;
                                y <
                                        control.drawBottom;
                                y += 2)
                        {
                                fillRect(
                                        screen,
                                        renderedLeft,
                                        bottomY + y,
                                        std::max(
                                                1,
                                                renderedRight -
                                                        renderedLeft),
                                        1,
                                        58,
                                        58,
                                        58);
                        }
                }
        }
}

void renderGeoscapePanel(
        SDL_Surface *screen,
        SDL_Surface *gameSurface,
        int bottomY,
        int bottomHeight)
{
        /*
         * Use the active game's original high-resolution geoscape
         * artwork as the bottom-screen background.
         */
        SDL_Surface *background =
                getResourceSurface(
                        "ALTGEOBORD.SCR");

        if (background &&
                background->w > 0 &&
                background->h > 0)
        {
                SDL_Rect sourceRect = {};
                sourceRect.x = 0;
                sourceRect.y = 0;
                sourceRect.w =
                        static_cast<Uint16>(
                                background->w);
                sourceRect.h =
                        static_cast<Uint16>(
                                background->h);

                SDL_Rect destinationRect = {};
                destinationRect.x = 0;
                destinationRect.y =
                        static_cast<Sint16>(
                                bottomY);
                destinationRect.w =
                        static_cast<Uint16>(
                                screen->w);
                destinationRect.h =
                        static_cast<Uint16>(
                                bottomHeight);

                stretchRegion(
                        background,
                        screen,
                        sourceRect,
                        destinationRect);
        }
        else
        {
                fillRect(
                        screen,
                        0,
                        bottomY,
                        screen->w,
                        bottomHeight,
                        8,
                        4,
                        16);
        }

        /*
         * Left side:
         *
         *   0..82   live funds, clock and date panel
         *   84..238 trackpad
         *
         * The space beside the information panel is reserved for the
         * later keyboard and cursor-focus controls.
         */
        drawTrackpad(
                screen,
                bottomY,
                2,
                82,
                204,
                238);

        SDL_Surface *geobord =
                getResourceSurface(
                        "GEOBORD.SCR");

        /*
         * Prefer the live 320x200 frame because it already contains
         * translated labels, funds, time, selected speed and pressed
         * states. Fall back to the static artwork when necessary.
         */
        SDL_Surface *panelSource =
                gameSurface &&
                gameSurface->w >= 64 &&
                gameSurface->h >= 200 ?
                        gameSurface :
                        geobord;

        if (!panelSource ||
                panelSource->w < 64 ||
                panelSource->h < 200)
        {
                return;
        }

        const int sidebarSourceX =
                panelSource->w >= 320 ?
                        256 :
                        0;

        /*
         * Live funds, time and date region. Exactly 2x in both axes,
         * keeping the original pixels evenly scaled.
         */
        SDL_Rect informationSource = {};
        informationSource.x = sidebarSourceX;
        informationSource.y = 72;
        informationSource.w = 64;
        informationSource.h = 40;

        stretchNativeRegion(
                panelSource,
                screen,
                informationSource,
                bottomY,
                4,
                2,
                132,
                82);

        /*
         * Six primary command buttons.
         *
         * Original buttons are 63x11. Each becomes 103x18,
         * preserving the source aspect ratio to the nearest pixel.
         */
        const int commandSourceY[] =
        {
                0,
                12,
                24,
                36,
                48,
                60
        };

        for (int i = 0; i < 6; ++i)
        {
                SDL_Rect sourceRect = {};
                sourceRect.x = sidebarSourceX + 1;
                sourceRect.y =
                        static_cast<Sint16>(
                                commandSourceY[i]);
                sourceRect.w = 63;
                sourceRect.h = 11;

                const int destinationTop =
                        1 + i * 20;

                stretchNativeRegion(
                        panelSource,
                        screen,
                        sourceRect,
                        bottomY,
                        GEOSCAPE_COMMAND_LEFT,
                        destinationTop,
                        GEOSCAPE_COMMAND_RIGHT,
                        destinationTop + 18);
        }

        /*
         * Time-speed grid: two columns by three rows. Each
         * 31x13 source becomes a centered 38x16 rectangle.
         */
        const int speedSourceX[] =
        {
                sidebarSourceX + 1,
                sidebarSourceX + 33
        };

        const int speedSourceY[] =
        {
                112,
                126,
                140
        };

        for (int row = 0; row < 3; ++row)
        {
                for (int column = 0;
                        column < 2;
                        ++column)
                {
                        SDL_Rect sourceRect = {};
                        sourceRect.x =
                                static_cast<Sint16>(
                                        speedSourceX[column]);
                        sourceRect.y =
                                static_cast<Sint16>(
                                        speedSourceY[row]);
                        sourceRect.w = 31;
                        sourceRect.h = 13;

                        const int destinationLeft =
                                column == 0 ?
                                        GEOSCAPE_SPEED_LEFT_1 :
                                        GEOSCAPE_SPEED_LEFT_2;

                        const int destinationRight =
                                column == 0 ?
                                        GEOSCAPE_SPEED_RIGHT_1 :
                                        GEOSCAPE_SPEED_RIGHT_2;

                        const int destinationTop =
                                123 +
                                row * 18;

                        stretchNativeRegion(
                                panelSource,
                                screen,
                                sourceRect,
                                bottomY,
                                destinationLeft,
                                destinationTop,
                                destinationRight,
                                destinationTop + 16);
                }
        }

        /*
         * Keep rotation and zoom together as one piece. The
         * original 63x44 block is uniformly fitted into 84x59.
         */
        SDL_Rect rotationSource = {};
        rotationSource.x = sidebarSourceX + 1;
        rotationSource.y = 156;
        rotationSource.w = 63;
        rotationSource.h = 44;

        /*
         * One-pixel frame around the complete rotation/zoom cluster.
         * The artwork and control hitboxes remain unchanged.
         */
        const int globeBorderLeft =
                nativeToRenderedX(
                        screen,
                        GEOSCAPE_GLOBE_LEFT) - 1;

        const int globeBorderRight =
                nativeToRenderedX(
                        screen,
                        GEOSCAPE_GLOBE_RIGHT) + 1;

        fillRect(
                screen,
                globeBorderLeft,
                bottomY +
                        GEOSCAPE_GLOBE_TOP - 1,
                globeBorderRight -
                        globeBorderLeft,
                GEOSCAPE_GLOBE_BOTTOM -
                        GEOSCAPE_GLOBE_TOP + 2,
                32,
                64,
                80);

        stretchNativeRegion(
                panelSource,
                screen,
                rotationSource,
                bottomY,
                GEOSCAPE_GLOBE_LEFT,
                GEOSCAPE_GLOBE_TOP,
                GEOSCAPE_GLOBE_RIGHT,
                GEOSCAPE_GLOBE_BOTTOM);
}


void renderTftdBattlescapePanel(
        SDL_Surface *screen,
        SDL_Surface *panelSource,
        int bottomY)
{
        if (!screen || !panelSource)
        {
                return;
        }

        /*
         * The live Battlescape interface occupies the final 56 rows
         * of the 320x200 source frame.
         */
        const int sourceBarHeight =
                std::min(
                        BATTLESCAPE_BAR_HEIGHT,
                        panelSource->h);

        const int sourceBarTop =
                panelSource->h -
                        sourceBarHeight;

        const int sourceBarLeft =
                std::max(
                        0,
                        (panelSource->w -
                                BOTTOM_NATIVE_WIDTH) /
                                2);

        auto drawPiece =
                [&](int sourceX,
                    int sourceY,
                    int sourceWidth,
                    int sourceHeight,
                    int nativeLeft,
                    int nativeTop,
                    int nativeRight,
                    int nativeBottom)
        {
                SDL_Rect sourceRect = {};

                sourceRect.x =
                        static_cast<Sint16>(
                                sourceBarLeft +
                                        sourceX);

                sourceRect.y =
                        static_cast<Sint16>(
                                sourceBarTop +
                                        sourceY);

                sourceRect.w =
                        static_cast<Uint16>(
                                sourceWidth);

                sourceRect.h =
                        static_cast<Uint16>(
                                sourceHeight);

                stretchNativeRegion(
                        panelSource,
                        screen,
                        sourceRect,
                        bottomY,
                        nativeLeft,
                        nativeTop,
                        nativeRight,
                        nativeBottom);
        };

        /*
         * Complete TFTD two-row command bank.
         *
         * Source:      224x32
         * Destination: 320x46
         *
         * The small fractional difference is only integer-pixel
         * rounding from the uniform 10/7 scale.
         */
        drawPiece(
                48,
                0,
                224,
                32,
                0,
                0,
                320,
                46);

        /*
         * Complete reserve and soldier-information strip.
         *
         * Keep this as one uninterrupted source region so all reserve
         * controls, the anchor, the soldier name, numeric statistics,
         * bars and weapon information remain aligned left-to-right.
         *
         * Source:      224x24
         * Destination: 320x34
         */
        /*
         * Complete TFTD lower strip, including the full vertical
         * Zero TUs button at its left edge.
         *
         * Source: x=43..272, 229x24
         * Destination: 320x34
         */
        drawPiece(
                40,
                32,
                232,
                24,
                0,
                46,
                320,
                79);

        /*
         * Both original 48x56 hand slots use the same destination size.
         * The right-hand slot is above the left-hand slot.
         */
        drawPiece(
                272,
                0,
                48,
                56,
                251,
                80,
                320,
                160);

        drawPiece(
                0,
                0,
                48,
                56,
                251,
                160,
                320,
                240);

        /*
         * Leave the remaining central area clean and usable as the
         * Battlescape touchpad.
         */
        drawTrackpad(
                screen,
                bottomY,
                BATTLESCAPE_TRACKPAD_LEFT,
                BATTLESCAPE_TRACKPAD_TOP,
                BATTLESCAPE_TRACKPAD_RIGHT,
                BATTLESCAPE_TRACKPAD_BOTTOM);
}

void renderBattlescapePanel(
        SDL_Surface *screen,
        SDL_Surface *gameSurface,
        int bottomY,
        int bottomHeight)
{
        fillRect(
                screen,
                0,
                bottomY,
                screen->w,
                bottomHeight,
                0,
                0,
                0);

        SDL_Surface *panelSource =
                gameSurface;

        BattlescapeState *battlescape =
                BattlescapeState::getActive3DS();

        if (battlescape)
        {
                SDL_Surface *livePanelSource =
                        battlescape->
                                getBottomPanelSource3DS();

                if (livePanelSource)
                {
                        panelSource =
                                livePanelSource;
                }
        }

        if (!panelSource)
        {
                return;
        }

        if (isTftdMaster3DS())
        {
                renderTftdBattlescapePanel(
                        screen,
                        panelSource,
                        bottomY);
                return;
        }

        constexpr int handSlotSourceWidth = 48;
        constexpr int handSlotSourceHeight = 56;

        /*
         * Same scale as the top strip:
         * 48 * (320 / 224) = 68.57..., rounded to 69
         */
        constexpr int handSlotTargetWidth = 69;
        constexpr int handSlotTargetHeight = 80;
        constexpr int handColumnLeft =
                BOTTOM_NATIVE_WIDTH - handSlotTargetWidth; // 251

        const int sourceBarHeight =
                std::min(
                        BATTLESCAPE_BAR_HEIGHT,
                        panelSource->h);

        const int sourceBarTop =
                panelSource->h -
                sourceBarHeight;

        const int sourceBarLeft =
                std::max(
                        0,
                        (panelSource->w -
                                BOTTOM_NATIVE_WIDTH) /
                                2);

        auto scaledX = [](
                int originalX)
        {
                constexpr int sourceLeft = 48;
                constexpr int sourceRight = 272;
                constexpr int sourceWidth = sourceRight - sourceLeft;

                const int clampedX =
                        std::max(
                                sourceLeft,
                                std::min(
                                        sourceRight,
                                        originalX));

                return (clampedX - sourceLeft) *
                        BOTTOM_NATIVE_WIDTH /
                        sourceWidth;
        };

        auto scaledY = [](
                int originalY)
        {
                constexpr int sourceHeight = 56;
                constexpr int targetHeight = 80;

                const int clampedY =
                        std::max(
                                0,
                                std::min(
                                        sourceHeight,
                                        originalY));

                return clampedY *
                        targetHeight /
                        sourceHeight;
        };

        auto drawScaledTopPiece = [&](
                int sourceX,
                int sourceY,
                int sourceW,
                int sourceH,
                int visualLeft,
                int visualTop,
                int visualRight,
                int visualBottom)
        {
                SDL_Rect sourceRect = {};
                sourceRect.x =
                        static_cast<Sint16>(
                                sourceBarLeft +
                                sourceX);
                sourceRect.y =
                        static_cast<Sint16>(
                                sourceBarTop +
                                sourceY);
                sourceRect.w =
                        static_cast<Uint16>(
                                sourceW);
                sourceRect.h =
                        static_cast<Uint16>(
                                sourceH);

                stretchNativeRegion(
                        panelSource,
                        screen,
                        sourceRect,
                        bottomY,
                        scaledX(visualLeft),
                        scaledY(visualTop),
                        scaledX(visualRight),
                        scaledY(visualBottom));
        };

        auto drawHandSlot = [&](
                int sourceX,
                int sourceY,
                int nativeTop)
        {
                SDL_Rect sourceRect = {};
                sourceRect.x =
                        static_cast<Sint16>(
                                sourceBarLeft +
                                sourceX);
                sourceRect.y =
                        static_cast<Sint16>(
                                sourceBarTop +
                                sourceY);
                sourceRect.w =
                        static_cast<Uint16>(
                                handSlotSourceWidth);
                sourceRect.h =
                        static_cast<Uint16>(
                                handSlotSourceHeight);

                stretchNativeRegion(
                        panelSource,
                        screen,
                        sourceRect,
                        bottomY,
                        handColumnLeft,
                        nativeTop,
                        BOTTOM_NATIVE_WIDTH,
                        nativeTop +
                                handSlotTargetHeight);
        };

        /*
         * Top strip only:
         * draw original x=48..272, y=0..56 uniformly scaled to
         * the full 320x80 top area.
         */

        /* Unit and map level controls. */
        drawScaledTopPiece( 48,  0, 32, 16,  48,  0,  80, 16);
        drawScaledTopPiece( 48, 16, 32, 16,  48, 16,  80, 32);
        drawScaledTopPiece( 80,  0, 32, 16,  80,  0, 112, 16);
        drawScaledTopPiece( 80, 16, 32, 16,  80, 16, 112, 32);

        /* Main command buttons. */
        drawScaledTopPiece(112,  0, 32, 16, 112,  0, 144, 16);
        drawScaledTopPiece(112, 16, 32, 16, 112, 16, 144, 32);
        drawScaledTopPiece(144,  0, 32, 16, 144,  0, 176, 16);
        drawScaledTopPiece(144, 16, 32, 16, 144, 16, 176, 32);
        drawScaledTopPiece(176,  0, 32, 16, 176,  0, 208, 16);
        drawScaledTopPiece(176, 16, 32, 16, 176, 16, 208, 32);
        drawScaledTopPiece(208,  0, 32, 16, 208,  0, 240, 16);
        drawScaledTopPiece(208, 16, 32, 16, 208, 16, 240, 32);
        drawScaledTopPiece(240,  0, 32, 16, 240,  0, 272, 16);
        drawScaledTopPiece(240, 16, 32, 16, 240, 16, 272, 32);

        /*
         * Lower center row:
         *
         * Copy the entire original x=48..272, y=32..56 area as
         * one continuous region. Besides the reserve controls and
         * soldier status display, this region is temporarily replaced
         * by WarningMessage. Keeping it continuous prevents black
         * button-shaped gaps from appearing inside warning popups.
         */
        drawScaledTopPiece(
                48,
                32,
                224,
                24,
                48,
                32,
                272,
                56);

        /*
         * Hand slots:
         * same aspect-preserving scale as the top strip,
         * stacked below it on the far right.
         */
        drawHandSlot(272, 0,  80);  // Right hand
        drawHandSlot(  0, 0, 160);  // Left hand

        drawTrackpad(
                screen,
                bottomY,
                BATTLESCAPE_TRACKPAD_LEFT,
                BATTLESCAPE_TRACKPAD_TOP,
                BATTLESCAPE_TRACKPAD_RIGHT,
                BATTLESCAPE_TRACKPAD_BOTTOM);

}










}

void requestPanelRefresh()
{
        invalidatePanel();
}


void setGame(Game *game)
{
        if (currentGame != game)
        {
                currentGame = game;

                geoscapeSidebarFrame = nullptr;
                geoscapeSidebarBuffer = nullptr;

                invalidatePanel();
        }
}

void captureGameFrame(SDL_Surface *gameSurface)
{
        if (!copySnapshot(
                gameSurface,
                cleanFrameBuffer,
                cleanFrame))
        {
                return;
        }

        /*
         * Check the actual top state rather than the input-selected
         * mode, which can lag during state transitions.
         */
        GeoscapeState *geoscape =
                currentGame ?
                        currentGame->getGeoscapeState() :
                        nullptr;

        if (geoscape &&
                currentGame->isState(geoscape))
        {
                copySnapshot(
                        gameSurface,
                        frozenGeoscapeBuffer,
                        frozenGeoscapeFrame);
        }
}

void setMode(Mode mode)
{
        if (currentMode != mode)
        {
                currentMode = mode;
                invalidatePanel();

                /*
                 * OXCE_3DS_INVENTORY_BOTTOM_PANEL
                 *
                 * Start inventory bottom-control focus from the central
                 * action button, but require Select before it is active.
                 */
                if (currentMode == MODE_INVENTORY)
                {
                        bottomCursorX = 160;
                        bottomCursorY = 31;
                        bottomCursorFocused = false;
                }
        }

        /*
         * The generic lower menu panel does not contain controls that
         * use the dedicated bottom cursor.
         */
        /* OXCE_3DS_KEYBOARD_MENU_FOCUS_PERSISTENCE */
        if (currentMode == MODE_MENU &&
                !Keyboard3DS::isVisible())
        {
                bottomCursorFocused = false;
        }
}

Mode getMode()
{
        return currentMode;
}

void setBottomCursorFocused(bool focused)
{
        if (currentMode == MODE_MENU &&
                !Keyboard3DS::isVisible())
        {
                bottomCursorFocused = false;
                return;
        }

        bottomCursorFocused = focused;
}

bool isBottomCursorFocused()
{
        return bottomCursorFocused;
}

void getBottomCursorPosition(int &x, int &y)
{
        x = bottomCursorX;
        y = bottomCursorY;
}

void toggleCursorFocus()
{
        setBottomCursorFocused(
                !bottomCursorFocused);
}

void moveBottomCursor(int dx, int dy)
{
        bottomCursorX =
                std::max(
                        0,
                        std::min(
                                BOTTOM_NATIVE_WIDTH - 1,
                                bottomCursorX + dx));

        bottomCursorY =
                std::max(
                        0,
                        std::min(
                                BOTTOM_NATIVE_HEIGHT - 1,
                                bottomCursorY + dy));
}

struct BattlescapeSnapTarget3DS
{
        int left;
        int top;
        int right;
        int bottom;
};

static const BattlescapeSnapTarget3DS BATTLESCAPE_SNAP_TARGETS[] =
{
        {  0,  0,  46, 23},
        {  0, 23,  46, 46},

        { 46,  0,  91, 23},
        { 46, 23,  91, 46},

        { 91,  0, 137, 23},
        { 91, 23, 137, 46},

        {137,  0, 183, 23},
        {137, 23, 183, 46},

        {183,  0, 229, 23},
        {183, 23, 229, 46},

        {229,  0, 274, 23},
        {229, 23, 274, 46},

        {274,  0, 320, 23},
        {274, 23, 320, 46},

        {  1, 47,  16, 80},
        { 17, 47,  41, 63},
        { 43, 47,  67, 63},
        { 17, 64,  41, 80},
        { 43, 64,  67, 80},
        { 69, 47,  83, 80},

        { 84, 47, 251, 80},

        {251,  80, 320, 160},
        {251, 160, 320, 240}
};

constexpr int BATTLESCAPE_SNAP_TARGET_COUNT =
        sizeof(BATTLESCAPE_SNAP_TARGETS) /
        sizeof(BATTLESCAPE_SNAP_TARGETS[0]);


BattlescapeSnapTarget3DS getBattlescapeSnapTarget3DS(
        int snapIndex)
{
        if (snapIndex < 0 ||
                snapIndex >=
                        BATTLESCAPE_SNAP_TARGET_COUNT)
        {
                return {0, 0, 0, 0};
        }

        if (Options::getActiveMaster() == "xcom2")
        {
                switch (snapIndex)
                {
                case 14: // Zero TUs
                        return {0, 46, 17, 80};

                case 15: // Don't reserve
                        return {17, 46, 50, 63};

                case 16: // Snapshot
                        return {50, 46, 80, 63};

                case 17: // Aimed
                        return {17, 63, 50, 80};

                case 18: // Auto
                        return {50, 63, 80, 80};

                case 19: // Reserve kneel
                        return {80, 46, 95, 80};

                default:
                        break;
                }
        }

        return BATTLESCAPE_SNAP_TARGETS[
                snapIndex];
}

/*
 * Explicit Battlescape snap graph.
 *
 * Columns are Up, Down, Left, Right.
 * -1 means no target in that direction.
 */
static const int BATTLESCAPE_SNAP_NEIGHBORS[
        BATTLESCAPE_SNAP_TARGET_COUNT][4] =
{
        /*  0 Unit up        */ {-1,  1, -1,  2},
        /*  1 Unit down      */ { 0, 15, -1,  3},

        /*  2 Map up         */ {-1,  3,  0,  4},
        /*  3 Map down       */ { 2, 16,  1,  5},

        /*  4 Show map       */ {-1,  5,  2,  6},
        /*  5 Kneel          */ { 4, 20,  3,  7},

        /*  6 Inventory      */ {-1,  7,  4,  8},
        /*  7 Center         */ { 6, 20,  5,  9},

        /*  8 Next soldier   */ {-1,  9,  6, 10},
        /*  9 Next stop      */ { 8, 20,  7, 11},

        /* 10 Show layers    */ {-1, 11,  8, 12},
        /* 11 Help           */ {10, 20,  9, 13},

        /* 12 End turn       */ {-1, 13, 10, -1},
        /* 13 Abort          */ {12, 21, 11, -1},

        /* 14 Zero TUs       */ { 1, 17, -1, 15},
        /* 15 Reserve none   */ { 1, 17, 14, 16},
        /* 16 Reserve snap   */ { 3, 18, 15, 19},
        /* 17 Reserve aimed  */ {15, -1, 14, 18},
        /* 18 Reserve auto   */ {16, -1, 17, 19},
        /* 19 Reserve kneel  */ {16, -1, 18, 20},

        /* 20 Stats panel    */ { 7, 22, 19, 21},

        /* 21 Right hand     */ {13, 22, 20, -1},
        /* 22 Left hand      */ {21, -1, 20, -1}
};

int abs3DS(int value)
{
        return value < 0 ? -value : value;
}

bool snapBattlescapeBottomCursor3DS(
        int horizontal,
        int vertical)
{
        int direction = -1;

        if (vertical < 0)
        {
                direction = 0;
        }
        else if (vertical > 0)
        {
                direction = 1;
        }
        else if (horizontal < 0)
        {
                direction = 2;
        }
        else if (horizontal > 0)
        {
                direction = 3;
        }

        if (direction < 0)
        {
                return false;
        }

        int currentTarget = -1;

        for (int index = 0;
                index < BATTLESCAPE_SNAP_TARGET_COUNT;
                ++index)
        {
                const BattlescapeSnapTarget3DS target =
                        getBattlescapeSnapTarget3DS(
                                index);

                if (pointInRect(
                        bottomCursorX,
                        bottomCursorY,
                        target.left,
                        target.top,
                        target.right,
                        target.bottom))
                {
                        currentTarget = index;
                        break;
                }
        }

        if (currentTarget >= 0)
        {
                const int graphTarget =
                        BATTLESCAPE_SNAP_NEIGHBORS[
                                currentTarget][
                                direction];

                if (graphTarget < 0)
                {
                        return false;
                }

                const BattlescapeSnapTarget3DS target =
                        getBattlescapeSnapTarget3DS(
                                graphTarget);

                bottomCursorX =
                        (target.left +
                                target.right) /
                        2;

                bottomCursorY =
                        (target.top +
                                target.bottom) /
                        2;

                return true;
        }

        int bestTarget = -1;
        int bestScore = INT_MAX;

        for (int index = 0;
                index < BATTLESCAPE_SNAP_TARGET_COUNT;
                ++index)
        {
                const BattlescapeSnapTarget3DS target =
                        getBattlescapeSnapTarget3DS(
                                index);

                const int centerX =
                        (target.left +
                                target.right) /
                        2;

                const int centerY =
                        (target.top +
                                target.bottom) /
                        2;

                const int deltaX =
                        centerX -
                        bottomCursorX;

                const int deltaY =
                        centerY -
                        bottomCursorY;

                if ((direction == 0 && deltaY >= 0) ||
                        (direction == 1 && deltaY <= 0) ||
                        (direction == 2 && deltaX >= 0) ||
                        (direction == 3 && deltaX <= 0))
                {
                        continue;
                }

                const int primaryDistance =
                        direction < 2 ?
                                abs3DS(deltaY) :
                                abs3DS(deltaX);

                const int secondaryDistance =
                        direction < 2 ?
                                abs3DS(deltaX) :
                                abs3DS(deltaY);

                const int score =
                        primaryDistance * 4 +
                        secondaryDistance * 16;

                if (score < bestScore)
                {
                        bestScore = score;
                        bestTarget = index;
                }
        }

        if (bestTarget < 0)
        {
                return false;
        }

        const BattlescapeSnapTarget3DS target =
                getBattlescapeSnapTarget3DS(
                        bestTarget);

        bottomCursorX =
                (target.left +
                        target.right) /
                2;

        bottomCursorY =
                (target.top +
                        target.bottom) /
                2;

        return true;
}


bool snapBottomCursor(
        int horizontal,
        int vertical)
{

        /*
         * 3DS Battlescape lower panel snapping. This runs before the
         * original Geoscape-only guard below.
         */
        if (!Keyboard3DS::isVisible() &&
                currentMode == MODE_BATTLESCAPE &&
                (horizontal != 0 ||
                        vertical != 0))
        {
                return snapBattlescapeBottomCursor3DS(
                        horizontal,
                        vertical);
        }

        if (horizontal == 0 &&
                vertical == 0)
        {
                return false;
        }

        if (Keyboard3DS::isVisible())
        {
                return Keyboard3DS::snapCursor(
                        bottomCursorX,
                        bottomCursorY,
                        horizontal,
                        vertical);
        }

        int direction = -1;

        if (vertical < 0)
        {
                direction = 0;
        }
        else if (vertical > 0)
        {
                direction = 1;
        }
        else if (horizontal < 0)
        {
                direction = 2;
        }
        else if (horizontal > 0)
        {
                direction = 3;
        }


        if (currentMode == MODE_INVENTORY)
        {
                std::vector<InventoryControlPlacement> layout;

                buildInventoryControlLayout(
                        layout);

                if (layout.empty())
                {
                        return false;
                }

                int currentControl = -1;

                for (std::size_t index = 0;
                        index < layout.size();
                        ++index)
                {
                        const InventoryControlPlacement &control =
                                layout[index];

                        if (pointInRect(
                                bottomCursorX,
                                bottomCursorY,
                                control.hitLeft,
                                control.hitTop,
                                control.hitRight,
                                control.hitBottom))
                        {
                                currentControl =
                                        static_cast<int>(
                                                index);
                                break;
                        }
                }

                int targetControl = -1;

                if (currentControl >= 0)
                {
                        if (direction == 2 &&
                                currentControl > 0)
                        {
                                targetControl =
                                        currentControl - 1;
                        }
                        else if (direction == 3 &&
                                currentControl + 1 <
                                        static_cast<int>(
                                                layout.size()))
                        {
                                targetControl =
                                        currentControl + 1;
                        }
                }
                else
                {
                        int bestDistance = INT_MAX;

                        for (std::size_t index = 0;
                                index < layout.size();
                                ++index)
                        {
                                const InventoryControlPlacement &control =
                                        layout[index];

                                const int centerX =
                                        (control.drawLeft +
                                         control.drawRight) /
                                        2;

                                const int centerY =
                                        (control.drawTop +
                                         control.drawBottom) /
                                        2;

                                const int deltaX =
                                        centerX -
                                        bottomCursorX;

                                const int deltaY =
                                        centerY -
                                        bottomCursorY;

                                if ((direction == 0 &&
                                                deltaY >= 0) ||
                                        (direction == 1 &&
                                                deltaY <= 0) ||
                                        (direction == 2 &&
                                                deltaX >= 0) ||
                                        (direction == 3 &&
                                                deltaX <= 0))
                                {
                                        continue;
                                }

                                const int distance =
                                        deltaX * deltaX +
                                        deltaY * deltaY;

                                if (distance < bestDistance)
                                {
                                        bestDistance = distance;

                                        targetControl =
                                                static_cast<int>(
                                                        index);
                                }
                        }
                }

                if (targetControl < 0)
                {
                        return false;
                }

                const InventoryControlPlacement &target =
                        layout[targetControl];

                bottomCursorX =
                        (target.drawLeft +
                         target.drawRight) /
                        2;

                bottomCursorY =
                        (target.drawTop +
                         target.drawBottom) /
                        2;

                return true;
        }

        if (currentMode != MODE_GEOSCAPE)
        {
                return false;
        }

        int currentControl = -1;

        for (int index = 0;
                index < BOTTOM_CONTROL_COUNT;
                ++index)
        {
                const ControlTarget &control =
                        BOTTOM_CONTROLS[index];

                if (pointInRect(
                        bottomCursorX,
                        bottomCursorY,
                        control.left,
                        control.top,
                        control.right,
                        control.bottom))
                {
                        currentControl = index;
                        break;
                }
        }

        int targetControl = -1;

        if (currentControl >= 0)
        {
                targetControl =
                        SNAP_NEIGHBORS[
                                currentControl][
                                direction];
        }
        else
        {
                /*
                 * After free Circle Pad movement, choose the closest
                 * reasonably aligned control in the requested
                 * direction.
                 */
                int bestScore = INT_MAX;

                for (int index = 0;
                        index < BOTTOM_CONTROL_COUNT;
                        ++index)
                {
                        const ControlTarget &control =
                                BOTTOM_CONTROLS[index];

                        const int centerX =
                                (control.left +
                                        control.right) /
                                2;

                        const int centerY =
                                (control.top +
                                        control.bottom) /
                                2;

                        const int deltaX =
                                centerX -
                                bottomCursorX;

                        const int deltaY =
                                centerY -
                                bottomCursorY;

                        if ((direction == 0 &&
                                        deltaY >= 0) ||
                                (direction == 1 &&
                                        deltaY <= 0) ||
                                (direction == 2 &&
                                        deltaX >= 0) ||
                                (direction == 3 &&
                                        deltaX <= 0))
                        {
                                continue;
                        }

                        const int primaryDistance =
                                direction < 2 ?
                                        (deltaY < 0 ?
                                                -deltaY :
                                                deltaY) :
                                        (deltaX < 0 ?
                                                -deltaX :
                                                deltaX);

                        const int secondaryDistance =
                                direction < 2 ?
                                        (deltaX < 0 ?
                                                -deltaX :
                                                deltaX) :
                                        (deltaY < 0 ?
                                                -deltaY :
                                                deltaY);

                        /*
                         * Favor directional distance, but strongly
                         * prefer controls aligned with the cursor.
                         */
                        const int score =
                                primaryDistance * 4 +
                                secondaryDistance * 8;

                        if (score < bestScore)
                        {
                                bestScore = score;
                                targetControl = index;
                        }
                }
        }

        if (targetControl < 0)
        {
                return false;
        }

        const ControlTarget &target =
                BOTTOM_CONTROLS[targetControl];

        bottomCursorX =
                (target.left +
                        target.right) /
                2;

        bottomCursorY =
                (target.top +
                        target.bottom) /
                2;

        return true;
}

void getBattlescapeControlNativeRect3DS(
        int index,
        int &left,
        int &top,
        int &right,
        int &bottom)
{
        const ControlTarget &control =
                BATTLESCAPE_CONTROLS[index];

        auto mapRange =
                [](int value,
                   int sourceStart,
                   int sourceEnd,
                   int destinationStart,
                   int destinationEnd)
        {
                const int clamped =
                        std::max(
                                sourceStart,
                                std::min(
                                        sourceEnd,
                                        value));

                return destinationStart +
                        (clamped - sourceStart) *
                        (destinationEnd -
                                destinationStart) /
                        (sourceEnd -
                                sourceStart);
        };

        const bool tftd =
                Options::getActiveMaster() ==
                        "xcom2";

        if (tftd)
        {
                /*
                 * Final cropped TFTD hand slots.
                 */
                if (index < 2)
                {
                        left = 257;
                        right = 320;
                        top = control.top;
                        bottom = control.bottom;
                        return;
                }

                /*
                 * Main two-row command bank.
                 */
                if (index <= 15)
                {
                        left =
                                mapRange(
                                        control.left,
                                        48,
                                        272,
                                        0,
                                        320);

                        right =
                                mapRange(
                                        control.right,
                                        48,
                                        272,
                                        0,
                                        320);

                        top =
                                mapRange(
                                        control.top,
                                        0,
                                        32,
                                        0,
                                        46);

                        bottom =
                                mapRange(
                                        control.bottom,
                                        0,
                                        32,
                                        0,
                                        46);

                        return;
                }

                /*
                 * The OXCE object rectangles are inset within the
                 * TFTD graphics and therefore appear too far right
                 * when scaled directly.
                 *
                 * Use the actual visible frame positions instead.
                 */
                switch (index)
                {
                case 16: // Don't reserve
                        left = 17;
                        top = 47;
                        right = 41;
                        bottom = 63;
                        return;

                case 17: // Reserve snap shot
                        left = 43;
                        top = 47;
                        right = 67;
                        bottom = 63;
                        return;

                case 18: // Reserve aimed shot
                        left = 17;
                        top = 64;
                        right = 41;
                        bottom = 80;
                        return;

                case 19: // Reserve auto shot
                        left = 43;
                        top = 64;
                        right = 67;
                        bottom = 80;
                        return;

                case 20: // Reserve kneeling
                        left = 69;
                        top = 47;
                        right = 83;
                        bottom = 80;
                        return;

                case 21: // Expend all TUs
                        left = 1;
                        top = 47;
                        right = 16;
                        bottom = 80;
                        return;

                default:
                        break;
                }
        }

        /*
         * Existing UFO Defense mapping.
         */
        if (index < 2)
        {
                left = control.left;
                top = control.top;
                right = control.right;
                bottom = control.bottom;
                return;
        }

        left =
                battlescapeVisualToNativeX(
                        control.left);

        right =
                battlescapeVisualToNativeX(
                        control.right);

        top =
                battlescapeVisualToNativeY(
                        control.top);

        bottom =
                battlescapeVisualToNativeY(
                        control.bottom);
}


bool getControlTargetAt(
        int x,
        int y,
        int &logicalX,
        int &logicalY,
        bool &opensTopPanel)
{
        if (Keyboard3DS::isVisible())
        {
                return false;
        }

        if (currentMode == MODE_INVENTORY)
        {
                std::vector<InventoryControlPlacement> layout;

                buildInventoryControlLayout(
                        layout);

                for (const InventoryControlPlacement &control :
                        layout)
                {
                        if (!control.enabled)
                        {
                                continue;
                        }

                        if (!pointInRect(
                                x,
                                y,
                                control.hitLeft,
                                control.hitTop,
                                control.hitRight,
                                control.hitBottom))
                        {
                                continue;
                        }

                        logicalX = control.logicalX;
                        logicalY = control.logicalY;
                        opensTopPanel = false;
                        return true;
                }

                return false;
        }

        if (currentMode == MODE_BATTLESCAPE)
        {
                const int logicalLeft =
                        battlescapeBarLogicalLeft();

                const int logicalTop =
                        battlescapeBarLogicalTop();

        /* OXCE_3DS_TFTD_RESERVE_DIRECT_HITBOXES
         *
         * Exact visible partitions in the final TFTD status strip:
         *
         *   Zero TUs:       x=0..16,  y=46..79
         *   Don't reserve:  x=17..49, y=46..62
         *   Snapshot:       x=50..79, y=46..62
         *   Aimed:          x=17..49, y=63..79
         *   Auto:           x=50..79, y=63..79
         *   Reserve kneel:  x=80..94, y=46..79
         */
        if (Options::getActiveMaster() == "xcom2" &&
                x >= 0 &&
                x < 95 &&
                y >= 46 &&
                y < 80)
        {
                int directControl = -1;

                if (x < 17)
                {
                        directControl = 21;
                }
                else if (x < 50)
                {
                        directControl =
                                y < 63 ?
                                        16 :
                                        18;
                }
                else if (x < 80)
                {
                        directControl =
                                y < 63 ?
                                        17 :
                                        19;
                }
                else
                {
                        directControl = 20;
                }

                const ControlTarget &control =
                        BATTLESCAPE_CONTROLS[
                                directControl];

                logicalX =
                        logicalLeft +
                        control.logicalX;

                logicalY =
                        logicalTop +
                        control.logicalY;

                opensTopPanel =
                        control.opensTopPanel;

                return true;
        }
        /* OXCE_3DS_TFTD_RESERVE_DIRECT_HITBOXES_END */


                /*
                 * Test the individual controls before the large
                 * soldier-status area.
                 */
                for (int index = 0;
                        index <
                                BATTLESCAPE_CONTROL_COUNT;
                        ++index)
                {
                        const ControlTarget &control =
                                BATTLESCAPE_CONTROLS[
                                        index];

                        int nativeLeft = 0;
                        int nativeTop = 0;
                        int nativeRight = 0;
                        int nativeBottom = 0;

                        getBattlescapeControlNativeRect3DS(
                                index,
                                nativeLeft,
                                nativeTop,
                                nativeRight,
                                nativeBottom);

                        if (!pointInRect(
                                x,
                                y,
                                nativeLeft,
                                nativeTop,
                                nativeRight,
                                nativeBottom))
                        {
                                continue;
                        }

                        logicalX =
                                logicalLeft +
                                control.logicalX;

                        logicalY =
                                logicalTop +
                                control.logicalY;

                        opensTopPanel =
                                control.opensTopPanel;

                        return true;
                }

                int statsLeft =
                        BATTLESCAPE_STATS_PANEL_LEFT;

                int statsTop =
                        BATTLESCAPE_STATS_PANEL_TOP;

                int statsRight =
                        BATTLESCAPE_STATS_PANEL_RIGHT;

                int statsBottom =
                        BATTLESCAPE_STATS_PANEL_BOTTOM;

                if (Options::getActiveMaster() ==
                        "xcom2")
                {
                        statsLeft =
                                (107 - 40) *
                                320 /
                                (272 - 40);

                        statsTop = 46;
                        statsRight = 320;
                        statsBottom = 79;
                }

                if (pointInRect(
                        x,
                        y,
                        statsLeft,
                        statsTop,
                        statsRight,
                        statsBottom))
                {
                        logicalX =
                                BATTLESCAPE_STATS_LOGICAL_X;

                        logicalY =
                                BATTLESCAPE_STATS_LOGICAL_Y;

                        opensTopPanel = false;
                        return true;
                }

                return false;
        }

        if (currentMode != MODE_GEOSCAPE &&
                currentMode !=
                        MODE_GEOSCAPE_FROZEN)
        {
                return false;
        }

        constexpr int FIRST_GLOBE_CONTROL = 12;
        constexpr int GLOBE_TOUCH_PADDING = 6;

        const int firstControl =
                currentMode ==
                        MODE_GEOSCAPE_FROZEN ?
                        FIRST_GLOBE_CONTROL :
                        0;

        for (int index = firstControl;
                index < BOTTOM_CONTROL_COUNT;
                ++index)
        {
                const ControlTarget &control =
                        BOTTOM_CONTROLS[index];

                if (pointInRect(
                        x,
                        y,
                        control.left,
                        control.top,
                        control.right,
                        control.bottom))
                {
                        logicalX =
                                control.logicalX;

                        logicalY =
                                control.logicalY;

                        opensTopPanel =
                                control.opensTopPanel;

                        return true;
                }
        }

        int nearestControl = -1;
        int nearestDistanceSquared = INT_MAX;

        const int firstPaddedControl =
                std::max(
                        firstControl,
                        FIRST_GLOBE_CONTROL);

        for (int index = firstPaddedControl;
                index < BOTTOM_CONTROL_COUNT;
                ++index)
        {
                const ControlTarget &control =
                        BOTTOM_CONTROLS[index];

                if (!pointInRect(
                        x,
                        y,
                        control.left -
                                GLOBE_TOUCH_PADDING,
                        control.top -
                                GLOBE_TOUCH_PADDING,
                        control.right +
                                GLOBE_TOUCH_PADDING,
                        control.bottom +
                                GLOBE_TOUCH_PADDING))
                {
                        continue;
                }

                const int centerX =
                        (control.left +
                                control.right) /
                        2;

                const int centerY =
                        (control.top +
                                control.bottom) /
                        2;

                const int deltaX =
                        x - centerX;

                const int deltaY =
                        y - centerY;

                const int distanceSquared =
                        deltaX * deltaX +
                        deltaY * deltaY;

                if (distanceSquared <
                        nearestDistanceSquared)
                {
                        nearestDistanceSquared =
                                distanceSquared;

                        nearestControl = index;
                }
        }

        if (nearestControl >= 0)
        {
                const ControlTarget &control =
                        BOTTOM_CONTROLS[
                                nearestControl];

                logicalX = control.logicalX;
                logicalY = control.logicalY;

                opensTopPanel =
                        control.opensTopPanel;

                return true;
        }

        return false;
}



bool getBottomCursorControlTarget(
        int &logicalX,
        int &logicalY,
        bool &opensTopPanel)
{
        return getControlTargetAt(
                bottomCursorX,
                bottomCursorY,
                logicalX,
                logicalY,
                opensTopPanel);
}

bool isTrackpadPoint(int x, int y)
{
        if (Keyboard3DS::isVisible())
        {
                return false;
        }

        switch (currentMode)
        {
        case MODE_GEOSCAPE:
        case MODE_GEOSCAPE_FROZEN:
                return
                        pointInRect(
                                x,
                                y,
                                4,
                                84,
                                202,
                                236) ||
                        pointInRect(
                                x,
                                y,
                                134,
                                4,
                                202,
                                82);

        case MODE_BATTLESCAPE:
                return pointInRect(
                        x,
                        y,
                        BATTLESCAPE_TRACKPAD_LEFT,
                        BATTLESCAPE_TRACKPAD_TOP,
                        BATTLESCAPE_TRACKPAD_RIGHT,
                        BATTLESCAPE_TRACKPAD_BOTTOM);

        case MODE_MENU:
                return pointInRect(
                        x,
                        y,
                        GENERIC_MENU_TRACKPAD_LEFT,
                        GENERIC_MENU_TRACKPAD_TOP,
                        GENERIC_MENU_TRACKPAD_RIGHT,
                        GENERIC_MENU_TRACKPAD_BOTTOM);

        default:
                break;
        }

        const bool insideTrackpad =
                pointInRect(
                        x,
                        y,
                        MENU_TRACKPAD_LEFT,
                        MENU_TRACKPAD_TOP,
                        MENU_TRACKPAD_RIGHT,
                        MENU_TRACKPAD_BOTTOM);

        const bool insideKeyboardButton =
                pointInRect(
                        x,
                        y,
                        KEYBOARD_BUTTON_LEFT,
                        KEYBOARD_BUTTON_TOP,
                        KEYBOARD_BUTTON_RIGHT,
                        KEYBOARD_BUTTON_BOTTOM);

        return insideTrackpad &&
                !insideKeyboardButton;
}

void renderPanel(
        SDL_Surface *screen,
        SDL_Surface *gameSurface)
{
        if (!screen || screen->h < 2)
        {
                return;
        }

        /*
         * OXCE_3DS_INVENTORY_MODE_RENDER_GUARD
         *
         * Keep the lower panel synchronized with the actual top state.
         */
        if (currentGame &&
                MenuNavigation3DS::isInventory(
                        currentGame) &&
                currentMode != MODE_INVENTORY)
        {
                setMode(MODE_INVENTORY);
        }

        const bool keyboardVisible =
                Keyboard3DS::isVisible();

        static bool previousKeyboardVisible = false;

        if (keyboardVisible != previousKeyboardVisible)
        {
                previousKeyboardVisible =
                        keyboardVisible;

                invalidatePanel();
        }

        /*
         * The keyboard redraws the whole lower screen and therefore
         * also erases any previously drawn bottom cursor.
         */
        if (keyboardVisible)
        {
                previousBottomCursorVisible = false;
                Keyboard3DS::render(screen);
                drawBottomCursor(
                        screen,
                        screen->h / 2);
                return;
        }

        const int bottomY =
                screen->h / 2;

        const int bottomHeight =
                screen->h -
                bottomY;

        /*
         * On gameplay screens, erase only the old cursor rectangle.
         * A full panel presentation below will simply overwrite it
         * again on rebuild frames.
         */
        if (currentMode != MODE_MENU)
        {
                restorePreviousBottomCursor(
                        screen,
                        bottomY);
        }

        if (currentMode == MODE_GEOSCAPE ||
                currentMode == MODE_GEOSCAPE_FROZEN)
        {
                GeoscapeState *geoscape =
                        currentGame ?
                                currentGame->
                                        getGeoscapeState() :
                                nullptr;

                if (geoscape)
                {
                        if (!geoscapeSidebarFrame)
                        {
                                auto created =
                                        Surface::NewPair8Bit(
                                                64,
                                                GEOSCAPE_SIDEBAR_HEIGHT);

                                geoscapeSidebarBuffer =
                                        std::move(
                                                created.first);

                                geoscapeSidebarFrame =
                                        std::move(
                                                created.second);
                        }

                        if (geoscapeSidebarFrame)
                        {
                                geoscape->renderSidebar3DS(
                                        geoscapeSidebarFrame.get());
                        }
                }
        }

        switch (currentMode)
        {
        case MODE_GEOSCAPE:
        {
                SDL_Surface *source =
                        geoscapeSidebarFrame ?
                                geoscapeSidebarFrame.get() :
                                getResourceSurface(
                                        "GEOBORD.SCR");

                bool rebuilt = false;

                if (shouldRebuildPanel())
                {
                        const bool ready =
                                preparePanel(
                                        source,
                                        screen->w,
                                        bottomHeight);

                        if (ready)
                        {
                                renderGeoscapePanel(
                                        panel.get(),
                                        source,
                                        0,
                                        bottomHeight);
                        }

                        finishPanelRebuild(ready);
                        rebuilt = ready;
                }

                /*
                 * Copy the complete cached panel only when its contents
                 * were rebuilt. Otherwise its existing screen pixels
                 * remain untouched.
                 */
                if (rebuilt)
                {
                        presentPanel(
                                screen,
                                bottomY);
                }

                /*
                 * Clock, date and funds remain live every frame.
                 */
                presentLiveGeoscapeInformation(
                        screen,
                        source,
                        bottomY);

                break;
        }

        case MODE_GEOSCAPE_FROZEN:
        {
                SDL_Surface *source =
                        geoscapeSidebarFrame ?
                                geoscapeSidebarFrame.get() :
                                getResourceSurface(
                                        "GEOBORD.SCR");

                bool rebuilt = false;

                if (shouldRebuildPanel())
                {
                        const bool ready =
                                preparePanel(
                                        source,
                                        screen->w,
                                        bottomHeight);

                        if (ready)
                        {
                                renderGeoscapePanel(
                                        panel.get(),
                                        source,
                                        0,
                                        bottomHeight);
                        }

                        finishPanelRebuild(ready);
                        rebuilt = ready;
                }

                if (rebuilt)
                {
                        presentPanel(
                                screen,
                                bottomY);
                }

                break;
        }

        case MODE_INVENTORY:
        {
                SDL_Surface *source =
                        cleanFrame ?
                                cleanFrame.get() :
                                gameSurface;

                bool rebuilt = false;

                if (shouldRebuildPanel())
                {
                        const bool ready =
                                preparePanel(
                                        source,
                                        screen->w,
                                        bottomHeight);

                        if (ready)
                        {
                                renderInventoryPanel(
                                        panel.get(),
                                        source,
                                        0,
                                        bottomHeight);
                        }

                        finishPanelRebuild(ready);
                        rebuilt = ready;
                }

                if (rebuilt)
                {
                        presentPanel(
                                screen,
                                bottomY);
                }

                break;
        }

        case MODE_BATTLESCAPE:
        {
                SDL_Surface *source =
                        cleanFrame ?
                                cleanFrame.get() :
                                gameSurface;

                bool rebuilt = false;

                if (shouldRebuildPanel())
                {
                        const bool ready =
                                preparePanel(
                                        source,
                                        screen->w,
                                        bottomHeight);

                        if (ready)
                        {
                                renderBattlescapePanel(
                                        panel.get(),
                                        source,
                                        0,
                                        bottomHeight);
                        }

                        finishPanelRebuild(ready);
                        rebuilt = ready;
                }

                if (rebuilt)
                {
                        presentPanel(
                                screen,
                                bottomY);
                }

                break;
        }

        case MODE_MENU:
        default:
                /*
                 * The current generic menu panel is inexpensive and
                 * still redraws the full bottom screen.
                 */
                previousBottomCursorVisible = false;

                renderMenuPanel(
                        screen,
                        bottomY,
                        bottomHeight);

                break;
        }

        drawBottomCursor(
                screen,
                bottomY);

        rememberBottomCursorRect(
                screen,
                bottomY);
}

}
}
