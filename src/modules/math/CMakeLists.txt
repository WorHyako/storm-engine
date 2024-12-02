cmake_minimum_required(VERSION 3.19)

# -------- #
#   Math   #
# -------- #
add_library(math INTERFACE)
add_library(storm::math ALIAS math)

target_include_directories(math
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/math3d)
