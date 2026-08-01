#include "MenuNavigation3DS.h"

#include "../Battlescape/BattlescapeState.h"
#include "../Battlescape/InventoryState.h"
#include "../Battlescape/Inventory.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/Font.h"
#include "../Engine/InteractiveSurface.h"
#include "../Engine/Screen.h"
#include "../Engine/State.h"
#include "../Engine/Surface.h"
#include "../Geoscape/GeoscapeState.h"
#include "../Interface/ArrowButton.h"
#include "../Interface/BattlescapeButton.h"
#include "../Interface/ComboBox.h"
#include "../Interface/ImageButton.h"
#include "../Interface/Slider.h"
#include "../Interface/ScrollBar.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextEdit.h"
#include "../Interface/TextList.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleInventory.h"
#include "../Mod/RuleItem.h"
#include "../Savegame/BattleItem.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/Tile.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <set>
#include <utility>
#include <vector>

namespace OpenXcom
{

namespace
{

ComboBox *trackedOpenCombo = nullptr;
size_t trackedComboRow = 0;

Slider *activeSlider = nullptr;
State *activeSliderState = nullptr;

const void *analogControlOwner = nullptr;
int analogControlRemainder = 0;

}

int MenuNavigation3DS::getTextListLineHeight(
        const TextList *list)
{
        if (!list || !list->_font)
        {
                return 1;
        }

        return std::max(
                1,
                list->_font->getHeight() +
                        list->_font->getSpacing());
}

std::size_t MenuNavigation3DS::getTextListVisibleRows(
        const TextList *list)
{
        if (!list)
        {
                return 1;
        }

        return std::max<std::size_t>(
                1,
                list->getVisibleRows());
}

bool MenuNavigation3DS::getTextListRowAtY(
        const TextList *list,
        int logicalY,
        std::size_t &row,
        int &lineHeight,
        std::size_t &visibleRows)
{
        if (!list || list->_rows.empty())
        {
                return false;
        }

        lineHeight = getTextListLineHeight(list);
        visibleRows = getTextListVisibleRows(list);

        const int relativeY =
                logicalY - list->getY();

        std::size_t visibleLine =
                static_cast<std::size_t>(
                        std::max(0, relativeY) /
                        lineHeight);

        if (visibleLine >= visibleRows)
        {
                visibleLine = visibleRows - 1;
        }

        std::size_t physicalLine =
                list->_scroll + visibleLine;

        if (physicalLine >= list->_rows.size())
        {
                physicalLine = list->_rows.size() - 1;
        }

        row = list->_rows[physicalLine];
        return true;
}

bool MenuNavigation3DS::getTextListPhysicalRowBounds(
        TextList *list,
        std::size_t row,
        std::size_t visibleRows,
        bool scrollIntoView,
        std::size_t &firstPhysicalLine,
        std::size_t &lastPhysicalLine)
{
        if (!list || list->_rows.empty())
        {
                return false;
        }

        firstPhysicalLine = list->_rows.size();
        lastPhysicalLine = 0;

        for (std::size_t line = 0;
                line < list->_rows.size();
                ++line)
        {
                if (list->_rows[line] != row)
                {
                        continue;
                }

                if (firstPhysicalLine ==
                        list->_rows.size())
                {
                        firstPhysicalLine = line;
                }

                lastPhysicalLine = line;
        }

        if (firstPhysicalLine == list->_rows.size())
        {
                return false;
        }

        if (scrollIntoView)
        {
                const std::size_t effectiveVisibleRows =
                        std::max<std::size_t>(
                                1,
                                visibleRows);

                std::size_t newScroll = list->_scroll;

                if (firstPhysicalLine < newScroll)
                {
                        newScroll = firstPhysicalLine;
                }
                else if (lastPhysicalLine >=
                        newScroll + effectiveVisibleRows)
                {
                        newScroll =
                                lastPhysicalLine -
                                effectiveVisibleRows + 1;
                }

                list->scrollTo(newScroll);
        }

        return true;
}

bool MenuNavigation3DS::getTextListRowTarget(
        const TextList *list,
        const Screen *screen,
        std::size_t firstPhysicalLine,
        std::size_t lastPhysicalLine,
        std::size_t visibleRows,
        int lineHeight,
        int &targetX,
        int &targetY)
{
        if (!list || !screen || lineHeight <= 0)
        {
                return false;
        }

        const std::size_t effectiveVisibleRows =
                std::max<std::size_t>(
                        1,
                        visibleRows);

        const std::size_t visibleFirst =
                std::max(
                        firstPhysicalLine,
                        list->_scroll);

        const std::size_t visibleLast =
                std::min(
                        lastPhysicalLine + 1,
                        list->_scroll +
                                effectiveVisibleRows);

        if (visibleFirst >= visibleLast)
        {
                return false;
        }

        const int rowTop =
                list->getY() +
                static_cast<int>(
                        visibleFirst -
                        list->_scroll) *
                        lineHeight;

        const int rowBottom =
                list->getY() +
                static_cast<int>(
                        visibleLast -
                        list->_scroll) *
                        lineHeight;

        const int logicalTargetX =
                list->getX() +
                list->getWidth() / 2;

        const int logicalTargetY =
                rowTop +
                std::max(
                        1,
                        rowBottom - rowTop) /
                        2;

        const double scaleX = screen->getXScale();
        const double scaleY = screen->getYScale();

        if (scaleX <= 0.0 || scaleY <= 0.0)
        {
                return false;
        }

        targetX =
                screen->getCursorLeftBlackBand() +
                static_cast<int>(
                        std::lround(
                                logicalTargetX *
                                scaleX));

        targetY =
                screen->getCursorTopBlackBand() +
                static_cast<int>(
                        std::lround(
                                logicalTargetY *
                                scaleY));

        return true;
}

TextEdit *MenuNavigation3DS::getFocusedTextEdit(
        const Game *game)
{
        if (!game || game->_states.empty())
        {
                return nullptr;
        }

        State *state = game->_states.back();

        auto getFocused = [](Surface *surface) -> TextEdit *
        {
                TextEdit *edit =
                        dynamic_cast<TextEdit *>(surface);

                if (edit &&
                        edit->isFocused() &&
                        edit->_visible &&
                        !edit->_hidden)
                {
                        return edit;
                }

                return nullptr;
        };

        /*
         * A modal surface owns input. Do not reach through it to an
         * underlying text field.
         */
        if (state->_modal)
        {
                return getFocused(state->_modal);
        }

        for (auto i = state->_surfaces.rbegin();
                i != state->_surfaces.rend();
                ++i)
        {
                TextEdit *edit = getFocused(*i);

                if (edit)
                {
                        return edit;
                }
        }

        return nullptr;
}

bool MenuNavigation3DS::isTextEditing(
        const Game *game)
{
        return getFocusedTextEdit(game) != nullptr;
}

bool MenuNavigation3DS::cancelTextEditing(Game *game)
{
        TextEdit *edit = getFocusedTextEdit(game);

        if (!edit)
        {
                return false;
        }

        /*
         * Mandatory fields consume B but remain active. Ordinary
         * fields are unfocused without sending desktop Escape, which
         * would erase their contents.
         */
        if (edit->isCancelAllowed())
        {
                edit->setFocus(false);
        }

        return true;
}

bool MenuNavigation3DS::focusNewlyOpenedComboBox(
        Game *game,
        int &targetX,
        int &targetY)
{
        if (!game || game->_states.empty() || !game->_screen)
        {
                return false;
        }

        State *state = game->_states.back();

        ComboBox *openCombo =
                dynamic_cast<ComboBox *>(state->_modal);

        if (!openCombo ||
                !openCombo->_list ||
                !openCombo->_list->getVisible())
        {
                trackedOpenCombo = nullptr;
                trackedComboRow = 0;
                return false;
        }

        /*
         * Move the cursor only once when this dropdown first opens.
         */
        if (trackedOpenCombo == openCombo)
        {
                return false;
        }

        TextList *list = openCombo->_list;
        const size_t itemCount = list->getTexts();

        if (itemCount == 0)
        {
                return false;
        }

        trackedOpenCombo = openCombo;
        trackedComboRow = openCombo->_sel;

        if (trackedComboRow >= itemCount)
        {
                trackedComboRow = itemCount - 1;
        }

        const size_t visibleRows =
                getTextListVisibleRows(list);

        size_t scroll = list->getScroll();

        if (trackedComboRow < scroll)
        {
                list->scrollTo(trackedComboRow);
        }
        else if (trackedComboRow >=
                scroll + visibleRows)
        {
                list->scrollTo(
                        trackedComboRow -
                        visibleRows + 1);
        }

        scroll = list->getScroll();

        const size_t visibleLine =
                trackedComboRow >= scroll ?
                        trackedComboRow - scroll : 0;

        const int lineHeight =
                getTextListLineHeight(list);

        const int logicalTargetX =
                list->getX() +
                list->getWidth() / 2;

        const int logicalTargetY =
                list->getY() +
                static_cast<int>(visibleLine) *
                        lineHeight +
                lineHeight / 2;

        Screen *screen = game->_screen;

        const double scaleX = screen->getXScale();
        const double scaleY = screen->getYScale();

        if (scaleX <= 0.0 || scaleY <= 0.0)
        {
                return false;
        }

        targetX =
                screen->getCursorLeftBlackBand() +
                static_cast<int>(
                        std::lround(
                                logicalTargetX * scaleX));

        targetY =
                screen->getCursorTopBlackBand() +
                static_cast<int>(
                        std::lround(
                                logicalTargetY * scaleY));

        return true;
}

bool MenuNavigation3DS::isGeoscape(
        const Game *game)
{
        if (!game || game->_states.empty())
        {
                return false;
        }

        return dynamic_cast<GeoscapeState *>(
                game->_states.back()) != nullptr;
}

bool MenuNavigation3DS::isBattlescape(
        const Game *game)
{
        if (!game || game->_states.empty())
        {
                return false;
        }

        return dynamic_cast<BattlescapeState *>(
                game->_states.back()) != nullptr;
}


/*
 * OXCE_3DS_INVENTORY_SNAP_NAV
 */
bool MenuNavigation3DS::isInventory(
        const Game *game)
{
        if (!game || game->_states.empty())
        {
                return false;
        }

        return dynamic_cast<InventoryState *>(
                game->_states.back()) != nullptr;
}

bool MenuNavigation3DS::cancelInventoryCarry(
        Game *game)
{
        if (!game || game->_states.empty())
        {
                return false;
        }

        InventoryState *state =
                dynamic_cast<InventoryState *>(
                        game->_states.back());

        if (!state ||
                !state->_inv ||
                !state->_inv->_selItem)
        {
                return false;
        }

        BattleItem *item =
                state->_inv->_selItem;

        /*
         * Match Inventory's existing right-click cancellation.
         * Ground stacks are decremented when picked up and must be
         * restored before clearing the selected item.
         */
        if (item->getSlot() &&
                item->getSlot()->getType() ==
                        INV_GROUND)
        {
                state->_inv->
                        _stackLevel[
                                item->getSlotX()]
                                [item->getSlotY()] += 1;
        }

        state->_inv->setSelectedItem(nullptr);
        return true;
}


/*
 * OXCE_3DS_INVENTORY_BOTTOM_PANEL
 */
Surface *MenuNavigation3DS::getInventoryBottomControlSurface(
        const Game *game,
        InventoryBottomControl control,
        int &logicalX,
        int &logicalY,
        bool &enabled)
{
        enabled = false;

        if (!game ||
                game->_states.empty())
        {
                return nullptr;
        }

        InventoryState *state =
                dynamic_cast<InventoryState *>(
                        game->_states.back());

        if (!state ||
                !state->_inv)
        {
                return nullptr;
        }

        BattlescapeButton *button = nullptr;
        Surface *graphic = nullptr;

        const bool templateControl =
                control == INVENTORY_SAVE_CONFIG ||
                control == INVENTORY_LOAD_CONFIG;

        switch (control)
        {
                case INVENTORY_OK:
                        button = state->_btnOk;
                        break;

                case INVENTORY_PREVIOUS:
                        button = state->_btnPrev;
                        break;

                case INVENTORY_NEXT:
                        button = state->_btnNext;
                        break;

                case INVENTORY_UNLOAD:
                        button = state->_btnUnload;
                        break;

                case INVENTORY_GROUND:
                        button = state->_btnGround;
                        break;

                case INVENTORY_SAVE_CONFIG:
                        button = state->_btnCreateTemplate;

                        graphic =
                                game->getMod()->getSurface(
                                        state->_curInventoryTemplate.empty()
                                                ? "InvCopy"
                                                : "InvCopyActive");
                        break;

                case INVENTORY_LOAD_CONFIG:
                        button = state->_btnApplyTemplate;

                        graphic =
                                game->getMod()->getSurface(
                                        state->_curInventoryTemplate.empty()
                                                ? "InvPasteEmpty"
                                                : "InvPaste");
                        break;

                case INVENTORY_LINKS:
                        button = state->_btnLinks;
                        break;
        }

        if (!button ||
                button->getWidth() <= 0 ||
                button->getHeight() <= 0)
        {
                return nullptr;
        }

        const bool visible =
                button->_visible &&
                !button->_hidden;

        /*
         * Hidden ordinary controls are omitted. Template controls stay
         * in the toolbar so TU inventory can display disabled versions.
         */
        if (!visible &&
                !templateControl)
        {
                return nullptr;
        }

        if (!graphic)
        {
                graphic = button;
        }

        if (!graphic ||
                !graphic->getSurface() ||
                graphic->getWidth() <= 0 ||
                graphic->getHeight() <= 0)
        {
                return nullptr;
        }

        logicalX =
                button->getX() +
                button->getWidth() / 2;

        logicalY =
                button->getY() +
                button->getHeight() / 2;

        enabled = visible;

        return graphic;
}

bool MenuNavigation3DS::isActive(const Game *game)
{
        if (!game || game->_states.empty())
        {
                return false;
        }

        State *state = game->_states.back();

        /*
         * Spatial menu snapping is suspended while a text field owns
         * controller input.
         */
        if (getFocusedTextEdit(game))
        {
                return false;
        }

        if (activeSlider && activeSliderState != state)
        {
                activeSlider = nullptr;
                activeSliderState = nullptr;
        }

        ComboBox *openCombo =
                dynamic_cast<ComboBox *>(state->_modal);

        if (!openCombo ||
                !openCombo->_list ||
                !openCombo->_list->getVisible())
        {
                trackedOpenCombo = nullptr;
                trackedComboRow = 0;
        }

        /*
         * Preserve free D-pad cursor movement in the actual geoscape
         * and battlescape. Popup states above them are separate State
         * objects and still use menu navigation.
         */
        if (isGeoscape(game) ||
                isBattlescape(game))
        {
                return false;
        }

        /*
         * Inventory uses a dedicated spatial target pass rather than
         * treating its full-screen Inventory surface as one huge button.
         */
        if (dynamic_cast<InventoryState *>(state))
        {
                return true;
        }

        auto isCandidate = [](Surface *surface)
        {
                if (!surface ||
                        !surface->_visible ||
                        surface->_hidden ||
                        surface->getWidth() <= 0 ||
                        surface->getHeight() <= 0)
                {
                        return false;
                }

                /*
                 * Generic selectable TextList activation.
                 */
                TextList *list =
                        dynamic_cast<TextList *>(surface);

                if (list &&
                        list->isScrollbarVisible())
                {
                        return true;
                }

                if (list && list->isSelectable())
                {
                        for (size_t row = 0;
                                row < list->getTexts();
                                ++row)
                        {
                                if (list->isRowSelectable(row))
                                {
                                        return true;
                                }
                        }
                }

                if (dynamic_cast<ArrowButton *>(surface))
                {
                        return true;
                }

                if (dynamic_cast<ComboBox *>(surface) ||
                        dynamic_cast<Slider *>(surface))
                {
                        return true;
                }

                InteractiveSurface *interactive =
                        dynamic_cast<TextButton *>(surface);

                if (!interactive)
                {
                        interactive =
                                dynamic_cast<ImageButton *>(
                                        surface);
                }

                /*
                 * Include plain primary-click InteractiveSurface controls,
                 * such as the seven icon tabs on the Graphs screen.
                 */
                if (!interactive)
                {
                        interactive =
                                dynamic_cast<InteractiveSurface *>(
                                        surface);
                }

                return interactive &&
                        interactive->
                        isMenuNavigationEnabled() &&
                        interactive->isButtonHandled(
                                SDL_BUTTON_LEFT);
        };

        if (state->_modal)
        {
                return isCandidate(state->_modal);
        }

        for (auto i = state->_surfaces.rbegin();
                i != state->_surfaces.rend();
                ++i)
        {
                if (isCandidate(*i))
                {
                        return true;
                }
        }

        return false;
}

bool MenuNavigation3DS::getTextListScrollBarThumb(
        TextList *list,
        int &left,
        int &top,
        int &right,
        int &bottom)
{
        if (!list ||
                !list->_scrollbar ||
                !list->isScrollbarVisible())
        {
                return false;
        }

        ScrollBar *scrollbar = list->_scrollbar;

        const int rowCount =
                std::max(
                        1,
                        static_cast<int>(
                                list->getRowsDoNotUse()));

        const int trackHeight =
                std::max(
                        1,
                        scrollbar->getHeight());

        const double scale =
                static_cast<double>(
                        trackHeight) /
                static_cast<double>(
                        rowCount);

        int thumbTop =
                static_cast<int>(
                        std::floor(
                                list->getScroll() *
                                scale));

        int thumbHeight =
                std::max(
                        1,
                        static_cast<int>(
                                std::ceil(
                                        list->
                                                getVisibleRows() *
                                        scale)));

        thumbTop =
                std::max(
                        0,
                        std::min(
                                trackHeight - 1,
                                thumbTop));

        thumbHeight =
                std::max(
                        1,
                        std::min(
                                trackHeight - thumbTop,
                                thumbHeight));

        left = scrollbar->getX();
        top = scrollbar->getY() + thumbTop;

        right =
                left +
                scrollbar->getWidth();

        bottom =
                top +
                thumbHeight;

        return right > left && bottom > top;
}


bool MenuNavigation3DS::getTextListScrollBarTarget(
        TextList *list,
        const Screen *screen,
        int &targetX,
        int &targetY)
{
        if (!screen)
        {
                return false;
        }

        int left = 0;
        int top = 0;
        int right = 0;
        int bottom = 0;

        if (!getTextListScrollBarThumb(
                list,
                left,
                top,
                right,
                bottom))
        {
                return false;
        }

        const double scaleX =
                screen->getXScale();

        const double scaleY =
                screen->getYScale();

        if (scaleX <= 0.0 || scaleY <= 0.0)
        {
                return false;
        }

        const int logicalTargetX =
                left +
                std::max(
                        1,
                        right - left) /
                        2;

        const int logicalTargetY =
                top +
                std::max(
                        1,
                        bottom - top) /
                        2;

        targetX =
                screen->
                        getCursorLeftBlackBand() +
                static_cast<int>(
                        std::lround(
                                logicalTargetX *
                                scaleX));

        targetY =
                screen->
                        getCursorTopBlackBand() +
                static_cast<int>(
                        std::lround(
                                logicalTargetY *
                                scaleY));

        return true;
}


bool MenuNavigation3DS::findTarget(
        const Game *game,
        Direction direction,
        int cursorX,
        int cursorY,
        int &targetX,
        int &targetY)
{
        if (!isActive(game) || game->_states.empty())
        {
                return false;
        }

        State *state = game->_states.back();
        Screen *screen = game->_screen;

        if (!screen)
        {
                return false;
        }

        const double scaleX = screen->getXScale();
        const double scaleY = screen->getYScale();

        if (scaleX <= 0.0 || scaleY <= 0.0)
        {
                return false;
        }

        const int leftBand =
                screen->getCursorLeftBlackBand();

        const int topBand =
                screen->getCursorTopBlackBand();

        const int logicalCursorX = static_cast<int>(
                (cursorX - leftBand) / scaleX);

        const int logicalCursorY = static_cast<int>(
                (cursorY - topBand) / scaleY);


        /*
         * OXCE_3DS_INVENTORY_SNAP_NAV
         *
         * Inventory has two navigation modes:
         *
         * 1. Browsing: visible items and normal controls are targets.
         * 2. Carrying: only geometrically possible item anchors are
         *    targets. Occupied anchors are intentionally retained so
         *    OXCE can show its normal placement rejection.
         */
        InventoryState *inventoryState =
                dynamic_cast<InventoryState *>(state);

        if (inventoryState)
        {
                struct InventoryCandidate
                {
                        int left;
                        int top;
                        int right;
                        int bottom;
                        int targetX;
                        int targetY;
                };

                std::vector<InventoryCandidate> candidates;

                auto addCandidate =
                        [&](int left,
                                int top,
                                int right,
                                int bottom,
                                int pointX,
                                int pointY)
                {
                        if (right <= left ||
                                bottom <= top)
                        {
                                return;
                        }

                        InventoryCandidate candidate = {};

                        candidate.left = left;
                        candidate.top = top;
                        candidate.right = right;
                        candidate.bottom = bottom;
                        candidate.targetX = pointX;
                        candidate.targetY = pointY;

                        candidates.push_back(candidate);
                };

                auto addPointCandidate =
                        [&](int pointX, int pointY)
                {
                        /*
                         * Carrying anchors overlap heavily if their full
                         * item footprint is used as the hit rectangle.
                         * A small box around the exact anchor target keeps
                         * one-cell D-pad movement deterministic.
                         */
                        constexpr int radius = 4;

                        addCandidate(
                                pointX - radius,
                                pointY - radius,
                                pointX + radius + 1,
                                pointY + radius + 1,
                                pointX,
                                pointY);
                };

                auto addSurfaceCandidate =
                        [&](Surface *surface)
                {
                        if (!surface ||
                                !surface->_visible ||
                                surface->_hidden ||
                                surface->getWidth() <= 0 ||
                                surface->getHeight() <= 0)
                        {
                                return;
                        }

                        const int left = surface->getX();
                        const int top = surface->getY();
                        const int right =
                                left + surface->getWidth();
                        const int bottom =
                                top + surface->getHeight();

                        addCandidate(
                                left,
                                top,
                                right,
                                bottom,
                                left +
                                        surface->
                                                getWidth() /
                                                2,
                                top +
                                        surface->
                                                getHeight() /
                                                2);
                };

                Inventory *inventory =
                        inventoryState->_inv;

                BattleUnit *unit =
                        inventory ?
                                inventory->_selUnit :
                                nullptr;

                BattleItem *heldItem =
                        inventory ?
                                inventory->_selItem :
                                nullptr;

                if (!inventory || !unit)
                {
                        return false;
                }

                if (heldItem)
                {
                        const RuleItem *rules =
                                heldItem->getRules();

                        if (!rules)
                        {
                                return false;
                        }

                        /*
                         * OXCE_3DS_INVENTORY_UNLOAD_SNAP_TIGHTER
                         *
                         * A loaded carried weapon may navigate to the
                         * normal Unload button. Its existing handler
                         * remains responsible for the actual operation.
                         */
                        if (heldItem->isWeaponWithAmmo() &&
                                heldItem->haveAnyAmmo())
                        {
                                addSurfaceCandidate(
                                        inventoryState->
                                                _btnUnload);
                        }

                        const int itemWidth =
                                std::max(
                                        1,
                                        rules->
                                        getInventoryWidth());

                        const int itemHeight =
                                std::max(
                                        1,
                                        rules->
                                        getInventoryHeight());

                        for (const auto &pair :
                                *game->getMod()->
                                        getInventories())
                        {
                                RuleInventory *slot =
                                        pair.second;

                                if (!slot ||
                                        !rules->
                                        canBePlacedIntoInventorySection(
                                                slot))
                                {
                                        continue;
                                }

                                if (slot->getType() ==
                                        INV_HAND)
                                {
                                        /*
                                         * Inventory's drop calculation
                                         * samples the item anchor from the
                                         * centre of the carried footprint.
                                         */
                                        const int pointX =
                                                slot->getX() +
                                                itemWidth *
                                                        RuleInventory::
                                                        SLOT_W / 2;

                                        const int pointY =
                                                slot->getY() +
                                                itemHeight *
                                                        RuleInventory::
                                                        SLOT_H / 2;

                                        addPointCandidate(
                                                pointX,
                                                pointY);

                                        continue;
                                }

                                if (slot->getType() ==
                                        INV_GROUND)
                                {
                                        for (int localY = 0;
                                                localY <
                                                        inventory->
                                                        _groundSlotsY;
                                                ++localY)
                                        {
                                                for (int localX = 0;
                                                        localX <
                                                                inventory->
                                                                _groundSlotsX;
                                                        ++localX)
                                                {
                                                        const int anchorX =
                                                                inventory->
                                                                _groundOffset +
                                                                localX;

                                                        const int anchorY =
                                                                localY;

                                                        if (!slot->
                                                                fitItemInSlot(
                                                                        rules,
                                                                        anchorX,
                                                                        anchorY))
                                                        {
                                                                continue;
                                                        }

                                                        const int pointX =
                                                                slot->getX() +
                                                                localX *
                                                                        RuleInventory::
                                                                        SLOT_W +
                                                                itemWidth *
                                                                        RuleInventory::
                                                                        SLOT_W /
                                                                        2;

                                                        const int pointY =
                                                                slot->getY() +
                                                                anchorY *
                                                                        RuleInventory::
                                                                        SLOT_H +
                                                                itemHeight *
                                                                        RuleInventory::
                                                                        SLOT_H /
                                                                        2;

                                                        addPointCandidate(
                                                                pointX,
                                                                pointY);
                                                }
                                        }

                                        continue;
                                }

                                std::set<
                                        std::pair<int, int>>
                                        seenAnchors;

                                for (const RuleSlot &anchor :
                                        *slot->getSlots())
                                {
                                        const std::pair<int, int>
                                                key(
                                                        anchor.x,
                                                        anchor.y);

                                        if (!seenAnchors.insert(
                                                key).second)
                                        {
                                                continue;
                                        }

                                        if (!slot->fitItemInSlot(
                                                rules,
                                                anchor.x,
                                                anchor.y))
                                        {
                                                continue;
                                        }

                                        const int pointX =
                                                slot->getX() +
                                                anchor.x *
                                                        RuleInventory::
                                                        SLOT_W +
                                                itemWidth *
                                                        RuleInventory::
                                                        SLOT_W / 2;

                                        const int pointY =
                                                slot->getY() +
                                                anchor.y *
                                                        RuleInventory::
                                                        SLOT_H +
                                                itemHeight *
                                                        RuleInventory::
                                                        SLOT_H / 2;

                                        addPointCandidate(
                                                pointX,
                                                pointY);
                                }
                        }
                }
                else
                {
                        /*
                         * Normal inventory controls.
                         */
                        addSurfaceCandidate(
                                inventoryState->_btnOk);
                        addSurfaceCandidate(
                                inventoryState->_btnPrev);
                        addSurfaceCandidate(
                                inventoryState->_btnNext);
                        addSurfaceCandidate(
                                inventoryState->_btnUnload);
                        addSurfaceCandidate(
                                inventoryState->_btnGround);
                        addSurfaceCandidate(
                                inventoryState->_btnRank);
                        addSurfaceCandidate(
                                inventoryState->_btnArmor);
                        addSurfaceCandidate(
                                inventoryState->
                                        _btnCreateTemplate);
                        addSurfaceCandidate(
                                inventoryState->
                                        _btnApplyTemplate);
                        addSurfaceCandidate(
                                inventoryState->_btnLinks);
                        addSurfaceCandidate(
                                inventoryState->_txtName);
                        addSurfaceCandidate(
                                inventoryState->
                                        _btnQuickSearch);

                        std::set<
                                std::pair<int, int>>
                                groundPositions;

                        auto addItemCandidate =
                                [&](BattleItem *item)
                        {
                                if (!item ||
                                        item->getRules()->
                                                isFixed() ||
                                        !item->getSlot())
                                {
                                        return;
                                }

                                const RuleItem *rules =
                                        item->getRules();

                                const int itemWidth =
                                        rules->
                                        getInventoryWidth();

                                const int itemHeight =
                                        rules->
                                        getInventoryHeight();

                                if (itemWidth <= 0 ||
                                        itemHeight <= 0)
                                {
                                        return;
                                }

                                const RuleInventory *slot =
                                        item->getSlot();

                                int left = 0;
                                int top = 0;
                                int right = 0;
                                int bottom = 0;

                                if (slot->getType() ==
                                        INV_HAND)
                                {
                                        left = slot->getX();
                                        top = slot->getY();

                                        right =
                                                left +
                                                RuleInventory::
                                                        HAND_W *
                                                RuleInventory::
                                                        SLOT_W;

                                        bottom =
                                                top +
                                                RuleInventory::
                                                        HAND_H *
                                                RuleInventory::
                                                        SLOT_H;
                                }
                                else if (slot->getType() ==
                                        INV_GROUND)
                                {
                                        if (item->getSlotX() <
                                                        inventory->
                                                        _groundOffset ||
                                                item->getSlotX() >=
                                                        inventory->
                                                        _groundOffset +
                                                        inventory->
                                                        _groundSlotsX)
                                        {
                                                return;
                                        }

                                        const std::pair<int, int>
                                                position(
                                                        item->
                                                        getSlotX(),
                                                        item->
                                                        getSlotY());

                                        /*
                                         * A displayed ground stack is one
                                         * visual target, not one target per
                                         * stacked BattleItem.
                                         */
                                        if (!groundPositions.insert(
                                                position).second)
                                        {
                                                return;
                                        }

                                        const int localX =
                                                item->getSlotX() -
                                                inventory->
                                                        _groundOffset;

                                        left =
                                                slot->getX() +
                                                localX *
                                                        RuleInventory::
                                                        SLOT_W;

                                        top =
                                                slot->getY() +
                                                item->getSlotY() *
                                                        RuleInventory::
                                                        SLOT_H;

                                        right =
                                                std::min(
                                                        320,
                                                        left +
                                                        itemWidth *
                                                                RuleInventory::
                                                                SLOT_W);

                                        bottom =
                                                std::min(
                                                        200,
                                                        top +
                                                        itemHeight *
                                                                RuleInventory::
                                                                SLOT_H);
                                }
                                else
                                {
                                        left =
                                                slot->getX() +
                                                item->getSlotX() *
                                                        RuleInventory::
                                                        SLOT_W;

                                        top =
                                                slot->getY() +
                                                item->getSlotY() *
                                                        RuleInventory::
                                                        SLOT_H;

                                        right =
                                                left +
                                                itemWidth *
                                                        RuleInventory::
                                                        SLOT_W;

                                        bottom =
                                                top +
                                                itemHeight *
                                                        RuleInventory::
                                                        SLOT_H;
                                }

                                addCandidate(
                                        left,
                                        top,
                                        right,
                                        bottom,
                                        left +
                                                std::max(
                                                        1,
                                                        right -
                                                                left) /
                                                2,
                                        top +
                                                std::max(
                                                        1,
                                                        bottom -
                                                                top) /
                                                2);
                        };

                        for (BattleItem *item :
                                *unit->getInventory())
                        {
                                addItemCandidate(item);
                        }

                        if (unit->getTile())
                        {
                                for (BattleItem *item :
                                        *unit->getTile()->
                                                getInventory())
                                {
                                        addItemCandidate(item);
                                }
                        }
                }

                if (candidates.empty())
                {
                        return false;
                }

                int originX = logicalCursorX;
                int originY = logicalCursorY;

                bool cursorOnCandidate = false;

                const InventoryCandidate
                        *originCandidate = nullptr;

                for (const InventoryCandidate &candidate :
                        candidates)
                {
                        if (logicalCursorX >=
                                        candidate.left &&
                                logicalCursorX <
                                        candidate.right &&
                                logicalCursorY >=
                                        candidate.top &&
                                logicalCursorY <
                                        candidate.bottom)
                        {
                                originX = candidate.targetX;
                                originY = candidate.targetY;
                                cursorOnCandidate = true;
                                originCandidate = &candidate;
                                break;
                        }
                }

                const InventoryCandidate *best = nullptr;

                long bestScore =
                        std::numeric_limits<long>::max();

                for (const InventoryCandidate &candidate :
                        candidates)
                {
                        const int dx =
                                candidate.targetX -
                                originX;

                        const int dy =
                                candidate.targetY -
                                originY;

                        int primary = 0;
                        int secondary = 0;
                        bool valid = false;

                        switch (direction)
                        {
                                case UP:
                                        valid = dy < 0;
                                        primary = -dy;
                                        secondary =
                                                std::abs(dx);
                                        break;

                                case DOWN:
                                        valid = dy > 0;
                                        primary = dy;
                                        secondary =
                                                std::abs(dx);
                                        break;

                                case LEFT:
                                        valid = dx < 0;
                                        primary = -dx;
                                        secondary =
                                                std::abs(dy);
                                        break;

                                case RIGHT:
                                        valid = dx > 0;
                                        primary = dx;
                                        secondary =
                                                std::abs(dy);
                                        break;
                        }

                        if (!valid)
                        {
                                continue;
                        }

                        /*
                         * OXCE_3DS_INVENTORY_DIRECTIONAL_SCORING
                         *
                         * Directional intent matters more than raw
                         * Euclidean proximity. For Left/Right, vertical
                         * deviation is expensive. For Up/Down, horizontal
                         * deviation is expensive.
                         *
                         * This keeps navigation in the same visual row or
                         * column without restoring the old absolute
                         * alignment penalty.
                         */
                        constexpr long secondaryWeight = 10L;

                        const long score =
                                static_cast<long>(
                                        primary) *
                                        primary +
                                static_cast<long>(
                                        secondary) *
                                        secondary *
                                        secondaryWeight;

                        if (score < bestScore)
                        {
                                bestScore = score;
                                best = &candidate;
                        }
                }

                if (!best && !cursorOnCandidate)
                {
                        for (const InventoryCandidate &candidate :
                                candidates)
                        {
                                const long dx =
                                        candidate.targetX -
                                        logicalCursorX;

                                const long dy =
                                        candidate.targetY -
                                        logicalCursorY;

                                const long score =
                                        dx * dx + dy * dy;

                                if (score < bestScore)
                                {
                                        bestScore = score;
                                        best = &candidate;
                                }
                        }
                }

                if (!best)
                {
                        return false;
                }

                targetX =
                        leftBand +
                        static_cast<int>(
                                std::lround(
                                        best->targetX *
                                        scaleX));

                targetY =
                        topBand +
                        static_cast<int>(
                                std::lround(
                                        best->targetY *
                                        scaleY));

                targetX =
                        std::max(
                                0,
                                std::min(
                                        screen->getWidth() - 1,
                                        targetX));

                targetY =
                        std::max(
                                0,
                                std::min(
                                        screen->getHeight() - 1,
                                        targetY));

                return true;
        }

        /*
         * OXCE_3DS_SCROLLBAR_THUMB_NAV
         *
         * A scrollbar thumb is a real navigation target even though it
         * is owned by TextList rather than State::_surfaces.
         */
        TextList *cursorScrollList = nullptr;

        auto findCursorScrollList =
                [&](TextList *list)
        {
                if (cursorScrollList || !list)
                {
                        return;
                }

                int thumbLeft = 0;
                int thumbTop = 0;
                int thumbRight = 0;
                int thumbBottom = 0;

                if (!getTextListScrollBarThumb(
                        list,
                        thumbLeft,
                        thumbTop,
                        thumbRight,
                        thumbBottom))
                {
                        return;
                }

                if (logicalCursorX >= thumbLeft &&
                        logicalCursorX < thumbRight &&
                        logicalCursorY >= thumbTop &&
                        logicalCursorY < thumbBottom)
                {
                        cursorScrollList = list;
                }
        };

        ComboBox *scrollCombo =
                dynamic_cast<ComboBox *>(
                        state->_modal);

        if (scrollCombo &&
                scrollCombo->_list &&
                scrollCombo->_list->getVisible())
        {
                findCursorScrollList(
                        scrollCombo->_list);
        }
        else if (state->_modal)
        {
                findCursorScrollList(
                        dynamic_cast<TextList *>(
                                state->_modal));
        }
        else
        {
                for (auto i =
                                state->_surfaces.rbegin();
                        i !=
                                state->_surfaces.rend();
                        ++i)
                {
                        findCursorScrollList(
                                dynamic_cast<TextList *>(
                                        *i));

                        if (cursorScrollList)
                        {
                                break;
                        }
                }
        }

        if (cursorScrollList)
        {
                if (direction == UP ||
                        direction == DOWN)
                {
                        if (direction == UP)
                        {
                                cursorScrollList->
                                        scrollUp(
                                                false,
                                                false,
                                                1);
                        }
                        else
                        {
                                cursorScrollList->
                                        scrollDown(
                                                false,
                                                false,
                                                1);
                        }

                        return
                                getTextListScrollBarTarget(
                                        cursorScrollList,
                                        screen,
                                        targetX,
                                        targetY);
                }

                if (direction == LEFT &&
                        cursorScrollList->
                                isSelectable() &&
                        !cursorScrollList->
                                _rows.empty())
                {
                        int lineHeight = 1;
                        size_t visibleRows = 1;
                        size_t row = 0;

                        if (getTextListRowAtY(
                                cursorScrollList,
                                logicalCursorY,
                                row,
                                lineHeight,
                                visibleRows))
                        {
                                size_t firstPhysicalLine = 0;
                                size_t lastPhysicalLine = 0;

                                if (getTextListPhysicalRowBounds(
                                        cursorScrollList,
                                        row,
                                        visibleRows,
                                        false,
                                        firstPhysicalLine,
                                        lastPhysicalLine) &&
                                        getTextListRowTarget(
                                                cursorScrollList,
                                                screen,
                                                firstPhysicalLine,
                                                lastPhysicalLine,
                                                visibleRows,
                                                lineHeight,
                                                targetX,
                                                targetY))
                                {
                                        return true;
                                }
                        }
                }
        }


        /*
         * Generic selectable TextList row navigation.
         *
         * Up and Down move between enabled logical rows. Left and
         * Right invoke exact keyboard handlers when a list defines
         * them, as the Advanced settings list does.
         */
        TextList *cursorList = nullptr;
        bool cursorOnTextListArrow = false;

        auto pointInsideSurface =
                [&](Surface *surface)
        {
                return surface &&
                        surface->_visible &&
                        !surface->_hidden &&
                        logicalCursorX >= surface->getX() &&
                        logicalCursorX <
                                surface->getX() +
                                surface->getWidth() &&
                        logicalCursorY >= surface->getY() &&
                        logicalCursorY <
                                surface->getY() +
                                surface->getHeight();
        };

        /*
         * Row arrows live inside a TextList rather than in State::_surfaces.
         * Detect them before the list's special Up/Down row navigation so
         * spatial navigation can remain in the same arrow column.
         */
        auto pointInsideVisibleTextListArrow =
                [&](TextList *list)
        {
                if (!list ||
                        !list->_visible ||
                        list->_hidden)
                {
                        return false;
                }

                if (pointInsideSurface(list->_up) ||
                        pointInsideSurface(list->_down))
                {
                        return true;
                }

                if (list->_arrowPos == -1 ||
                        list->_rows.empty() ||
                        list->_texts.empty() ||
                        !list->_font ||
                        list->_scroll >=
                                list->_rows.size())
                {
                        return false;
                }

                const int lineHeight =
                        getTextListLineHeight(list);

                int arrowY = list->getY();

                for (std::size_t physical =
                                list->_scroll;
                        physical > 0 &&
                                list->_rows[physical] ==
                                list->_rows[physical - 1];
                        --physical)
                {
                        arrowY -= lineHeight;
                }

                const int maximumY =
                        list->getY() +
                        list->getHeight();

                const std::size_t firstLogicalRow =
                        list->_rows[list->_scroll];

                const std::size_t logicalEnd =
                        std::min(
                                list->_texts.size(),
                                firstLogicalRow +
                                        getTextListVisibleRows(
                                                list));

                for (std::size_t row = firstLogicalRow;
                        row < logicalEnd &&
                                arrowY < maximumY;
                        ++row)
                {
                        if (row <
                                        list->_arrowLeft.size() &&
                                row <
                                        list->_arrowRight.size())
                        {
                                ArrowButton *left =
                                        list->_arrowLeft[row];

                                ArrowButton *right =
                                        list->_arrowRight[row];

                                left->setY(arrowY);
                                right->setY(arrowY);

                                if (arrowY >= list->getY() &&
                                        (pointInsideSurface(left) ||
                                         pointInsideSurface(right)))
                                {
                                        return true;
                                }
                        }

                        if (!list->_texts[row].empty())
                        {
                                arrowY +=
                                        list->_texts[row]
                                                .front()
                                                ->getHeight() +
                                        list->_font
                                                ->getSpacing();
                        }
                        else
                        {
                                arrowY += lineHeight;
                        }
                }

                return false;
        };

        auto findCursorList = [&](Surface *surface)
        {
                TextList *list =
                        dynamic_cast<TextList *>(surface);

                if (cursorList ||
                        cursorOnTextListArrow ||
                        !list ||
                        !list->_visible ||
                        list->_hidden ||
                        !list->isSelectable() ||
                        list->_rows.empty() ||
                        !list->_font)
                {
                        return;
                }

                if (pointInsideVisibleTextListArrow(list))
                {
                        cursorOnTextListArrow = true;
                        return;
                }

                if (logicalCursorX >= list->getX() &&
                        logicalCursorX <
                                list->getX() +
                                list->getWidth() &&
                        logicalCursorY >= list->getY() &&
                        logicalCursorY <
                                list->getY() +
                                list->getHeight())
                {
                        cursorList = list;
                }
        };

        if (state->_modal)
        {
                findCursorList(state->_modal);
        }
        else
        {
                for (auto i = state->_surfaces.rbegin();
                        i != state->_surfaces.rend();
                        ++i)
                {
                        findCursorList(*i);

                        if (cursorList)
                        {
                                break;
                        }
                }
        }

        if (cursorList)
        {
                int lineHeight = 1;
                size_t visibleRows = 1;
                size_t currentRow = 0;

                if (!getTextListRowAtY(
                        cursorList,
                        logicalCursorY,
                        currentRow,
                        lineHeight,
                        visibleRows))
                {
                        return false;
                }

                /*
                 * Only use exact Left/Right handlers. Controls has a
                 * wildcard key-binding handler, which must not receive
                 * D-pad navigation.
                 */
                if ((direction == LEFT ||
                        direction == RIGHT) &&
                        cursorList->
                                isRowSelectable(currentRow))
                {
                        const SDLKey key =
                                direction == LEFT ?
                                        SDLK_LEFT :
                                        SDLK_RIGHT;

                        auto handler =
                                cursorList->_keyPress.find(key);

                        if (handler !=
                                cursorList->_keyPress.end())
                        {
                                SDL_Event event = {};
                                event.type = SDL_KEYDOWN;
                                event.key.type = SDL_KEYDOWN;
                                event.key.state = SDL_PRESSED;
                                event.key.keysym.sym = key;
                                event.key.keysym.mod = KMOD_NONE;

                                Action action(
                                        &event,
                                        scaleX,
                                        scaleY,
                                        topBand,
                                        leftBand);

                                action.setSender(cursorList);

                                (state->*(handler->second))(
                                        &action);

                                targetX = cursorX;
                                targetY = cursorY;
                                return true;
                        }
                }

                /*
                 * Right from a list row enters its scrollbar thumb.
                 * Exact row-specific Right handlers still take priority.
                 */
                if (direction == RIGHT &&
                        getTextListScrollBarTarget(
                                cursorList,
                                screen,
                                targetX,
                                targetY))
                {
                        return true;
                }

                if (direction == UP ||
                        direction == DOWN)
                {
                        int targetRow =
                                static_cast<int>(
                                        currentRow);

                        const int step =
                                direction == DOWN ?
                                        1 : -1;

                        const int rowCount =
                                static_cast<int>(
                                        cursorList->
                                                _texts.size());

                        do
                        {
                                targetRow += step;
                        }
                        while (targetRow >= 0 &&
                                targetRow < rowCount &&
                                !cursorList->
                                        isRowSelectable(
                                                static_cast<
                                                        size_t>(
                                                        targetRow)));

                        /*
                         * List-edge spatial escape.
                         *
                         * When there is no further selectable row, do
                         * not consume the direction. Fall through to
                         * ordinary spatial navigation so Up can reach
                         * controls above the list and Down can reach
                         * controls below it.
                         */
                        if (targetRow >= 0 &&
                                targetRow < rowCount)
                        {
                        size_t firstPhysicalLine = 0;
                        size_t lastPhysicalLine = 0;

                        if (!getTextListPhysicalRowBounds(
                                cursorList,
                                static_cast<size_t>(
                                        targetRow),
                                visibleRows,
                                true,
                                firstPhysicalLine,
                                lastPhysicalLine))
                        {
                                return false;
                        }

                        if (!getTextListRowTarget(
                                cursorList,
                                screen,
                                firstPhysicalLine,
                                lastPhysicalLine,
                                visibleRows,
                                lineHeight,
                                targetX,
                                targetY))
                        {
                                return false;
                        }

                        return true;
                        }
                }
        }

        /*
         * Slider adjustment mode captures Left and Right until B
         * returns to ordinary menu navigation.
         */
        if (activeSlider &&
                activeSliderState == state)
        {
                if (direction != LEFT &&
                        direction != RIGHT)
                {
                        return false;
                }

                const int rangeDirection =
                        activeSlider->_max >= activeSlider->_min ?
                                1 : -1;

                const int delta =
                        direction == RIGHT ?
                                rangeDirection : -rangeDirection;

                const int oldValue =
                        activeSlider->_value;

                activeSlider->setValue(
                        oldValue + delta);

                const int newValue =
                        activeSlider->_value;

                const int logicalTargetX =
                        activeSlider->_button->getX() +
                        activeSlider->_button->getWidth() / 2;

                const int logicalTargetY =
                        activeSlider->_button->getY() +
                        activeSlider->_button->getHeight() / 2;

                targetX = leftBand + static_cast<int>(
                        std::lround(
                                logicalTargetX * scaleX));

                targetY = topBand + static_cast<int>(
                        std::lround(
                                logicalTargetY * scaleY));

                if (newValue != oldValue &&
                        activeSlider->_change)
                {
                        SDL_Event event = {};
                        event.type = SDL_MOUSEMOTION;

                        event.motion.type =
                                SDL_MOUSEMOTION;

                        event.motion.x =
                                static_cast<Uint16>(targetX);

                        event.motion.y =
                                static_cast<Uint16>(targetY);

                        Action action(
                                &event,
                                scaleX,
                                scaleY,
                                topBand,
                                leftBand);

                        action.setSender(activeSlider);

                        (state->*activeSlider->_change)(
                                &action);
                }

                return true;
        }

        /*
         * While a ComboBox is open, Up and Down move through its
         * rows rather than navigating to other menu controls.
         */
        ComboBox *openCombo =
                dynamic_cast<ComboBox *>(state->_modal);

        if (openCombo &&
                openCombo->_list &&
                openCombo->_list->getVisible())
        {
                if (direction == RIGHT &&
                        getTextListScrollBarTarget(
                                openCombo->_list,
                                screen,
                                targetX,
                                targetY))
                {
                        return true;
                }

                if (direction != UP && direction != DOWN)
                {
                        return false;
                }

                TextList *list = openCombo->_list;
                const size_t itemCount = list->getTexts();

                if (itemCount == 0)
                {
                        return false;
                }

                /*
                 * Mouse hover is updated only after OXCE processes the
                 * SDL mouse-motion event. Track the navigation row
                 * directly so rapid D-pad presses cannot use stale hover
                 * information after the list scrolls.
                 */
                if (trackedOpenCombo != openCombo)
                {
                        trackedOpenCombo = openCombo;
                        trackedComboRow = openCombo->_sel;

                        if (trackedComboRow >= itemCount)
                        {
                                trackedComboRow = itemCount - 1;
                        }
                }

                size_t row = trackedComboRow;

                if (direction == UP)
                {
                        if (row == 0)
                        {
                                return false;
                        }

                        --row;
                }
                else
                {
                        if (row + 1 >= itemCount)
                        {
                                return false;
                        }

                        ++row;
                }

                trackedComboRow = row;

                const size_t visibleRows =
                        getTextListVisibleRows(list);

                const size_t scroll = list->getScroll();

                if (row < scroll)
                {
                        list->scrollTo(row);
                }
                else if (row >= scroll + visibleRows)
                {
                        list->scrollTo(
                                row - visibleRows + 1);
                }

                const int logicalTargetX =
                        list->getX() +
                        list->getWidth() / 2;

                /*
                 * ComboBox rows are stored at their original list
                 * coordinates. Account for the current scroll offset
                 * when placing the cursor over the visible row.
                 */
                const size_t visibleLine =
                        row >= list->getScroll() ?
                                row - list->getScroll() : 0;

                const int rowHeight =
                        getTextListLineHeight(list);

                const int logicalTargetY =
                        list->getY() +
                        static_cast<int>(visibleLine) *
                                rowHeight +
                        rowHeight / 2;

                targetX = leftBand + static_cast<int>(
                        std::lround(
                                logicalTargetX * scaleX));

                targetY = topBand + static_cast<int>(
                        std::lround(
                                logicalTargetY * scaleY));

                return true;
        }

        struct Candidate
        {
                int left;
                int top;
                int right;
                int bottom;
                int centerX;
                int centerY;
        };

        std::vector<Candidate> candidates;

        auto addSurfaceCandidate =
                [&](Surface *surface)
        {
                if (!surface ||
                        !surface->_visible ||
                        surface->_hidden ||
                        surface->getWidth() <= 0 ||
                        surface->getHeight() <= 0)
                {
                        return;
                }

                Candidate candidate = {};

                candidate.left = surface->getX();
                candidate.top = surface->getY();

                candidate.right =
                        candidate.left +
                        surface->getWidth();

                candidate.bottom =
                        candidate.top +
                        surface->getHeight();

                candidate.centerX =
                        candidate.left +
                        surface->getWidth() / 2;

                candidate.centerY =
                        candidate.top +
                        surface->getHeight() / 2;

                candidates.push_back(candidate);
        };

        /*
         * TextList row arrows are owned and handled by TextList rather
         * than being registered as independent State surfaces. Recreate
         * the same visible-row positioning used by TextList::blit(), then
         * expose those child buttons to spatial menu navigation.
         */
        auto addTextListArrowCandidates =
                [&](TextList *list)
        {
                if (!list ||
                        !list->_visible ||
                        list->_hidden)
                {
                        return;
                }

                /*
                 * Ordinary list scrolling arrows.
                 */
                addSurfaceCandidate(list->_up);
                addSurfaceCandidate(list->_down);

                if (list->_arrowPos == -1 ||
                        list->_rows.empty() ||
                        list->_texts.empty() ||
                        !list->_font ||
                        list->_scroll >=
                                list->_rows.size())
                {
                        return;
                }

                const int lineHeight =
                        getTextListLineHeight(list);

                int arrowY = list->getY();

                /*
                 * Match TextList's handling of a wrapped logical row
                 * whose first physical line lies above the viewport.
                 */
                for (std::size_t physical =
                                list->_scroll;
                        physical > 0 &&
                                list->_rows[physical] ==
                                list->_rows[physical - 1];
                        --physical)
                {
                        arrowY -= lineHeight;
                }

                const int maximumY =
                        list->getY() +
                        list->getHeight();

                const std::size_t firstLogicalRow =
                        list->_rows[list->_scroll];

                const std::size_t logicalEnd =
                        std::min(
                                list->_texts.size(),
                                firstLogicalRow +
                                        getTextListVisibleRows(
                                                list));

                for (std::size_t row = firstLogicalRow;
                        row < logicalEnd &&
                                arrowY < maximumY;
                        ++row)
                {
                        if (row <
                                        list->_arrowLeft.size() &&
                                row <
                                        list->_arrowRight.size())
                        {
                                ArrowButton *left =
                                        list->_arrowLeft[row];

                                ArrowButton *right =
                                        list->_arrowRight[row];

                                /*
                                 * TextList::blit() normally updates these
                                 * positions. Updating them here as well
                                 * keeps the later synthetic A click aligned
                                 * after D-pad scrolling.
                                 */
                                left->setY(arrowY);
                                right->setY(arrowY);

                                if (arrowY >= list->getY())
                                {
                                        addSurfaceCandidate(left);
                                        addSurfaceCandidate(right);
                                }
                        }

                        if (!list->_texts[row].empty())
                        {
                                arrowY +=
                                        list->_texts[row]
                                                .front()
                                                ->getHeight() +
                                        list->_font
                                                ->getSpacing();
                        }
                        else
                        {
                                arrowY += lineHeight;
                        }
                }
        };

        auto addCandidate = [&](Surface *surface)
        {
                if (!surface ||
                        !surface->_visible ||
                        surface->_hidden ||
                        surface->getWidth() <= 0 ||
                        surface->getHeight() <= 0)
                {
                        return;
                }

                /*
                 * Generic selectable TextList candidates.
                 */
                TextList *genericList =
                        dynamic_cast<TextList *>(surface);

                if (genericList &&
                        genericList->isSelectable())
                {
                        if (!genericList->_font ||
                                genericList->_rows.empty())
                        {
                                return;
                        }

                        const int lineHeight =
                                getTextListLineHeight(
                                        genericList);

                        const size_t visibleRows =
                                getTextListVisibleRows(
                                        genericList);

                        const size_t lineEnd =
                                std::min(
                                        genericList->_rows.size(),
                                        genericList->_scroll +
                                                visibleRows);

                        size_t line =
                                genericList->_scroll;

                        while (line < lineEnd)
                        {
                                const size_t row =
                                        genericList->_rows[line];

                                const size_t firstLine =
                                        line;

                                while (line < lineEnd &&
                                        genericList->
                                                _rows[line] ==
                                                row)
                                {
                                        ++line;
                                }

                                if (!genericList->
                                        isRowSelectable(row))
                                {
                                        continue;
                                }

                                Candidate candidate = {};

                                candidate.left =
                                        genericList->getX();

                                candidate.right =
                                        genericList->getX() +
                                        genericList->getWidth();

                                candidate.top =
                                        genericList->getY() +
                                        static_cast<int>(
                                                firstLine -
                                                genericList->_scroll) *
                                                lineHeight;

                                candidate.bottom =
                                        genericList->getY() +
                                        static_cast<int>(
                                                line -
                                                genericList->_scroll) *
                                                lineHeight;

                                candidate.centerX =
                                        candidate.left +
                                        (candidate.right -
                                                candidate.left) /
                                                2;

                                candidate.centerY =
                                        candidate.top +
                                        (candidate.bottom -
                                                candidate.top) /
                                                2;

                                candidates.push_back(
                                        candidate);
                        }

                        return;
                }

                bool valid = false;

                if (dynamic_cast<ArrowButton *>(surface))
                {
                        valid = true;
                }
                else if (dynamic_cast<ComboBox *>(surface))
                {
                        valid = true;
                }
                else if (Slider *slider =
                        dynamic_cast<Slider *>(surface))
                {
                        if (!slider->_button ||
                                !slider->_button->_visible ||
                                slider->_button->_hidden)
                        {
                                return;
                        }

                        Candidate candidate = {};

                        candidate.left =
                                slider->_button->getX();

                        candidate.top =
                                slider->_button->getY();

                        candidate.right =
                                candidate.left +
                                slider->_button->
                                        getWidth();

                        candidate.bottom =
                                candidate.top +
                                slider->_button->
                                        getHeight();

                        candidate.centerX =
                                candidate.left +
                                slider->_button->
                                        getWidth() /
                                        2;

                        candidate.centerY =
                                candidate.top +
                                slider->_button->
                                        getHeight() /
                                        2;

                        candidates.push_back(
                                candidate);

                        return;
                }
                else
                {
                        InteractiveSurface *interactive =
                                dynamic_cast<TextButton *>(
                                        surface);

                        if (!interactive)
                        {
                                interactive =
                                        dynamic_cast<ImageButton *>(
                                                surface);
                        }

                        /*
                         * Include plain primary-click InteractiveSurface controls,
                         * such as the seven icon tabs on the Graphs screen.
                         */
                        if (!interactive)
                        {
                                interactive =
                                        dynamic_cast<InteractiveSurface *>(
                                                surface);
                        }

                        if (interactive)
                        {
                                valid =
                                        interactive->
                                        isMenuNavigationEnabled() &&
                                        interactive->
                                        isButtonHandled(
                                                SDL_BUTTON_LEFT);
                        }
                }

                if (!valid)
                {
                        return;
                }

                Candidate candidate = {};

                candidate.left = surface->getX();
                candidate.top = surface->getY();

                candidate.right =
                        candidate.left + surface->getWidth();

                candidate.bottom =
                        candidate.top + surface->getHeight();

                candidate.centerX =
                        candidate.left +
                        surface->getWidth() / 2;

                candidate.centerY =
                        candidate.top +
                        surface->getHeight() / 2;

                candidates.push_back(candidate);
        };

        if (state->_modal)
        {
                addTextListArrowCandidates(
                        dynamic_cast<TextList *>(
                                state->_modal));

                addCandidate(state->_modal);
        }
        else
        {
                /*
                 * State::handle processes surfaces in reverse order.
                 * Use the same ordering so uppermost overlapping
                 * controls win ties.
                 */
                for (auto i = state->_surfaces.rbegin();
                        i != state->_surfaces.rend();
                        ++i)
                {
                        addTextListArrowCandidates(
                                dynamic_cast<TextList *>(*i));

                        addCandidate(*i);
                }
        }

        if (candidates.empty())
        {
                return false;
        }

        int originX = logicalCursorX;
        int originY = logicalCursorY;
        bool cursorOnCandidate = false;
        const Candidate *originCandidate = nullptr;

        for (const Candidate &candidate : candidates)
        {
                if (logicalCursorX >= candidate.left &&
                        logicalCursorX < candidate.right &&
                        logicalCursorY >= candidate.top &&
                        logicalCursorY < candidate.bottom)
                {
                        originX = candidate.centerX;
                        originY = candidate.centerY;
                        cursorOnCandidate = true;
                        originCandidate = &candidate;
                        break;
                }
        }

        const Candidate *best = nullptr;
        long bestScore = std::numeric_limits<long>::max();

        for (const Candidate &candidate : candidates)
        {
                const int dx =
                        candidate.centerX - originX;

                const int dy =
                        candidate.centerY - originY;

                int primary = 0;
                int secondary = 0;
                bool valid = false;

                switch (direction)
                {
                        case UP:
                                valid = dy < 0;
                                primary = -dy;
                                secondary = std::abs(dx);
                                break;

                        case DOWN:
                                valid = dy > 0;
                                primary = dy;
                                secondary = std::abs(dx);
                                break;

                        case LEFT:
                                valid = dx < 0;
                                primary = -dx;
                                secondary = std::abs(dy);
                                break;

                        case RIGHT:
                                valid = dx > 0;
                                primary = dx;
                                secondary = std::abs(dy);
                                break;
                }

                if (!valid)
                {
                        continue;
                }

                /*
                 * Prefer controls occupying the same visual row for
                 * Left/Right, or the same visual column for Up/Down.
                 * This prevents a nearby diagonal control from beating
                 * the control directly beside the current one.
                 */
                bool spansOverlap = true;

                if (originCandidate)
                {
                        if (direction == LEFT ||
                                direction == RIGHT)
                        {
                                spansOverlap =
                                        candidate.bottom >
                                                originCandidate->top &&
                                        candidate.top <
                                                originCandidate->bottom;
                        }
                        else
                        {
                                spansOverlap =
                                        candidate.right >
                                                originCandidate->left &&
                                        candidate.left <
                                                originCandidate->right;
                        }
                }

                const long alignmentPenalty =
                        spansOverlap ? 0L : 1000000L;

                const long score =
                        alignmentPenalty +
                        static_cast<long>(primary) *
                                primary +
                        static_cast<long>(secondary) *
                                secondary * 4;

                if (score < bestScore)
                {
                        bestScore = score;
                        best = &candidate;
                }
        }

        /*
         * When starting outside all controls, choose the nearest
         * candidate if none lies in the requested direction.
         */
        if (!best && !cursorOnCandidate)
        {
                for (const Candidate &candidate : candidates)
                {
                        const long dx =
                                candidate.centerX -
                                logicalCursorX;

                        const long dy =
                                candidate.centerY -
                                logicalCursorY;

                        const long score =
                                dx * dx + dy * dy;

                        if (score < bestScore)
                        {
                                bestScore = score;
                                best = &candidate;
                        }
                }
        }

        if (!best)
        {
                return false;
        }

        targetX = leftBand + static_cast<int>(
                std::lround(best->centerX * scaleX));

        targetY = topBand + static_cast<int>(
                std::lround(best->centerY * scaleY));

        return true;
}

bool MenuNavigation3DS::adjustAnalogControlAtCursor(
        Game *game,
        int cursorX,
        int cursorY,
        int circleX,
        int circleY,
        int &targetX,
        int &targetY)
{
        auto resetAnalog =
                []()
        {
                analogControlOwner = nullptr;
                analogControlRemainder = 0;
        };

        if (!game ||
                game->_states.empty() ||
                !game->_screen ||
                !isActive(game))
        {
                resetAnalog();
                return false;
        }

        State *state = game->_states.back();
        Screen *screen = game->_screen;

        const double scaleX =
                screen->getXScale();

        const double scaleY =
                screen->getYScale();

        if (scaleX <= 0.0 || scaleY <= 0.0)
        {
                resetAnalog();
                return false;
        }

        const int leftBand =
                screen->getCursorLeftBlackBand();

        const int topBand =
                screen->getCursorTopBlackBand();

        const int logicalCursorX =
                static_cast<int>(
                        (cursorX - leftBand) /
                        scaleX);

        const int logicalCursorY =
                static_cast<int>(
                        (cursorY - topBand) /
                        scaleY);

        constexpr int analogDeadzone = 20;
        constexpr int analogStepDivisor = 160;

        auto consumeAnalogSteps =
                [&](const void *owner,
                    int axis) -> int
        {
                if (analogControlOwner != owner)
                {
                        analogControlOwner = owner;
                        analogControlRemainder = 0;
                }

                if (std::abs(axis) <=
                        analogDeadzone)
                {
                        analogControlRemainder = 0;
                        return 0;
                }

                const int effectiveAxis =
                        axis > 0 ?
                                axis -
                                        analogDeadzone :
                                axis +
                                        analogDeadzone;

                analogControlRemainder +=
                        effectiveAxis;

                int steps =
                        analogControlRemainder /
                        analogStepDivisor;

                analogControlRemainder -=
                        steps *
                        analogStepDivisor;

                return std::max(
                        -4,
                        std::min(
                                4,
                                steps));
        };

        auto setSliderTarget =
                [&](Slider *slider)
        {
                const int logicalTargetX =
                        slider->_button->getX() +
                        slider->_button->
                                getWidth() /
                                2;

                const int logicalTargetY =
                        slider->_button->getY() +
                        slider->_button->
                                getHeight() /
                                2;

                targetX =
                        leftBand +
                        static_cast<int>(
                                std::lround(
                                        logicalTargetX *
                                        scaleX));

                targetY =
                        topBand +
                        static_cast<int>(
                                std::lround(
                                        logicalTargetY *
                                        scaleY));
        };

        /*
         * Locked slider mode remains active until B. When no slider is
         * locked, placing the cursor on a slider thumb allows direct
         * Circle Pad adjustment without requiring A first.
         */
        if (activeSlider &&
                activeSliderState != state)
        {
                activeSlider = nullptr;
                activeSliderState = nullptr;
        }

        Slider *slider = activeSlider;
        const bool lockedSlider =
                slider != nullptr;

        auto findSliderAtCursor =
                [&](Surface *surface)
        {
                if (slider)
                {
                        return;
                }

                Slider *candidate =
                        dynamic_cast<Slider *>(
                                surface);

                if (!candidate ||
                        !candidate->_visible ||
                        candidate->_hidden ||
                        !candidate->_button ||
                        !candidate->_button->_visible ||
                        candidate->_button->_hidden)
                {
                        return;
                }

                if (logicalCursorX >=
                                candidate->_button->getX() &&
                        logicalCursorX <
                                candidate->_button->getX() +
                                candidate->_button->
                                        getWidth() &&
                        logicalCursorY >=
                                candidate->_button->getY() &&
                        logicalCursorY <
                                candidate->_button->getY() +
                                candidate->_button->
                                        getHeight())
                {
                        slider = candidate;
                }
        };

        if (!slider)
        {
                if (state->_modal)
                {
                        findSliderAtCursor(
                                state->_modal);
                }
                else
                {
                        for (auto i =
                                        state->_surfaces.rbegin();
                                i !=
                                        state->_surfaces.rend();
                                ++i)
                        {
                                findSliderAtCursor(*i);

                                if (slider)
                                {
                                        break;
                                }
                        }
                }
        }

        if (slider)
        {
                const bool horizontalIntent =
                        std::abs(circleX) >
                                analogDeadzone &&
                        std::abs(circleX) >=
                                std::abs(circleY);

                if (!lockedSlider &&
                        !horizontalIntent)
                {
                        resetAnalog();
                        return false;
                }

                const int steps =
                        consumeAnalogSteps(
                                slider,
                                circleX);

                const int oldValue =
                        slider->_value;

                if (steps != 0)
                {
                        const int rangeDirection =
                                slider->_max >=
                                        slider->_min ?
                                        1 : -1;

                        slider->setValue(
                                oldValue +
                                steps *
                                        rangeDirection);
                }

                setSliderTarget(slider);

                if (slider->_value != oldValue &&
                        slider->_change)
                {
                        SDL_Event event = {};

                        event.type =
                                SDL_MOUSEMOTION;

                        event.motion.type =
                                SDL_MOUSEMOTION;

                        event.motion.x =
                                static_cast<Uint16>(
                                        targetX);

                        event.motion.y =
                                static_cast<Uint16>(
                                        targetY);

                        Action action(
                                &event,
                                scaleX,
                                scaleY,
                                topBand,
                                leftBand);

                        action.setSender(slider);

                        (state->*
                                slider->_change)(
                                        &action);
                }

                return true;
        }

        TextList *scrollList = nullptr;

        auto findScrollListAtCursor =
                [&](TextList *list)
        {
                if (scrollList || !list)
                {
                        return;
                }

                int thumbLeft = 0;
                int thumbTop = 0;
                int thumbRight = 0;
                int thumbBottom = 0;

                if (!getTextListScrollBarThumb(
                        list,
                        thumbLeft,
                        thumbTop,
                        thumbRight,
                        thumbBottom))
                {
                        return;
                }

                if (logicalCursorX >=
                                thumbLeft &&
                        logicalCursorX <
                                thumbRight &&
                        logicalCursorY >=
                                thumbTop &&
                        logicalCursorY <
                                thumbBottom)
                {
                        scrollList = list;
                }
        };

        ComboBox *combo =
                dynamic_cast<ComboBox *>(
                        state->_modal);

        if (combo &&
                combo->_list &&
                combo->_list->getVisible())
        {
                findScrollListAtCursor(
                        combo->_list);
        }
        else if (state->_modal)
        {
                findScrollListAtCursor(
                        dynamic_cast<TextList *>(
                                state->_modal));
        }
        else
        {
                for (auto i =
                                state->_surfaces.rbegin();
                        i !=
                                state->_surfaces.rend();
                        ++i)
                {
                        findScrollListAtCursor(
                                dynamic_cast<TextList *>(
                                        *i));

                        if (scrollList)
                        {
                                break;
                        }
                }
        }

        const bool verticalIntent =
                scrollList &&
                std::abs(circleY) >
                        analogDeadzone &&
                std::abs(circleY) >=
                        std::abs(circleX);

        if (!verticalIntent)
        {
                resetAnalog();
                return false;
        }

        /*
         * 3DS Circle Pad positive Y points upward. Convert it so
         * positive steps mean scrolling downward through the list.
         */
        const int steps =
                consumeAnalogSteps(
                        scrollList->_scrollbar,
                        -circleY);

        if (steps > 0)
        {
                scrollList->scrollDown(
                        false,
                        false,
                        static_cast<size_t>(
                                steps));
        }
        else if (steps < 0)
        {
                scrollList->scrollUp(
                        false,
                        false,
                        static_cast<size_t>(
                                -steps));
        }

        return getTextListScrollBarTarget(
                scrollList,
                screen,
                targetX,
                targetY);
}


bool MenuNavigation3DS::activateSliderAtCursor(
        Game *game,
        int cursorX,
        int cursorY,
        int &targetX,
        int &targetY)
{
        if (!game || game->_states.empty())
        {
                return false;
        }

        State *state = game->_states.back();
        Screen *screen = game->_screen;

        if (!screen)
        {
                return false;
        }

        const double scaleX = screen->getXScale();
        const double scaleY = screen->getYScale();

        if (scaleX <= 0.0 || scaleY <= 0.0)
        {
                return false;
        }

        const int leftBand =
                screen->getCursorLeftBlackBand();

        const int topBand =
                screen->getCursorTopBlackBand();

        const int logicalCursorX = static_cast<int>(
                (cursorX - leftBand) / scaleX);

        const int logicalCursorY = static_cast<int>(
                (cursorY - topBand) / scaleY);

        Slider *slider = nullptr;

        auto findSlider = [&](Surface *surface)
        {
                Slider *candidate =
                        dynamic_cast<Slider *>(surface);

                if (!candidate ||
                        !surface->_visible ||
                        surface->_hidden)
                {
                        return;
                }

                if (logicalCursorX >= candidate->getX() &&
                        logicalCursorX <
                                candidate->getX() +
                                candidate->getWidth() &&
                        logicalCursorY >= candidate->getY() &&
                        logicalCursorY <
                                candidate->getY() +
                                candidate->getHeight())
                {
                        slider = candidate;
                }
        };

        if (state->_modal)
        {
                findSlider(state->_modal);
        }
        else
        {
                for (auto i = state->_surfaces.rbegin();
                        i != state->_surfaces.rend();
                        ++i)
                {
                        findSlider(*i);

                        if (slider)
                        {
                                break;
                        }
                }
        }

        if (!slider)
        {
                return false;
        }

        activeSlider = slider;
        activeSliderState = state;

        analogControlOwner = nullptr;
        analogControlRemainder = 0;

        const int logicalTargetX =
                slider->_button->getX() +
                slider->_button->getWidth() / 2;

        const int logicalTargetY =
                slider->_button->getY() +
                slider->_button->getHeight() / 2;

        targetX = leftBand + static_cast<int>(
                std::lround(logicalTargetX * scaleX));

        targetY = topBand + static_cast<int>(
                std::lround(logicalTargetY * scaleY));

        return true;
}

bool MenuNavigation3DS::leaveSliderMode()
{
        if (!activeSlider)
        {
                return false;
        }

        activeSlider = nullptr;
        activeSliderState = nullptr;

        analogControlOwner = nullptr;
        analogControlRemainder = 0;

        return true;
}

bool MenuNavigation3DS::isAdjustingSlider()
{
        return activeSlider != nullptr;
}

bool MenuNavigation3DS::closeOpenComboBox(Game *game)
{
        if (!game || game->_states.empty())
        {
                return false;
        }

        State *state = game->_states.back();

        ComboBox *combo =
                dynamic_cast<ComboBox *>(state->_modal);

        if (!combo ||
                !combo->_list ||
                !combo->_list->getVisible())
        {
                return false;
        }

        combo->toggle(false, false);

        trackedOpenCombo = nullptr;
        trackedComboRow = 0;

        return true;
}

}
