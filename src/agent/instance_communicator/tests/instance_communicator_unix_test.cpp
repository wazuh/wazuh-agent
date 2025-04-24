#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <optional>

#include <instance_communicator.hpp>
#include <isocket_wrapper.hpp>

using namespace testing;

class MockCallbacks
{
public:
    MOCK_METHOD(void, ReloadModules, (const std::optional<std::string>& moduleName), ());
};

class MockSocketWrapper : public instance_communicator::ISocketWrapper
{
public:
    MOCK_METHOD(void, AcceptorOpen, (), (override));
    MOCK_METHOD(void, AcceptorBind, (const boost::asio::local::stream_protocol::endpoint&), (override));
    MOCK_METHOD(void, AcceptorListen, (), (override));
    MOCK_METHOD(void, AcceptorClose, (), (override));
    MOCK_METHOD(boost::asio::awaitable<void>, AcceptorAsyncAccept, (boost::system::error_code & ec), (override));
    MOCK_METHOD(boost::asio::awaitable<void>,
                SocketReadUntil,
                (boost::asio::streambuf & buffer, boost::system::error_code& ec),
                (override));
    MOCK_METHOD(void, SocketShutdown, (boost::system::error_code & ec), (override));
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
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorListen()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorClose()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::system::error_code& retCode) -> boost::asio::awaitable<void>
            {
                retCode = {};
                co_return;
            }));

    const std::optional<std::string> expectedModuleName = std::nullopt;
    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName))
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [this](boost::asio::streambuf& buff, boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    EXPECT_CALL(*m_mockWrapper, SocketShutdown(testing::_)).Times(::testing::Exactly(1));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [this]() -> boost::asio::awaitable<void> { co_await m_communicator->Listen("/tmp", std::move(m_mockWrapper)); },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsAcceptorOpen)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen())
        .WillOnce([]() { throw std::runtime_error("Test exception"); })
        .WillOnce(testing::Return());
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorListen()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorClose()).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::system::error_code& retCode) -> boost::asio::awaitable<void>
            {
                retCode = {};
                co_return;
            }));

    const std::optional<std::string> expectedModuleName = std::nullopt;
    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName))
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [this](boost::asio::streambuf& buff, boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    EXPECT_CALL(*m_mockWrapper, SocketShutdown(testing::_)).Times(::testing::Exactly(1));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [this]() -> boost::asio::awaitable<void> { co_await m_communicator->Listen("/tmp", std::move(m_mockWrapper)); },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsAcceptorBind)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen()).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_))
        .WillOnce([]() { throw std::runtime_error("Test exception"); })
        .WillOnce(testing::Return());
    EXPECT_CALL(*m_mockWrapper, AcceptorListen()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorClose()).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::system::error_code& retCode) -> boost::asio::awaitable<void>
            {
                retCode = {};
                co_return;
            }));

    const std::optional<std::string> expectedModuleName = std::nullopt;
    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName))
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [this](boost::asio::streambuf& buff, boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    EXPECT_CALL(*m_mockWrapper, SocketShutdown(testing::_)).Times(::testing::Exactly(1));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [this]() -> boost::asio::awaitable<void> { co_await m_communicator->Listen("/tmp", std::move(m_mockWrapper)); },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsAcceptorListen)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen()).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_)).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorListen())
        .WillOnce([]() { throw std::runtime_error("Test exception"); })
        .WillOnce(testing::Return());
    EXPECT_CALL(*m_mockWrapper, AcceptorClose()).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::system::error_code& retCode) -> boost::asio::awaitable<void>
            {
                retCode = {};
                co_return;
            }));

    const std::optional<std::string> expectedModuleName = std::nullopt;
    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName))
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [this](boost::asio::streambuf& buff, boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    EXPECT_CALL(*m_mockWrapper, SocketShutdown(testing::_)).Times(::testing::Exactly(1));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [this]() -> boost::asio::awaitable<void> { co_await m_communicator->Listen("/tmp", std::move(m_mockWrapper)); },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsAcceptorAsyncAccept)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorListen()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorClose()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_))
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
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [this](boost::asio::streambuf& buff, boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    EXPECT_CALL(*m_mockWrapper, SocketShutdown(testing::_)).Times(::testing::Exactly(2));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [this]() -> boost::asio::awaitable<void> { co_await m_communicator->Listen("/tmp", std::move(m_mockWrapper)); },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsReadUntil)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorListen()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorClose()).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::system::error_code& retCode) -> boost::asio::awaitable<void>
            {
                retCode = {};
                co_return;
            }));

    const std::optional<std::string> expectedModuleName = std::nullopt;
    EXPECT_CALL(m_mockCallbacks, ReloadModules(expectedModuleName))
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [](boost::asio::streambuf&, boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                ec = boost::asio::error::operation_aborted;
                co_return;
            }))
        .WillOnce(testing::Invoke(
            [this](boost::asio::streambuf& buff, boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    EXPECT_CALL(*m_mockWrapper, SocketShutdown(testing::_)).Times(::testing::Exactly(2));

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
