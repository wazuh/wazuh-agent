# Wazuh Agent tests

find_package(GTest CONFIG REQUIRED)

set(TEST_SOURCES
    agent_test.cpp
)

add_executable(agent_test agent_test.cpp)
target_link_libraries(agent_test PRIVATE agent GTest::gtest)
add_test(NAME AgentTest COMMAND agent_test)
