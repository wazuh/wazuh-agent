# Wazuh Agent tests

set(TEST_SOURCES
    agent_test.cpp
)

add_executable(agent_test agent_test.cpp)
target_link_libraries(agent_test PRIVATE agent)
add_test(NAME AgentTest COMMAND agent_test)
