add_executable(inventory_unit_test inventory_test.cpp)
configure_target(inventory_unit_test)
target_include_directories(inventory_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(inventory_unit_test PRIVATE
    Inventory
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
if(NOT WIN32)
add_test(NAME InventoryTest COMMAND inventory_unit_test)
endif()
