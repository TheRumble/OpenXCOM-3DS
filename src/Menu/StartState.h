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
#include <string>
#include <sstream>
#ifdef NINTENDO_3DS
#include <atomic>
#endif

namespace OpenXcom
{

class Text;
class Surface;
class Font;
class Timer;
class Language;

enum LoadingPhase { LOADING_STARTED, LOADING_FAILED, LOADING_SUCCESSFUL, LOADING_DONE };

#ifdef NINTENDO_3DS
enum StartupLoadStage
{
	STARTUP_PREPARING,
	STARTUP_SCANNING_RESOURCES,
	STARTUP_FINDING_DATA_FOLDER,
	STARTUP_INDEXING_COMMON,
	STARTUP_OPENING_STANDARD_ARCHIVE,
	STARTUP_SCANNING_EMBEDDED_STANDARD,
	STARTUP_SCANNING_STANDARD_DATA,
	STARTUP_SCANNING_USER_MODS,
	STARTUP_CHECKING_DEPENDENCIES,
	STARTUP_CHECKING_MASTER_CHAINS,
	STARTUP_MAPPING_EXTERNAL_RESOURCES,
	STARTUP_RECONCILING_MODS,
	STARTUP_VALIDATING_ACTIVE_CONTENT,
	STARTUP_BUILDING_RESOURCE_MAP,
	STARTUP_FINALIZING_RESOURCE_SCAN,
	STARTUP_LOADING_GAME_DATA,
	STARTUP_PREPARING_RULESETS,
	STARTUP_PRELOADING_RESOURCE_CONFIG,
	STARTUP_LOADING_VANILLA_RESOURCES,
	STARTUP_LOADING_RULESETS,
	STARTUP_POSTPROCESSING_RULES,
	STARTUP_LOADING_EXTRA_RESOURCES,
	STARTUP_LINKING_RULES,
	STARTUP_SORTING_RULES,
	STARTUP_LOADING_MOD_RESOURCES,
	STARTUP_LOADING_LANGUAGE,
	STARTUP_FINALIZING,
	STARTUP_COMPLETE,
	STARTUP_FAILED
};
#endif

/**
 * Initializes the game and loads all required content.
 */
class StartState : public State
{
private:
	Text *_text, *_cursor;
#ifdef NINTENDO_3DS
	Text *_progressText;
	Surface *_progressBar;
	int _lastStartupStage;
	int _lastStartupProgress;
#endif
	Font *_font;
	Timer *_timer;
	Language *_lang;
	int _anim;
	std::string _oldMaster;

	SDL_Thread *_thread;
	std::ostringstream _output;

#ifdef NINTENDO_3DS
	void updateStartupProgress();
	static const char *getStartupStageLabel(
		int stage);
#endif
public:
	static LoadingPhase loading;
	static std::string error;
#ifdef NINTENDO_3DS
	static std::atomic<int> startupStage;
	static std::atomic<int> startupProgress;
	static void setStartupProgress(
		StartupLoadStage stage,
		int progress);
	static void setStartupWorkProgress(
		StartupLoadStage stage,
		int rangeStart,
		int rangeEnd,
		unsigned int completed,
		unsigned int total);
#endif

	/// Creates the Start state.
	StartState();
	/// Cleans up the Start state.
	~StartState();
	/// Reset everything.
	void init() override;
	/// Displays messages.
	void think() override;
	/// Handles key clicks.
	void handle(Action *action) override;
	/// Animates the terminal.
	void animate();
	/// Adds a line of text.
	void addLine(const std::string &str);
	/// Loads the game resources.
	static int load(void *game_ptr);
};

}
