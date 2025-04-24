#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <optional>

#include <boost/asio.hpp>

#include <ilistener_wrapper.hpp>
#include <instance_communicator.hpp>

using namespace testing;

class MockCallbacks
{
public:
    MOCK_METHOD(void, ReloadModules, (const std::optional<std::string>& moduleName), ());
};

class MockSocketWrapper : public instance_communicator::IListenerWrapper
{
public:
    MOCK_METHOD(bool, CreateOrOpen, (const std::string& runPath, const std::size_t bufferSize), (override));
    MOCK_METHOD(boost::asio::awaitable<void>, AsyncAccept, (boost::system::error_code & ec), (override));
    MOCK_METHOD(boost::asio::awaitable<std::size_t>,
                AsyncRead,
                (char* data, const std::size_t size, boost::system::error_code& ec),
                (override));
    MOCK_METHOD(void, Close, (), (override));
};

class InstanceCommunicatorTest : public Test
{
protected:
    void SetUp() override
    {
        m_mockWrapper = std::make_unique<MockSocketWrapper>();

        m_communicator = std::make_shared<instance_communicator::InstanceCommunicator>(
            [this](const std::optional<std::string>& moduleName) { m_mockCallbacks.ReloadModules(moduleName); });
    }

    void TearDown() override
    {
        m_mockWrapper.reset();
    }

    MockCallbacks m_mockCallbacks;
    std::unique_ptr<MockSocketWrapper> m_mockWrapper;
    std::shared_ptr<instance_communicator::IInstanceCommunicator> m_communicator;
};

TEST(TestInstanceCommunicator, ConstructorTest)
{
    EXPECT_NO_THROW(
        const instance_communicator::InstanceCommunicator communicator([](const std::optional<std::string>&) {}));
    EXPECT_ANY_THROW(const instance_communicator::InstanceCommunicator communicator(nullptr));
}

TEST_F(InstanceCommunicatorTest, ReloadModulesHandlerIsCalled)
{
    const std::string message = "RELOAD";
    const std::optional<std::string> expectedModuleName = std::nullopt;

    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName)).Times(::testing::Exactly(1));

    const instance_communicator::InstanceCommunicator communicator([this](const std::optional<std::string>& moduleName)
                                                                   { m_mockCallbacks.ReloadModules(moduleName); });

    communicator.HandleSignal(message);
}

TEST_F(InstanceCommunicatorTest, ReloadModuleHandlerIsCalled)
{
    const std::string message = "RELOAD-MODULE:my_test_module";
    const std::optional<std::string> expectedModuleName = "my_test_module";

    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName)).Times(::testing::Exactly(1));

    const instance_communicator::InstanceCommunicator communicator([this](const std::optional<std::string>& moduleName)
                                                                   { m_mockCallbacks.ReloadModules(moduleName); });

    communicator.HandleSignal(message);
}

