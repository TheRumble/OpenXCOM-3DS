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
#include "../Engine/InteractiveSurface.h"

namespace OpenXcom
{

class Text;
class Font;
class Language;
class Sound;
class ComboBox;

/**
 * Coloured button with a text label.
 * Drawn to look like a 3D-shaped box with text on top,
 * responds to mouse clicks. Can be attached to a group of
 * buttons to turn it into a radio button (only one button
 * pushed at a time).
 */
class TextButton : public InteractiveSurface
{
private:
	Uint8 _color;
	Text *_text;
	TextButton **_group;
	bool _contrast, _geoscapeButton;
	ComboBox *_comboBox;
	// for use by RuleInterface
	void setSecondaryColor(Uint8 color)  override { setTextColor(color); }
protected:
	bool isButtonHandled(Uint8 button = 0) override;
public:
	static Sound *soundPress;
	/// Creates a new text button with the specified size and position.
	TextButton(int width, int height, int x = 0, int y = 0);
	/// Cleans up the text button.
	~TextButton();
	/// Sets the text button's color.
	void setColor(Uint8 color) override;
	/// Gets the text button's color.
	Uint8 getColor() const;
	/// Sets the text button's text color.
	void setTextColor(Uint8 color);
	/// Sets the text size to big.
	void setBig();
	/// Sets the text size to small.
	void setSmall();
	/// Gets the text button's current font.
	Font *getFont() const;
	/// Initializes the text edit's resources.
	void initText(Font *big, Font *small, Language *lang) override;
	/// Sets the text button's high contrast color setting.
	void setHighContrast(bool contrast) override;
	/// Sets the text button's text.
	void setText(const std::string &text);
	/// Gets the text button's text.
	std::string getText() const;
	/// Sets the text button's group.
	void setGroup(TextButton **group);
	/// Sets the text button's palette.
	void setPalette(const SDL_Color *colors, int firstcolor = 0, int ncolors = 256) override;
	/// Draws the text button.
	void draw() override;
	/// Special handling for mouse presses.
	void mousePress(Action *action, State *state) override;
	/// Special handling for mouse releases.
	void mouseRelease(Action *action, State *state) override;
	/// Attaches this button to a combobox.
	void setComboBox(ComboBox *comboBox);
	void setWidth(int width) override;
	void setHeight(int height) override;
	void setGeoscapeButton(bool geo);
};

}
