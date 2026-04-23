cmake_minimum_required(VERSION 3.9)
project(MacLumon)

# Build with:
#   cmake -DCMAKE_TOOLCHAIN_FILE=~/.retro68-deps/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake ..
#   make

list(APPEND REZ_INCLUDE_PATH "${RETRO68_ROOT}/RIncludes")

# Classic Mac OS code uses old-style names (helvetica, times, etc.)
add_compile_definitions(OLDROUTINENAMES=1)

add_application(MacLumon
    CREATOR LuMN
    MAKEAPPL_ARGS --copy ${CMAKE_CURRENT_SOURCE_DIR}/Lumon.rsrc.as
    FILES
        Lumon.cpp
        WorkMode.cpp
        IdleMode.cpp
        Logo.cpp
)
