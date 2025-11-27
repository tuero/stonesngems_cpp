# stonesngems_cpp 
A simple C++ implementation of Boulderdash/Emerald Mine style games.

## Include to Your Project: CMake

### FetchContent
```shell
include(FetchContent)
# ...
FetchContent_Declare(stonesngems
    GIT_REPOSITORY https://github.com/tuero/stonesngems_cpp.git
    GIT_TAG master
)

# make available
FetchContent_MakeAvailable(stonesngems)
link_libraries(stonesngems)
```

### Git Submodules
```shell
# assumes project is cloned into external/stonesngems_cpp
add_subdirectory(external/stonesngems_cpp)
link_libraries(stonesngems)
```

## Usage
A Game state is initialized using a `GameParameters` struct, which defines the properties of the game. 
All properties are given default values to match the behaviour of the original Boudlerdash game.
For most usecases, you just want to set the `game_board_str` to the string representation of the level (see below). 
```cpp
std::string board_str = "...";
GameParameters params = kDefaultGameParams;
params["game_board_str"] = GameParameter(board_str);
RNDGameState state(params);
```

See `GameParameters kDefaultGameParams` in `src/stonesngems_base.h` for the expected values, their meaning, and default values.
There are some additional game parameters which change the gameplay from the original which may be useful for certain applications:
- `gravity`: Flag to disable the gravity
- `disable_explosions`: Flag to disable explosions
- `butterfly_explosion_ver`: A `ButterflyExplosionVersion` value which can either cause butterflys to explode (default) or instantly change to their swap element
- `butterfly_move_ver`: A `ButterflyMoveVersion` value which can either cause butterflys to have a frame delay when chaning directions (default) or change directions and moves along the new direction in the same frame

## Level Format
Levels are expected to be formatted as `|` delimited strings, where the first 2 entries are the rows/columns of the level,
the third entry is the maximum number of time steps before the game is over,
the fourth entry is the number of gems requires to open the exit,
then the following `rows * cols` entries are the element ID (see `HiddenCellType` in `definitions.h`).

The levels of the original game in the correct format can be found in `bd_levels/bd_levels.txt`.

## Notice
The image tile assets under `/tiles/` are taken from [Rocks'n'Diamonds](https://www.artsoft.org/). 
A copy of the license for those materials can be found alongside the assets.
