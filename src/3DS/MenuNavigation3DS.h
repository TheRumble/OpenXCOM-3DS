#pragma once

#include <cstddef>

namespace OpenXcom
{

class Game;
class Screen;
class Surface;
class TextEdit;
class TextList;

class MenuNavigation3DS
{
private:
        static int getTextListLineHeight(
                const TextList *list);

        static std::size_t getTextListVisibleRows(
                const TextList *list);

        static bool getTextListRowAtY(
                const TextList *list,
                int logicalY,
                std::size_t &row,
                int &lineHeight,
                std::size_t &visibleRows);

        static bool getTextListPhysicalRowBounds(
                TextList *list,
                std::size_t row,
                std::size_t visibleRows,
                bool scrollIntoView,
                std::size_t &firstPhysicalLine,
                std::size_t &lastPhysicalLine);

        static bool getTextListRowTarget(
                const TextList *list,
                const Screen *screen,
                std::size_t firstPhysicalLine,
                std::size_t lastPhysicalLine,
                std::size_t visibleRows,
                int lineHeight,
                int &targetX,
                int &targetY);


        static bool getTextListScrollBarThumb(
                TextList *list,
                int &left,
                int &top,
                int &right,
                int &bottom);

        static bool getTextListScrollBarTarget(
                TextList *list,
                const Screen *screen,
                int &targetX,
                int &targetY);

public:
        enum Direction
        {
                UP,
                DOWN,
                LEFT,
                RIGHT
        };

        /*
         * OXCE_3DS_INVENTORY_BOTTOM_PANEL
         */
        enum InventoryBottomControl
        {
                INVENTORY_OK,
                INVENTORY_PREVIOUS,
                INVENTORY_NEXT,
                INVENTORY_UNLOAD,
                INVENTORY_GROUND,
                INVENTORY_SAVE_CONFIG,
                INVENTORY_LOAD_CONFIG,
                INVENTORY_LINKS
        };

        static TextEdit *getFocusedTextEdit(
                const Game *game);

        static bool isGeoscape(const Game *game);
        static bool isBattlescape(const Game *game);

        /*
         * OXCE_3DS_INVENTORY_SNAP_NAV
         */
        static bool isInventory(const Game *game);
        static bool cancelInventoryCarry(Game *game);

        static Surface *getInventoryBottomControlSurface(
                const Game *game,
                InventoryBottomControl control,
                int &logicalX,
                int &logicalY,
                bool &enabled);

        static bool isActive(const Game *game);

        static bool focusNewlyOpenedComboBox(
                Game *game,
                int &targetX,
                int &targetY);

        static bool isTextEditing(const Game *game);
        static bool cancelTextEditing(Game *game);

        static bool findTarget(
                const Game *game,
                Direction direction,
                int cursorX,
                int cursorY,
                int &targetX,
                int &targetY);

        static bool closeOpenComboBox(Game *game);

        static bool adjustAnalogControlAtCursor(
                Game *game,
                int cursorX,
                int cursorY,
                int circleX,
                int circleY,
                int &targetX,
                int &targetY);

        static bool activateSliderAtCursor(
                Game *game,
                int cursorX,
                int cursorY,
                int &targetX,
                int &targetY);

        static bool leaveSliderMode();
        static bool isAdjustingSlider();
};

}
