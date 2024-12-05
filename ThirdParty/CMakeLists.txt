cmake_minimum_required(VERSION 3.15)

# -------------- #
#   dear imgui   #
# -------------- #
add_library(imgui)
add_library(storm::imgui ALIAS imgui)

set(ImGui_Dir ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

file(GLOB Sources ${ImGui_Dir}/*.cpp)
list(APPEND
        ${ImGui_Dir}/backend/imgui_impl_sdl2.cpp
        ${ImGui_Dir}/misc/cpp/imgui_stdlib.cpp
        $<$<BOOL:${WIN32}>:${ImGui_Dir}/backend/imgui_impl_dx9.cpp>)

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
add_subdirectory(fmt)

# ---------- #
#   spdlog   #
# ---------- #
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
set(SDL2_DISABLE_SDL2MAIN ON)
set(SDL2_DISABLE_INSTALL ON)
set(SDL2_DISABLE_UNINSTALL ON)
add_subdirectory(SDL)

# ---------- #
#   Catch2   #
# ---------- #
add_subdirectory(catch2)

# -------- #
#   zlib   #
# -------- #
add_subdirectory(zlib)

# ------------- #
#   pfl_stack   #
# ------------- #
add_library(plf_stack INTERFACE)
add_library(storm::plf_stack ALIAS plf_stack)

target_include_directories(plf_stack
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/plf_stack)

# --------------- #
#   dxvk-native   #
# --------------- #
add_library(dxvk-native INTERFACE)
add_library(storm::dxvk-natiive ALIAS dxvk-native)

target_include_directories(dxvk-native
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}dxvk-native/include/native/directx
        ${CMAKE_CURRENT_SOURCE_DIR}dxvk-native/include/native/windows)

unset(Sources)

# --------- #
#   dxsdk   #
# --------- #
add_library(dxsdk INTERFACE)
add_library(storm::dxsdk ALIAS dxsdk)

target_include_directories(dxsdk
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/dxsdk/Include)
