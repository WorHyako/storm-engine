cmake_minimum_required(VERSION 3.19)

# -------- #
#   Math   #
# -------- #
add_library(shared_headers INTERFACE)
add_library(storm::shared_headers ALIAS shared_headers)

target_include_directories(shared_headers
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
