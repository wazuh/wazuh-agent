#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <memory>

#include <iinstance_communicator_wrapper.hpp>
#include <instance_communicator.hpp>

using namespace testing;

class MockCallbacks
{
public:
    MOCK_METHOD(void, ReloadModules, (), ());
    MOCK_METHOD(void, ReloadModule, (const std::string& moduleName), ());
};

class MockInstanceCommunicatorWrapper : public instance_communicator::IInstanceCommunicatorWrapper
{
public:
    MOCK_METHOD(void, AcceptorOpen, (boost::asio::local::stream_protocol::acceptor&), (override));
    MOCK_METHOD(void,
                AcceptorBind,
                (boost::asio::local::stream_protocol::acceptor&, const boost::asio::local::stream_protocol::endpoint&),
                (override));
    MOCK_METHOD(void, AcceptorListen, (boost::asio::local::stream_protocol::acceptor&), (override));
    MOCK_METHOD(void, AcceptorClose, (boost::asio::local::stream_protocol::acceptor&), (override));
    MOCK_METHOD(boost::asio::awaitable<void>,
                AcceptorAsyncAccept,
                (boost::asio::local::stream_protocol::acceptor&,
                 boost::asio::local::stream_protocol::socket& socket,
                 boost::system::error_code& ec),
                (override));
    MOCK_METHOD(boost::asio::awaitable<void>,
                SocketReadUntil,
                (boost::asio::local::stream_protocol::socket & socket,
                 boost::asio::streambuf& buffer,
                 boost::system::error_code& ec),
                (override));
};

class InstanceCommunicatorTest : public Test
{
protected:
    void SetUp() override
    {
        m_mockWrapper = std::make_unique<MockInstanceCommunicatorWrapper>();

        m_communicator = std::make_shared<instance_communicator::InstanceCommunicator>(
            [this]() { m_mockCallbacks.ReloadModules(); },
            [this](const std::string& moduleName) { m_mockCallbacks.ReloadModule(moduleName); });
    }

    void TearDown() override
    {
        m_mockWrapper.reset();
    }

    MockCallbacks m_mockCallbacks;
    std::unique_ptr<MockInstanceCommunicatorWrapper> m_mockWrapper;
    std::shared_ptr<instance_communicator::IInstanceCommunicator> m_communicator;
};

TEST(TestInstanceCommunicator, ConstructorTest)
{
    EXPECT_NO_THROW(const instance_communicator::InstanceCommunicator communicator([]() {}, [](const std::string&) {}));
    EXPECT_ANY_THROW(
        const instance_communicator::InstanceCommunicator communicator(nullptr, [](const std::string&) {}));
    EXPECT_ANY_THROW(const instance_communicator::InstanceCommunicator communicator([]() {}, nullptr));
}

TEST_F(InstanceCommunicatorTest, ReloadModuleHandlerIsCalled)
{
    EXPECT_CALL(m_mockCallbacks, ReloadModules()).Times(::testing::Exactly(1));

    const instance_communicator::InstanceCommunicator communicator([this]() { m_mockCallbacks.ReloadModules(); },
                                                                   [](const std::string&) {});

    communicator.HandleSignal("RELOAD");
}

TEST_F(InstanceCommunicatorTest, ReloadModulesHandlerIsCalled)
{
    const std::string message = "RELOAD-MODULE:my_test_module";
    const std::string expectedModuleName = "my_test_module";

    EXPECT_CALL(m_mockCallbacks, ReloadModule(expectedModuleName)).Times(::testing::Exactly(1));

    const instance_communicator::InstanceCommunicator communicator(
        []() {}, [this](const std::string& moduleName) { m_mockCallbacks.ReloadModule(moduleName); });

    communicator.HandleSignal(message);
}

