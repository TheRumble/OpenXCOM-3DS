#pragma once

namespace OpenXcom
{

class Game;

namespace Input3DS
{

void pump(Game *game);

/// Whether the physical X button is holding middle mouse.
bool isMiddleMouseHeld();

}
}
