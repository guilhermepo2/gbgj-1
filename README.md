# gueepo2D Sample Project

## Building
0. Download and install [https://cmake.org/](CMake) if you don't have it already.
1. Download the repository `git clone --recursive https://github.com/guilhermepo2/gueepo2D-sample.git`
2. Run CMake `cmake -Bbuild .`, or run it with VSCode or a GUI tool. You can download and manually set `SDL2_PATH` on `CMakeLists.txt` on the `src/lib/gueepo2D/gueepo2D/engine/` folder, if that's not defined, CMake will download SDL2 2.0.18 automatically.
3. Open `gueepo2D-sample.sln` on the `build/` folder.
