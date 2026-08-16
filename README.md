# OpenXCOM-3DS

OpenXCOM-3DS is a native port of OpenXcom Extended 8.6.1 for the New Nintendo 3DS family.

## Features

- Supports both X-COM: UFO Defense and X-COM: Terror From the Deep
- Runs using an optimized software rendering path
- Uses the top screen for gameplay and the bottom touchscreen for 3DS-specific controls
- Touchscreen interface designed around the original OpenXcom/OXCE UI
- Support for the Circle Pad, C-Stick, D-Pad, face buttons, and all four shoulder buttons
- C-Stick camera panning on the Geoscape and the Battlescape
- D-Pad navigation for bottom-screen controls
- Quick-access controls for:
  - inventory
  - left and right hand items
  - elevation levels
  - right-click actions
  - middle-click actions
  - confirmation and back/cancel
- On-screen keyboard support when text entry is required
- Native OXCE save and load support
- Native OXCE options and configuration support
- Optimized Geoscape and Battlescape rendering for 3DS hardware
- Available as:
  - `.3dsx` for the Homebrew Launcher
  - `.cia` for installation directly to the HOME Menu
- CIA version includes a nifty banner and audio stinger
- Both versions use the same game-data and save locations, making it easy to switch between them

> **Note:** OpenXCOM-3DS does not include the original commercial X-COM game
> files. You must provide your own legally obtained UFO Defense and/or Terror
> From the Deep data.

## Instructions

You need:

- A New Nintendo 3DS, New Nintendo 3DS XL, or New Nintendo 2DS XL
- A homebrewed system. If you haven't done it yet, this is a good guide: https://3ds.hacks.guide/get-started.html
- The release package (or the files you built yourself)
- Your legal asset files from X-COM: UFO Defense, Terror From the
  Deep, or both

Note: The original Nintendo 3DS, Nintendo 3DS XL, and Nintendo 2DS are not supported. They simply don't have the power to run the game as it stands, since it
currently runs entirely off of the CPU.

OXCE mods should theoretically work but are not directly supported at this time. You are welcome to test them out.

## Installation

### 1. Install OpenXCOM-3DS

Download the latest release archive from the Releases page.

OpenXCOM-3DS is provided in two formats.

#### Homebrew Launcher (`.3dsx`)

Copy the included `OXCE` folder into the `3ds` directory on your SD card.
The same `OXCE` folder is also used for game data, configuration, and saved games.

The resulting application path should resemble:

```text
sdmc:/3ds/OXCE/openxcom.3dsx
```

#### HOME Menu (`.cia`)

