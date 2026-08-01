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
#include <list>
#include "../Engine/InteractiveSurface.h"
#include "../Engine/FastLineClip.h"
#include "Cord.h"

namespace OpenXcom
{

class Game;
class Polygon;
class SurfaceSet;
class Timer;
class Target;
class LocalizedText;
class RuleGlobe;
class Craft;

#ifdef NINTENDO_3DS
/**
 * Compact cached normal used by the 3DS software globe shadow.
 *
 * Desktop builds retain the original double-precision Cord cache.
 */
/*
 * OXCE_3DS_FIXED_POINT_SHADOW
 *
 * Q14 cached unit normal. A value of 16384 represents 1.0.
 * Three Sint16 values use six bytes per cached globe pixel.
 */
/*
 * OXCE_3DS_Z_ONLY_NORMAL_CACHE
 *
 * Only Z varies uniquely for every sphere pixel. X depends only
 * on the source column, and Y depends only on the source row.
 */
struct GlobeNormal3DS
{
	static constexpr Sint32 SCALE = 1 << 14;

	Sint16 z = 0;

	static inline Sint16 encode(
			double value)
	{
		return static_cast<Sint16>(
				value *
				static_cast<double>(
						SCALE));
	}

	GlobeNormal3DS() = default;

	explicit GlobeNormal3DS(
			const Cord &source)
	{
		*this = source;
	}

	GlobeNormal3DS& operator=(
			const Cord &source)
	{
		z = encode(source.z);
		return *this;
	}
};


/*
 * The sun direction is one vector per globe draw, so keeping
 * all three components here has negligible memory cost.
 */
struct GlobeSun3DS
{
	Sint16 x = 0;
	Sint16 y = 0;
	Sint16 z = 0;

	explicit GlobeSun3DS(
			const Cord &source)
		:
		x(GlobeNormal3DS::encode(source.x)),
		y(GlobeNormal3DS::encode(source.y)),
		z(GlobeNormal3DS::encode(source.z))
	{
	}
};


static_assert(
		sizeof(GlobeNormal3DS) == 2,
		"Unexpected padding in GlobeNormal3DS");

static_assert(
		sizeof(GlobeSun3DS) == 6,
		"Unexpected padding in GlobeSun3DS");




#endif

/**
 * Interactive globe view of the world.
 * Takes a flat world map made out of land polygons with
 * polar coordinates and renders it as a 3D-looking globe
 * with cartesian coordinates that the player can interact with.
 */
class Globe : public InteractiveSurface
{
private:
	static const int NUM_LANDSHADES = 48;
	static const int NUM_SEASHADES = 72;
	static const int NEAR_RADIUS = 25;
	static const int MAX_DRAW_RADAR_CIRCLE_RADIUS = 10000;
	static const size_t DOGFIGHT_ZOOM = 3;
	static const int CITY_MARKER = 8;
	static const double ROTATE_LONGITUDE;
	static const double ROTATE_LATITUDE;

	RuleGlobe *_rules;
	Sint16 _cenX, _cenY;
	double _cenLon, _cenLat, _rotLon, _rotLat, _hoverLon, _hoverLat;
	double _craftLon, _craftLat, _craftRange;
	size_t _zoom, _zoomOld, _zoomTexture;
	SurfaceSet *_texture, *_markerSet;
	Game *_game;
	Surface *_markers, *_countries, *_radars;
	bool _hover, _craft;
	int _blink;
	Timer *_blinkTimer, *_rotTimer;
	/// Visible projected polygons owned by _cacheLandPool.
	std::vector<Polygon*> _cacheLand;

	/// Persistent projected copy of every source globe polygon.
	std::vector<Polygon*> _cacheLandPool;

#ifdef NINTENDO_3DS
	/*
	 * OXCE_3DS_CACHED_LAND_DRAWS
	 *
	 * Compact draw records rebuilt alongside the projected polygon
	 * cache. They avoid repeating point extraction and texture-frame
	 * lookup every time the globe is drawn.
	 */
	struct GlobeLandDraw3DS
	{
		Sint16 x[4];
		Sint16 y[4];
		Surface *texture;
		Uint8 points;
	};

