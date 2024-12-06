cmake_minimum_required(VERSION 3.15)

# -------------- #
#   dear imgui   #
# -------------- #
add_library(imgui)
add_library(storm::imgui ALIAS imgui)

set(ImGui_Dir ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

file(GLOB Sources ${ImGui_Dir}/*.cpp)
list(APPEND Sources
        ${ImGui_Dir}/backends/imgui_impl_sdl2.cpp
        ${ImGui_Dir}/misc/cpp/imgui_stdlib.cpp
        $<$<BOOL:${WIN32}>:${ImGui_Dir}/backends/imgui_impl_dx9.cpp>)

target_sources(imgui
        PRIVATE ${Sources})

target_include_directories(imgui
        PUBLIC
        ${ImGui_Dir}
        ${ImGui_Dir}/backends
        ${ImGui_Dir}/misc/cpp)

target_link_libraries(imgui
        PUBLIC
        SDL2::SDL2
        d3d9)
unset(Sources)

# ---------- #
#   Toml++   #
# ---------- #
add_subdirectory(tomlplusplus)

# ---------- #
#   fmtlib   #
# ---------- #
set(FMT_INSTALL OFF)
set(FMT_TEST OFF)
set(FMT_DOC OFF)
set(FMT_CUDA_TEST OFF)
add_subdirectory(fmt)

# ---------- #
#   spdlog   #
# ---------- #
set(SPDLOG_BUILD_EXAMPLE OFF)
add_subdirectory(spdlog)

# ----------------- #
#   sentry-native   #
# ----------------- #
add_subdirectory(sentry-native)

# -------------- #
#   fast_float   #
# -------------- #
add_subdirectory(fast_float)

# ----------------- #
#   nlohmann-json   #
# ----------------- #
add_subdirectory(nlohmann-json)

# -------- #
#   SDL2   #
# -------- #
set(SDL2_DISABLE_INSTALL ON)
set(SDL2_DISABLE_UNINSTALL ON)
set(SDL_TEST OFF)
add_subdirectory(SDL)

# ---------- #
#   Catch2   #
# ---------- #
set(CATCH_INSTALL_DOCS OFF)
set(CATCH_INSTALL_EXTRAS OFF)
add_subdirectory(catch2)

# -------- #
#   zlib   #
# -------- #
set(ZLIB_BUILD_EXAMPLES OFF)
add_subdirectory(zlib)

# ------------- #
#   pfl_stack   #
# ------------- #
add_library(plf_stack INTERFACE)
add_library(storm::plf_stack ALIAS plf_stack)

target_include_directories(plf_stack
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/plf_stack)

# --------- #
#   dxsdk   #
# --------- #
if("$ENV{DXSDK_DIR}" STREQUAL "")
    message(FATAL_ERROR "DXSDK was not found")
endif ()

add_library(d3dx9 SHARED IMPORTED GLOBAL)
add_library(storm::d3dx9 ALIAS d3dx9)

set_target_properties(d3dx9 PROPERTIES
        IMPORTED_LOCATION "$ENV{DXSDK_DIR}/Developer Runtime/x64/d3dx9d_33.dll"
        IMPORTED_IMPLIB "$ENV{DXSDK_DIR}/Lib/x64/d3dx9d.lib"
        INTERFACE_INCLUDE_DIRECTORIES "$ENV{DXSDK_DIR}/Include")

# -------- #
#   fmod   #
# -------- #
add_library(fmod SHARED IMPORTED GLOBAL)
add_library(storm::fmod ALIAS fmod)

set_target_properties(fmod PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/fmod/Libraries x64/fmod.dll"
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/fmod/Libraries x64/fmod_vc.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/fmod/Include")

# --------- #
#   cli11   #
# --------- #
set(CLI11_BUILD_TESTS OFF)
set(CLI11_BUILD_EXAMPLES OFF)
set(CLI11_INSTALL OFF)
set(CLI11_BUILD_DOCS OFF)
add_subdirectory(cli11)

# ------------ #
#   mimalloc   #
# ------------ #
add_subdirectory(mimalloc)
