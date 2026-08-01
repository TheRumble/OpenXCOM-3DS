#include "SoldierAvatar.h"

namespace OpenXcom
{

/**
 * Initializes a new empty soldier avatar.
 */
SoldierAvatar::SoldierAvatar() : _gender(GENDER_MALE), _look(LOOK_BLONDE), _lookVariant(0)
{
}

/**
 * Initializes a new soldier avatar.
 * @param avatarName ID of the avatar (for translation).
 * @param gender Soldier's gender.
 * @param look Soldier's look.
 * @param lookVariant Soldier's look variant.
 */
SoldierAvatar::SoldierAvatar(const std::string &avatarName, SoldierGender gender, SoldierLook look, int lookVariant)
	: _avatarName(avatarName), _gender(gender), _look(look), _lookVariant(lookVariant)
{
}

/**
 *
 */
SoldierAvatar::~SoldierAvatar()
{
}

/**
 * Returns the avatar's name.
 * @return Code for translation
 */
std::string SoldierAvatar::getAvatarName() const
{
	return _avatarName;
}

/**
 * Returns the avatar's first component (gender).
 * @return Gender
 */
SoldierGender SoldierAvatar::getGender() const
{
	return _gender;
}

/**
 * Returns the avatar's second component (look).
 * @return Look
 */
SoldierLook SoldierAvatar::getLook() const
{
	return _look;
}

/**
 * Returns the avatar's third component (lookVariant).
 * @return Look variant
 */
int SoldierAvatar::getLookVariant() const
{
	return _lookVariant;
}

}
