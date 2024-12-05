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
#        "comctl32"
#        "dbghelp"
#        "winhttp"
        mimalloc
        SDL2::SDL2
        zlib
        CLI11
        sentry::sentry
        storm::sound
        storm::core
        storm::diagnostics
        storm::utils
        storm::window)
