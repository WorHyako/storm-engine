cmake_minimum_required(VERSION 3.19)

# ---------- #
#   Island   #
# ---------- #
add_library(island)
add_library(storm::island ALIAS island)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(island
        PRIVATE ${Sources})

target_include_directories(island
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(island
        PRIVATE NOMINMAX)

target_link_libraries(island
        PUBLIC
        storm::model
        storm::sea
        storm::ai)
