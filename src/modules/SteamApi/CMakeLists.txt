cmake_minimum_required(VERSION 3.19)

# ------------ #
#   SteamApi   #
# ------------ #
add_library(steam_api)
add_library(storm::steam_api ALIAS steam_api)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(steam_api
        PRIVATE ${Sources})

target_include_directories(steam_api
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(steam_api
        PUBLIC
        storm::core
        $<$<BOOL:${STORM_ENABLE_STEAM}>:steamworks>)
