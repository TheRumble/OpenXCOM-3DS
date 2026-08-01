#include "Keyboard3DS.h"

#include <SDL.h>
#include <SDL_gfxPrimitives.h>

#include <cstring>
#include <climits>
#include <vector>

namespace OpenXcom
{
namespace Keyboard3DS
{

namespace
{

constexpr int KEY_GAP = 3;

constexpr int SPECIAL_Y = 174;
constexpr int SPECIAL_HEIGHT = 48;

constexpr int SHIFT_X = 4;
constexpr int SHIFT_WIDTH = 64;

constexpr int SPACE_X = 72;
constexpr int SPACE_WIDTH = 188;

constexpr int BACKSPACE_X = 264;
constexpr int BACKSPACE_WIDTH = 64;

constexpr int ENTER_X = 332;
constexpr int ENTER_WIDTH = 64;


struct LetterRow
{
        const char *letters;
        const char *shifted;
        int y;
        int keyWidth;
        int keyHeight;
        bool letterRow;
};

constexpr LetterRow KEY_ROWS[] =
{
        {
                "1234567890",
                "!@#$%^&*()",
                12,
                32,
                30,
                false
        },
        {
                "QWERTYUIOP",
                "QWERTYUIOP",
                44,
                32,
                30,
                true
        },
        {
                "ASDFGHJKL",
                "ASDFGHJKL",
                76,
                35,
                30,
                true
        },
        {
                "ZXCVBNM",
                "ZXCVBNM",
                108,
                42,
                30,
                true
        },
        {
                "-=[];',./",
                "_+{}:\"<>?",
                140,
                35,
                30,
                false
        }
};

bool keyboardVisible = false;
bool shiftEnabled = false;
SDLKey pressedKey = SDLK_UNKNOWN;
/* OXCE_3DS_KEYBOARD_MODE_STATE */
Mode keyboardMode = MODE_TEXT;

/* OXCE_3DS_FULL_BINDING_KEYBOARD
 *
 * Original OXCE touchscreen layout for assigning desktop keys.
 * Coordinates use the 400-pixel-wide rendered lower-screen space.
 */
constexpr int BINDING_LAYOUT_WIDTH = 400;
constexpr int COMPLETE_KEYBOARD_VERTICAL_OFFSET = -8;

struct BindingKey
{
        const char *label;
        SDLKey key;
        int x;
        int y;
        int width;
        int height;
        bool special;
};

constexpr BindingKey BINDING_KEYS[] =
{
        /* Function row. */
        {"ESC", SDLK_ESCAPE, 4, 24, 30, 20, true},
        {"F1", SDLK_F1, 38, 24, 28, 20, true},
        {"F2", SDLK_F2, 68, 24, 28, 20, true},
        {"F3", SDLK_F3, 98, 24, 28, 20, true},
        {"F4", SDLK_F4, 128, 24, 28, 20, true},
        {"F5", SDLK_F5, 158, 24, 28, 20, true},
        {"F6", SDLK_F6, 188, 24, 28, 20, true},
        {"F7", SDLK_F7, 218, 24, 28, 20, true},
        {"F8", SDLK_F8, 248, 24, 28, 20, true},
        {"F9", SDLK_F9, 278, 24, 28, 20, true},
        {"F10", SDLK_F10, 308, 24, 28, 20, true},
        {"F11", SDLK_F11, 338, 24, 28, 20, true},
        {"F12", SDLK_F12, 368, 24, 28, 20, true},

        /* Number row. */
        {"`", SDLK_BACKQUOTE, 4, 47, 23, 25, false},
        {"1", SDLK_1, 29, 47, 23, 25, false},
        {"2", SDLK_2, 54, 47, 23, 25, false},
        {"3", SDLK_3, 79, 47, 23, 25, false},
        {"4", SDLK_4, 104, 47, 23, 25, false},
        {"5", SDLK_5, 129, 47, 23, 25, false},
        {"6", SDLK_6, 154, 47, 23, 25, false},
        {"7", SDLK_7, 179, 47, 23, 25, false},
        {"8", SDLK_8, 204, 47, 23, 25, false},
        {"9", SDLK_9, 229, 47, 23, 25, false},
        {"0", SDLK_0, 254, 47, 23, 25, false},
        {"-", SDLK_MINUS, 279, 47, 23, 25, false},
        {"=", SDLK_EQUALS, 304, 47, 23, 25, false},
        {"BKSP", SDLK_BACKSPACE, 329, 47, 67, 25, true},

        /* QWERTY row. */
        {"TAB", SDLK_TAB, 4, 75, 38, 25, true},
        {"Q", SDLK_q, 44, 75, 24, 25, false},
        {"W", SDLK_w, 70, 75, 24, 25, false},
        {"E", SDLK_e, 96, 75, 24, 25, false},
        {"R", SDLK_r, 122, 75, 24, 25, false},
        {"T", SDLK_t, 148, 75, 24, 25, false},
        {"Y", SDLK_y, 174, 75, 24, 25, false},
        {"U", SDLK_u, 200, 75, 24, 25, false},
        {"I", SDLK_i, 226, 75, 24, 25, false},
        {"O", SDLK_o, 252, 75, 24, 25, false},
        {"P", SDLK_p, 278, 75, 24, 25, false},
        {"[", SDLK_LEFTBRACKET, 304, 75, 22, 25, false},
        {"]", SDLK_RIGHTBRACKET, 328, 75, 22, 25, false},
        {"\\", SDLK_BACKSLASH, 352, 75, 44, 25, false},

        /* Home row. */
        {"CAPS", SDLK_CAPSLOCK, 4, 103, 44, 25, true},
        {"A", SDLK_a, 50, 103, 25, 25, false},
        {"S", SDLK_s, 77, 103, 25, 25, false},
        {"D", SDLK_d, 104, 103, 25, 25, false},
        {"F", SDLK_f, 131, 103, 25, 25, false},
        {"G", SDLK_g, 158, 103, 25, 25, false},
        {"H", SDLK_h, 185, 103, 25, 25, false},
        {"J", SDLK_j, 212, 103, 25, 25, false},
        {"K", SDLK_k, 239, 103, 25, 25, false},
        {"L", SDLK_l, 266, 103, 25, 25, false},
        {";", SDLK_SEMICOLON, 293, 103, 23, 25, false},
        {"'", SDLK_QUOTE, 318, 103, 23, 25, false},
        {"ENTER", SDLK_RETURN, 343, 103, 53, 25, true},

        /* Bottom letter row. */
        {"SHIFT", SDLK_LSHIFT, 4, 131, 52, 25, true},
        {"Z", SDLK_z, 58, 131, 26, 25, false},
        {"X", SDLK_x, 86, 131, 26, 25, false},
        {"C", SDLK_c, 114, 131, 26, 25, false},
        {"V", SDLK_v, 142, 131, 26, 25, false},
        {"B", SDLK_b, 170, 131, 26, 25, false},
        {"N", SDLK_n, 198, 131, 26, 25, false},
        {"M", SDLK_m, 226, 131, 26, 25, false},
        {",", SDLK_COMMA, 254, 131, 24, 25, false},
        {".", SDLK_PERIOD, 280, 131, 24, 25, false},
        {"/", SDLK_SLASH, 306, 131, 24, 25, false},
        {"SHIFT", SDLK_RSHIFT, 332, 131, 64, 25, true},

        /* Modifier and navigation row. */
        {"CTRL", SDLK_LCTRL, 4, 159, 42, 25, true},
        {"ALT", SDLK_LALT, 48, 159, 36, 25, true},
        {"SPACE", SDLK_SPACE, 86, 159, 134, 25, true},
        {"ALT", SDLK_RALT, 222, 159, 36, 25, true},
        {"CTRL", SDLK_RCTRL, 260, 159, 42, 25, true},

        {"INS", SDLK_INSERT, 306, 159, 28, 25, true},
        {"HM", SDLK_HOME, 336, 159, 28, 25, true},
        {"PU", SDLK_PAGEUP, 366, 159, 28, 25, true},

        /* System, navigation, and arrow clusters. */
        {"PRT", SDLK_PRINT, 4, 187, 30, 22, true},
        {"SCR", SDLK_SCROLLOCK, 36, 187, 30, 22, true},
        {"PAU", SDLK_PAUSE, 68, 187, 30, 22, true},

        {"UP", SDLK_UP, 232, 187, 30, 22, true},

        {"DEL", SDLK_DELETE, 306, 187, 28, 22, true},
        {"END", SDLK_END, 336, 187, 28, 22, true},
        {"PD", SDLK_PAGEDOWN, 366, 187, 28, 22, true},

        {"LT", SDLK_LEFT, 198, 211, 30, 22, true},
        {"DN", SDLK_DOWN, 232, 211, 30, 22, true},
        {"RT", SDLK_RIGHT, 266, 211, 30, 22, true}
};


bool pointInRect(
        int x,
        int y,
        int left,
        int top,
        int width,
        int height)
{
        return x >= left &&
                x < left + width &&
                y >= top &&
                y < top + height;
}

int getRowStartX(
        int screenWidth,
        const LetterRow &row)
{
        const int count =
                static_cast<int>(
                        std::strlen(row.letters));

        const int totalWidth =
                count * row.keyWidth +
                (count - 1) * KEY_GAP;

        return (screenWidth - totalWidth) / 2;
}

SDLKey getLetterKey(char uppercaseLetter)
{
        return static_cast<SDLKey>(
                SDLK_a +
                uppercaseLetter - 'A');
}

Uint16 getLetterUnicode(char uppercaseLetter)
{
        const char firstLetter =
                shiftEnabled ? 'A' : 'a';

        return static_cast<Uint16>(
                firstLetter +
                uppercaseLetter - 'A');
}

SDLKey getPrintableKey(char character)
{
        return static_cast<SDLKey>(
                static_cast<unsigned char>(
                        character));
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

void drawCenteredLabel(
        SDL_Surface *surface,
        int x,
        int y,
        int width,
        int height,
        const char *label)
{
        if (!label)
        {
                return;
        }

        const int textWidth =
                static_cast<int>(
                        std::strlen(label)) * 8;

        const int labelX =
                x + (width - textWidth) / 2;

        const int labelY =
                y + (height - 8) / 2;

        stringRGBA(
                surface,
                static_cast<Sint16>(labelX),
                static_cast<Sint16>(labelY),
                label,
                232,
                236,
                244,
                255);
}

void drawKey(
        SDL_Surface *surface,
        int x,
        int y,
        int width,
        int height,
        const char *label,
        SDLKey key,
        bool special)
{
        const bool shiftActive =
                key == SDLK_LSHIFT &&
                shiftEnabled;

        const bool pressed =
                shiftActive ||
                (key != SDLK_UNKNOWN &&
                        pressedKey == key);

        fillRect(
                surface,
                x,
                y,
                width,
                height,
                pressed ? 220 : 104,
                pressed ? 230 : 124,
                pressed ? 242 : 148);

        fillRect(
                surface,
                x + 2,
                y + 2,
                width - 4,
                height - 4,
                pressed ? 76 : (special ? 54 : 38),
                pressed ? 104 : (special ? 76 : 56),
                pressed ? 132 : (special ? 98 : 76));

        drawCenteredLabel(
                surface,
                x,
                y,
                width,
                height,
                label);
}

void drawLetterRow(
        SDL_Surface *surface,
        int screenWidth,
        int bottomY,
        const LetterRow &row)
{
        const int count =
                static_cast<int>(
                        std::strlen(row.letters));

        int x = getRowStartX(screenWidth, row);

        for (int index = 0;
                index < count;
                ++index)
        {
                const char normalCharacter =
                        row.letters[index];

                char displayedCharacter =
                        shiftEnabled ?
                                row.shifted[index] :
                                normalCharacter;

                SDLKey key =
                        getPrintableKey(
                                displayedCharacter);

                if (row.letterRow)
                {
                        displayedCharacter =
                                shiftEnabled ?
                                        normalCharacter :
                                        static_cast<char>(
                                                'a' +
                                                normalCharacter - 'A');

                        key = getLetterKey(
                                normalCharacter);
                }

                char label[2] =
                {
                        displayedCharacter,
                        '\0'
                };

                drawKey(
                        surface,
                        x,
                        bottomY + row.y,
                        row.keyWidth,
                        row.keyHeight,
                        label,
                        key,
                        false);

                x += row.keyWidth + KEY_GAP;
        }
}

bool getLetterAt(
        int screenWidth,
        int x,
        int y,
        const LetterRow &row,
        SDLKey &key,
        Uint16 &unicode)
{
        if (y < row.y ||
                y >= row.y + row.keyHeight)
        {
                return false;
        }

        const int startX =
                getRowStartX(screenWidth, row);

        if (x < startX)
        {
                return false;
        }

        const int relativeX = x - startX;
        const int cellWidth =
                row.keyWidth + KEY_GAP;

        const int index =
                relativeX / cellWidth;

        const int count =
                static_cast<int>(
                        std::strlen(row.letters));

        if (index < 0 || index >= count)
        {
                return false;
        }

        /*
         * Do not accept the small gap between adjacent keys.
         */
        if (relativeX % cellWidth >= row.keyWidth)
        {
                return false;
        }

        const char normalCharacter =
                row.letters[index];

        if (row.letterRow)
        {
                key = getLetterKey(
                        normalCharacter);

                unicode = getLetterUnicode(
                        normalCharacter);
        }
        else
        {
                const char character =
                        shiftEnabled ?
                                row.shifted[index] :
                                normalCharacter;

                key = getPrintableKey(character);

                unicode = static_cast<Uint16>(
                        static_cast<unsigned char>(
                                character));
        }

        return true;
}

/* OXCE_3DS_BINDING_KEYBOARD_HELPERS */
int getBindingOffsetX(int screenWidth)
{
        return (screenWidth - BINDING_LAYOUT_WIDTH) / 2;
}

bool getBindingKeyAt(
        int screenWidth,
        int x,
        int y,
        SDLKey &key,
        Uint16 &unicode)
{
        const int offsetX =
                getBindingOffsetX(screenWidth);

        for (const BindingKey &bindingKey : BINDING_KEYS)
        {
                if (pointInRect(
                        x,
                        y,
                        offsetX + bindingKey.x,
                        bindingKey.y +
                                COMPLETE_KEYBOARD_VERTICAL_OFFSET,
                        bindingKey.width,
                        bindingKey.height))
                {
                        key = bindingKey.key;
                        unicode = 0;
                        return true;
                }
        }

        return false;
}

void renderBindingKeyboard(
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

        const int offsetX =
                getBindingOffsetX(screen->w);

        for (const BindingKey &bindingKey : BINDING_KEYS)
        {
                drawKey(
                        screen,
                        offsetX + bindingKey.x,
                        bottomY +
                                bindingKey.y +
                                COMPLETE_KEYBOARD_VERTICAL_OFFSET,
                        bindingKey.width,
                        bindingKey.height,
                        bindingKey.label,
                        bindingKey.key,
                        bindingKey.special);
        }
}

/* OXCE_3DS_KEYBOARD_CURSOR_CONTROL */
struct KeyboardSnapTarget
{
        int left;
        int top;
        int right;
        int bottom;
};

void addKeyboardSnapTarget(
        std::vector<KeyboardSnapTarget> &targets,
        int left,
        int top,
        int width,
        int height)
{
        KeyboardSnapTarget target = {};
        target.left = left;
        target.top = top;
        target.right = left + width;
        target.bottom = top + height;
        targets.push_back(target);
}

void buildKeyboardSnapTargets(
        std::vector<KeyboardSnapTarget> &targets)
{
        targets.clear();

        if (keyboardMode != MODE_TEXT)
        {
                const int offsetX =
                        getBindingOffsetX(
                                BINDING_LAYOUT_WIDTH);

                for (const BindingKey &key : BINDING_KEYS)
                {
                        addKeyboardSnapTarget(
                                targets,
                                offsetX + key.x,
                                key.y +
                                        COMPLETE_KEYBOARD_VERTICAL_OFFSET,
                                key.width,
                                key.height);
                }

                return;
        }

        for (const LetterRow &row : KEY_ROWS)
        {
                const int count =
                        static_cast<int>(
                                std::strlen(row.letters));

                int x =
                        getRowStartX(
                                BINDING_LAYOUT_WIDTH,
                                row);

                for (int index = 0;
                        index < count;
                        ++index)
                {
                        addKeyboardSnapTarget(
                                targets,
                                x,
                                row.y,
                                row.keyWidth,
                                row.keyHeight);

                        x += row.keyWidth + KEY_GAP;
                }
        }

        addKeyboardSnapTarget(
                targets,
                SHIFT_X,
                SPECIAL_Y,
                SHIFT_WIDTH,
                SPECIAL_HEIGHT);

        addKeyboardSnapTarget(
                targets,
                SPACE_X,
                SPECIAL_Y,
                SPACE_WIDTH,
                SPECIAL_HEIGHT);

        addKeyboardSnapTarget(
                targets,
                BACKSPACE_X,
                SPECIAL_Y,
                BACKSPACE_WIDTH,
                SPECIAL_HEIGHT);

        addKeyboardSnapTarget(
                targets,
                ENTER_X,
                SPECIAL_Y,
                ENTER_WIDTH,
                SPECIAL_HEIGHT);
}

int absoluteKeyboardDistance(int value)
{
        return value < 0 ? -value : value;
}

}

/* OXCE_3DS_KEYBOARD_MODE_IMPLEMENTATION */
void setMode(Mode mode)
{
        if (keyboardMode == mode)
        {
                return;
        }

        keyboardMode = mode;
        shiftEnabled = false;
        clearPressedKey();
}

Mode getMode()
{
        return keyboardMode;
}

void setVisible(bool visible)
{
        keyboardVisible = visible;

        if (!visible)
        {
                shiftEnabled = false;
                clearPressedKey();
        }
}

bool isVisible()
{
        return keyboardVisible;
}

bool getKeyAt(
        int x,
        int y,
        int renderedWidth,
        SDLKey &key,
        Uint16 &unicode)
{
        key = SDLK_UNKNOWN;
        unicode = 0;

        if (!keyboardVisible ||
                renderedWidth <= 0)
        {
                return false;
        }

        /*
         * The physical bottom touchscreen is 320 pixels wide, but
         * SDL_DUALSCR with SDL_FITWIDTH gives us a 400-pixel-wide
         * combined rendering surface. Convert touch X into the same
         * coordinate space used to draw the keyboard.
         */
        constexpr int touchScreenWidth = 320;

        const int renderedX =
                x * renderedWidth /
                touchScreenWidth;

        /* OXCE_3DS_BINDING_KEYBOARD_HIT_TEST */
        if (keyboardMode != MODE_TEXT)
        {
                return getBindingKeyAt(
                        renderedWidth,
                        renderedX,
                        y,
                        key,
                        unicode);
        }
        for (const LetterRow &row : KEY_ROWS)
        {
                if (getLetterAt(
                        renderedWidth,
                        renderedX,
                        y,
                        row,
                        key,
                        unicode))
                {
                        return true;
                }
        }

        if (pointInRect(
                renderedX,
                y,
                SHIFT_X,
                SPECIAL_Y,
                SHIFT_WIDTH,
                SPECIAL_HEIGHT))
        {
                shiftEnabled = !shiftEnabled;
                return false;
        }

        if (pointInRect(
                renderedX,
                y,
                SPACE_X,
                SPECIAL_Y,
                SPACE_WIDTH,
                SPECIAL_HEIGHT))
        {
                key = SDLK_SPACE;
                unicode = ' ';
                return true;
        }

        if (pointInRect(
                renderedX,
                y,
                BACKSPACE_X,
                SPECIAL_Y,
                BACKSPACE_WIDTH,
                SPECIAL_HEIGHT))
        {
                key = SDLK_BACKSPACE;
                unicode = 0;
                return true;
        }

        if (pointInRect(
                renderedX,
                y,
                ENTER_X,
                SPECIAL_Y,
                ENTER_WIDTH,
                SPECIAL_HEIGHT))
        {
                key = SDLK_RETURN;
                unicode = 0;
                return true;
        }

        return false;
}

/* OXCE_3DS_KEYBOARD_ROW_AWARE_SNAPPING */
bool snapCursor(
        int &physicalX,
        int &physicalY,
        int horizontal,
        int vertical)
{
        if (!keyboardVisible ||
                (horizontal == 0 &&
                 vertical == 0))
        {
                return false;
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

        if (direction < 0)
        {
                return false;
        }

        std::vector<KeyboardSnapTarget> targets;
        buildKeyboardSnapTargets(targets);

        if (targets.empty())
        {
                return false;
        }

        constexpr int physicalWidth = 320;
        constexpr int renderedWidth =
                BINDING_LAYOUT_WIDTH;

        const int renderedCursorX =
                physicalX *
                renderedWidth /
                physicalWidth;

        /*
         * First identify the key currently containing the cursor.
         * Circle Pad movement may leave it between keys, so fall back to
         * the nearest key center when necessary.
         */
        int currentTarget = -1;

        for (std::size_t index = 0;
                index < targets.size();
                ++index)
        {
                const KeyboardSnapTarget &target =
                        targets[index];

                if (renderedCursorX >= target.left &&
                        renderedCursorX < target.right &&
                        physicalY >= target.top &&
                        physicalY < target.bottom)
                {
                        currentTarget =
                                static_cast<int>(index);
                        break;
                }
        }

        if (currentTarget < 0)
        {
                int bestDistance = INT_MAX;

                for (std::size_t index = 0;
                        index < targets.size();
                        ++index)
                {
                        const KeyboardSnapTarget &target =
                                targets[index];

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
                                renderedCursorX;

                        const int deltaY =
                                centerY -
                                physicalY;

                        const int distance =
                                deltaX * deltaX +
                                deltaY * deltaY;

                        if (distance < bestDistance)
                        {
                                bestDistance = distance;
                                currentTarget =
                                        static_cast<int>(
                                                index);
                        }
                }
        }

        if (currentTarget < 0)
        {
                return false;
        }

        const KeyboardSnapTarget &current =
                targets[currentTarget];

        const int currentCenterX =
                (current.left +
                 current.right) /
                2;

        const int currentCenterY =
                (current.top +
                 current.bottom) /
                2;

        int nextTarget = -1;

        /*
         * Left and right stay on exactly the same keyboard row and
         * choose the nearest neighboring key.
         */
        if (direction == 2 ||
                direction == 3)
        {
                int bestDistance = INT_MAX;

                for (std::size_t index = 0;
                        index < targets.size();
                        ++index)
                {
                        const KeyboardSnapTarget &target =
                                targets[index];

                        const int centerY =
                                (target.top +
                                 target.bottom) /
                                2;

                        if (centerY != currentCenterY)
                        {
                                continue;
                        }

                        const int centerX =
                                (target.left +
                                 target.right) /
                                2;

                        const int deltaX =
                                centerX -
                                currentCenterX;

                        if ((direction == 2 &&
                             deltaX >= 0) ||
                                (direction == 3 &&
                                 deltaX <= 0))
                        {
                                continue;
                        }

                        const int distance =
                                absoluteKeyboardDistance(
                                        deltaX);

                        if (distance < bestDistance)
                        {
                                bestDistance = distance;
                                nextTarget =
                                        static_cast<int>(
                                                index);
                        }
                }
        }
        else
        {
                /*
                 * First locate the immediately adjacent row. Do not let
                 * horizontal alignment cause a row to be skipped.
                 */
                int adjacentRowY = -1;
                int adjacentRowDistance = INT_MAX;

                for (const KeyboardSnapTarget &target :
                        targets)
                {
                        const int centerY =
                                (target.top +
                                 target.bottom) /
                                2;

                        const int deltaY =
                                centerY -
                                currentCenterY;

                        if ((direction == 0 &&
                             deltaY >= 0) ||
                                (direction == 1 &&
                                 deltaY <= 0))
                        {
                                continue;
                        }

                        const int distance =
                                absoluteKeyboardDistance(
                                        deltaY);

                        if (distance <
                                adjacentRowDistance)
                        {
                                adjacentRowDistance =
                                        distance;

                                adjacentRowY = centerY;
                        }
                }

                if (adjacentRowY < 0)
                {
                        return false;
                }

                /*
                 * On that adjacent row, prefer a key whose rectangle
                 * lies directly above or below the cursor. Otherwise,
                 * choose the closest horizontal key.
                 */
                int bestScore = INT_MAX;

                for (std::size_t index = 0;
                        index < targets.size();
                        ++index)
                {
                        const KeyboardSnapTarget &target =
                                targets[index];

                        const int centerY =
                                (target.top +
                                 target.bottom) /
                                2;

                        if (centerY != adjacentRowY)
                        {
                                continue;
                        }

                        int horizontalGap = 0;

                        if (renderedCursorX <
                                target.left)
                        {
                                horizontalGap =
                                        target.left -
                                        renderedCursorX;
                        }
                        else if (renderedCursorX >=
                                target.right)
                        {
                                horizontalGap =
                                        renderedCursorX -
                                        (target.right - 1);
                        }

                        const int centerX =
                                (target.left +
                                 target.right) /
                                2;

                        const int centerDistance =
                                absoluteKeyboardDistance(
                                        centerX -
                                        renderedCursorX);

                        const int score =
                                horizontalGap * 1000 +
                                centerDistance;

                        if (score < bestScore)
                        {
                                bestScore = score;
                                nextTarget =
                                        static_cast<int>(
                                                index);
                        }
                }
        }

        if (nextTarget < 0)
        {
                return false;
        }

        const KeyboardSnapTarget &target =
                targets[nextTarget];

        const int targetCenterX =
                (target.left +
                 target.right) /
                2;

        physicalX =
                targetCenterX *
                physicalWidth /
                renderedWidth;

        physicalY =
                (target.top +
                 target.bottom) /
                2;

        return true;
}

void setPressedKey(SDLKey key)
{
        pressedKey = key;
}

void clearPressedKey()
{
        pressedKey = SDLK_UNKNOWN;
}

void render(SDL_Surface *screen)
{
        if (!screen || screen->h < 2)
        {
                return;
        }

        const int bottomY = screen->h / 2;
        const int bottomHeight =
                screen->h - bottomY;
        /* OXCE_3DS_BINDING_KEYBOARD_RENDER */
        if (keyboardMode != MODE_TEXT)
        {
                renderBindingKeyboard(
                        screen,
                        bottomY,
                        bottomHeight);
                return;
        }


        fillRect(
                screen,
                0,
                bottomY,
                screen->w,
                bottomHeight,
                8,
                14,
                24);

        for (const LetterRow &row : KEY_ROWS)
        {
                drawLetterRow(
                        screen,
                        screen->w,
                        bottomY,
                        row);
        }

        drawKey(
                screen,
                SHIFT_X,
                bottomY + SPECIAL_Y,
                SHIFT_WIDTH,
                SPECIAL_HEIGHT,
                "SHIFT",
                SDLK_LSHIFT,
                true);

        drawKey(
                screen,
                SPACE_X,
                bottomY + SPECIAL_Y,
                SPACE_WIDTH,
                SPECIAL_HEIGHT,
                "SPACE",
                SDLK_SPACE,
                true);

        drawKey(
                screen,
                BACKSPACE_X,
                bottomY + SPECIAL_Y,
                BACKSPACE_WIDTH,
                SPECIAL_HEIGHT,
                "<-",
                SDLK_BACKSPACE,
                true);

        drawKey(
                screen,
                ENTER_X,
                bottomY + SPECIAL_Y,
                ENTER_WIDTH,
                SPECIAL_HEIGHT,
                "ENTER",
                SDLK_RETURN,
                true);

        drawCenteredLabel(
                screen,
                8,
                bottomY + 224,
                screen->w - 16,
                12,
                shiftEnabled ?
                        "CAPS + SHIFTED SYMBOLS" :
                        "LOWERCASE + NUMBERS");
}

}
}
