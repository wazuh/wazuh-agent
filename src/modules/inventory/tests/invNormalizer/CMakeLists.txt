add_executable(inv_normalizer_unit_test invNormalizer_test.cpp)
configure_target(inv_normalizer_unit_test)
target_include_directories(inv_normalizer_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_link_libraries(inv_normalizer_unit_test PRIVATE
    Inventory
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME InvNormalizerTest COMMAND inv_normalizer_unit_test)
