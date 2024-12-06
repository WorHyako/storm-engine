cmake_minimum_required(VERSION 3.19)

# ---------- #
#   Dialog   #
# ---------- #
add_library(dialog)
add_library(storm::dialog ALIAS dialog)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(dialog
        PRIVATE ${Sources})

target_link_libraries(dialog
        PUBLIC
        storm::model
        storm::sound_service)
