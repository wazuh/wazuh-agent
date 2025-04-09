find_package(GTest CONFIG REQUIRED)

add_executable(module_manager_test moduleManager_test.cpp)
configure_target(module_manager_test)
target_include_directories(module_manager_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_compile_options(module_manager_test PRIVATE $<$<NOT:$<PLATFORM_ID:Windows>>:-Wno-suggest-override>)
target_link_libraries(module_manager_test PRIVATE ModuleManager GTest::gtest GTest::gtest_main GTest::gmock
                                                  GTest::gmock_main)
add_test(NAME ModuleManagerTest COMMAND module_manager_test)
