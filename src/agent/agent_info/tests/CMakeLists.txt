find_package(GTest CONFIG REQUIRED)

add_executable(agent_info_test agent_info_test.cpp)
target_include_directories(agent_info_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(agent_info_test PRIVATE AgentInfo GTest::gtest)
add_test(NAME AgentInfoTest COMMAND agent_info_test)

add_executable(agent_info_persistance_test agent_info_persistance_test.cpp)
target_include_directories(agent_info_persistance_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(agent_info_persistance_test PRIVATE AgentInfo GTest::gtest)
add_test(NAME AgentInfoPersistanceTest COMMAND agent_info_persistance_test)
