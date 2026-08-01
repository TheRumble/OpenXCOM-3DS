# OpenXCOM-3DS

OpenXCOM-3DS is a native port of OpenXcom Extended for the New Nintendo 3DS
family.

The port uses both screens, the Circle Pad, C-stick, touchscreen, D-pad, and
the additional New 3DS shoulder buttons. It has been tested on a New Nintendo
3DS XL.

## Read This First

You need:

- A New Nintendo 3DS, New Nintendo 3DS XL, or New Nintendo 2DS XL
- A homebrewed system. If you haven't done it yet, this is a good guide: https://3ds.hacks.guide/get-started.html
- The release package
- Your legal asset files from X-COM: UFO Defense, Terror From the
  Deep, or both

Note: The original Nintendo 3DS, Nintendo 3DS XL, and Nintendo 2DS are not supported.

The original commercial X-COM data is not included in this repository or in
release downloads.

Third-party OXCE mods are not directly supported at this time, but you are welcome to test them out.

## Installation

### 1. Install OpenXCOM-3DS

Download the latest release archive from the GitHub Releases page.

Copy the included `OpenXCOM-3DS` application folder into the `3ds` directory
on your SD card.

The resulting application path should resemble:

```text
sdmc:/3ds/OpenXCOM-3DS/openxcom.3dsx
```

Do not launch the game until data for UFO Defense, Terror From the Deep, or
both has been copied using the steps below.

### 2. Create the Game Data Directory

Create:

```text
sdmc:/3ds/OXCE/
```

OpenXCOM-3DS always looks for its game data in this location.

The game automatically creates and uses:

```text
sdmc:/3ds/OXCE/user/
```

The `user` directory stores configuration, saved games, and other
generated files.

### 3. Install UFO Defense Data

Locate the original `XCOM` folder from your legally owned installation.

A Steam installation normally stores it under:

```text
Steam/steamapps/common/XCom UFO Defense/XCOM/
```

Create:

```text
sdmc:/3ds/OXCE/UFO/
```

Copy the **contents** of the original `XCOM` folder into that `UFO` folder.

Correct:

```text
sdmc:/3ds/OXCE/UFO/GEODATA/
sdmc:/3ds/OXCE/UFO/GEOGRAPH/
sdmc:/3ds/OXCE/UFO/MAPS/
sdmc:/3ds/OXCE/UFO/ROUTES/
sdmc:/3ds/OXCE/UFO/SOUND/
sdmc:/3ds/OXCE/UFO/TERRAIN/
sdmc:/3ds/OXCE/UFO/UFOGRAPH/
sdmc:/3ds/OXCE/UFO/UFOINTRO/
sdmc:/3ds/OXCE/UFO/UNITS/
```

Incorrect:

```text
sdmc:/3ds/OXCE/UFO/XCOM/GEODATA/
```

Do not leave an extra `XCOM` directory level.

### 4. Install Terror From the Deep Data

Locate the original `TFD` folder from your legally owned installation.

A Steam installation normally stores it under:

```text
Steam/steamapps/common/X-COM Terror from the Deep/TFD/
```

Create:

```text
sdmc:/3ds/OXCE/TFTD/
```

Copy the **contents** of the original `TFD` folder into that `TFTD` folder.

Do not leave an extra `TFD` directory level.

### 5. Check the Final Layout

A system with both games installed should resemble:

```text
sdmc:/3ds/
├── OpenXCOM-3DS/
│   ├── openxcom.3dsx
│   └── openxcom.smdh
│
└── OXCE/
    ├── UFO/
    │   ├── GEODATA/
    │   ├── GEOGRAPH/
    │   ├── MAPS/
    │   ├── ROUTES/
    │   ├── SOUND/
    │   ├── TERRAIN/
    │   ├── UFOGRAPH/
    │   ├── UFOINTRO/
    │   └── UNITS/
    │
    ├── TFTD/
    │   └── original TFTD data folders
    │
    └── user/
```

Only the `UFO` or `TFTD` folder for a game you own is required.

### 6. Launch the Game

Open the Homebrew Launcher and start OpenXCOM-3DS.

## First Startup

The DOS-style startup screen is normal.

During startup, the screen displays:

- The current loading stage
- A percentage
- A progress bar

These represent real loading progress. Initial loading can take some time, usually a minute or two
especially when the game is scanning original resources for the first time.

OpenXCOM-3DS release builds contain OXCE's 8.6.1 own `common` and `standard` support
resources inside the `.3dsx`. You do not need to download or install those
folders separately.

## Selecting UFO Defense or Terror From the Deep

When both games are installed, open:

```text
Options -> Mods
```

Select one of the master games:

- `UFO: Enemy Unknown / X-Com: UFO Defense`
- `X-Com: Terror From the Deep`

Only one master game can be active at a time.

After changing the master, OXCE reloads its resources. The DOS-style startup
screen may display `Restarting.` during this process. Wait for the progress
display to finish.

## Universal Controls

- **Circle Pad:** Move the mouse cursor
- **A:** Left click or confirm
- **B:** Back or Escape
- **Y:** Right click
- **Start:** Enter or confirm
- **Select:** Switch cursor focus between the top and bottom screens where
  available
- **Touchscreen:** Activate lower-screen controls or use the trackpad area
- **ZL + L + R + ZR:** Toggle the complete on-screen keyboard

During supported intro and cutscene playback, **A** skips the current video.

## Menu Controls

- Use the **D-pad** to move between menu controls.
- Press **A** to activate the selected control.
- Press **B** to go back.
- Use the **Circle Pad** for free cursor movement.
- Touch visible lower-screen controls directly.
- Sliders and lists can be adjusted with their normal cursor actions or
  physical navigation.

