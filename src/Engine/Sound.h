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
#include <SDL_rwops.h>
#include <SDL_mixer.h>
#include <string>
#include <memory>

namespace OpenXcom
{

/**
 * Container for sound effects.
 * Handles loading and playing various formats through SDL_mixer.
 */
class Sound
{
public:
	struct UniqueSoundDeleter
	{
		void operator()(Mix_Chunk*);
	};

	using UniqueSoundPtr = std::unique_ptr<Mix_Chunk, UniqueSoundDeleter>;

	/// Smart pointer for for Mix_Chunk.
	static UniqueSoundPtr NewSound(Mix_Chunk* sound);

private:
	UniqueSoundPtr _sound;

public:
	/// Creates a blank sound effect.
	Sound() = default;
	/// Cleans up the sound effect.
	~Sound() = default;
	/// Move sound to another place.
	Sound(Sound&& other) = default;
	/// Move assignment
	Sound& operator=(Sound&& other) = default;

	/// Loads sound from the specified file.
	void load(const std::string &filename);
	/// Loads sound from SDL_RWops
	void load(SDL_RWops *rw);
	/// Plays the sound.
	void play(int channel = -1, int angle = 0, int distance = 0) const;
	/// Stops all sounds.
	static void stop();
	/// Plays the sound repeatedly.
	void loop();
	/// Stops the looping sound effect.
	void stopLoop();
};

}
