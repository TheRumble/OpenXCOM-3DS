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
#include "ShaderDraw.h"

namespace OpenXcom
{


template<typename Pixel>
class ShaderMove : public helper::ShaderBase<Pixel>
{
	int _move_x;
	int _move_y;

public:
	typedef helper::ShaderBase<Pixel> _base;
	friend struct helper::controler<ShaderMove<Pixel> >;

	inline ShaderMove(SurfaceRaw<Pixel> s):
		_base(s),
		_move_x(0), _move_y(0)
	{

	}

	inline ShaderMove(SurfaceRaw<Pixel> s, int move_x, int move_y):
		_base(s),
		_move_x(move_x), _move_y(move_y)
	{

	}

	inline ShaderMove(const ShaderMove& f):
		_base(f),
		_move_x(f._move_x), _move_y(f._move_y)
	{

	}

	inline GraphSubset getImage() const
	{
		return _base::_range_domain.offset(_move_x, _move_y);
	}

	inline void setMove(int x, int y)
	{
		_move_x = x;
		_move_y = y;
	}
	inline void addMove(int x, int y)
	{
		_move_x += x;
		_move_y += y;
	}
};



namespace helper
{

template<typename Pixel>
struct controler<ShaderMove<Pixel> > : public controler_base<typename ShaderMove<Pixel>::PixelPtr, typename ShaderMove<Pixel>::PixelRef>
{
	typedef typename ShaderMove<Pixel>::PixelPtr PixelPtr;
	typedef typename ShaderMove<Pixel>::PixelRef PixelRef;

	typedef controler_base<PixelPtr, PixelRef> base_type;

	controler(const ShaderMove<Pixel>& f) : base_type(f.ptr(), f.getDomain(), f.getImage(), std::make_pair(sizeof(Pixel), f.pitch()))
	{

	}

};

}//namespace helper

#ifdef NINTENDO_3DS

namespace detail
{

/**
 * Precomputed result of the normal Battlescape palette-index shading
 * operation. Index zero remains transparent and is handled by the
 * caller, so the table contains only the replacement value.
 */
struct StandardShadeTables3DS
{
	Uint8 value[17][256];

