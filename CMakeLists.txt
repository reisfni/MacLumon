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

# ---------------------------------------------------------------------------
# After Dark screen saver module (file type ADGM, code resource ADgm)
# ---------------------------------------------------------------------------

add_executable(LumonAD
    AfterDark.cpp
    WorkMode.cpp
    IdleMode.cpp
    Logo.cpp
)

if(TARGET retrocrt)
    add_dependencies(LumonAD retrocrt)
endif()

if(CMAKE_SYSTEM_NAME MATCHES Retro68)

    set_target_properties(LumonAD PROPERTIES
        COMPILE_DEFINITIONS "COMPILING_AS_CODE_RESOURCE;OLDROUTINENAMES=1"
        COMPILE_OPTIONS     "-ffunction-sections"
        LINK_FLAGS          "-Wl,--mac-flat -Wl,-eADGMENTRY -Wl,-gc-sections")

    set(rez_include_options ${REZ_INCLUDE_PATH})
    list(TRANSFORM rez_include_options PREPEND -I)

    add_custom_command(
        OUTPUT  LumonAD.bin LumonAD.dsk "%LumonAD.ad"
        COMMAND ${REZ}
                ${CMAKE_CURRENT_SOURCE_DIR}/AfterDark.r
                ${rez_include_options}
                -o  "LumonAD.bin"
                --cc "LumonAD.dsk"
                --cc "%LumonAD.ad"
                -t "ADGM" -c "LuMN"
                --copy ${CMAKE_CURRENT_SOURCE_DIR}/Lumon.rsrc.as
        DEPENDS LumonAD ${CMAKE_CURRENT_SOURCE_DIR}/AfterDark.r
                        ${CMAKE_CURRENT_SOURCE_DIR}/Lumon.rsrc.as)

    add_custom_target(LumonAD_MODULE ALL DEPENDS LumonAD.bin)

endif()
