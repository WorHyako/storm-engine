cmake_minimum_required(VERSION 3.19)

# -------------- #
#   BallSplash   #
# -------------- #
add_library(ball_splash)
add_library(storm::ball_splash ALIAS ball_splash)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(ball_splash
        PRIVATE ${Sources})

target_include_directories(ball_splash
        PUBLIC src)

target_link_libraries(ball_splash
        PUBLIC
        storm::geometry
        storm::sea)
