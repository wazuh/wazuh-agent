add_executable(inventoryimp_unit_test inventoryImp_test.cpp)
configure_target(inventoryimp_unit_test)
target_include_directories(inventoryimp_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(inventoryimp_unit_test PRIVATE
    Inventory
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME InventoryImpTest COMMAND inventoryimp_unit_test)