	std::vector<GlobeLandDraw3DS> _cacheLandDraw3DS;
#endif

	/*
	 * Static trigonometric values for every source polygon point.
	 * These are flattened in source-polygon order.
	 */
	std::vector<double> _cacheCosLatitude;
	std::vector<double> _cacheSinLatitude;
	std::vector<double> _cacheCosLongitude;
	std::vector<double> _cacheSinLongitude;
	std::vector<size_t> _cachePointOffsets;

	FastLineClip *_clipper;
	double _radius, _radiusStep;
	///normal of each pixel in earth globe per zoom level
#ifdef NINTENDO_3DS
	std::vector<std::vector<GlobeNormal3DS> > _earthData;

	/*
	 * Q14 X coordinates are shared by a complete column.
	 * Q14 Y coordinates are shared by a complete row.
	 */
	std::vector<std::vector<Sint16> > _earthXQ14;
	std::vector<std::vector<Sint16> > _earthYQ14;

	/*
	 * Reused each draw to hold X * sunX in Q28.
	 * Keeping this as a member avoids per-frame allocation.
	 */
	std::vector<Sint32> _shadowXSunQ28;
#else
	std::vector<std::vector<Cord> > _earthData;
#endif

#ifdef NINTENDO_3DS
	/*
	 * Valid nonzero earth-normal span for each screen row and zoom.
	 * Right values are one past the final valid pixel.
	 */
	std::vector<std::vector<int> > _earthSpanLeft;
	std::vector<std::vector<int> > _earthSpanRight;
#endif

	///list of dimension of earth on screen per zoom level
	std::vector<double> _zoomRadius;

	bool _isMouseScrolling, _isMouseScrolled;
	int _xBeforeMouseScrolling, _yBeforeMouseScrolling;
	double _lonBeforeMouseScrolling, _latBeforeMouseScrolling;
	Uint32 _mouseScrollingStartTime;
	int _totalMouseMoveX, _totalMouseMoveY;
	bool _mouseMovedOverThreshold;

	/// Sets the globe zoom factor.
	void setZoom(size_t zoom);
	/// Checks if a point is behind the globe.
	bool pointBack(double lon, double lat) const;
	/// Get polygon pointer
	Polygon* getPolygonFromLonLat(double lon, double lat) const;
	/// Checks if a target is near a point.
	bool targetNear(Target* target, int x, int y) const;
	/// Caches a set of polygons.
	void cache(
			std::list<Polygon*> *polygons,
			std::vector<Polygon*> *cache,
			std::vector<Polygon*> *pool);
	/// Get position of sun relative to given position in polar cords and date.
	Cord getSunDirection(double lon, double lat) const;
	/// Draw globe range circle.
	void drawGlobeCircle(double lat, double lon, double radius, int segments, int frac = 1);
	/// Special "transparent" line.
	void XuLine(Surface* surface, Surface* src, double x1, double y1, double x2, double y2, int shade);
	/// Draw line on globe surface.
	void drawVHLine(Surface *surface, double lon1, double lat1, double lon2, double lat2, Uint8 color);
	/// Draw flight path.
	void drawPath(Surface *surface, double lon1, double lat1, double lon2, double lat2);
	/// Draw target marker.
	void drawTarget(Target *target, Surface *surface);
	/// Set up the radius of earth and stuff.
	void setupRadii(int width, int height);
public:
	static Uint8 OCEAN_COLOR;
	static bool OCEAN_SHADING;
	static Uint8 COUNTRY_LABEL_COLOR;
	static Uint8 LINE_COLOR;
	static Uint8 CITY_LABEL_COLOR;
	static Uint8 BASE_LABEL_COLOR;

