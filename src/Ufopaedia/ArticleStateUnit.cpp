/*
 * Copyright 2010-2025 OpenXcom Developers.
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

#include <sstream>
#include "ArticleStateUnit.h"
#include "../Mod/ArticleDefinition.h"
#include "../Mod/Mod.h"
#include "../Mod/Armor.h"
#include "../Mod/Unit.h"
#include "../Engine/Game.h"
#include "../Engine/Palette.h"
#include "../Engine/Surface.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Unicode.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Mod/RuleInterface.h"
#include "../fmath.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

	ArticleStateUnit::ArticleStateUnit(ArticleDefinitionUnit *defs, std::shared_ptr<ArticleCommonState> state) : ArticleState(defs->id, std::move(state))
	{
		Unit *unit = _game->getMod()->getUnit(defs->id, true);

		// add screen elements
		_txtTitle = new Text(310, 17, 5, 23);

		// Set palette
		if (defs->customPalette)
		{
			setCustomPalette(_game->getMod()->getSurface(defs->image_id)->getPalette(), Mod::UFOPAEDIA_CURSOR);
		}
		else
		{
			setStandardPalette("PAL_UFOPAEDIA");
		}

		RuleInterface* itf = _game->getMod()->getInterface("articleUnit");
		int buttonColor = itf->getElement("button")->color;
		int titleColor1 = itf->getElement("title")->color;
		int titleColor2 = itf->getElement("title")->color2;
		int textColor1 = itf->getElement("text")->color;
		int textColor2 = itf->getElement("text")->color2;
		int listColor1 = itf->getElement("list")->color;
		int listColor2 = itf->getElement("list")->color2;

		ArticleState::initLayout();

		// add other elements
		add(_txtTitle);

		// Set up objects
		_game->getMod()->getSurface(defs->image_id)->blitNShade(_bg, 0, 0);
		_btnOk->setColor(buttonColor);
		_btnPrev->setColor(buttonColor);
		_btnNext->setColor(buttonColor);
		_btnInfo->setColor(buttonColor);
		_btnInfo->setVisible(_game->getMod()->getShowPediaInfoButton());

		_txtTitle->setColor(titleColor1);
		_txtTitle->setSecondaryColor(titleColor2);
		_txtTitle->setBig();
		_txtTitle->setWordWrap(true);
		_txtTitle->setText(tr(defs->getTitleForPage(_state->current_page)));

		_txtInfo = new Text(defs->rect_text.width, defs->rect_text.height, defs->rect_text.x, defs->rect_text.y);
		add(_txtInfo);

		_txtInfo->setColor(textColor1);
		_txtInfo->setSecondaryColor(textColor2);
		_txtInfo->setWordWrap(true);
		_txtInfo->setScrollable(true);
		_txtInfo->setText(tr(defs->getTextForPage(_state->current_page)));

		int widthStats = Clamp(defs->rect_stats.width, 60, 320);
		_lstStats = new TextList(widthStats, defs->rect_stats.height, defs->rect_stats.x, defs->rect_stats.y);
		add(_lstStats);

		_lstStats->setColor(listColor1);
		_lstStats->setSecondaryColor(listColor2);
		_lstStats->setColumns(3, widthStats - 40, 20, 20);
		_lstStats->setDot(true);

		int widthArmor = Clamp(defs->rect_armor.width, 60, 320);
		_lstArmor = new TextList(widthArmor, defs->rect_armor.height, defs->rect_armor.x, defs->rect_armor.y);
		add(_lstArmor);

		_lstArmor->setColor(listColor1);
		_lstArmor->setSecondaryColor(listColor2);
		_lstArmor->setColumns(3, widthArmor - 40, 20, 20);
		_lstArmor->setDot(true);
		if (defs->rect_armor.height == 0)
		{
			_lstArmor->setVisible(false);
		}

		_txtDifficulty = new Text(160, 9, defs->rect_stats.x + widthStats - (defs->unit_mode == 2 ? 20 : 40), defs->rect_stats.y - 10);
		add(_txtDifficulty);

		_txtDifficulty->setColor(textColor1);
		_txtDifficulty->setSecondaryColor(textColor2);

		UnitStats civ = *unit->getStats();
		UnitStats alien = civ;

		int civArmor[SIDE_MAX];
		civArmor[SIDE_FRONT] = unit->getArmor()->getFrontArmor();
		civArmor[SIDE_LEFT] = unit->getArmor()->getLeftSideArmor();
		civArmor[SIDE_RIGHT] = unit->getArmor()->getRightSideArmor();
		civArmor[SIDE_REAR] = unit->getArmor()->getRearArmor();
		civArmor[SIDE_UNDER] = unit->getArmor()->getUnderArmor();

		int alienArmor[SIDE_MAX];
		alienArmor[SIDE_FRONT] = civArmor[SIDE_FRONT];
		alienArmor[SIDE_LEFT] = civArmor[SIDE_LEFT];
		alienArmor[SIDE_RIGHT] = civArmor[SIDE_RIGHT];
		alienArmor[SIDE_REAR] = civArmor[SIDE_REAR];
		alienArmor[SIDE_UNDER] = civArmor[SIDE_UNDER];

		if (_game->getSavedGame())
		{
			if (defs->unit_mode != 1)
			{
				std::string diff;
				switch (_game->getSavedGame()->getDifficulty())
				{
				case DIFF_SUPERHUMAN: diff = tr("STR_5_SUPERHUMAN"); break;
				case DIFF_GENIUS: diff = tr("STR_4_GENIUS"); break;
				case DIFF_VETERAN: diff = tr("STR_3_VETERAN"); break;
				case DIFF_EXPERIENCED: diff = tr("STR_2_EXPERIENCED"); break;
				default: diff = tr("STR_1_BEGINNER"); break;
				}
				_txtDifficulty->setText(diff);
			}

			// FACTION_HOSTILE scales with difficulty
			auto* adjustment = _game->getMod()->getStatAdjustment(_game->getSavedGame()->getDifficulty());

			alien += UnitStats::percent(alien, adjustment->statGrowth, adjustment->growthMultiplier);

			alien.firing *= adjustment->aimMultiplier;
			alien += adjustment->statGrowthAbs;

			for (int i = 0; i < SIDE_MAX; ++i)
			{
				alienArmor[i] *= adjustment->armorMultiplier;
				alienArmor[i] += adjustment->armorMultiplierAbs;
			}
		}

		// defs->unit_mode == 0  show as hostile (incl. difficulty)
		// defs->unit_mode == 1  show as neutral/allied
		// defs->unit_mode == 2  show as neutral/allied first, then also as hostile (incl. difficulty)
		int columns = defs->unit_mode > 1 ? 3 : 2;

		if (defs->unit_mode == 0)
		{
			civ = alien;

			for (int i = 0; i < SIDE_MAX; ++i)
			{
				civArmor[i] = alienArmor[i];
			}
		}

		_lstStats->addRow(columns, tr("STR_TIME_UNITS").c_str(), std::to_string(civ.tu).c_str(), std::to_string(alien.tu).c_str());
		_lstStats->addRow(columns, tr("STR_STAMINA").c_str(), std::to_string(civ.stamina).c_str(), std::to_string(alien.stamina).c_str());
		_lstStats->addRow(columns, tr("STR_HEALTH").c_str(), std::to_string(civ.health).c_str(), std::to_string(alien.health).c_str());
		_lstStats->addRow(columns, tr("STR_BRAVERY").c_str(), std::to_string(civ.bravery).c_str(), std::to_string(alien.bravery).c_str());
		_lstStats->addRow(columns, tr("STR_REACTIONS").c_str(), std::to_string(civ.reactions).c_str(), std::to_string(alien.reactions).c_str());
		_lstStats->addRow(columns, tr("STR_FIRING_ACCURACY").c_str(), std::to_string(civ.firing).c_str(), std::to_string(alien.firing).c_str());
		_lstStats->addRow(columns, tr("STR_THROWING_ACCURACY").c_str(), std::to_string(civ.throwing).c_str(), std::to_string(alien.throwing).c_str());
		_lstStats->addRow(columns, tr("STR_MELEE_ACCURACY").c_str(), std::to_string(civ.melee).c_str(), std::to_string(alien.melee).c_str());
		_lstStats->addRow(columns, tr("STR_STRENGTH").c_str(), std::to_string(civ.strength).c_str(), std::to_string(alien.strength).c_str());
		if (_game->getMod()->isManaFeatureEnabled())
		{
			_lstStats->addRow(columns, tr("STR_MANA_POOL").c_str(), std::to_string(civ.mana).c_str(), std::to_string(alien.mana).c_str());
		}
		_lstStats->addRow(columns, tr("STR_PSIONIC_STRENGTH").c_str(), std::to_string(civ.psiStrength).c_str(), std::to_string(alien.psiStrength).c_str());
		_lstStats->addRow(columns, tr("STR_PSIONIC_SKILL").c_str(), std::to_string(civ.psiSkill).c_str(), std::to_string(alien.psiSkill).c_str());

		_lstArmor->addRow(columns, tr("STR_FRONT_ARMOR").c_str(), std::to_string(civArmor[SIDE_FRONT]).c_str(), std::to_string(alienArmor[SIDE_FRONT]).c_str());
		_lstArmor->addRow(columns, tr("STR_LEFT_ARMOR").c_str(), std::to_string(civArmor[SIDE_LEFT]).c_str(), std::to_string(alienArmor[SIDE_LEFT]).c_str());
		_lstArmor->addRow(columns, tr("STR_RIGHT_ARMOR").c_str(), std::to_string(civArmor[SIDE_RIGHT]).c_str(), std::to_string(alienArmor[SIDE_RIGHT]).c_str());
		_lstArmor->addRow(columns, tr("STR_REAR_ARMOR").c_str(), std::to_string(civArmor[SIDE_REAR]).c_str(), std::to_string(alienArmor[SIDE_REAR]).c_str());
		_lstArmor->addRow(columns, tr("STR_UNDER_ARMOR").c_str(), std::to_string(civArmor[SIDE_UNDER]).c_str(), std::to_string(alienArmor[SIDE_UNDER]).c_str());

		centerAllSurfaces();
	}

	ArticleStateUnit::~ArticleStateUnit()
	{}

}