// NOLINTBEGIN(cppcoreguidelines-avoid-reference-coroutine-parameters,
// cppcoreguidelines-avoid-capturing-lambda-coroutines)
TEST_F(InstanceCommunicatorTest, Listen)
{
    EXPECT_CALL(*m_mockWrapper, CreateOrOpen(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockWrapper, AsyncAccept(testing::_))
        .WillOnce(testing::Invoke(
            [](boost::system::error_code& retCode) -> boost::asio::awaitable<void>
            {
                retCode = {};
                co_return;
            }));

    const std::optional<std::string> expectedModuleName = std::nullopt;
    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName))
        .WillOnce(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, AsyncRead(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [](char* data, std::size_t size, boost::system::error_code& ec) -> boost::asio::awaitable<std::size_t>
            {
                auto buff = std::make_shared<boost::asio::streambuf>();
                std::ostream os(buff.get());
                os << "RELOAD\n";
                std::istream is(buff.get());
                is.read(data, static_cast<std::streamsize>(size));
                ec.clear();
                co_return static_cast<std::size_t>(is.gcount());
            }));

    EXPECT_CALL(*m_mockWrapper, Close()).WillOnce(testing::Invoke([this]() { m_communicator->Stop(); }));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [this]() -> boost::asio::awaitable<void> { co_await m_communicator->Listen("/tmp", std::move(m_mockWrapper)); },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsAcceptorOpen)
{
    EXPECT_CALL(*m_mockWrapper, CreateOrOpen(testing::_, testing::_))
        .WillOnce(testing::Return(false))
        .WillOnce(testing::Return(true));

    EXPECT_CALL(*m_mockWrapper, AsyncAccept(testing::_))
        .WillOnce(testing::Invoke(
            [](boost::system::error_code& retCode) -> boost::asio::awaitable<void>
            {
                retCode = {};
                co_return;
            }));

    const std::optional<std::string> expectedModuleName = std::nullopt;
    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName))
        .WillOnce(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, AsyncRead(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [](char* data, std::size_t size, boost::system::error_code& ec) -> boost::asio::awaitable<std::size_t>
            {
                auto buff = std::make_shared<boost::asio::streambuf>();
                std::ostream os(buff.get());
                os << "RELOAD\n";
                std::istream is(buff.get());
                is.read(data, static_cast<std::streamsize>(size));
                ec.clear();
                co_return static_cast<std::size_t>(is.gcount());
            }));

    EXPECT_CALL(*m_mockWrapper, Close()).WillOnce(testing::Invoke([this]() { m_communicator->Stop(); }));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [this]() -> boost::asio::awaitable<void> { co_await m_communicator->Listen("/tmp", std::move(m_mockWrapper)); },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsAcceptorAsyncAccept)
{
    EXPECT_CALL(*m_mockWrapper, CreateOrOpen(testing::_, testing::_)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*m_mockWrapper, AsyncAccept(testing::_))
        .WillOnce(testing::Invoke(
            [](boost::system::error_code& retCode) -> boost::asio::awaitable<void>
            {
                retCode = boost::asio::error::operation_aborted;
                co_return;
            }))
        .WillOnce(testing::Invoke(
            [](boost::system::error_code& retCode) -> boost::asio::awaitable<void>
            {
                retCode = {};
                co_return;
            }));

    const std::optional<std::string> expectedModuleName = std::nullopt;
    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName))
        .WillOnce(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, AsyncRead(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [](char* data, std::size_t size, boost::system::error_code& ec) -> boost::asio::awaitable<std::size_t>
            {
                auto buff = std::make_shared<boost::asio::streambuf>();
                std::ostream os(buff.get());
                os << "RELOAD\n";
                std::istream is(buff.get());
                is.read(data, static_cast<std::streamsize>(size));
                ec.clear();
                co_return static_cast<std::size_t>(is.gcount());
            }));

    EXPECT_CALL(*m_mockWrapper, Close())
        .WillOnce(testing::DoDefault())
        .WillOnce(testing::Invoke([this]() { (m_communicator->Stop()); }));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [this]() -> boost::asio::awaitable<void> { co_await m_communicator->Listen("/tmp", std::move(m_mockWrapper)); },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsReadUntil)
{
    EXPECT_CALL(*m_mockWrapper, CreateOrOpen(testing::_, testing::_)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*m_mockWrapper, AsyncAccept(testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::system::error_code& retCode) -> boost::asio::awaitable<void>
            {
                retCode = {};
                co_return;
            }));

    const std::optional<std::string> expectedModuleName = std::nullopt;
    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName))
        .WillOnce(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, AsyncRead(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [](char* data, std::size_t size, boost::system::error_code& ec) -> boost::asio::awaitable<std::size_t>
            {
                (void)size;
                data[0] = '\0';
                ec = boost::asio::error::operation_aborted;
                co_return 0;
            }))
        .WillOnce(testing::Invoke(
            [](char* data, std::size_t size, boost::system::error_code& ec) -> boost::asio::awaitable<std::size_t>
            {
                auto buff = std::make_shared<boost::asio::streambuf>();
                std::ostream os(buff.get());
                os << "RELOAD\n";
                std::istream is(buff.get());
                is.read(data, static_cast<std::streamsize>(size));
                ec.clear();
                co_return static_cast<std::size_t>(is.gcount());
            }));

    EXPECT_CALL(*m_mockWrapper, Close())
        .WillOnce(testing::DoDefault())
        .WillOnce(testing::Invoke([this]() { (m_communicator->Stop()); }));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [this]() -> boost::asio::awaitable<void> { co_await m_communicator->Listen("/tmp", std::move(m_mockWrapper)); },
        boost::asio::detached);

    ioContext.run();
}

// NOLINTEND(cppcoreguidelines-avoid-reference-coroutine-parameters,
// cppcoreguidelines-avoid-capturing-lambda-coroutines)

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