## Geoscape Controls

- **Circle Pad:** Move the cursor
- **C-stick:** Rotate or pan the globe
- **D-pad:** Move between lower-screen Geoscape controls while bottom focus is
  active
- **Select:** Switch between top-screen and bottom-screen cursor focus
- **A:** Left click or activate
- **Y:** Right click
- **Touchscreen:** Use lower-screen commands or the context-sensitive trackpad

The lower screen contains Geoscape information and commonly used commands.

## Battlescape Controls

- **Circle Pad:** Move the cursor
- **C-stick:** Pan the Battlescape camera
- **L / R:** Change the current view level
- **ZL:** Use the item in the unit's left hand
- **ZR:** Use the item in the unit's right hand
- **A:** Left click or confirm
- **Y:** Right click
- **X:** Middle mouse in supported Battlescape contexts
- **D-pad:** Move between lower-screen Battlescape controls while bottom focus
  is active
- **Select:** Switch between top-screen and bottom-screen cursor focus
- **Touchscreen:** Activate lower-screen Battlescape controls

The lower screen displays the Battlescape command interface, unit information,
hand items, and other context-sensitive controls.

## Inventory Controls

- **Circle Pad:** Move the cursor
- **A:** Pick up, place, or activate an item using the normal left-click action
- **Y:** Right click
- **X:** Quick-move the item under the cursor
- **D-pad:** Move between lower-screen inventory controls
- **Touchscreen:** Select items and controls directly

## Keyboard Controls

The text-entry keyboard opens automatically when a text field is selected.

- Press **B** to hide the text-entry keyboard without abandoning the field.
- Press **A** on the active text field to reopen it.
- Press **ZL + L + R + ZR** to toggle the complete desktop-style keyboard.
- The complete keyboard can be used for controls that do not fit on the
  physical 3DS buttons.

## 3DS Options

Open the dedicated **3DS** category in the normal Options menu.

### Cursor

Separate cursor-speed settings are available for:

- Menus
- Geoscape
- Battlescape
- Inventory

### Camera

Settings include:

- Geoscape camera speed
- Battlescape camera speed
- Geoscape X-axis inversion
- Geoscape Y-axis inversion
- Battlescape X-axis inversion
- Battlescape Y-axis inversion

### Touch

Settings include:

- Touch sensitivity
- Drag threshold
- Hold delay

## Important Notes

- This port supports only the New Nintendo 3DS hardware family.
- Use unmodified original game files.
- Third-party mods are unsupported.
- Do not install separate OXCE `common` or `standard` folders when using an
  official embedded-assets release.
- Changing the selected master game causes a full resource reload.
- Configuration and saves are stored under:

```text
sdmc:/3ds/OXCE/user/
```

- This release is distributed as a Homebrew Launcher `.3dsx` application.

## Troubleshooting

### The Game Cannot Find UFO Defense

Check that this exists:

```text
sdmc:/3ds/OXCE/UFO/GEODATA/
```

If the path instead contains:

```text
sdmc:/3ds/OXCE/UFO/XCOM/GEODATA/
```

move the contents of `XCOM` up one level.

### The Game Cannot Find Terror From the Deep

Check that the original TFTD data directories are located directly inside:

```text
sdmc:/3ds/OXCE/TFTD/
```

Do not place another `TFD` folder inside it.

### The Game Appears to Pause During Startup

Wait for the current stage to finish. Resource loading is not instantaneous,
and some stages take longer than others.

### The Wrong Game Starts

Open:

```text
Options -> Mods
```

Select the correct UFO Defense or Terror From the Deep master and allow OXCE to
restart.

### A Menu Is Difficult to Control

Use the D-pad to snap between controls or use the Circle Pad for free cursor
movement. Press Select where supported to change which screen currently owns
cursor focus.

## Reporting Problems

When opening an issue, include:

1. Whether you were using UFO Defense or Terror From the Deep
2. The menu, Geoscape screen, Battlescape, or inventory screen involved
3. The exact physical or touchscreen control you used
4. What you expected to happen
5. What actually happened
6. Whether the problem happens every time
7. Whether you were loading an old save or starting a new game
8. A photograph, video, or crash information when available

Do not report problems involving third-party mods unless the same problem also
occurs with the unmodified UFO Defense or Terror From the Deep master.

## Building From Source

Use devkitPro MSYS2 with the Nintendo 3DS development environment installed.

Install the required tools:

```bash
pacman -S --needed git cmake ninja 3ds-dev
```

Clone the repository:

```bash
git clone https://github.com/TheRumble/OpenXCOM-3DS.git
cd OpenXCOM-3DS
```

Configure a release build:

```bash
cmake -S . -B build/3ds -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DEMBED_ASSETS=ON \
    -DDEV_BUILD=OFF
```

Build:

```bash
cmake --build build/3ds -j4
```

The resulting executable is:

```text
build/3ds/src/openxcom.3dsx
```

The software-renderer 3DS build does not require desktop OpenGL. A CMake warning
that OpenGL was not found is expected for this target.

### Testing With 3dslink

Open the Homebrew Launcher receiver on the 3DS, then run:

```bash
3dslink --retries 10 \
    -a YOUR_3DS_IP_ADDRESS \
    build/3ds/src/openxcom.3dsx
```

## Credits and License

OpenXCOM-3DS is based on:

- [OpenXcom](https://openxcom.org/)
- [OpenXcom Extended](https://github.com/MeridianOXC/OpenXcom)

OpenXcom is an open-source recreation of the original X-COM games. The original
commercial game data remains the property of its respective owners and is not
distributed by this project.

The source code retains the upstream GNU General Public License. See
[LICENSE.txt](LICENSE.txt) for the complete license text.
