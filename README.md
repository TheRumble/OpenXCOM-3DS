# OpenXCOM-3DS

OpenXCOM-3DS is a native port of OpenXcom Extended 8.6.1 for the New Nintendo 3DS
family.

## Instructions

You need:

- A New Nintendo 3DS, New Nintendo 3DS XL, or New Nintendo 2DS XL
- A homebrewed system. If you haven't done it yet, this is a good guide: https://3ds.hacks.guide/get-started.html
- The release package
- Your legal asset files from X-COM: UFO Defense, Terror From the
  Deep, or both

Note: The original Nintendo 3DS, Nintendo 3DS XL, and Nintendo 2DS are not supported.

The original commercial X-COM data is not included in this repository or in
release downloads.

OXCE mods should theoretically work but are not directly supported at this time. You are welcome to test them out.

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

## Building From Source

OpenXCOM-3DS can be built from source using the
[devkitPro](https://devkitpro.org/) Nintendo 3DS development toolchain.

This section is written as a complete step-by-step guide. You do not need
previous experience building Nintendo 3DS homebrew.

The build system can produce both release formats:

- a `.3dsx` application for the Homebrew Launcher
- an installable `.cia` application for the Nintendo 3DS HOME Menu

The CIA build also generates the HOME Menu icon, banner, and banner audio from
the assets included in this repository.

### 1. Install devkitPro

OpenXCOM-3DS is built using
[devkitPro](https://devkitpro.org/).

Start with the official:

[devkitPro Getting Started Guide](https://devkitpro.org/wiki/Getting_Started)

Windows users can also find the devkitPro installer here:

[devkitPro Installer](https://github.com/devkitPro/installer)

[devkitPro Installer Releases](https://github.com/devkitPro/installer/releases)

On Windows, this guide assumes you are using the **devkitPro MSYS2** terminal
installed by devkitPro.

> **Important:** Run the commands in this section from the devkitPro MSYS2
> terminal, not Windows Command Prompt or PowerShell.

After installing devkitPro, open the devkitPro MSYS2 terminal.

### 2. Install the Required Packages

First update the devkitPro package database:

```bash
pacman -Syu
```

If `pacman` asks you to close or restart the terminal during an update, do so,
open devkitPro MSYS2 again, and rerun:

```bash
pacman -Syu
```

Then install the tools and Nintendo 3DS development packages used by
OpenXCOM-3DS:

```bash
pacman -S --needed git cmake ninja zip 3ds-dev
```

The main tools used by the build are:

- [Git](https://git-scm.com/) - downloads and manages the source code
- [CMake](https://cmake.org/) - configures the project
- [Ninja](https://ninja-build.org/) - performs the actual compilation
- [devkitPro](https://devkitpro.org/) / devkitARM - provides the Nintendo 3DS
  compiler and libraries
- [bannertool](https://github.com/diasurgical/bannertool) - creates Nintendo
  3DS icon and banner metadata
- [makerom / Project_CTR](https://github.com/3DSGuy/Project_CTR) - creates the
  installable CIA

`bannertool` and `makerom` are used only for the CIA packaging process.

### 3. Check That the Tools Are Available

Before downloading the source, it is a good idea to make sure the important
programs can be found.

Run:

```bash
command -v git
command -v cmake
command -v ninja
command -v arm-none-eabi-g++
command -v bannertool
command -v makerom
```

Each command should print a path instead of returning nothing.

For example, devkitPro tools will normally be somewhere under:

```text
/opt/devkitpro/
```

You can also check the compiler with:

```bash
arm-none-eabi-g++ --version
```

If `arm-none-eabi-g++`, `bannertool`, or `makerom` cannot be found, make sure
you are using the devkitPro MSYS2 terminal and that the devkitPro installation
completed successfully.

### 4. Clone OpenXCOM-3DS

Choose a directory where you want to keep the source code.

Then clone the repository:

```bash
git clone https://github.com/TheRumble/OpenXCOM-3DS.git
```

Enter the new directory:

```bash
cd OpenXCOM-3DS
```

You should now be in the root of the OpenXCOM-3DS source tree.

You can confirm this with:

```bash
pwd
```

and:

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

### 5. Configure the Nintendo 3DS Release Build

CMake first needs to create a Nintendo 3DS build directory.

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

This does not compile the game yet. It configures the project and creates the
Ninja build files inside:

```text
build/3ds/
```

The options used above mean:

- `-S .` - use the current directory as the source directory
- `-B build/3ds` - place generated build files in `build/3ds`
- `-G Ninja` - use the Ninja build system
- `CMAKE_TOOLCHAIN_FILE` - use the devkitPro Nintendo 3DS toolchain
- `CMAKE_BUILD_TYPE=Release` - create an optimized release build
- `EMBED_ASSETS=ON` - embed the OpenXcom/OXCE support assets required by the
  3DS port
- `DEV_BUILD=OFF` - disable development-build behavior
- `BUILD_PACKAGE=OFF` - disable the upstream desktop packaging system
- `BUILD_DOCUMENTATION=OFF` - do not build the upstream API documentation
- `BUILD_3DS_CIA=ON` - enable the additional CIA packaging target

The original commercial X-COM files are **not** embedded by this process.

A successful configuration should include:

```text
Nintendo 3DS target: desktop OpenGL support disabled.
```

Because CIA building is enabled, it should also include:

```text
Nintendo 3DS CIA package target enabled: openxcom_cia
```

The Nintendo 3DS port uses its software rendering path and does not require the
desktop OpenGL renderer.

### 6. Build the `.3dsx`

Compile OpenXCOM-3DS with:

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

You can verify that it exists with:

```bash
ls -lh build/3ds/src/openxcom.3dsx
```

### 7. Build the `.cia`

The CIA build is a separate packaging target.

After the normal build succeeds, run:

```bash
cmake --build build/3ds --target openxcom_cia -j4
```

This performs the additional steps required to create a Nintendo 3DS HOME Menu
application.

The process automatically:

1. creates the OpenXCOM-3DS SMDH application metadata and icon
2. creates the HOME Menu banner
3. includes the banner audio
4. creates a stripped copy of the OpenXCOM-3DS ELF
5. packages the application using
   [makerom](https://github.com/3DSGuy/Project_CTR)

The generated files are placed in:

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

The main CIA file is:

```text
build/3ds/package/3ds/OpenXCOM-3DS.cia
```

You can verify it with:

```bash
ls -lh build/3ds/package/3ds/OpenXCOM-3DS.cia
```

### 8. Build Both Release Formats

For convenience, the complete configure-and-build process is:

```bash
cmake -S . -B build/3ds -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DEMBED_ASSETS=ON \
    -DDEV_BUILD=OFF \
    -DBUILD_PACKAGE=OFF \
    -DBUILD_DOCUMENTATION=OFF \
    -DBUILD_3DS_CIA=ON

cmake --build build/3ds -j4

cmake --build build/3ds --target openxcom_cia -j4
```

After all three commands finish successfully, the two main outputs are:

```text
build/3ds/src/openxcom.3dsx
build/3ds/package/3ds/OpenXCOM-3DS.cia
```

The generated SMDH is:

```text
build/3ds/package/3ds/openxcom.smdh
```

### 9. Preparing the `.3dsx` for the Homebrew Launcher

For a normal Homebrew Launcher installation, create a folder named:

```text
OpenXCOM-3DS
```

Place the `.3dsx` and SMDH inside it:

```text
OpenXCOM-3DS/
├── openxcom.3dsx
└── openxcom.smdh
```

The files can be copied from:

```text
build/3ds/src/openxcom.3dsx
build/3ds/package/3ds/openxcom.smdh
```

Then place the `OpenXCOM-3DS` folder inside the `3ds` directory on the SD card:

```text
sdmc:/3ds/OpenXCOM-3DS/
├── openxcom.3dsx
└── openxcom.smdh
```

The `.3dsx` contains the actual application.

The `.smdh` contains Homebrew Launcher metadata such as the application name
and icon.

### 10. Installing a Locally Built CIA

The generated CIA is:

```text
build/3ds/package/3ds/OpenXCOM-3DS.cia
```

Copy it to your Nintendo 3DS SD card.

It can then be installed using a compatible title installer such as
[FBI](https://github.com/Steveice10/FBI).

After installation, OpenXCOM-3DS appears directly on the Nintendo 3DS HOME
Menu with its icon, banner, and banner audio.

The CIA does **not** contain UFO Defense or Terror From the Deep.

The commercial game data must still be installed separately as described in
the main installation section of this README.

Both the `.3dsx` and `.cia` versions use:

```text
sdmc:/3ds/OXCE/
```

for game data.

Configuration and save files are stored under:

```text
sdmc:/3ds/OXCE/user/
```

### 11. Testing the `.3dsx` With 3dslink

During development, rebuilding and manually copying the `.3dsx` to the SD card
every time can be inconvenient.

[3dslink](https://github.com/devkitPro/3dslink) can send a `.3dsx` directly
from the computer to a Nintendo 3DS over the local network.

First, start the network loader/receiver from the Homebrew Launcher on the
Nintendo 3DS.

Find the IP address of your 3DS.

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

`3dslink` sends and launches the `.3dsx`. It does **not** install or launch a
CIA.

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

Ninja will determine which files need to be rebuilt.

Rerun the full CMake configuration command if you:

- delete the build directory
- change important CMake options
- change toolchain settings
- update the project in a way that requires CMake to regenerate the build

### 13. Creating a Completely Fresh Build

If a build begins behaving strangely, or you want to make sure no old build
files are being reused, delete the entire build directory:

```bash
rm -rf build/3ds
```

Then configure it again:

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

### Troubleshooting

#### `cmake: command not found`

Make sure you installed the required packages:

```bash
pacman -S --needed cmake ninja
```

Also make sure you are using the devkitPro MSYS2 terminal.

#### `arm-none-eabi-g++: command not found`

The Nintendo 3DS compiler is missing or devkitPro is not configured correctly.

Install or repair the Nintendo 3DS development environment:

```bash
pacman -S --needed 3ds-dev
```

Then close and reopen the devkitPro MSYS2 terminal.

#### CMake cannot find `/opt/devkitpro/cmake/3DS.cmake`

Check that devkitPro is installed and that this file exists:

```bash
ls -l /opt/devkitpro/cmake/3DS.cmake
```

If it does not exist, check your devkitPro installation using the
[official Getting Started guide](https://devkitpro.org/wiki/Getting_Started).

#### `bannertool` cannot be found

Check:

```bash
command -v bannertool
```

The CIA target requires `bannertool` to create its Nintendo 3DS metadata and
banner.

See:

[bannertool on GitHub](https://github.com/diasurgical/bannertool)

#### `makerom` cannot be found

Check:

```bash
command -v makerom
```

The CIA target requires `makerom`.

See:

[Project_CTR / makerom](https://github.com/3DSGuy/Project_CTR)

[Project_CTR Releases](https://github.com/3DSGuy/Project_CTR/releases)

#### The `.3dsx` built successfully but there is no CIA

The CIA is not generated by the normal build command alone.

Make sure the project was configured with:

```text
-DBUILD_3DS_CIA=ON
```

Then run:

```bash
cmake --build build/3ds --target openxcom_cia -j4
```

#### I changed something but Ninja says `no work to do`

This means Ninja does not believe any input used by that target has changed.

For a completely fresh rebuild, remove the build directory and configure it
again:

```bash
rm -rf build/3ds
```

Then repeat the configuration and build steps above.

#### My CIA has a different SHA-256 hash after rebuilding it

This does not necessarily mean the application itself changed.

`makerom` generates some CIA container metadata when creating the package, so
separate CIA packaging runs can produce different binary CIA files even when
the executable and packaging inputs are otherwise identical.

When testing changes during development, compare the source revision and build
inputs rather than assuming two independently generated CIA files must always
have identical hashes.

### Additional Development Resources

Useful references for Nintendo 3DS development:

- [devkitPro](https://devkitpro.org/)
- [devkitPro Getting Started](https://devkitpro.org/wiki/Getting_Started)
- [devkitPro Installer](https://github.com/devkitPro/installer)
- [devkitPro 3DS Examples](https://github.com/devkitPro/3ds-examples)
- [devkitPro 3dslink](https://github.com/devkitPro/3dslink)
- [CMake](https://cmake.org/)
- [Ninja](https://ninja-build.org/)
- [Git](https://git-scm.com/)
- [bannertool](https://github.com/diasurgical/bannertool)
- [makerom / Project_CTR](https://github.com/3DSGuy/Project_CTR)

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
