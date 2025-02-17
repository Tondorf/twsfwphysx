cmake_minimum_required(VERSION 3.21)

project(twsfwphysxTests LANGUAGES C)

include(../cmake/folders.cmake)

# ---- Dependencies ----

if (PROJECT_IS_TOP_LEVEL)
    find_package(twsfwphysx REQUIRED)
    enable_testing()
endif ()

# ---- Tests ----

add_executable(twsfwphysx_test source/twsfwphysx_test.c)
set_property(
        SOURCE source/twsfwphysx_test.c PROPERTY
        COMPILE_DEFINITIONS TWSFWPHYSX_IMPLEMENTATION
)

target_compile_definitions(
        twsfwphysx_test
        PRIVATE
        TWSFWPHYSX_IMPLEMENTATION
        TWSFWPHYSX_VERSION="${twsfwphysx_VERSION}"
)

target_link_libraries(twsfwphysx_test PRIVATE twsfwphysx::twsfwphysx)
target_compile_features(twsfwphysx_test PRIVATE c_std_11)

add_test(NAME twsfwphysx_test COMMAND twsfwphysx_test)

# ---- End-of-file commands ----

add_folders(Test)
