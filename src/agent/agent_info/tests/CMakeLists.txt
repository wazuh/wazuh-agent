find_package(GTest CONFIG REQUIRED)

add_executable(agent_info_test agent_info_test.cpp)
configure_target(agent_info_test)
target_include_directories(agent_info_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/../../persistence/tests/mocks)
target_link_libraries(agent_info_test PRIVATE AgentInfo Persistence GTest::gtest GTest::gmock GTest::gmock_main)
add_test(NAME AgentInfoTest COMMAND agent_info_test)

add_executable(agent_info_persistence_test agent_info_persistence_test.cpp)
configure_target(agent_info_persistence_test)
target_include_directories(agent_info_persistence_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../persistence/tests/mocks)
target_link_libraries(agent_info_persistence_test PRIVATE AgentInfo Persistence GTest::gtest GTest::gmock)
add_test(NAME AgentInfoPersistenceTest COMMAND agent_info_persistence_test)
