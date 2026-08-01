#pragma once
/*
 * Copyright 2010-2021 OpenXcom Developers.
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

class TextButton;
class Window;
class Text;
class BattlescapeState;
class SavedBattleGame;

/**
 * A screen with links to the OXCE battlescape functionality.
 */
class ExtendedBattlescapeLinksState : public State
{
private:
	TextButton *_btnOk;
	TextButton *_btnTouch, *_btnNightVision, *_btnPersonalLights, *_btnBrightness, *_btnTurnDiary, *_btnBriefing, *_btnNotes, *_btnMusic;
	TextButton *_btnKillAll;
	Window *_window;
	Text *_txtTitle;
	BattlescapeState *_parent;
	SavedBattleGame* _save;
public:
	/// Creates the ExtendedBattlescapeLinks state.
	ExtendedBattlescapeLinksState(BattlescapeState* parent, SavedBattleGame* save);
	/// Cleans up the ExtendedBattlescapeLinks state.
	~ExtendedBattlescapeLinksState() = default;
	/// Handlers for clicking the buttons.
	void btnTouchClick(Action* action);
	void btnNightVisionClick(Action* action);
	void btnPersonalLightsClick(Action* action);
	void btnBrightnessClick(Action* action);
	void btnTurnDiaryClick(Action* action);
	void btnBriefingClick(Action* action);
	void btnNotesClick(Action* action);
	void btnMusicClick(Action* action);
	void btnKillAllClick(Action* action);
	void btnOkClick(Action* action);
};

}
