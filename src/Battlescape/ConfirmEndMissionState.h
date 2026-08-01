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
#include "../Engine/State.h"

namespace OpenXcom
{

class Window;
class Text;
class TextButton;
class SavedBattleGame;
class BattlescapeGame;

/**
 * Screen which asks for confirmation to end the mission.
 * Note: only if auto-end battle option is enabled and if there are still soldiers with fatal wounds
 */
class ConfirmEndMissionState : public State
{
private:
	Window *_window;
	Text *_txtTitle, *_txtWounded, *_txtConfirm;
	TextButton *_btnOk, *_btnCancel;
	SavedBattleGame *_battleGame;
	int _wounded;
	BattlescapeGame *_parent;
public:
	/// Creates the ConfirmEndMission state.
	ConfirmEndMissionState(SavedBattleGame *battleGame, int wounded, BattlescapeGame *parent);
	/// Cleans up the ConfirmEndMission state.
	~ConfirmEndMissionState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);

};

}