Copy `OpenXCOM-3DS.cia` to your SD card and install it using a compatible title
installer such as [FBI](https://github.com/Steveice10/FBI).

After installation, OpenXCOM-3DS will appear directly on the HOME Menu.

Both versions use the same directory on the SD card:

```text
sdmc:/3ds/OXCE/
```

For the Homebrew Launcher version, this directory also contains
`openxcom.3dsx` and `openxcom.smdh`.

For the CIA version, the application itself is installed to the HOME Menu,
while game data and user files remain under `sdmc:/3ds/OXCE/`.

Do not launch the game until data for UFO Defense, Terror From the Deep, or
both has been copied using the steps below.

### 2. Prepare the Game Data Directory

OpenXCOM-3DS always uses:

```text
sdmc:/3ds/OXCE/
```

If you installed the Homebrew Launcher version, this folder was already
created when you copied the included `OXCE` folder to your SD card.

If you installed only the CIA version, create the `OXCE` folder inside
`sdmc:/3ds/` if it does not already exist.

The game automatically creates and uses:

```text
sdmc:/3ds/OXCE/user/
```

The `user` directory stores configuration, saved games, and other
generated files.

### 3. Install UFO Defense Data

Locate the original `XCOM` folder from your installation.

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

### 4. Install TFTD Data (optional)

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

A Homebrew Launcher installation with both games installed should resemble:

```text
sdmc:/3ds/OXCE/
├── openxcom.3dsx
├── openxcom.smdh
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

If you use only the CIA version, `openxcom.3dsx` and `openxcom.smdh` are not
required on the SD card. The `UFO`, `TFTD`, and `user` directories remain in
the same `sdmc:/3ds/OXCE/` location.

Only the `UFO` or `TFTD` folder for a game you own is required.

### 6. Launch the Game

- **Homebrew Launcher:** Open the Homebrew Launcher and start OpenXCOM-3DS.
- **CIA:** Start OpenXCOM-3DS directly from the Nintendo 3DS HOME Menu.

## Selecting UFO Defense or Terror From the Deep

When both games are installed, open:

```text
Options -> Mods
```

From here you can select your active game (or mod). If you've installed both games, it should show:

- `UFO: Enemy Unknown / X-Com: UFO Defense`
- `X-Com: Terror From the Deep`

## Universal Controls

- **Circle Pad:** Move the mouse cursor
- **A:** Left click or confirm
- **B:** Back or Escape
- **Y:** Right click
- **Start:** Enter or confirm
- **Select:** Switch cursor focus between the top and bottom screens where
  available
- **Touchscreen:** Activate lower-screen controls or use the trackpad
- **ZL + L + R + ZR:** Toggle the on-screen keyboard

During intro and cutscenes, **A** skips the current video.

## Menu Controls

- Use the **D-pad** to snap between menu buttons.
- Press **A** to activate the selected button.
- Press **B** to go back.
- Use the **Circle Pad** for free cursor movement.
- If applicable, you can tap visible lower-screen buttons directly.
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
- **Touchscreen:** Use lower-screen buttons or the trackpad

The lower screen contains Geoscape information and commonly used commands.

## Battlescape Controls

- **Circle Pad:** Move the cursor
- **C-stick:** Pan the Battlescape camera
- **L / R:** Change the current view level
- **ZL:** Use the item in the unit's left hand
- **ZR:** Use the item in the unit's right hand
- **A:** Left click or confirm
- **Y:** Right click
- **X:** Middle mouse in supported Battlescape screens
- **D-pad:** Move between lower-screen Battlescape controls while bottom focus
  is active
- **Select:** Switch between top-screen and bottom-screen cursor focus
- **Touchscreen:** Activate Battlescape buttons, and use the trackpad

The lower screen displays the Battlescape command interface, unit information,
hand items, and other context-sensitive controls.

## Inventory Controls

- **Circle Pad:** Move the cursor
- **A:** Pick up, place, or activate an item using the normal left-click action
- **Y:** Right click
- **X:** Quick-move the item under the cursor
- **D-pad:** Move between lower-screen inventory controls
- **Touchscreen:** Select items with the trackpad, and tap the buttons directly

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

Settings for the C-Stick camera include:

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

## User Files

- Configuration and saves are stored under:

```text
sdmc:/3ds/OXCE/user/
```

## Reporting Problems

When opening an issue, include:

1. Whether you were playing UFO Defense or Terror From the Deep
2. The menu, Geoscape screen, Battlescape, or other screen involved
3. The exact physical or touchscreen control you used
4. What you expected to happen (if applicable)
5. What actually happened
6. Whether the problem happens every time
7. A photograph, video, or crash information when available

Do not report problems involving third-party mods unless the same problem also
occurs with the unmodified UFO Defense or Terror From the Deep master.

The game should be pretty stable, however there might be Luma crashes if you exit to the 3DS menu at an inopportune time, but I haven't seen any thing
like 'shooting this specific muton with the laser rifle causes the game to crash' or whatever. If it does happen, send the dump and a report.

## Building From Source

Note: this is a guide for Windows only, currently.

OpenXCOM-3DS can be built from source using the
[devkitPro](https://devkitpro.org/) Nintendo 3DS development toolchain.

The build system can produce both release formats:

- a `.3dsx` application for the Homebrew Launcher
- an installable `.cia` application for the Nintendo 3DS HOME Menu

The CIA build also generates the HOME Menu icon, banner, and banner audio from
assets included in this repository.

---

### 1. Install devkitPro

OpenXCOM-3DS is built using
[devkitPro](https://devkitpro.org/).

Useful links:

- [devkitPro Getting Started Guide](https://devkitpro.org/wiki/Getting_Started)
- [devkitPro Installer](https://github.com/devkitPro/installer)
- [devkitPro Installer Releases](https://github.com/devkitPro/installer/releases)

Install devkitPro using the official installer. Make sure you select the option for the 3DS toolkit stuff. Should be auto selected.

---

### 2. MinGW64

Please run all commands in this guide from the
**devkitPro MinGW64** terminal installed with devkitPro.

The correct terminal should normally show `MINGW64` in the prompt.

For example:

```text
username@COMPUTER MINGW64 ~
```

Check that the terminal can see your devkitPro installation:

```bash
echo "$DEVKITPRO"
```

This should normally print:

```text
/opt/devkitpro
```

You can also confirm that the installation directory exists:

```bash
ls /opt/devkitpro
```

---

### 3. Install the Required Packages

First update the devkitPro package database:

```bash
pacman -Syu
```

If `pacman` asks you to close or restart the terminal during the update, close
it, reopen the **devkitPro MinGW64** terminal, and run:

```bash
pacman -Syu
```

again.

Then install the tools and Nintendo 3DS development packages required by
OpenXCOM-3DS:

```bash
pacman -S --needed git cmake ninja zip 3ds-dev
```

The main tools used by the build are:

- [Git](https://git-scm.com/) - downloads and manages the source code
- [CMake](https://cmake.org/) - configures the project
- [Ninja](https://ninja-build.org/) - compiles the project
- [devkitPro](https://devkitpro.org/) / devkitARM - provides the Nintendo 3DS
  compiler and libraries
- [bannertool](https://github.com/diasurgical/bannertool) - creates Nintendo
  3DS application metadata, icons, and banners
- [makerom / Project_CTR](https://github.com/3DSGuy/Project_CTR) - creates the
  installable `.cia`

`bannertool` and `makerom` are used during CIA packaging.

---

### 4. Verify the Build Tools

Before continuing, make sure the main build and packaging tools can be found:

```bash
command -v git
command -v cmake
command -v ninja
command -v bannertool
command -v makerom
```

Each command should print a path.

If one of these commands does not print a path, make sure you are using the
**devkitPro MinGW64** terminal and rerun:

```bash
pacman -S --needed git cmake ninja zip 3ds-dev
```

The CMake configuration step below will verify that the Nintendo 3DS toolchain
itself is installed and usable.

---

### 5. Clone OpenXCOM-3DS

In the **devkitPro MinGW64** terminal, run:

```bash
git clone https://github.com/TheRumble/OpenXCOM-3DS.git
```

This downloads the OpenXCOM-3DS source code into a new folder named:

```text
OpenXCOM-3DS
```

By default, this folder will be created inside your devkitPro MinGW64 home
directory.

You can see the exact location of that directory by running:

```bash
pwd
```

Then enter the project folder:

```bash
cd OpenXCOM-3DS
```

You are now inside the OpenXCOM-3DS source directory and are ready to configure
the build.

You can list the repository contents with:

```bash
ls
```

You should see files and directories such as:

```text
CMakeLists.txt
LICENSE.txt
README.md
bin/
libs/
res/
src/
```

---

### 6. Configure the build

Before compiling the game, CMake needs to configure a Nintendo 3DS build
directory.

Run this command exactly as shown:

```bash
cmake -S . -B build/3ds -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DEMBED_ASSETS=ON \
    -DDEV_BUILD=OFF \
    -DBUILD_PACKAGE=OFF \
    -DBUILD_DOCUMENTATION=OFF \
    -DBUILD_3DS_CIA=ON
```

This does **not** compile the game yet.

It configures the project and creates the Ninja build files inside:

```text
build/3ds/
```

The options mean:

- `-S .` - use the current directory as the source directory
- `-B build/3ds` - place generated build files in `build/3ds`
- `-G Ninja` - use Ninja as the build system
- `CMAKE_TOOLCHAIN_FILE` - use the devkitPro Nintendo 3DS toolchain
- `CMAKE_BUILD_TYPE=Release` - create an optimized release build
- `EMBED_ASSETS=ON` - embed the OpenXcom/OXCE support assets required by the
  3DS port
- `DEV_BUILD=OFF` - disable development-build behavior
- `BUILD_PACKAGE=OFF` - disable the upstream desktop packaging system
- `BUILD_DOCUMENTATION=OFF` - do not build the upstream API documentation
- `BUILD_3DS_CIA=ON` - enable the additional CIA packaging target

A successful configuration should include:

```text
Nintendo 3DS target: desktop OpenGL support disabled.
```

Because CIA building is enabled, it should also include:

```text
Nintendo 3DS CIA package target enabled: openxcom_cia
```

Note: this port uses its own software rendering path and does not require the
desktop OpenGL renderer.

---

### 7. Build the `.3dsx`

Compile the .3dsx with:

```bash
cmake --build build/3ds -j4
```

The first build can take a while because the entire project has to be compiled.

Later builds are normally much faster because
[Ninja](https://ninja-build.org/) recompiles only files that have changed.

After a successful build, the Homebrew Launcher executable will be:

```text
build/3ds/src/openxcom.3dsx
```

Verify that it exists:

```bash
ls -lh build/3ds/src/openxcom.3dsx
```

#### Build fails while generating `common.zip` or `standard.zip`

Current OpenXCOM-3DS source serializes embedded asset ZIP generation to avoid
an intermittent parallel-build failure observed with devkitPro/MSYS2.

If you encounter an error similar to:

```text
zip I/O error: No such file or directory
zip error: Could not create output file (was replacing the original zip file)
ninja: build stopped: subcommand failed.
```

generate the asset ZIPs serially:

```bash
cmake --build build/3ds --target zips -j1
```

Then resume the normal parallel build:

```bash
cmake --build build/3ds -j4
```

You do not need to delete the build directory or reconfigure the project.

---

### 8. Build the `.cia`

The CIA is created using a separate packaging target.

After the normal build succeeds, run:

```bash
cmake --build build/3ds --target openxcom_cia -j4
```

This automatically:

1. creates the OpenXCOM-3DS SMDH application metadata and icon
2. creates the HOME Menu banner
3. includes the banner audio
4. creates a stripped copy of the OpenXCOM-3DS ELF
5. packages the application using
   [makerom](https://github.com/3DSGuy/Project_CTR)

The generated CIA files are placed in:

```text
build/3ds/package/3ds/
```

You should see:

```text
build/3ds/package/3ds/
├── OpenXCOM-3DS.cia
├── openxcom-banner.bnr
├── openxcom-stripped.elf
└── openxcom.smdh
```

The installable CIA is:

```text
build/3ds/package/3ds/OpenXCOM-3DS.cia
```

Verify that it exists:

```bash
ls -lh build/3ds/package/3ds/OpenXCOM-3DS.cia
```

---

### 9. Install the Built `.3dsx`

After the build finishes, the Homebrew Launcher files are located at:

```text
build/3ds/src/openxcom.3dsx
build/3ds/package/3ds/openxcom.smdh
```

OpenXCOM-3DS uses the following directory on the SD card:

```text
sdmc:/3ds/OXCE/
```

Create that folder if it does not already exist, then copy both files into it.

Your SD card should contain:

```text
sdmc:/3ds/OXCE/
├── openxcom.3dsx
└── openxcom.smdh
```

If you have already installed your UFO Defense or Terror From the Deep data,
keep it in the same `OXCE` directory:

```text
sdmc:/3ds/OXCE/
├── openxcom.3dsx
├── openxcom.smdh
├── UFO/
├── TFTD/
└── user/
```

Only the `UFO` or `TFTD` directory for a game you own is required.

Once the files are copied to the SD card, open the Homebrew Launcher and select
**OpenXCOM-3DS**.

---

### 10. Install the Built CIA

The generated CIA is:

```text
build/3ds/package/3ds/OpenXCOM-3DS.cia
```

Copy it to your Nintendo 3DS SD card.

It can then be installed using a compatible title installer such as
[FBI](https://github.com/Steveice10/FBI).

After installation, OpenXCOM-3DS appears directly on the Nintendo 3DS HOME
Menu with its icon, banner, and banner audio.

Both the `.3dsx` and `.cia` versions use:

```text
sdmc:/3ds/OXCE/
```

for game data.

Configuration files and saved games are stored under:

```text
sdmc:/3ds/OXCE/user/
```

---

### 11. Test the `.3dsx` With 3dslink

During development, repeatedly copying the `.3dsx` to the SD card was pretty inconvenient.

[3dslink](https://github.com/devkitPro/3dslink) can send a `.3dsx` directly
from your computer to a Nintendo 3DS over the local network.

First, start the network loader or receiver from the Homebrew Launcher on your
Nintendo 3DS and find the IP address of the system.

Then, from the OpenXCOM-3DS source directory, run:

```bash
3dslink --retries 10 \
    -a YOUR_3DS_IP_ADDRESS \
    build/3ds/src/openxcom.3dsx
```

For example:

```bash
3dslink --retries 10 \
    -a 192.168.1.100 \
    build/3ds/src/openxcom.3dsx
```

Replace `192.168.1.100` with the actual IP address of your Nintendo 3DS.

`3dslink` sends and launches the `.3dsx`.

It cannot install or launch a CIA.

---

### 12. Rebuilding After Making Changes

You do not normally need to rerun the full CMake configuration every time you
edit the source code.

After changing source files, rebuild the `.3dsx` with:

```bash
cmake --build build/3ds -j4
```

Then regenerate the CIA with:

```bash
cmake --build build/3ds --target openxcom_cia -j4
```

Ninja will determine which files actually need to be rebuilt.

Rerun the full CMake configuration command if you:

- delete the build directory
- change important CMake options
- change toolchain settings
- update the project in a way that requires CMake to regenerate the build

---

### 13. Creating a Fresh Build

If the build begins behaving strangely, or you want to guarantee that no old
build files are being reused, delete the entire build directory:

```bash
rm -rf build/3ds
```

> **Warning:** Make sure you are in the OpenXCOM-3DS source directory before
> running this command. It deletes only the generated `build/3ds` directory.

Configure the project again:

```bash
cmake -S . -B build/3ds -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DEMBED_ASSETS=ON \
    -DDEV_BUILD=OFF \
    -DBUILD_PACKAGE=OFF \
    -DBUILD_DOCUMENTATION=OFF \
    -DBUILD_3DS_CIA=ON
```

Build the `.3dsx`:

```bash
cmake --build build/3ds -j4
```

Then build the CIA:

```bash
cmake --build build/3ds --target openxcom_cia -j4
```

After both commands succeed, verify the outputs:

```bash
ls -lh \
    build/3ds/src/openxcom.3dsx \
    build/3ds/package/3ds/OpenXCOM-3DS.cia
```

---

## AI Disclosure

I created this port using OpenAI's ChatGPT. I'm not a coder, I mean I probably know more about it than somebody who's never done it before, but not a whole lot. I'm just a guy who wanted to play X-COM on my new 3DS. Everything I've done I've tested or had some test extensively, I 'built' this so I and others could have something to enjoy, I'm not interested in shoveling out broken slop for the sake of it. If you don't want to play this because of the AI use, I get it, no hard feelings, just please don't scream in my face about it. 

---

## Credits and License

OpenXCOM-3DS is based on:

- [OpenXcom](https://openxcom.org/)
- [OpenXcom Extended](https://github.com/MeridianOXC/OpenXcom)

OpenXcom is an open-source recreation of the original X-COM games. The original
commercial game data remains the property of its respective owners and is not
distributed by this project.

The source code retains the upstream GNU General Public License. See
[LICENSE.txt](LICENSE.txt) for the complete license text.
