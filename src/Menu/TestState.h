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
#include <map>
#include <set>

namespace OpenXcom
{

class Window;
class TextButton;
class Text;
class ComboBox;
class TextList;
class MapBlock;
class RuleTerrain;
class Palette;

struct PaletteTestMetadata {
	std::string paletteName;
	int firstIndexToCheck;
	int lastIndexToCheck;
	int maxTolerance;
	std::string palettePath;
	bool usesBackPals;

	PaletteTestMetadata() : firstIndexToCheck(0), lastIndexToCheck(255), maxTolerance(0), usesBackPals(false) { };
	PaletteTestMetadata(const std::string &_paletteName, int _firstIndexToCheck, int _lastIndexToCheck, int _maxTolerance, const std::string &_palettePath, bool _usesBackPals) :
		paletteName(_paletteName), firstIndexToCheck(_firstIndexToCheck), lastIndexToCheck(_lastIndexToCheck), maxTolerance(_maxTolerance), palettePath(_palettePath), usesBackPals(_usesBackPals)
	{
		// empty
	};
};

/**
 * A state for testing most common modding mistakes.
 */
class TestState : public State
{
private:
	Window *_window;
	TextButton *_btnRun, *_btnCancel;
	Text *_txtPalette;
	Text *_txtTitle, *_txtTestCase, *_txtDescription;
	ComboBox *_cbxPalette;
	ComboBox *_cbxPaletteAction;
	ComboBox *_cbxTestCase;
	TextList *_lstOutput;
	std::vector<std::string> _paletteList;
	std::map<int, Palette*> _vanillaPalettes;
	std::vector<std::string> _testCases;
	/// Test cases.
	void testCase4();
	void testCase3();
	void testCase2();
	int checkPalette(const std::string& fullPath, int width, int height);
	int matchPalette(Surface *image, int index, Palette *test);
	void testCase1();
	int checkMCD(RuleTerrain *terrainRule, std::map<std::string, std::set<int>> &uniqueResults);
	void testCase0();
	int checkRMP(MapBlock *mapblock);
	int loadMAP(MapBlock *mapblock);
public:
	/// Creates the Test state.
	TestState();
	/// Cleans up the Test state.
	~TestState();
	/// Handler for changing the Test Case combobox.
	void cbxTestCaseChange(Action *action);
	/// Handler for clicking the Run button.
	void btnRunClick(Action *action);
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
	/// Handler for selecting the Palette Action from the combobox.
	void cbxPaletteAction(Action *action);
};

}
