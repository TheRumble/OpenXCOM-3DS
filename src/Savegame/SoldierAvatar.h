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
#include "Soldier.h"

namespace OpenXcom
{

/**
 * Represents a soldier's avatar/paperdoll.
 * Avatar is a combination of soldier's gender, look and look variant
 */
class SoldierAvatar
{
private:
	std::string _avatarName;
	SoldierGender _gender;
	SoldierLook _look;
	int _lookVariant;
public:
	/// Creates a new empty avatar.
	SoldierAvatar();
	/// Creates a new avatar.
	SoldierAvatar(const std::string &avatarName, SoldierGender gender, SoldierLook look, int lookVariant);
	/// Cleans up the avatar.
	~SoldierAvatar();
	/// returns avatar name
	std::string getAvatarName() const;
	/// returns gender
	SoldierGender getGender() const;
	/// returns look
	SoldierLook getLook() const;
	/// returns look variant
	int getLookVariant() const;
};

}
