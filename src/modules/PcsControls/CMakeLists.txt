cmake_minimum_required(VERSION 3.19)

# --------------- #
#   PcsControls   #
# --------------- #
add_library(pcs_controls)
add_library(storm::pcs_controls ALIAS pcs_controls)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(pcs_controls
        PRIVATE ${Sources})

target_include_directories(pcs_controls
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include src)

target_link_libraries(pcs_controls
        PUBLIC storm::core)
