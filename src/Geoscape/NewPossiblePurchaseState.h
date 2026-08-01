#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "../Engine/State.h"

namespace OpenXcom
{

class Game;
class Window;
class TextButton;
class Text;
class Base;
class TextList;
class RuleItem;

/**
 * Window which inform the player of new possible items to buy.
 * Also allows to go to the PurchaseState to order some new items.
 */
class NewPossiblePurchaseState : public State
{
private:
	Window *_window;
	Text *_txtTitle;
	TextList * _lstPossibilities;
	Text* _txtCaveat;
	TextButton *_btnPurchase, *_btnOk;
	Base * _base;
public:
	/// Creates the NewPossiblePurchaseState state.
	NewPossiblePurchaseState(Base * base, const std::vector<RuleItem *> & possibilities);
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Purchase button.
	void btnPurchaseClick(Action *action);
};

}