TEST_F(InstanceCommunicatorTest, Listen)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_, testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorListen(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorClose(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::asio::local::stream_protocol::acceptor&,                     // NOLINT
               boost::asio::local::stream_protocol::socket&,                       // NOLINT
               boost::system::error_code& retCode) -> boost::asio::awaitable<void> // NOLINT
            {
                retCode = {};
                co_return;
            }));

    EXPECT_CALL(m_mockCallbacks, ReloadModules())
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [this](boost::asio::local::stream_protocol::socket&,                  // NOLINT
                   boost::asio::streambuf& buff,                                  // NOLINT
                   boost::system::error_code& ec) -> boost::asio::awaitable<void> // NOLINT
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [&ioContext, this]() -> boost::asio::awaitable<void> // NOLINT
        { co_await m_communicator->Listen(ioContext, std::move(m_mockWrapper)); },
        boost::asio::detached); // NOLINT (unused-result)

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsAcceptorOpen)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen(testing::_))
        .WillOnce([]() { throw std::runtime_error("Test exception"); })
        .WillOnce(testing::Return());
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_, testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorListen(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorClose(testing::_)).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::asio::local::stream_protocol::acceptor&,                     // NOLINT
               boost::asio::local::stream_protocol::socket&,                       // NOLINT
               boost::system::error_code& retCode) -> boost::asio::awaitable<void> // NOLINT
            {
                retCode = {};
                co_return;
            }));

    EXPECT_CALL(m_mockCallbacks, ReloadModules())
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [this](boost::asio::local::stream_protocol::socket&,                  // NOLINT
                   boost::asio::streambuf& buff,                                  // NOLINT
                   boost::system::error_code& ec) -> boost::asio::awaitable<void> // NOLINT
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [&ioContext, this]() -> boost::asio::awaitable<void> // NOLINT
        { co_await m_communicator->Listen(ioContext, std::move(m_mockWrapper)); },
        boost::asio::detached); // NOLINT (unused-result)

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsAcceptorBind)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen(testing::_)).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_, testing::_))
        .WillOnce([]() { throw std::runtime_error("Test exception"); })
        .WillOnce(testing::Return());
    EXPECT_CALL(*m_mockWrapper, AcceptorListen(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorClose(testing::_)).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::asio::local::stream_protocol::acceptor&,                     // NOLINT
               boost::asio::local::stream_protocol::socket&,                       // NOLINT
               boost::system::error_code& retCode) -> boost::asio::awaitable<void> // NOLINT
            {
                retCode = {};
                co_return;
            }));

    EXPECT_CALL(m_mockCallbacks, ReloadModules())
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [this](boost::asio::local::stream_protocol::socket&,                  // NOLINT
                   boost::asio::streambuf& buff,                                  // NOLINT
                   boost::system::error_code& ec) -> boost::asio::awaitable<void> // NOLINT
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [&ioContext, this]() -> boost::asio::awaitable<void> // NOLINT
        { co_await m_communicator->Listen(ioContext, std::move(m_mockWrapper)); },
        boost::asio::detached); // NOLINT (unused-result)

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsAcceptorListen)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen(testing::_)).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_, testing::_)).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorListen(testing::_))
        .WillOnce([]() { throw std::runtime_error("Test exception"); })
        .WillOnce(testing::Return());
    EXPECT_CALL(*m_mockWrapper, AcceptorClose(testing::_)).Times(::testing::Exactly(2));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::asio::local::stream_protocol::acceptor&,                     // NOLINT
               boost::asio::local::stream_protocol::socket&,                       // NOLINT
               boost::system::error_code& retCode) -> boost::asio::awaitable<void> // NOLINT
            {
                retCode = {};
                co_return;
            }));

    EXPECT_CALL(m_mockCallbacks, ReloadModules())
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [this](boost::asio::local::stream_protocol::socket&,                  // NOLINT
                   boost::asio::streambuf& buff,                                  // NOLINT
                   boost::system::error_code& ec) -> boost::asio::awaitable<void> // NOLINT
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [&ioContext, this]() -> boost::asio::awaitable<void> // NOLINT
        { co_await m_communicator->Listen(ioContext, std::move(m_mockWrapper)); },
        boost::asio::detached); // NOLINT (unused-result)

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsAcceptorAsyncAccept)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_, testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorListen(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorClose(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [](boost::asio::local::stream_protocol::acceptor&,                     // NOLINT
               boost::asio::local::stream_protocol::socket&,                       // NOLINT
               boost::system::error_code& retCode) -> boost::asio::awaitable<void> // NOLINT
            {
                retCode = boost::asio::error::operation_aborted;
                co_return;
            }))
        .WillOnce(testing::Invoke(
            [](boost::asio::local::stream_protocol::acceptor&,                     // NOLINT
               boost::asio::local::stream_protocol::socket&,                       // NOLINT
               boost::system::error_code& retCode) -> boost::asio::awaitable<void> // NOLINT
            {
                retCode = {};
                co_return;
            }));

    EXPECT_CALL(m_mockCallbacks, ReloadModules())
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [this](boost::asio::local::stream_protocol::socket&,                  // NOLINT
                   boost::asio::streambuf& buff,                                  // NOLINT
                   boost::system::error_code& ec) -> boost::asio::awaitable<void> // NOLINT
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [&ioContext, this]() -> boost::asio::awaitable<void> // NOLINT
        { co_await m_communicator->Listen(ioContext, std::move(m_mockWrapper)); },
        boost::asio::detached); // NOLINT (unused-result)

    ioContext.run();
}

TEST_F(InstanceCommunicatorTest, ListenFailsReadUntil)
{
    EXPECT_CALL(*m_mockWrapper, AcceptorOpen(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorBind(testing::_, testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorListen(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorClose(testing::_)).Times(::testing::Exactly(1));
    EXPECT_CALL(*m_mockWrapper, AcceptorAsyncAccept(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [](boost::asio::local::stream_protocol::acceptor&,                     // NOLINT
               boost::asio::local::stream_protocol::socket&,                       // NOLINT
               boost::system::error_code& retCode) -> boost::asio::awaitable<void> // NOLINT
            {
                retCode = {};
                co_return;
            }));

    EXPECT_CALL(m_mockCallbacks, ReloadModules())
        .WillRepeatedly(testing::Invoke([]() { std::cout << "ReloadModules\n"; }));

    EXPECT_CALL(*m_mockWrapper, SocketReadUntil(testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke(
            [](boost::asio::local::stream_protocol::socket&,                  // NOLINT
               boost::asio::streambuf&,                                       // NOLINT
               boost::system::error_code& ec) -> boost::asio::awaitable<void> // NOLINT
            {
                ec = boost::asio::error::operation_aborted;
                co_return;
            }))
        .WillOnce(testing::Invoke(
            [this](boost::asio::local::stream_protocol::socket&,                  // NOLINT
                   boost::asio::streambuf& buff,                                  // NOLINT
                   boost::system::error_code& ec) -> boost::asio::awaitable<void> // NOLINT
            {
                std::ostream os(&buff);
                os << "RELOAD\n";
                ec = {};
                m_communicator->Stop();
                co_return;
            }));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [&ioContext, this]() -> boost::asio::awaitable<void> // NOLINT
        { co_await m_communicator->Listen(ioContext, std::move(m_mockWrapper)); },
        boost::asio::detached); // NOLINT (unused-result)

    ioContext.run();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
