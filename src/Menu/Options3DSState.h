#pragma once

#include "OptionsBaseState.h"

namespace OpenXcom
{

class Slider;
class Text;
class TextButton;
class ToggleTextButton;

/**
 * Nintendo 3DS-specific options.
 */
class Options3DSState : public OptionsBaseState
{
private:
	TextButton *_btnCursor;
	TextButton *_btnCamera;
	TextButton *_btnTouchpad;
	TextButton *_pageGroup;


	Text *_txtMenuCursor;
	Text *_txtGeoscapeCursor;
	Text *_txtBattlescapeCursor;
	Text *_txtInventoryCursor;

	Slider *_slrMenuCursor;
	Slider *_slrGeoscapeCursor;
	Slider *_slrBattlescapeCursor;
	Slider *_slrInventoryCursor;

	Text *_txtGeoscapeCamera;
	Text *_txtBattlescapeCamera;

	Slider *_slrGeoscapeCamera;
	Slider *_slrBattlescapeCamera;

	ToggleTextButton *_btnInvertGeoX;
	ToggleTextButton *_btnInvertGeoY;
	ToggleTextButton *_btnInvertBattleX;
	ToggleTextButton *_btnInvertBattleY;

	Text *_txtTouchpadSensitivity;
	Text *_txtTouchpadDragThreshold;
	Text *_txtTouchpadHoldDelay;

	Slider *_slrTouchpadSensitivity;
	Slider *_slrTouchpadDragThreshold;
	Slider *_slrTouchpadHoldDelay;


	void setCursorControlsVisible(bool visible);
	void setCameraControlsVisible(bool visible);
	void setTouchpadControlsVisible(bool visible);

	void updateCursorLabels();
	void updateCameraLabels();
	void updateTouchpadLabels();

	void showCursorPage();
	void showCameraPage();
	void showTouchpadPage();

public:
	explicit Options3DSState(OptionsOrigin origin);
	~Options3DSState();

	void btnPagePress(Action *action);

	void slrMenuCursorChange(Action *action);
	void slrGeoscapeCursorChange(Action *action);
	void slrBattlescapeCursorChange(Action *action);
	void slrInventoryCursorChange(Action *action);

	void slrGeoscapeCameraChange(Action *action);
	void slrBattlescapeCameraChange(Action *action);

	void btnInvertGeoXClick(Action *action);
	void btnInvertGeoYClick(Action *action);
	void btnInvertBattleXClick(Action *action);
	void btnInvertBattleYClick(Action *action);

	void slrTouchpadSensitivityChange(Action *action);
	void slrTouchpadDragThresholdChange(Action *action);
	void slrTouchpadHoldDelayChange(Action *action);

};

}
