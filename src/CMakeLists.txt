cmake_minimum_required(VERSION 3.15)

# ----------- #
#   Modules   #
# ----------- #
add_subdirectory(modules)




return()

if (STORM_ENABLE_STEAM)
    add_definitions(-DSTORM_ENABLE_STEAM=1)
endif()

if(STORM_ENABLE_CRASH_REPORTS)
	add_definitions(-DSTORM_ENABLE_CRASH_REPORTS=1)
endif()

if(STORM_WATERMARK_FILE)
  add_definitions(-DSTORM_WATERMARK_FILE="${STORM_WATERMARK_FILE}")
endif()

if(STORM_ENABLE_SAFE_MODE)
  add_definitions(-DSTORM_ENABLE_SAFE_MODE=1)
endif()
