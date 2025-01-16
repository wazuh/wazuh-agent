find_package(GTest CONFIG REQUIRED)

add_executable(statelessEvent_unit_test statelessEvent_test.cpp)
configure_target(statelessEvent_unit_test)

target_include_directories(statelessEvent_unit_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/inventory/events
)

if(NOT WIN32)
    target_link_libraries(statelessEvent_unit_test PRIVATE
        Inventory
        GTest::gtest
        GTest::gtest_main
        pthread
    )
else()
    target_link_libraries(statelessEvent_unit_test PRIVATE
        Inventory
        GTest::gtest
        GTest::gtest_main
    )
endif()

add_test(NAME StatelessEventUnitTest COMMAND statelessEvent_unit_test)
