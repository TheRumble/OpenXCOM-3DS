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
#include "ArticleStateSoldier.h"
#include "../Mod/ArticleDefinition.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleSoldier.h"
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

namespace OpenXcom
{

	ArticleStateSoldier::ArticleStateSoldier(ArticleDefinitionSoldier *defs, std::shared_ptr<ArticleCommonState> state) : ArticleState(defs->id, std::move(state))
	{
		const RuleSoldier *soldier = _game->getMod()->getSoldier(defs->id, true);

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

		RuleInterface* itf = _game->getMod()->getInterface("articleSoldier");
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

		int width = Clamp(defs->rect_stats.width, 80, 320);
		_lstStats = new TextList(width, defs->rect_stats.height, defs->rect_stats.x, defs->rect_stats.y);
		add(_lstStats);

		_lstStats->setColor(listColor1);
		_lstStats->setSecondaryColor(listColor2);
		_lstStats->setColumns(4, width - 60, 20, 20, 20);
		_lstStats->setDot(true);

		auto min = soldier->getMinStats();
		auto max = soldier->getMaxStats();
		auto cap = soldier->getStatCaps();
		_lstStats->addRow(4, tr("STR_TIME_UNITS").c_str(), std::to_string(min.tu).c_str(), std::to_string(max.tu).c_str(), std::to_string(cap.tu).c_str());
		_lstStats->addRow(4, tr("STR_STAMINA").c_str(), std::to_string(min.stamina).c_str(), std::to_string(max.stamina).c_str(), std::to_string(cap.stamina).c_str());
		_lstStats->addRow(4, tr("STR_HEALTH").c_str(), std::to_string(min.health).c_str(), std::to_string(max.health).c_str(), std::to_string(cap.health).c_str());
		_lstStats->addRow(4, tr("STR_BRAVERY").c_str(), std::to_string(min.bravery).c_str(), std::to_string(max.bravery).c_str(), std::to_string(cap.bravery).c_str());
		_lstStats->addRow(4, tr("STR_REACTIONS").c_str(), std::to_string(min.reactions).c_str(), std::to_string(max.reactions).c_str(), std::to_string(cap.reactions).c_str());
		_lstStats->addRow(4, tr("STR_FIRING_ACCURACY").c_str(), std::to_string(min.firing).c_str(), std::to_string(max.firing).c_str(), std::to_string(cap.firing).c_str());
		_lstStats->addRow(4, tr("STR_THROWING_ACCURACY").c_str(), std::to_string(min.throwing).c_str(), std::to_string(max.throwing).c_str(), std::to_string(cap.throwing).c_str());
		_lstStats->addRow(4, tr("STR_MELEE_ACCURACY").c_str(), std::to_string(min.melee).c_str(), std::to_string(max.melee).c_str(), std::to_string(cap.melee).c_str());
		_lstStats->addRow(4, tr("STR_STRENGTH").c_str(), std::to_string(min.strength).c_str(), std::to_string(max.strength).c_str(), std::to_string(cap.strength).c_str());
		if (_game->getMod()->isManaFeatureEnabled())
		{
			_lstStats->addRow(4, tr("STR_MANA_POOL").c_str(), std::to_string(min.mana).c_str(), std::to_string(max.mana).c_str(), std::to_string(cap.mana).c_str());
		}
		_lstStats->addRow(4, tr("STR_PSIONIC_STRENGTH").c_str(), std::to_string(min.psiStrength).c_str(), std::to_string(max.psiStrength).c_str(), std::to_string(cap.psiStrength).c_str());
		if (defs->psi_skill_mode == 0)
		{
			// not shown
			_lstStats->addRow(4, tr("STR_PSIONIC_SKILL").c_str(), "", "", std::to_string(cap.psiSkill).c_str());
		}
		else if (defs->psi_skill_mode == 1)
		{
			// shown processed
			_lstStats->addRow(4, tr("STR_PSIONIC_SKILL").c_str(), std::to_string(max.psiSkill).c_str(), std::to_string(max.psiSkill*3/2).c_str(), std::to_string(cap.psiSkill).c_str());
		}
		else
		{
			// shown raw
			_lstStats->addRow(4, tr("STR_PSIONIC_SKILL").c_str(), std::to_string(min.psiSkill).c_str(), std::to_string(max.psiSkill).c_str(), std::to_string(cap.psiSkill).c_str());
		}

		centerAllSurfaces();
	}

	ArticleStateSoldier::~ArticleStateSoldier()
	{}

}
