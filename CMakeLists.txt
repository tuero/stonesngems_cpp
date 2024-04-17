cmake_minimum_required (VERSION 3.13)

project(stonesngems)

# Sources
set(STONESNGEMS_SOURCES
    src/definitions.h
    src/stonesngems_base.cpp 
    src/stonesngems_base.h 
    src/util.cpp 
    src/util.h
)

# Build library
add_library(stonesngems STATIC ${STONESNGEMS_SOURCES})
target_compile_features(stonesngems PUBLIC cxx_std_20)
target_include_directories(stonesngems PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_include_directories(stonesngems PUBLIC ${PROJECT_SOURCE_DIR}/include/libnop/include)

# Build tests
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(BUILD_TESTS "Build the unit tests" OFF)
    if (${BUILD_TESTS})
        enable_testing()
        add_subdirectory(test)
    endif()
endif()