	/// Creates a new globe at the specified position and size.
	Globe(Game* game, int cenX, int cenY, int width, int height, int x = 0, int y = 0);
	/// Cleans up the globe.
	~Globe();
	/// Converts polar coordinates to cartesian coordinates.
	void polarToCart(double lon, double lat, Sint16 *x, Sint16 *y) const;
	/// Converts polar coordinates to cartesian coordinates.
	void polarToCart(double lon, double lat, double *x, double *y) const;
	/// Converts cartesian coordinates to polar coordinates.
	void cartToPolar(Sint16 x, Sint16 y, double *lon, double *lat) const;
	/// Starts rotating the globe left.
	void rotateLeft();
	/// Starts rotating the globe right.
	void rotateRight();
	/// Starts rotating the globe up.
	void rotateUp();
	/// Starts rotating the globe down.
	void rotateDown();
	/// Stops rotating the globe.
	void rotateStop();
	/// Stops longitude rotation of the globe.
	void rotateStopLon();
	/// Stops latitude rotation of the globe.
	void rotateStopLat();
	/// Zooms the globe in.
	void zoomIn();
	/// Zooms the globe out.
	void zoomOut();
	/// Zooms the globe minimum.
	void zoomMin();
	/// Zooms the globe maximum.
	void zoomMax();
	/// Saves the zoom level for dogfights.
	void saveZoomDogfight();
	/// Zooms the globe in for dogfights.
	bool zoomDogfightIn();
	/// Zooms the globe out for dogfights.
	bool zoomDogfightOut();
	/// Gets the current zoom.
	size_t getZoom() const;
	/// Centers the globe on a point.
	void center(double lon, double lat);
	/// Checks if a point is inside land.
	bool insideLand(double lon, double lat) const;
	/// Checks if a point is inside fakeUnderwater texture.
	bool insideFakeUnderwaterTexture(double lon, double lat) const;
	/// Turns on/off the globe detail.
	void toggleDetail();
	/// Gets all the targets near a point on the globe.
	std::vector<Target*> getTargets(int x, int y, bool craft, Craft *currentCraft) const;
	/// Caches visible globe polygons.
	void cachePolygons();
	/// Sets the palette of the globe.
	void setPalette(const SDL_Color *colors, int firstcolor = 0, int ncolors = 256) override;
	/// Handles the timers.
	void think() override;
	/// Blinks the markers.
	void blink();
	/// Rotates the globe.
	void rotate();
	/// Draws the whole globe.
	void draw() override;
	/// Draws the ocean of the globe.
	void drawOcean();
	/// Draws the land of the globe.
	void drawLand();
	/// Draws the shadow.
	void drawShadow();
	/// Draws the radar ranges of the globe.
	void drawRadars();
	/// Draws the flight paths of the globe.
	void drawFlights();
	/// Draws the country details of the globe.
	void drawDetail();
	/// Draws all the markers over the globe.
	void drawMarkers();
	/// Blits the globe onto another surface.
	void blit(SDL_Surface *surface) override;
	/// Special handling for mouse hover.
	void mouseOver(Action *action, State *state) override;
	/// Special handling for mouse presses.
	void mousePress(Action *action, State *state) override;
	/// Special handling for mouse releases.
	void mouseRelease(Action *action, State *state) override;
	/// Special handling for mouse clicks.
	void mouseClick(Action *action, State *state) override;
	/// Special handling for key presses.
	void keyboardPress(Action *action, State *state) override;
	/// Get the polygons texture and shade at the given point.
	void getPolygonTextureAndShade(double lon, double lat, int *texture, int *shade) const;
	/// Sets hover base position.
	void setNewBaseHoverPos(double lon, double lat);
	/// Turns on new base hover mode.
	void setNewBaseHover(bool hover);
	/// Sets craft range mode.
	void setCraftRange(double lon, double lat, double range);
	/// set the _radarLines variable
	void toggleRadarLines();
	/// Update the resolution settings, we just resized the window.
	void resize();
#ifdef NINTENDO_3DS
	/// Applies native relative Circle Pad movement to the globe.
	void dragWithCirclePad3DS(int dx, int dy);
#endif
	/// Move the mouse back to where it started after we finish drag scrolling.
	void stopScrolling(Action *action);
};

}
