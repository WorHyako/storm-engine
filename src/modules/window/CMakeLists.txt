cmake_minimum_required(VERSION 3.19)

# ---------- #
#   Window   #
# ---------- #
add_library(window)
add_library(storm::window ALIAS window)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(window
        PRIVATE ${Sources})

target_include_directories(window
        PUBLIC
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(window
        PUBLIC $<$<BOOL:${STORM_MESA_NINE}>:STORM_MESA_NINE>)

target_link_libraries(window
        PUBLIC ${SDL2_LIBRARIES})

