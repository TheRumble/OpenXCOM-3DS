#pragma once
/*
 * Copyright 2010-2015 OpenXcom Developers.
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
#include <vector>
#include "../Engine/State.h"

namespace OpenXcom
{

class Base;
class Soldier;
class TextButton;
class Window;
class Text;
class TextList;

/**
 * Select Pilot window that allows assigning a pilot to a craft.
 */
class CraftPilotSelectState : public State
{
private:
	Base *_base;
	size_t _craft;

	TextButton *_btnCancel;
	Window *_window;
	Text *_txtTitle, *_txtName, *_txtFiringAcc, *_txtReactions, *_txtBravery;
	TextList *_lstPilot;
	std::vector<int> _pilot;
public:
	/// Creates the Select Pilot state.
	CraftPilotSelectState(Base *base, size_t craft);
	/// Cleans up the Select Pilot state.
	~CraftPilotSelectState();
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
	/// Handler for clicking the Pilot list.
	void lstPilotClick(Action *action);
};

}
