cmake_minimum_required(VERSION 3.15)

# Ignore warnings about missing pdb
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")

#if (MSVC)
    # Always generate PDBs
#    add_compile_options(/Zi)
#    add_link_options(/DEBUG)
#
#    add_compile_options(/W2) # Warning level
#    add_compile_options(/WX) # Treats compiler warnings as errors
#    add_compile_options(/std:c++latest) # Language Standard Version
#    add_compile_options(/MP) # Build with Multiple Processes
#    add_compile_options(/fp:fast) # Build with Multiple Processes
#
#
#    add_compile_options("$<$<CONFIG:Release>:/Gy>") # Enable Function-Level Linking
#    add_compile_options("$<$<CONFIG:Release>:/Oi>") # Generate intrinsic functions
#    add_compile_options("$<$<CONFIG:Release>:/Ot>") # Favor fast code, should be implied by /O2 though
#    add_compile_options("$<$<CONFIG:Release>:/GL>") # Whole program optimization
#
#    add_link_options("$<$<CONFIG:Release>:/OPT:REF>") # Eliminate functions and data that are never referenced, needs to be explicitly specified with /DEBUG
#    add_link_options("$<$<CONFIG:Release>:/OPT:ICF>") # Perform identical COMDAT folding, needs to be explicitly specified with /DEBUG
#    add_link_options("$<$<CONFIG:Release>:/LTCG>") # Enable link-time code generation
#else()
#    add_compile_options(-std=c++20)
#
#    # Verbose output
#    add_compile_options(-v)
#    add_link_options(-v)
#
#    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
#    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
#
#    # Add _DEBUG flag
#    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
#endif()
