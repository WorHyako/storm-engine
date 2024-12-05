cmake_minimum_required(VERSION 3.19)

# ---------------- #
#   SoundService   #
# ---------------- #
add_library(sound_service)
add_library(storm::sound_service ALIAS sound_service)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sound_service
        PRIVATE ${Sources})

target_include_directories(sound_service
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(sound_service
        PUBLIC
        storm::core
        storm::fmod
        storm::renderer)
