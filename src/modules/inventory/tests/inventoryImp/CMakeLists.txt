find_package(GTest CONFIG REQUIRED)

add_executable(inventoryimp_unit_test inventoryImp_test.cpp)
target_include_directories(inventoryimp_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(inventoryimp_unit_test PRIVATE
    inventory
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME InventoryImpTest COMMAND inventoryimp_unit_test)