#if (WIN32)
#    set(SYSTEM_DEPS "legacy_stdio_definitions")
#else ()
#    set(SYSTEM_DEPS "${NATIVE_D3D9_LIBS}")
#endif ()
#
#STORM_SETUP(
#        TARGET_NAME renderer
#        TYPE storm_module
#        DEPENDENCIES core config directx util ${SYSTEM_DEPS}
#)
cmake_minimum_required(VERSION 3.19)

# ------------ #
#   Renderer   #
# ------------ #
add_library(renderer)
add_library(storm::renderer ALIAS renderer)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(renderer
        PRIVATE ${Sources})

target_include_directories(renderer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(renderer
        PRIVATE
        UNICODE
        NOMINMAX)

target_link_libraries(renderer
        PUBLIC
        storm::dxsdk
        storm::dxvk-natiive
        storm::core
        storm::config
        storm::utils)
