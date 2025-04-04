find_package(GTest CONFIG REQUIRED)

add_executable(sca_test sca_test.cpp)
configure_target(sca_test)
target_link_libraries(sca_test PRIVATE SCA GTest::gtest GTest::gtest_main)
add_test(NAME SCATest COMMAND sca_test)
