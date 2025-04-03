#include <gtest/gtest.h>

#include <isca_policy_loader.hpp>
#include <sca_policy_loader.hpp>

TEST(ScaPolicyLoaderTest, Contruction)
{
    [[maybe_unused]] SCAPolicyLoader policyLoader;
    SUCCEED();
}
