#include <gtest/gtest.h>

#include <command_handler.hpp>
#include <command_store.hpp>

TEST(CommandHandlerTest, CommandHandlerConstructor)
{
    auto configurationParser = std::make_shared<configuration::ConfigurationParser>();

    EXPECT_NO_THROW(const command_handler::CommandHandler cm(configurationParser));
}

TEST(CommandHandlerTest, CommandHandlerConstructorNoConfigParser)
{
    EXPECT_THROW(const command_handler::CommandHandler cm(nullptr), std::runtime_error);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
