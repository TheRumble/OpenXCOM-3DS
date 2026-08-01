#pragma once
/*
 * Copyright 2010-2015 OpenXcom Developers.
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
#include "../Engine/State.h"

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextList;
class Globe;
class Target;
class GeoscapeState;

/**
 * UFO Tracker window that allows the player to track
 * various things (UFOs, Crash sites, Landing sites, Terror sites, Mission sites, Alien bases) on the Geoscape.
 */
class UfoTrackerState : public State
{
private:
	TextButton *_btnCancel;
	Window *_window;
	Text *_txtTitle, *_txtObject, *_txtSize, *_txtAltitude, *_txtHeading, *_txtSpeed;
	TextList *_lstObjects;
	GeoscapeState *_state;
	Globe *_globe;
	std::vector<Target*> _objects;
public:
	/// Creates the UfoTracker state.
	UfoTrackerState(GeoscapeState *state, Globe *globe);
	/// Cleans up the UfoTracker state.
	~UfoTrackerState();
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
	/// Popup for a target.
	void popupTarget(Target *target);
	/// Handler for clicking the Objects list.
	void lstObjectsLeftClick(Action *action);
	/// Handler for right clicking the Objects list.
	void lstObjectsRightClick(Action *action);
	/// Handler for middle clicking the Objects list.
	void lstObjectsMiddleClick(Action *action);
};

}
