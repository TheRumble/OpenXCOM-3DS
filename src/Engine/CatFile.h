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
#include <vector>
#include <string>
#include <tuple>
#include <SDL_rwops.h>


namespace OpenXcom
{

/**
 * Handles CAT files
 */
class CatFile
{
private:
	std::string _filename;
	Uint8 *_data;
	std::vector<std::tuple<void *, size_t>> _items;

public:
	/// Creates a CAT file stream.
	CatFile(const std::string& filename);
	/// Cleans up the stream.
	~CatFile();
	/// Get amount of objects.
	size_t size() const { return _items.size(); }
	/// Return a pointer to the object data.
	SDL_RWops *getRWops(Uint32 i);
	/// Return the original file name
	const std::string& fileName() const { return _filename; }
};

}
