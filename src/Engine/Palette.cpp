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
#include "Palette.h"
#include <sstream>
#include "CrossPlatform.h"
#include "Exception.h"
#include "FileMap.h"

namespace OpenXcom
{

/**
 * Initializes a brand new palette.
 */
Palette::Palette() : _colors(0), _count(0)
{
}

/**
 * Deletes any colors contained within.
 */
Palette::~Palette()
{
	delete[] _colors;
}

/**
 * Loads an X-Com palette from a file. X-Com palettes are just a set
 * of RGB colors in a row, on a 0-63 scale, which have to be adjusted
 * for modern computers (0-255 scale).
 * @param filename Filename of the palette.
 * @param ncolors Number of colors in the palette.
 * @param offset Position of the palette in the file (in bytes).
 * @sa http://www.ufopaedia.org/index.php?title=PALETTES.DAT
 */
void Palette::loadDat(const std::string &filename, int ncolors, int offset)
{
	auto palFile = FileMap::getIStream(filename);
	if (_colors != 0)
		throw Exception("loadDat can be run only once");
	_count = ncolors;
	_colors = new SDL_Color[_count];
	memset(_colors, 0, sizeof(SDL_Color) * _count);

	// Move pointer to proper palette
	palFile->seekg(offset, std::ios::beg);

	Uint8 value[3];

	for (int i = 0; i < _count && palFile->read((char*)value, 3); ++i)
	{
		// Correct X-Com colors to RGB colors
		_colors[i].r = value[0] * 4;
		_colors[i].g = value[1] * 4;
		_colors[i].b = value[2] * 4;
		_colors[i].unused = 255;
	}
	_colors[0].unused = 0;
}

/**
 * Initializes an all-black palette.
 */
void Palette::initBlack()
{
	if (_colors != 0)
		throw Exception("initBlack can be run only once");
	_count = 256;
	_colors = new SDL_Color[_count];
	memset(_colors, 0, sizeof(SDL_Color) * _count);

	for (int i = 0; i < _count; ++i)
	{
		_colors[i].r = 0;
		_colors[i].g = 0;
		_colors[i].b = 0;
		_colors[i].unused = 255;
	}
	_colors[0].unused = 0;
}

/**
 * Loads the colors from an existing palette.
 */
void Palette::copyFrom(Palette *srcPal)
{
	for (int i = 0; i < srcPal->getColorCount(); i++)
	{
		setColor(i, srcPal->getColors(i)->r, srcPal->getColors(i)->g, srcPal->getColors(i)->b);
	}
}

/**
 * Provides access to colors contained in the palette.
 * @param offset Offset to a specific color.
 * @return Pointer to the requested SDL_Color.
 */
SDL_Color *Palette::getColors(int offset) const
{
	return _colors + offset;
}

/**
 * Converts an SDL_Color struct into an hexadecimal RGBA color value.
 * Mostly used for operations with SDL_gfx that require colors in this format.
 * @param pal Requested palette.
 * @param color Requested color in the palette.
 * @return Hexadecimal RGBA value.
 */
Uint32 Palette::getRGBA(SDL_Color* pal, Uint8 color)
{
	return ((Uint32) pal[color].r << 24) | ((Uint32) pal[color].g << 16) | ((Uint32) pal[color].b << 8) | (Uint32) 0xFF;
}

void Palette::savePal(const std::string &file) const
{
	std::stringstream out;
	short count = _count;

	// RIFF header
	out << "RIFF";
	int length = 4 + 4 + 4 + 4 + 2 + 2 + count * 4;
	out.write((char*) &length, sizeof(length));
	out << "PAL ";

	// Data chunk
	out << "data";
	int data = count * 4 + 4;
	out.write((char*) &data, sizeof(data));
	short version = 0x0300;
	out.write((char*) &version, sizeof(version));
	out.write((char*) &count, sizeof(count));

	// Colors
	SDL_Color *color = getColors();
	for (short i = 0; i < count; ++i)
	{
		char c = 0;
		out.write((char*) &color->r, 1);
		out.write((char*) &color->g, 1);
		out.write((char*) &color->b, 1);
		out.write(&c, 1);
		color++;
	}
	CrossPlatform::writeFile(file, out.str());
}

void Palette::savePalMod(const std::string &file, const std::string &type, const std::string &target) const
{
	std::stringstream out;
	short count = _count;

	// header
	out << "customPalettes:\n";
	out << "  - type: " << type << "\n";
	out << "    target: " << target << "\n";
	out << "    palette:\n";

	// Colors
	for (int i = 0; i < count; ++i)
	{
		out << "      ";
		out << std::to_string(i);
		out << ": [";
		out << std::to_string(_colors[i].r);
		out << ",";
		out << std::to_string(_colors[i].g);
		out << ",";
		out << std::to_string(_colors[i].b);
		out << "]\n";
	}
	CrossPlatform::writeFile(file, out.str());
}

void Palette::savePalJasc(const std::string &file) const
{
	std::stringstream out;
	short count = _count;

	// header
	out << "JASC-PAL\n";
	out << "0100\n";
	out << "256\n";

	// Colors
	for (int i = 0; i < count; ++i)
	{
		out << std::to_string(_colors[i].r);
		out << " ";
		out << std::to_string(_colors[i].g);
		out << " ";
		out << std::to_string(_colors[i].b);
		out << "\n";
	}
	CrossPlatform::writeFile(file, out.str());
}

void Palette::setColors(SDL_Color* pal, int ncolors)
{
	if (_colors != 0)
		throw Exception("setColors can be run only once");
	_count = ncolors;
	_colors = new SDL_Color[_count];
	memset(_colors, 0, sizeof(SDL_Color) * _count);

	for (int i = 0; i < _count; ++i)
	{
		// TFTD's LBM colors are good the way they are - no need for adjustment here, except...
		_colors[i].r = pal[i].r;
		_colors[i].g = pal[i].g;
		_colors[i].b = pal[i].b;
		_colors[i].unused = 255;
		if (i > 15 && _colors[i].r == _colors[0].r &&
			_colors[i].g == _colors[0].g &&
			_colors[i].b == _colors[0].b)
		{
			// SDL "optimizes" surfaces by using RGB colour matching to reassign pixels to an "earlier" matching colour in the palette,
			// meaning any pixels in a surface that are meant to be black will be reassigned as colour 0, rendering them transparent.
			// avoid this eventuality by altering the "later" colours just enough to disambiguate them without causing them to look significantly different.
			// SDL 2.0 has some functionality that should render this hack unnecessary.
			_colors[i].r++;
			_colors[i].g++;
			_colors[i].b++;
		}
	}
	_colors[0].unused = 0;
}

void Palette::setColor(int index, int r, int g, int b)
{
	_colors[index].r = r;
	_colors[index].g = g;
	_colors[index].b = b;
	if (index > 15 && _colors[index].r == _colors[0].r &&
		_colors[index].g == _colors[0].g &&
		_colors[index].b == _colors[0].b)
	{
		// SDL "optimizes" surfaces by using RGB colour matching to reassign pixels to an "earlier" matching colour in the palette,
		// meaning any pixels in a surface that are meant to be black will be reassigned as colour 0, rendering them transparent.
		// avoid this eventuality by altering the "later" colours just enough to disambiguate them without causing them to look significantly different.
		// SDL 2.0 has some functionality that should render this hack unnecessary.
		_colors[index].r++;
		_colors[index].g++;
		_colors[index].b++;
	}
}

void Palette::copyColor(int index, int r, int g, int b)
{
	_colors[index].r = r;
	_colors[index].g = g;
	_colors[index].b = b;
}

}
