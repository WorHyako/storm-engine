cmake_minimum_required(VERSION 3.18)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(Options)
include(CmakeFlags)
include(CompilerFlags)
include(ShadersConfig)

project(StormEngine
        LANGUAGES CXX C)

# -------------- #
#   ThirdParty   #
# -------------- #
add_subdirectory(ThirdParty)

# ----------------- #
#   Storm sources   #
# ----------------- #
add_subdirectory(src)
