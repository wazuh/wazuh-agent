find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_WINDOWS_SRC
    "networkWindowsHelper_test.cpp"
)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC
    "main.cpp"
)

if(WIN32)
    add_executable(network_helper_tests
        ${UTIL_CXX_UNITTEST_COMMON_SRC}
        ${UTIL_CXX_UNITTEST_WINDOWS_SRC}
    )

    configure_target(network_helper_tests)

    target_include_directories(network_helper_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )

    target_link_libraries(network_helper_tests PRIVATE network_helper GTest::gtest)

    add_test(NAME networkHelperTests COMMAND network_helper_tests)
endif()
