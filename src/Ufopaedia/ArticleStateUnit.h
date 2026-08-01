#pragma once
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
#include "ArticleState.h"

namespace OpenXcom
{
	class Game;
	class Text;
	class TextList;
	class ArticleDefinitionUnit;

	/**
	 * ArticleStateUnit has a caption, text, background image, an armor block and a stats block.
	 * The layout of the description text, armor block and stats block can vary,
	 * depending on the background image.
	 */

	class ArticleStateUnit : public ArticleState
	{
	public:
		ArticleStateUnit(ArticleDefinitionUnit *article_defs, std::shared_ptr<ArticleCommonState> state);
		virtual ~ArticleStateUnit();

	protected:
		Text *_txtTitle;
		Text *_txtInfo;
		Text *_txtDifficulty;
		TextList *_lstStats;
		TextList *_lstArmor;
	};
}
