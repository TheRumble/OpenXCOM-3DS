#include "Options3DSState.h"

#include <SDL.h>

#include "../Engine/Action.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/Slider.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"


namespace OpenXcom
{

Options3DSState::Options3DSState(
	OptionsOrigin origin) :
	OptionsBaseState(origin),
	_pageGroup(nullptr)
{
	setCategory(_btn3DS);

	/*
	 * Page-selection buttons.
	 */
	_btnCursor =
		new TextButton(68, 16, 94, 8);

	_btnCamera =
		new TextButton(68, 16, 168, 8);

	_btnTouchpad =
		new TextButton(68, 16, 242, 8);




	_pageGroup = _btnCursor;

	_btnCursor->setGroup(&_pageGroup);
	_btnCamera->setGroup(&_pageGroup);
	_btnTouchpad->setGroup(&_pageGroup);

	_btnCursor->setText(tr("STR_3DS_CURSOR"));
	_btnCamera->setText(tr("STR_3DS_CAMERA"));
	_btnTouchpad->setText(tr("STR_3DS_TOUCH"));

	_btnCursor->onMousePress(
		(ActionHandler)&Options3DSState::btnPagePress,
		SDL_BUTTON_LEFT);

	_btnCamera->onMousePress(
		(ActionHandler)&Options3DSState::btnPagePress,
		SDL_BUTTON_LEFT);

	_btnTouchpad->onMousePress(
		(ActionHandler)&Options3DSState::btnPagePress,
		SDL_BUTTON_LEFT);


	/*
	 * Reuse the standard Audio-options content styling. It provides
	 * the same established label and control colors as other settings
	 * submenus without changing the common Options sidebar.
	 */
	add(_btnCursor, "button", "audioMenu");
	add(_btnCamera, "button", "audioMenu");
	add(_btnTouchpad, "button", "audioMenu");



	/*
	 * Cursor-speed page.
	 */
	_txtMenuCursor =
		new Text(114, 9, 94, 34);

	_txtGeoscapeCursor =
		new Text(114, 9, 94, 66);

	_txtBattlescapeCursor =
		new Text(114, 9, 206, 34);

	_txtInventoryCursor =
		new Text(114, 9, 206, 66);

	_slrMenuCursor =
		new Slider(104, 16, 94, 44);

	_slrGeoscapeCursor =
		new Slider(104, 16, 94, 76);

	_slrBattlescapeCursor =
		new Slider(104, 16, 206, 44);

	_slrInventoryCursor =
		new Slider(104, 16, 206, 76);

	_slrMenuCursor->setRange(50, 200);
	_slrGeoscapeCursor->setRange(50, 200);
	_slrBattlescapeCursor->setRange(50, 200);
	_slrInventoryCursor->setRange(50, 200);

	_slrMenuCursor->onChange(
		(ActionHandler)&Options3DSState::
			slrMenuCursorChange);

	_slrGeoscapeCursor->onChange(
		(ActionHandler)&Options3DSState::
			slrGeoscapeCursorChange);

	_slrBattlescapeCursor->onChange(
		(ActionHandler)&Options3DSState::
			slrBattlescapeCursorChange);

	_slrInventoryCursor->onChange(
		(ActionHandler)&Options3DSState::
			slrInventoryCursorChange);

	add(_txtMenuCursor, "text", "audioMenu");
	add(_txtGeoscapeCursor, "text", "audioMenu");
	add(_txtBattlescapeCursor, "text", "audioMenu");
	add(_txtInventoryCursor, "text", "audioMenu");

	add(_slrMenuCursor, "button", "audioMenu");
	add(_slrGeoscapeCursor, "button", "audioMenu");
	add(_slrBattlescapeCursor, "button", "audioMenu");
	add(_slrInventoryCursor, "button", "audioMenu");

	/*
	 * Camera page.
	 */
	_txtGeoscapeCamera =
		new Text(114, 9, 94, 34);

	_txtBattlescapeCamera =
		new Text(114, 9, 206, 34);

	_slrGeoscapeCamera =
		new Slider(104, 16, 94, 44);

	_slrBattlescapeCamera =
		new Slider(104, 16, 206, 44);

	_slrGeoscapeCamera->setRange(50, 200);
	_slrBattlescapeCamera->setRange(50, 200);

	_slrGeoscapeCamera->onChange(
		(ActionHandler)&Options3DSState::
			slrGeoscapeCameraChange);

	_slrBattlescapeCamera->onChange(
		(ActionHandler)&Options3DSState::
			slrBattlescapeCameraChange);

	_btnInvertGeoX =
		new ToggleTextButton(104, 16, 94, 76);

	_btnInvertGeoY =
		new ToggleTextButton(104, 16, 94, 98);

	_btnInvertBattleX =
		new ToggleTextButton(104, 16, 206, 76);

	_btnInvertBattleY =
		new ToggleTextButton(104, 16, 206, 98);

	_btnInvertGeoX->setText(
		tr("STR_3DS_GEOSCAPE_INVERT_X"));
	_btnInvertGeoY->setText(
		tr("STR_3DS_GEOSCAPE_INVERT_Y"));
	_btnInvertBattleX->setText(
		tr("STR_3DS_BATTLESCAPE_INVERT_X"));
	_btnInvertBattleY->setText(
		tr("STR_3DS_BATTLESCAPE_INVERT_Y"));

	_btnInvertGeoX->onMouseClick(
		(ActionHandler)&Options3DSState::
			btnInvertGeoXClick);

	_btnInvertGeoY->onMouseClick(
		(ActionHandler)&Options3DSState::
			btnInvertGeoYClick);

	_btnInvertBattleX->onMouseClick(
		(ActionHandler)&Options3DSState::
			btnInvertBattleXClick);

	_btnInvertBattleY->onMouseClick(
		(ActionHandler)&Options3DSState::
			btnInvertBattleYClick);

	add(_txtGeoscapeCamera, "text", "audioMenu");
	add(_txtBattlescapeCamera, "text", "audioMenu");

	add(_slrGeoscapeCamera, "button", "audioMenu");
	add(_slrBattlescapeCamera, "button", "audioMenu");

	add(_btnInvertGeoX, "button", "audioMenu");
	add(_btnInvertGeoY, "button", "audioMenu");
	add(_btnInvertBattleX, "button", "audioMenu");
	add(_btnInvertBattleY, "button", "audioMenu");

	/*
	 * Touchpad page.
	 */
	_txtTouchpadSensitivity =
		new Text(114, 9, 94, 34);

	_txtTouchpadDragThreshold =
		new Text(114, 9, 94, 66);

	_txtTouchpadHoldDelay =
		new Text(114, 9, 206, 34);

	_slrTouchpadSensitivity =
		new Slider(104, 16, 94, 44);

	_slrTouchpadDragThreshold =
		new Slider(104, 16, 94, 76);

	_slrTouchpadHoldDelay =
		new Slider(104, 16, 206, 44);

	_slrTouchpadSensitivity->setRange(50, 200);
	_slrTouchpadDragThreshold->setRange(1, 20);

	/*
	 * Store the hold slider as 2-12 steps so D-pad adjustment moves
	 * by useful 50 ms increments instead of one millisecond at a time.
	 */
	_slrTouchpadHoldDelay->setRange(2, 12);

	_slrTouchpadSensitivity->onChange(
		(ActionHandler)&Options3DSState::
			slrTouchpadSensitivityChange);

	_slrTouchpadDragThreshold->onChange(
		(ActionHandler)&Options3DSState::
			slrTouchpadDragThresholdChange);

	_slrTouchpadHoldDelay->onChange(
		(ActionHandler)&Options3DSState::
			slrTouchpadHoldDelayChange);

	add(_txtTouchpadSensitivity, "text", "audioMenu");
	add(_txtTouchpadDragThreshold, "text", "audioMenu");
	add(_txtTouchpadHoldDelay, "text", "audioMenu");

	add(_slrTouchpadSensitivity, "button", "audioMenu");
	add(_slrTouchpadDragThreshold, "button", "audioMenu");
	add(_slrTouchpadHoldDelay, "button", "audioMenu");






	/*
	 * Center first. Slider::setX() currently recalculates its value from
	 * the normalized position, so setting saved values before centering
	 * would incorrectly move every 50-200 slider to its minimum.
	 */
	centerAllSurfaces();

	_slrMenuCursor->setValue(
		Options::threeDSMenuCursorSpeed);

	_slrGeoscapeCursor->setValue(
		Options::threeDSGeoscapeCursorSpeed);

	_slrBattlescapeCursor->setValue(
		Options::threeDSBattlescapeCursorSpeed);

	_slrInventoryCursor->setValue(
		Options::threeDSInventoryCursorSpeed);

	_slrGeoscapeCamera->setValue(
		Options::threeDSGeoscapeCameraSpeed);

	_slrBattlescapeCamera->setValue(
		Options::threeDSBattlescapeCameraSpeed);

	_slrTouchpadSensitivity->setValue(
		Options::threeDSTouchpadSensitivity);

	_slrTouchpadDragThreshold->setValue(
		Options::threeDSTouchpadDragThreshold);

	_slrTouchpadHoldDelay->setValue(
		(Options::threeDSTouchpadHoldDelay + 25) /
		50);



	_btnInvertGeoX->setPressed(
		Options::threeDSInvertGeoscapeCameraX);

	_btnInvertGeoY->setPressed(
		Options::threeDSInvertGeoscapeCameraY);

	_btnInvertBattleX->setPressed(
		Options::threeDSInvertBattlescapeCameraX);

	_btnInvertBattleY->setPressed(
		Options::threeDSInvertBattlescapeCameraY);

	/*
	 * Normalize any manually edited out-of-range config values.
	 */
	Options::threeDSMenuCursorSpeed =
		_slrMenuCursor->getValue();

	Options::threeDSGeoscapeCursorSpeed =
		_slrGeoscapeCursor->getValue();

	Options::threeDSBattlescapeCursorSpeed =
		_slrBattlescapeCursor->getValue();

	Options::threeDSInventoryCursorSpeed =
		_slrInventoryCursor->getValue();

	Options::threeDSGeoscapeCameraSpeed =
		_slrGeoscapeCamera->getValue();

	Options::threeDSBattlescapeCameraSpeed =
		_slrBattlescapeCamera->getValue();

	Options::threeDSTouchpadSensitivity =
		_slrTouchpadSensitivity->getValue();

	Options::threeDSTouchpadDragThreshold =
		_slrTouchpadDragThreshold->getValue();

	Options::threeDSTouchpadHoldDelay =
		_slrTouchpadHoldDelay->getValue() * 50;

	updateCursorLabels();
	updateCameraLabels();
	updateTouchpadLabels();

	showCursorPage();
}

Options3DSState::~Options3DSState()
{
}

void Options3DSState::setCursorControlsVisible(
	bool visible)
{
	_txtMenuCursor->setVisible(visible);
	_txtGeoscapeCursor->setVisible(visible);
	_txtBattlescapeCursor->setVisible(visible);
	_txtInventoryCursor->setVisible(visible);

	_slrMenuCursor->setVisible(visible);
	_slrGeoscapeCursor->setVisible(visible);
	_slrBattlescapeCursor->setVisible(visible);
	_slrInventoryCursor->setVisible(visible);
}

void Options3DSState::setCameraControlsVisible(
	bool visible)
{
	_txtGeoscapeCamera->setVisible(visible);
	_txtBattlescapeCamera->setVisible(visible);

	_slrGeoscapeCamera->setVisible(visible);
	_slrBattlescapeCamera->setVisible(visible);

	_btnInvertGeoX->setVisible(visible);
	_btnInvertGeoY->setVisible(visible);
	_btnInvertBattleX->setVisible(visible);
	_btnInvertBattleY->setVisible(visible);
}

void Options3DSState::setTouchpadControlsVisible(
	bool visible)
{
	_txtTouchpadSensitivity->setVisible(visible);
	_txtTouchpadDragThreshold->setVisible(visible);
	_txtTouchpadHoldDelay->setVisible(visible);

	_slrTouchpadSensitivity->setVisible(visible);
	_slrTouchpadDragThreshold->setVisible(visible);
	_slrTouchpadHoldDelay->setVisible(visible);
}

void Options3DSState::updateCursorLabels()
{
	_txtMenuCursor->setText(
		tr("STR_3DS_MENU_CURSOR_SPEED").arg(
			Options::threeDSMenuCursorSpeed));

	_txtGeoscapeCursor->setText(
		tr("STR_3DS_GEOSCAPE_CURSOR_SPEED").arg(
			Options::threeDSGeoscapeCursorSpeed));

	_txtBattlescapeCursor->setText(
		tr("STR_3DS_BATTLESCAPE_CURSOR_SPEED").arg(
			Options::threeDSBattlescapeCursorSpeed));

	_txtInventoryCursor->setText(
		tr("STR_3DS_INVENTORY_CURSOR_SPEED").arg(
			Options::threeDSInventoryCursorSpeed));
}

void Options3DSState::updateCameraLabels()
{
	_txtGeoscapeCamera->setText(
		tr("STR_3DS_GEOSCAPE_CAMERA_SPEED").arg(
			Options::threeDSGeoscapeCameraSpeed));

	_txtBattlescapeCamera->setText(
		tr("STR_3DS_BATTLESCAPE_CAMERA_SPEED").arg(
			Options::threeDSBattlescapeCameraSpeed));
}

void Options3DSState::updateTouchpadLabels()
{
	_txtTouchpadSensitivity->setText(
		tr("STR_3DS_TOUCHPAD_SENSITIVITY").arg(
			Options::threeDSTouchpadSensitivity));

	_txtTouchpadDragThreshold->setText(
		tr("STR_3DS_DRAG_THRESHOLD").arg(
			Options::threeDSTouchpadDragThreshold));

	_txtTouchpadHoldDelay->setText(
		tr("STR_3DS_HOLD_DELAY").arg(
			Options::threeDSTouchpadHoldDelay));
}

void Options3DSState::showCursorPage()
{

	setCursorControlsVisible(true);
	setCameraControlsVisible(false);
	setTouchpadControlsVisible(false);
}

void Options3DSState::showCameraPage()
{

	setCursorControlsVisible(false);
	setCameraControlsVisible(true);
	setTouchpadControlsVisible(false);
}

void Options3DSState::showTouchpadPage()
{

	setCursorControlsVisible(false);
	setCameraControlsVisible(false);
	setTouchpadControlsVisible(true);
}

void Options3DSState::btnPagePress(Action *action)
{
	if (action->getSender() == _btnCursor)
	{
		showCursorPage();
	}
	else if (action->getSender() == _btnCamera)
	{
		showCameraPage();
	}
	else if (action->getSender() == _btnTouchpad)
	{
		showTouchpadPage();
	}
}

void Options3DSState::slrMenuCursorChange(Action *)
{
	Options::threeDSMenuCursorSpeed =
		_slrMenuCursor->getValue();

	updateCursorLabels();
}

void Options3DSState::slrGeoscapeCursorChange(Action *)
{
	Options::threeDSGeoscapeCursorSpeed =
		_slrGeoscapeCursor->getValue();

	updateCursorLabels();
}

void Options3DSState::slrBattlescapeCursorChange(Action *)
{
	Options::threeDSBattlescapeCursorSpeed =
		_slrBattlescapeCursor->getValue();

	updateCursorLabels();
}

void Options3DSState::slrInventoryCursorChange(Action *)
{
	Options::threeDSInventoryCursorSpeed =
		_slrInventoryCursor->getValue();

	updateCursorLabels();
}

void Options3DSState::slrGeoscapeCameraChange(Action *)
{
	Options::threeDSGeoscapeCameraSpeed =
		_slrGeoscapeCamera->getValue();

	updateCameraLabels();
}

void Options3DSState::slrBattlescapeCameraChange(Action *)
{
	Options::threeDSBattlescapeCameraSpeed =
		_slrBattlescapeCamera->getValue();

	updateCameraLabels();
}

void Options3DSState::btnInvertGeoXClick(Action *)
{
	Options::threeDSInvertGeoscapeCameraX =
		_btnInvertGeoX->getPressed();
}

void Options3DSState::btnInvertGeoYClick(Action *)
{
	Options::threeDSInvertGeoscapeCameraY =
		_btnInvertGeoY->getPressed();
}

void Options3DSState::btnInvertBattleXClick(Action *)
{
	Options::threeDSInvertBattlescapeCameraX =
		_btnInvertBattleX->getPressed();
}

void Options3DSState::btnInvertBattleYClick(Action *)
{
	Options::threeDSInvertBattlescapeCameraY =
		_btnInvertBattleY->getPressed();
}

void Options3DSState::slrTouchpadSensitivityChange(Action *)
{
	Options::threeDSTouchpadSensitivity =
		_slrTouchpadSensitivity->getValue();

	updateTouchpadLabels();
}

void Options3DSState::slrTouchpadDragThresholdChange(Action *)
{
	Options::threeDSTouchpadDragThreshold =
		_slrTouchpadDragThreshold->getValue();

	updateTouchpadLabels();
}

void Options3DSState::slrTouchpadHoldDelayChange(Action *)
{
	Options::threeDSTouchpadHoldDelay =
		_slrTouchpadHoldDelay->getValue() * 50;

	updateTouchpadLabels();
}

}
