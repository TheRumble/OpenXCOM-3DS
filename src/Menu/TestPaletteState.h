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

enum PaletteActionType { PAT_PREVIEW, PAT_TINY_BORDERLESS, PAT_TINY_BORDER, PAT_SMALL_LOW, PAT_SMALL_HIGH, PAT_BIG_LOW, PAT_BIG_HIGH };

class Surface;
class TextButton;

/**
 * A state for preview of text color code per palette and contrast.
 */
class TestPaletteState : public State
{
private:
	Surface *_bg;
	TextButton *_btnCancel;
public:
	/// Creates the Test state.
	TestPaletteState(const std::string &palette, PaletteActionType action);
	/// Cleans up the Test state.
	~TestPaletteState();
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
};

}
