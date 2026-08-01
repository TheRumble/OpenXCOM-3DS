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
#include "BattleState.h"
#include "../Mod/RuleItem.h"

namespace OpenXcom
{

class BattlescapeGame;
class BattleUnit;

/* Refactoring tip : UnitDieBState */
/**
 * State for dying units.
 */
class UnitDieBState : public BattleState
{
private:
	BattleUnit *_unit;
	const RuleDamageType *_damageType;
	bool _noSound;
	int _extraFrame;
	bool _overKill;
public:
	/// Creates a new UnitDieBState class
	UnitDieBState(BattlescapeGame *parent, BattleUnit *unit, const RuleDamageType *damageType, bool noSound);
	/// Cleans up the UnitDieBState.
	~UnitDieBState();
	/// Initializes the state.
	void init() override;
	/// Handles a cancels request.
	void cancel() override;
	/// Runs state functionality every cycle.
	void think() override;
	/// Converts a unit to a corpse.
	void convertUnitToCorpse();
	/// Plays the death sound.
	void playDeathSound();
};

}
