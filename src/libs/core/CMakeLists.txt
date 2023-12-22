if (WIN32)
set(SYSTEM_DEPS
    "comctl32"
)
endif()

STORM_SETUP(
    TARGET_NAME core
    TYPE library
    DEPENDENCIES diagnostics math shared_headers steam_api fast_float ${SDL2_LIBRARIES} window tomlplusplus nlohmann_json ${SYSTEM_DEPS}
)
