cmake_minimum_required(VERSION 3.21)

project(twsfwphysxTests LANGUAGES C)

include(../cmake/folders.cmake)

# ---- Dependencies ----

if (PROJECT_IS_TOP_LEVEL)
    find_package(twsfwphysx REQUIRED)
    enable_testing()
endif ()

# ---- Assert Test ----

add_executable(assert_test source/assert_test.c)
target_compile_features(assert_test PRIVATE c_std_11)
add_test(NAME assert_test COMMAND assert_test)

# ---- Version Test ----

add_executable(
        twsfwphysx_version_test
        source/twsfwphysx_impl.c
        source/twsfwphysx_version_test.c
)
target_compile_definitions(
        twsfwphysx_version_test
        PRIVATE
        TWSFWPHYSX_VERSION="${twsfwphysx_VERSION}"
)
target_link_libraries(twsfwphysx_version_test PRIVATE twsfwphysx::twsfwphysx)
target_compile_features(twsfwphysx_version_test PRIVATE c_std_11)
add_test(NAME twsfwphysx_version_test COMMAND twsfwphysx_version_test)

# ---- Unit Tests ----

function(add_unit_test test_name source_file)
    add_executable(${test_name}
            source/twsfwphysx_impl.c
            source/utils.c
            source/${source_file}
    )
    target_include_directories(${test_name} PRIVATE source)

    target_link_libraries(${test_name} PRIVATE twsfwphysx::twsfwphysx)

    target_compile_features(${test_name} PRIVATE c_std_11)

    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_unit_test(agent_rotate_tests agent_rotate_tests.c)
add_unit_test(propagation_tests propagation_tests.c)
add_unit_test(no_agents_tests no_agents_tests.c)
add_unit_test(missile_hit_tests missile_hit_tests.c)
add_unit_test(collision_tests collision_tests.c)

# ---- End-of-file commands ----

add_folders(Test)
