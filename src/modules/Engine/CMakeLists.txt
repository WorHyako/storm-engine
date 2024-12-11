cmake_minimum_required(VERSION 3.19)

# ---------- #
#   Engine   #
# ---------- #
add_executable(engine)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(engine
        PRIVATE ${Sources})

target_compile_definitions(engine
        PRIVATE NOMINMAX)

target_include_directories(engine
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(engine
        PUBLIC
#        "dbghelp"
#        "winhttp"
        storm::animals
        storm::ball_splash
        storm::battle_interface
        storm::blade
        storm::blot
        storm::dialog
        storm::island
        storm::lighter
        storm::location
        storm::locator
        storm::mast
        storm::model
        storm::particles
        storm::renderer
        storm::rigging
        storm::sailors
        storm::sea
        storm::sea_ai
        storm::sea_cameras
        storm::sea_creatures
        storm::sea_foam
        storm::sea_operator
        storm::shadow
        storm::ship
        storm::config
        storm::sink_effect
        storm::sound
        storm::teleport
        storm::tornado
        storm::touch
        storm::water_rings
        storm::weather
        storm::worldmap
        storm::xinterface
        storm::geometry

        mimalloc
        SDL2main
        storm::imgui
        zlib
        CLI11
        sentry::sentry
        storm::sound
        storm::core
        storm::diagnostics
        storm::utils
        storm::window)

set_target_properties(engine
PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/steamGame")