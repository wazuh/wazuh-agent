find_package(GTest CONFIG REQUIRED)

add_executable(inv_normalizer_unit_test invNormalizer_test.cpp)
target_include_directories(inv_normalizer_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(inv_normalizer_unit_test PRIVATE
    inventory
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME InvNormalizerTest COMMAND inv_normalizer_unit_test)
