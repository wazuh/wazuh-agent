#include <gtest/gtest.h>

#include <isca_policy_loader.hpp>
#include <sca_policy_loader.hpp>

TEST(ScaPolicyLoaderTest, Contruction)
{
    [[maybe_unused]] const SCAPolicyLoader policyLoader;
    SUCCEED();
}
