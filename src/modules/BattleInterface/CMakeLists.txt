cmake_minimum_required(VERSION 3.19)

# ------------------- #
#   BattleInterface   #
# ------------------- #
add_library(battle_interface)
add_library(storm::battle_interface ALIAS battle_interface)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(battle_interface
        PRIVATE ${Sources})

target_include_directories(battle_interface
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(battle_interface
        PRIVATE NOMINMAX)

target_link_libraries(battle_interface
        PUBLIC storm::weather)
