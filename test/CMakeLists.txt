cmake_minimum_required(VERSION 3.21)

project(twsfwphysxTests LANGUAGES C CXX)

include(../cmake/folders.cmake)

# ---- Dependencies ----

if (PROJECT_IS_TOP_LEVEL)
    find_package(twsfwphysx REQUIRED)
    enable_testing()
endif ()

# ---- Unit Tests ----

add_executable(
        twsfwphysx_unit_tests
        source/twsfwphysx_impl.c
        source/twsfwphysx_unit_tests.cpp
)

target_compile_definitions(
        twsfwphysx_unit_tests
        PRIVATE
        TWSFWPHYSX_VERSION="${twsfwphysx_VERSION}"
)

target_link_libraries(twsfwphysx_unit_tests PRIVATE twsfwphysx::twsfwphysx)
target_compile_features(twsfwphysx_unit_tests PRIVATE cxx_std_20)

add_test(NAME twsfwphysx_unit_tests COMMAND twsfwphysx_unit_tests)

# ---- Fuzz Tests ----

option(BUILD_FUZZING "Build fuzzing tests using libFuzzer" OFF)
if (BUILD_FUZZING)
    add_executable(
            twsfwphysx_fuzz_tests
            source/twsfwphysx_impl.c
            source/twsfwphysx_fuzz_target.cpp
    )

    target_link_libraries(twsfwphysx_fuzz_tests PRIVATE twsfwphysx::twsfwphysx -fsanitize=fuzzer,address)
    target_compile_features(twsfwphysx_fuzz_tests PRIVATE cxx_std_20)
    target_compile_options(twsfwphysx_fuzz_tests PRIVATE -fsanitize=fuzzer,address)

    add_test(NAME twsfwphysx_fuzz_tests COMMAND twsfwphysx_fuzz_tests -max_total_time=10)
endif ()

# ---- End-of-file commands ----

add_folders(Test)
