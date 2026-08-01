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
#include "StartState.h"
#include "../version.h"
#include "../Engine/Logger.h"
#include "../Engine/Game.h"
#include "../Engine/Screen.h"
#include "../Engine/Action.h"
#include "../Engine/Surface.h"
#include "../Engine/Options.h"
#include "../Engine/Language.h"
#include "../Engine/Sound.h"
#include "../Engine/Music.h"
#include "../Engine/Font.h"
#include "../Engine/Timer.h"
#include "../Engine/CrossPlatform.h"
#include "../Interface/FpsCounter.h"
#include "../Interface/Cursor.h"
#include "../Interface/Text.h"
#include "MainMenuState.h"
#include "CutsceneState.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_thread.h>

namespace OpenXcom
{

LoadingPhase StartState::loading;
std::string StartState::error;

#ifdef NINTENDO_3DS
std::atomic<int> StartState::startupStage(
	STARTUP_PREPARING);
std::atomic<int> StartState::startupProgress(0);

void StartState::setStartupProgress(
	StartupLoadStage stage,
	int progress)
{
	if (progress < 0)
	{
		progress = 0;
	}
	else if (progress > 100)
	{
		progress = 100;
	}

	int observed = startupProgress.load(
		std::memory_order_relaxed);

	while (observed < progress &&
		!startupProgress.compare_exchange_weak(
			observed,
			progress,
			std::memory_order_relaxed,
			std::memory_order_relaxed))
	{
	}

	startupStage.store(
		stage,
		std::memory_order_relaxed);
}

void StartState::setStartupWorkProgress(
	StartupLoadStage stage,
	int rangeStart,
	int rangeEnd,
	unsigned int completed,
	unsigned int total)
{
	if (rangeEnd < rangeStart)
	{
		const int temporary = rangeStart;
		rangeStart = rangeEnd;
		rangeEnd = temporary;
	}

	if (total == 0)
	{
		setStartupProgress(stage, rangeEnd);
		return;
	}

	if (completed > total)
	{
		completed = total;
	}

	const long long span =
		static_cast<long long>(rangeEnd - rangeStart);
	const int progress =
		rangeStart +
		static_cast<int>(
			span * completed / total);

	setStartupProgress(stage, progress);
}
#endif

/**
 * Initializes all the elements in the Loading screen.
 * @param game Pointer to the core game.
 */
StartState::StartState() : _anim(0)
{
	//updateScale() uses newDisplayWidth/Height and needs to be set ahead of time
	Options::newDisplayWidth = Options::displayWidth;
	Options::newDisplayHeight = Options::displayHeight;
	Screen::updateScale(Options::geoscapeScale, Options::baseXGeoscape, Options::baseYGeoscape, false);
	Screen::updateScale(Options::battlescapeScale, Options::baseXBattlescape, Options::baseYBattlescape, false);
	Options::baseXResolution = Options::displayWidth;
	Options::baseYResolution = Options::displayHeight;
	_game->getScreen()->resetDisplay(false, true);

	// Create objects
	_thread = 0;
	loading = LOADING_STARTED;
	error = "";
	_oldMaster = Options::getActiveMaster();

#ifdef NINTENDO_3DS
	startupStage.store(
		STARTUP_PREPARING,
		std::memory_order_relaxed);
	startupProgress.store(
		0,
		std::memory_order_relaxed);
	_lastStartupStage = -1;
	_lastStartupProgress = -1;
	_progressText = 0;
	_progressBar = 0;
#endif

	_font = new Font();
	_font->loadTerminal();
	_lang = new Language();

	_text = new Text(Options::baseXResolution, Options::baseYResolution, 0, 0);
	_cursor = new Text(_font->getWidth(), _font->getHeight(), 0, 0);
	_timer = new Timer(150);

#ifdef NINTENDO_3DS
	const int progressMargin = 12;
	int progressWidth =
		Options::baseXResolution -
		progressMargin * 2;

	if (progressWidth < 1)
	{
		progressWidth = 1;
	}

	int progressTextY = 160;
	const int maximumProgressTextY =
		Options::baseYResolution -
		_font->getHeight() - 14;

	if (progressTextY > maximumProgressTextY)
	{
		progressTextY = maximumProgressTextY;
	}

	if (progressTextY < 0)
	{
		progressTextY = 0;
	}

	const int progressBarY =
		progressTextY +
		_font->getHeight() + 2;

	_progressText = new Text(
		progressWidth,
		_font->getHeight(),
		progressMargin,
		progressTextY);

	_progressBar = new Surface(
		progressWidth,
		7,
		progressMargin,
		progressBarY);
#endif

	setStatePalette(Font::TerminalColors, 0, std::size(Font::TerminalColors));

	add(_text);
	add(_cursor);
#ifdef NINTENDO_3DS
	add(_progressText);
	add(_progressBar);
#endif

	// Set up objects
	_text->initText(_font, _font, _lang);
	_text->setColor(0);
	_text->setWordWrap(true);

	_cursor->initText(_font, _font, _lang);
	_cursor->setColor(0);
	_cursor->setText("_");

#ifdef NINTENDO_3DS
	_progressText->initText(
		_font,
		_font,
		_lang);
	_progressText->setColor(0);
	_progressText->setAlign(ALIGN_LEFT);
	updateStartupProgress();
#endif

	_timer->onTimer((StateHandler)&StartState::animate);
	_timer->start();

	// Hide UI
	_game->getCursor()->setVisible(false);
	_game->getFpsCounter()->setVisible(false);

	if (Options::reload)
	{
		if (Options::oxceStartUpTextMode < 2)
		{
			addLine("Restarting...");
			addLine("");
		}
	}
	else
	{
		if (Options::oxceStartUpTextMode < 2)
		{
			addLine(CrossPlatform::getDosPath() + ">openxcom");
		}
	}
}

/**
 * Kill the thread in case the game is quit early.
 */
StartState::~StartState()
{
	if (_thread != 0)
	{
		SDL_KillThread(_thread);
	}
	delete _font;
	delete _timer;
	delete _lang;
}

/**
 * Reset and reload data.
 */
void StartState::init()
{
	State::init();

	// Silence!
	Sound::stop();
	Music::stop();
	if (!Options::mute && Options::reload)
	{
		Mix_CloseAudio();
		_game->initAudio();
	}

	// Load the game data in a separate thread
	_thread = SDL_CreateThread(load, (void*)_game);
	if (_thread == 0)
	{
		// If we can't create the thread, just load it as usual
		load((void*)_game);
	}
}

/**
 * If the loading fails, it shows an error, otherwise moves on to the game.
 */
void StartState::think()
{
	State::think();
	_timer->think(this, 0);

#ifdef NINTENDO_3DS
	updateStartupProgress();
#endif

	switch (loading)
	{
	case LOADING_FAILED:
		CrossPlatform::flashWindow();
		addLine("");
		addLine("ERROR: " + error);
		addLine("");
		addLine("More details here: " + CrossPlatform::getLogFileName());
		addLine("Make sure OpenXcom and any mods are installed correctly.");
		addLine("");
		addLine("Press any key to continue.");
		loading = LOADING_DONE;
		break;
	case LOADING_SUCCESSFUL:
		CrossPlatform::flashWindow();
		Log(LOG_INFO) << "OpenXcom started successfully!";
		_game->setState(new GoToMainMenuState(true));
		if (_oldMaster != Options::getActiveMaster() && Options::playIntro)
		{
			_game->pushState(new CutsceneState("intro"));
		}
		if (Options::reload)
		{
			Options::reload = false;
		}
		_game->getCursor()->setVisible(true);
		_game->getFpsCounter()->setVisible(Options::fpsCounter);
		break;
	default:
		break;
	}
}

/**
 * The game quits if the player presses any key when an error
 * message is on display.
 * @param action Pointer to an action.
 */
void StartState::handle(Action *action)
{
	State::handle(action);
	if (loading == LOADING_DONE)
	{
		if (action->getDetails()->type == SDL_KEYDOWN)
		{
			_game->quit();
		}
	}
}

/**
 * Blinks the cursor and spreads out terminal output.
 */
void StartState::animate()
{
	_cursor->setVisible(!_cursor->getVisible());
	_anim++;

	if (loading == LOADING_STARTED)
	{
		std::ostringstream ss;
		ss << "Loading OpenXcom " << OPENXCOM_VERSION_SHORT << OPENXCOM_VERSION_GIT << "...";
		if (Options::reload)
		{
			if (Options::oxceStartUpTextMode < 2)
			{
				if (_anim == 2)
					addLine(ss.str());
			}
		}
		else
		{
			switch (_anim)
			{
			case 1:
				if (Options::oxceStartUpTextMode < 1)
				{
					addLine("DOS/4GW Protected Mode Run-time  Version 1.9");
					addLine("Copyright (c) Rational Systems, Inc. 1990-1993");
				}
				break;
			case 6:
				if (Options::oxceStartUpTextMode < 2)
				{
					addLine("");
					addLine("OpenXcom initialisation");
				}
				break;
			case 7:
#ifndef NINTENDO_3DS
				if (Options::oxceStartUpTextMode < 1)
				{
					addLine("");
					if (Options::mute)
					{
						addLine("No Sound Detected");
					}
					else
					{
						addLine("SoundBlaster Sound Effects");
						if (Options::preferredMusic == MUSIC_MIDI)
							addLine("General MIDI Music");
						else
							addLine("SoundBlaster Music");
						addLine("Base Port 220  Irq 7  Dma 1");
					}
				}
#endif
#ifndef NINTENDO_3DS
				if (Options::oxceStartUpTextMode < 2)
				{
					addLine("");
				}
#endif
				break;
			case 9:
				if (Options::oxceStartUpTextMode < 2)
				{
					addLine(ss.str());
				}
				break;
			}
		}
	}
}

#ifdef NINTENDO_3DS
/**
 * Returns the visible label for a startup phase.
 */
const char *StartState::getStartupStageLabel(
	int stage)
{
	switch (stage)
	{
	case STARTUP_SCANNING_RESOURCES:
		return "Scanning Resources";
	case STARTUP_FINDING_DATA_FOLDER:
		return "Finding Data Folder";
	case STARTUP_INDEXING_COMMON:
		return "Indexing Common Resources";
	case STARTUP_OPENING_STANDARD_ARCHIVE:
		return "Opening Standard Archive";
	case STARTUP_SCANNING_EMBEDDED_STANDARD:
		return "Scanning Embedded Content";
	case STARTUP_SCANNING_STANDARD_DATA:
		return "Scanning Standard Folder";
	case STARTUP_SCANNING_USER_MODS:
		return "Scanning User Folder";
	case STARTUP_CHECKING_DEPENDENCIES:
		return "Checking Dependencies";
	case STARTUP_CHECKING_MASTER_CHAINS:
		return "Checking Master Chains";
	case STARTUP_MAPPING_EXTERNAL_RESOURCES:
		return "Mapping External Resources";
	case STARTUP_RECONCILING_MODS:
		return "Organizing Content";
	case STARTUP_VALIDATING_ACTIVE_CONTENT:
		return "Validating Content";
	case STARTUP_BUILDING_RESOURCE_MAP:
		return "Building Resource Map";
	case STARTUP_FINALIZING_RESOURCE_SCAN:
		return "Finalizing Resource Scan";
	case STARTUP_LOADING_GAME_DATA:
		return "Loading Game Data";
	case STARTUP_PREPARING_RULESETS:
		return "Preparing Rulesets";
	case STARTUP_PRELOADING_RESOURCE_CONFIG:
		return "Preloading Resource Config";
	case STARTUP_LOADING_VANILLA_RESOURCES:
		return "Loading Vanilla Resources";
	case STARTUP_LOADING_RULESETS:
		return "Loading Rulesets";
	case STARTUP_POSTPROCESSING_RULES:
		return "Processing Loaded Rules";
	case STARTUP_LOADING_EXTRA_RESOURCES:
		return "Loading Extra Resources";
	case STARTUP_LINKING_RULES:
		return "Linking Rules";
	case STARTUP_SORTING_RULES:
		return "Sorting Rules";
	case STARTUP_LOADING_MOD_RESOURCES:
		return "Finalizing Resources";
	case STARTUP_LOADING_LANGUAGE:
		return "Loading Language";
	case STARTUP_FINALIZING:
		return "Finalizing";
	case STARTUP_COMPLETE:
		return "Complete";
	case STARTUP_FAILED:
		return "Startup Failed";
	case STARTUP_PREPARING:
	default:
		return "Preparing";
	}
}

/**
 * Updates the startup display from worker-thread atomics.
 * Drawing remains exclusively on the main UI thread.
 */
void StartState::updateStartupProgress()
{
	if (!_progressText || !_progressBar)
	{
		return;
	}

	const int stage = startupStage.load(
		std::memory_order_relaxed);

	int progress = startupProgress.load(
		std::memory_order_relaxed);

	if (progress < 0)
	{
		progress = 0;
	}
	else if (progress > 100)
	{
		progress = 100;
	}

	if (stage == _lastStartupStage &&
		progress == _lastStartupProgress)
	{
		return;
	}

	_lastStartupStage = stage;
	_lastStartupProgress = progress;

	std::ostringstream label;
	label << getStartupStageLabel(stage)
		<< "  "
		<< progress
		<< "%";

	_progressText->setText(label.str());

	const int width = _progressBar->getWidth();
	const int height = _progressBar->getHeight();

	_progressBar->clear();

	if (width <= 0 || height <= 0)
	{
		return;
	}

	_progressBar->drawRect(
		0,
		0,
		width,
		height,
		1);

	if (width > 2 && height > 2)
	{
		const int innerWidth = width - 2;
		const int fillWidth =
			innerWidth * progress / 100;

		_progressBar->drawRect(
			1,
			1,
			innerWidth,
			height - 2,
			0);

		if (fillWidth > 0)
		{
			_progressBar->drawRect(
				1,
				1,
				fillWidth,
				height - 2,
				1);
		}
	}
}
#endif

/**
 * Adds a line of text to the terminal and moves
 * the cursor appropriately.
 * @param str Text line to add.
 */
void StartState::addLine(const std::string &str)
{
	_output << "\n" << str;
	_text->setText(_output.str());
	int y = _text->getTextHeight() - _font->getHeight();
	int x = _text->getTextWidth(y / _font->getHeight());
	_cursor->setX(x);
	_cursor->setY(y);
}

/**
 * Loads game data and updates status accordingly.
 * @param game_ptr Pointer to the game.
 * @return Thread status, 0 = ok
 */
int StartState::load(void *game_ptr)
{
	Game *game = (Game*)game_ptr;
	try
	{
#ifdef NINTENDO_3DS

		setStartupProgress(
		STARTUP_SCANNING_RESOURCES,
		0);
#endif

		Log(LOG_INFO) << "Loading data...";
		Options::updateMods();

#ifdef NINTENDO_3DS
		setStartupProgress(
		STARTUP_LOADING_GAME_DATA,
		35);
#endif

		game->loadMods();

#ifdef NINTENDO_3DS
		setStartupProgress(
		STARTUP_LOADING_LANGUAGE,
		95);
#endif

		Log(LOG_INFO) << "Data loaded successfully.";
		Log(LOG_INFO) << "Loading language...";
		game->loadLanguages();

#ifdef NINTENDO_3DS
		setStartupProgress(
		STARTUP_FINALIZING,
		99);
#endif

		Log(LOG_INFO) << "Language loaded successfully.";

#ifdef NINTENDO_3DS
		setStartupProgress(
		STARTUP_COMPLETE,
		100);
#endif

		loading = LOADING_SUCCESSFUL;
	}
	catch (std::exception &e)
	{
		error = e.what();
		Log(LOG_ERROR) << error;
#ifdef NINTENDO_3DS
		setStartupProgress(
		STARTUP_FAILED,
		startupProgress.load(
			std::memory_order_relaxed));
#endif
		loading = LOADING_FAILED;
	}

	return 0;
}

}
