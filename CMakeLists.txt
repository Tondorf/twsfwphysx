cmake_minimum_required(VERSION 3.21)

include(cmake/prelude.cmake)

project(
        twsfwphysx
        VERSION 0.1.0
        DESCRIPTION "The twsfw physics engine (the 'w' stands for WASM)"
        HOMEPAGE_URL "https://github.com/tondorf/twsfwphysx"
        LANGUAGES NONE
)

include(cmake/variables.cmake)

# ---- Declare library ----

add_library(twsfwphysx_twsfwphysx INTERFACE)
add_library(twsfwphysx::twsfwphysx ALIAS twsfwphysx_twsfwphysx)

set_property(
        TARGET twsfwphysx_twsfwphysx PROPERTY
        EXPORT_NAME twsfwphysx
)

target_include_directories(
        twsfwphysx_twsfwphysx ${warning_guard}
        INTERFACE
        "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(twsfwphysx_twsfwphysx INTERFACE c_std_11)

# ---- Install rules ----

if (NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif ()

# ---- Documentation ----

if (PROJECT_IS_TOP_LEVEL)
    option(BUILD_DOCS "Build documentation using Doxygen" OFF)
    if (BUILD_DOCS)
        include(cmake/docs.cmake)
    endif ()
endif ()

# ---- Developer mode ----

if (NOT twsfwphysx_DEVELOPER_MODE)
    return()
elseif (NOT PROJECT_IS_TOP_LEVEL)
    message(
            AUTHOR_WARNING
            "Developer mode is intended for developers of twsfwphysx"
    )
endif ()

include(cmake/dev-mode.cmake)
