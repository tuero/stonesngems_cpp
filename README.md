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

## Level Format
Levels are expected to be formatted as `|` delimited strings, where the first 2 entries are the rows/columns of the level,
the third entry is the maximum number of time steps before the game is over,
the fourth entry is the number of gems requires to open the exit,
then the following `rows * cols` entries are the element ID (see `HiddenCellType` in `definitions.h`).

## Notice
The image tile assets under `/tiles/` are taken from [Rocks'n'Diamonds](https://www.artsoft.org/). 
A copy of the license for those materials can be found alongside the assets.
