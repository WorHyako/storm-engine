cmake_minimum_required(VERSION 3.19)

# -------- #
#   Ai   #
# -------- #
add_library(ai INTERFACE)
add_library(storm::ai ALIAS ai)

target_include_directories(ai
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