	StandardShadeTables3DS()
	{
		for (int shade = 0; shade <= 16; ++shade)
		{
			for (int source = 0; source < 256; ++source)
			{
				Uint8 destination = 0;
				const Uint8 sourcePixel = static_cast<Uint8>(source);
				helper::StandardShade::func(destination, sourcePixel, shade);
				value[shade][source] = destination;
			}
		}
	}
};

inline const Uint8 *getStandardShadeTable3DS(int shade)
{
	static const StandardShadeTables3DS tables;
	return shade >= 0 && shade <= 16 ? tables.value[shade] : nullptr;
}

}

/**
 * Direct 8-bit standard-shade blitter for the 3DS software renderer.
 *
 * The generic ShaderDraw controller is intentionally flexible, but the
 * Battlescape hot path always uses two moved 8-bit surfaces plus one
 * scalar shade. Clip that common case once and then walk direct row
 * pointers. Other shader combinations continue using ShaderDraw's
 * original generic implementation.
 */
template<typename ColorFunc, typename Shade>
inline typename std::enable_if<
	std::is_same<ColorFunc, helper::StandardShade>::value,
	void>::type
ShaderDraw(
	const ShaderMove<Uint8>& destination,
	const ShaderMove<const Uint8>& source,
	const helper::Scalar<Shade>& shade)
{
	GraphSubset drawRange = GraphSubset::intersection(
		destination.getImage(),
		source.getImage());

	if (!drawRange)
	{
		return;
	}

	const GraphSubset destinationDomain = destination.getDomain();
	const GraphSubset destinationImage = destination.getImage();
	const GraphSubset sourceDomain = source.getDomain();
	const GraphSubset sourceImage = source.getImage();

	const int destinationStartX =
		destinationDomain.beg_x +
		(drawRange.beg_x - destinationImage.beg_x);
	const int destinationStartY =
		destinationDomain.beg_y +
		(drawRange.beg_y - destinationImage.beg_y);

	const int sourceStartX =
		sourceDomain.beg_x +
		(drawRange.beg_x - sourceImage.beg_x);
	const int sourceStartY =
		sourceDomain.beg_y +
		(drawRange.beg_y - sourceImage.beg_y);

	const int width = drawRange.size_x();
	const int height = drawRange.size_y();
	const int shadeValue = static_cast<int>(shade.ref);
	const Uint8 *shadeTable =
		detail::getStandardShadeTable3DS(shadeValue);

	Uint8 *destinationRow =
		destination.ptr() +
		destinationStartY * destination.pitch() +
		destinationStartX;

	const Uint8 *sourceRow =
		source.ptr() +
		sourceStartY * source.pitch() +
		sourceStartX;

	for (int y = 0; y < height; ++y)
	{
		Uint8 *destinationPixel = destinationRow;
		const Uint8 *sourcePixel = sourceRow;

		if (shadeTable)
		{
			for (int x = 0; x < width; ++x)
			{
				const Uint8 pixel = sourcePixel[x];
				if (pixel)
				{
					destinationPixel[x] = shadeTable[pixel];
				}
			}
		}
		else
		{
			for (int x = 0; x < width; ++x)
			{
				helper::StandardShade::func(
					destinationPixel[x],
					sourcePixel[x],
					shadeValue);
			}
		}

		destinationRow += destination.pitch();
		sourceRow += source.pitch();
	}
}

/**
 * Direct counterpart for the recolored Battlescape path used by night
 * vision, custom markers and faction indicators.
 */
template<typename ColorFunc, typename Shade, typename NewColor>
inline typename std::enable_if<
	std::is_same<ColorFunc, helper::ColorReplace>::value,
	void>::type
ShaderDraw(
	const ShaderMove<Uint8>& destination,
	const ShaderMove<const Uint8>& source,
	const helper::Scalar<Shade>& shade,
	const helper::Scalar<NewColor>& newColor)
{
	GraphSubset drawRange = GraphSubset::intersection(
		destination.getImage(),
		source.getImage());

	if (!drawRange)
	{
		return;
	}

	const GraphSubset destinationDomain = destination.getDomain();
	const GraphSubset destinationImage = destination.getImage();
	const GraphSubset sourceDomain = source.getDomain();
	const GraphSubset sourceImage = source.getImage();

	const int destinationStartX =
		destinationDomain.beg_x +
		(drawRange.beg_x - destinationImage.beg_x);
	const int destinationStartY =
		destinationDomain.beg_y +
		(drawRange.beg_y - destinationImage.beg_y);

	const int sourceStartX =
		sourceDomain.beg_x +
		(drawRange.beg_x - sourceImage.beg_x);
	const int sourceStartY =
		sourceDomain.beg_y +
		(drawRange.beg_y - sourceImage.beg_y);

	const int width = drawRange.size_x();
	const int height = drawRange.size_y();
	const int shadeValue = static_cast<int>(shade.ref);
	const int colorValue = static_cast<int>(newColor.ref);

	Uint8 *destinationRow =
		destination.ptr() +
		destinationStartY * destination.pitch() +
		destinationStartX;

	const Uint8 *sourceRow =
		source.ptr() +
		sourceStartY * source.pitch() +
		sourceStartX;

	for (int y = 0; y < height; ++y)
	{
		Uint8 *destinationPixel = destinationRow;
		const Uint8 *sourcePixel = sourceRow;

		for (int x = 0; x < width; ++x)
		{
			const Uint8 pixel = sourcePixel[x];
			if (pixel)
			{
				const Uint8 newShade =
					(pixel & helper::ColorShade) +
					shadeValue;

				destinationPixel[x] =
					(newShade & helper::ColorGroup) ?
						helper::ColorShade :
						static_cast<Uint8>(
							colorValue |
							newShade);
			}
		}

		destinationRow += destination.pitch();
		sourceRow += source.pitch();
	}
}

#endif

/**
 * Create warper from Surface
 * @param s standard 8bit OpenXcom surface
 * @return
 */
template<typename T>
inline ShaderMove<T> ShaderSurface(SurfaceRaw<T> s)
{
	return ShaderMove<T>(s);
}

/**
 * Create warper from Surface
 * @param s standard 8bit OpenXcom surface
 * @return
 */
inline ShaderMove<Uint8> ShaderSurface(SurfaceRaw<Uint8> s)
{
	return ShaderMove<Uint8>(s);
}

/**
 * Create warper from Surface and provided offset
 * @param s standard 8bit OpenXcom surface
 * @param x offset on x
 * @param y offset on y
 * @return
 */
inline ShaderMove<Uint8> ShaderSurface(SurfaceRaw<Uint8> s, int x, int y)
{
	return ShaderMove<Uint8>(s, x, y);
}

/**
 * Create warper from cropped Surface and provided offset
 * @param s standard 8bit OpenXcom surface
 * @param x offset on x
 * @param y offset on y
 * @return
 */
inline ShaderMove<const Uint8> ShaderCrop(SurfaceCrop s, int x, int y)
{
	ShaderMove<const Uint8> ret(s.getSurface(), x, y);
	SDL_Rect* s_crop = s.getCrop();
	if (s_crop->w || s_crop->h)
	{
		GraphSubset crop(std::make_pair(s_crop->x, s_crop->x + s_crop->w), std::make_pair(s_crop->y, s_crop->y + s_crop->h));
		ret.setDomain(crop);
		ret.addMove(-s_crop->x, -s_crop->y);
	}
	return ret;
}

/**
 * Create warper from cropped Surface
 * @param s standard 8bit OpenXcom surface
 * @return
 */
inline ShaderMove<const Uint8> ShaderCrop(SurfaceCrop s)
{
	return ShaderCrop(s, s.getX(), s.getY());
}

}//namespace OpenXcom
