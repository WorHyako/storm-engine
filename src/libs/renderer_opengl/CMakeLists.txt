if (STORM_ENABLE_OPENGL)
    STORM_SETUP(
            TARGET_NAME renderer_opengl
            TYPE storm_module
            DEPENDENCIES core util renderer window glm glbinding ${SYSTEM_DEPS}
    )

    target_compile_definitions(renderer_opengl INTERFACE "-DSTORM_OPENGL_ENABLED")
endif ()
