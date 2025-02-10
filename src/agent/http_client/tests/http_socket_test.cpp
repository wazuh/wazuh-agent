#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../src/http_socket.hpp"
#include "../src/ihttp_socket.hpp"
#include "mocks/mock_http_wrapper.hpp"

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>

using namespace testing;

// NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines,cppcoreguidelines-avoid-reference-coroutine-parameters,
// clang-diagnostic-unused-result)

class HttpSocketTest : public ::testing::Test
{
protected:
    HttpSocketTest()
    {
        const auto port = 80;
        dummyResults = boost::asio::ip::tcp::resolver::results_type::create(
            boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port),
            "127.0.0.1",
            "80");
    }

    void SetUp() override
    {
        // Set up the io_context for the socket
        m_ioContext = std::make_unique<boost::asio::io_context>();
        m_mockHelper = std::make_shared<MockHttpHelper>();
        m_socket = std::make_unique<http_client::HttpSocket>(m_ioContext->get_executor(), m_mockHelper);
    }

    void TearDown() override
    {
        m_socket.reset();
        m_ioContext.reset();
    }

    boost::asio::ip::tcp::resolver::results_type dummyResults;
    std::unique_ptr<boost::asio::io_context> m_ioContext;
    std::shared_ptr<MockHttpHelper> m_mockHelper;
    std::unique_ptr<http_client::HttpSocket> m_socket;
};

TEST_F(HttpSocketTest, SetVerificationMode)
{
    EXPECT_NO_THROW(m_socket->SetVerificationMode("www.google.com", "none"));
    EXPECT_NO_THROW(m_socket->SetVerificationMode("www.google.com", "certificate"));
    EXPECT_NO_THROW(m_socket->SetVerificationMode("www.google.com", "full"));
}

TEST_F(HttpSocketTest, ConnectSocketSuccess)
{
    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, connect(_, _)).Times(1);

    boost::system::error_code ec;
    m_socket->Connect(dummyResults, ec);
    m_ioContext->run();
    EXPECT_FALSE(ec);
}

TEST_F(HttpSocketTest, ConnectSocketFailure)
{
    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, connect(_, _))
        .WillOnce([](const boost::asio::ip::tcp::resolver::results_type&, boost::system::error_code& ec)
                  { ec = boost::asio::error::operation_aborted; });

    boost::system::error_code ec;
    m_socket->Connect(dummyResults, ec);
    m_ioContext->run();
    EXPECT_TRUE(ec);
}

TEST_F(HttpSocketTest, ConnectSocketException)
{
    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, connect(_, _))
        .WillOnce([](const boost::asio::ip::tcp::resolver::results_type&, boost::system::error_code&)
                  { throw std::runtime_error("Test exception"); });

    boost::system::error_code ec;
    m_socket->Connect(dummyResults, ec);
    m_ioContext->run();
    EXPECT_TRUE(ec);
}

TEST_F(HttpSocketTest, AsyncConnectSuccess)
{
    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, async_connect(dummyResults, testing::_))
        .WillOnce(
            [](const boost::asio::ip::tcp::resolver::results_type&,
               boost::system::error_code& code) -> boost::asio::awaitable<void>
            {
                code = boost::system::error_code {};
                co_return;
            });

    boost::system::error_code result_ec;
    co_spawn(
        *m_ioContext,
        [&]() -> boost::asio::awaitable<void> { co_await m_socket->AsyncConnect(dummyResults, result_ec); },
        boost::asio::detached);

    m_ioContext->run();

    EXPECT_FALSE(result_ec);
}

TEST_F(HttpSocketTest, AsyncConnectFailure)
{
    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, async_connect(dummyResults, testing::_))
        .WillOnce(
            [](const boost::asio::ip::tcp::resolver::results_type&,
               boost::system::error_code& code) -> boost::asio::awaitable<void>
            {
                code = boost::system::error_code {boost::asio::error::operation_aborted};
                co_return;
            });

    boost::system::error_code result_ec;
    co_spawn(
        *m_ioContext,
        [&]() -> boost::asio::awaitable<void> { co_await m_socket->AsyncConnect(dummyResults, result_ec); },
        boost::asio::detached);

    m_ioContext->run();

    EXPECT_TRUE(result_ec);
    EXPECT_EQ(result_ec, boost::asio::error::operation_aborted);
}

TEST_F(HttpSocketTest, AsyncConnectException)
{
    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, async_connect(dummyResults, testing::_))
        .WillOnce([](const boost::asio::ip::tcp::resolver::results_type&,
                     boost::system::error_code&) -> boost::asio::awaitable<void>
                  { throw std::runtime_error("Test exception"); });

    boost::system::error_code result_ec;
    co_spawn(
        *m_ioContext,
        [&]() -> boost::asio::awaitable<void> { co_await m_socket->AsyncConnect(dummyResults, result_ec); },
        boost::asio::detached);

    m_ioContext->run();

    EXPECT_TRUE(result_ec);
}

TEST_F(HttpSocketTest, WriteSuccess)
{
    boost::beast::http::request<boost::beast::http::string_body> req {boost::beast::http::verb::get, "/", 11};

    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, write(_, _))
        .WillOnce([](const boost::beast::http::request<boost::beast::http::string_body>&, boost::system::error_code& ec)
                  { ec = boost::system::error_code {}; });

    boost::system::error_code ec;
    m_socket->Write(req, ec);

    EXPECT_FALSE(ec);
}

TEST_F(HttpSocketTest, WriteFailure)
{
    boost::beast::http::request<boost::beast::http::string_body> req {boost::beast::http::verb::get, "/", 11};

    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, write(_, _))
        .WillOnce([](const boost::beast::http::request<boost::beast::http::string_body>&, boost::system::error_code& ec)
                  { ec = boost::asio::error::connection_refused; });

    boost::system::error_code ec;
    m_socket->Write(req, ec);

    EXPECT_TRUE(ec);
    EXPECT_EQ(ec, boost::asio::error::connection_refused);
}

TEST_F(HttpSocketTest, WriteException)
{
    boost::beast::http::request<boost::beast::http::string_body> req {boost::beast::http::verb::get, "/", 11};

    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, write(_, _))
        .WillOnce([](const boost::beast::http::request<boost::beast::http::string_body>&, boost::system::error_code&)
                  { throw std::runtime_error("Test exception"); });

    boost::system::error_code ec;
    m_socket->Write(req, ec);

    EXPECT_FALSE(ec);
}

TEST_F(HttpSocketTest, AsyncWriteSuccess)
{
    boost::beast::http::request<boost::beast::http::string_body> req {boost::beast::http::verb::get, "/", 11};

    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, async_write(_, _))
        .WillOnce(
            [](const boost::beast::http::request<boost::beast::http::string_body>&,
               boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                ec = boost::system::error_code {};
                co_return;
            });

    boost::system::error_code ec;
    boost::asio::co_spawn(
        *m_ioContext,
        [&]() -> boost::asio::awaitable<void> { co_await m_socket->AsyncWrite(req, ec); },
        boost::asio::detached);

    m_ioContext->run();
    EXPECT_FALSE(ec);
}

TEST_F(HttpSocketTest, AsyncWriteFailure)
{
    boost::beast::http::request<boost::beast::http::string_body> req {boost::beast::http::verb::get, "/", 11};

    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, async_write(_, _))
        .WillOnce(
            [](const boost::beast::http::request<boost::beast::http::string_body>&,
               boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                ec = boost::asio::error::connection_refused;
                co_return;
            });

    boost::system::error_code ec;
    boost::asio::co_spawn(
        *m_ioContext,
        [&]() -> boost::asio::awaitable<void> { co_await m_socket->AsyncWrite(req, ec); },
        boost::asio::detached);

    m_ioContext->run();
    EXPECT_TRUE(ec);
}

TEST_F(HttpSocketTest, AsyncWriteException)
{
    boost::beast::http::request<boost::beast::http::string_body> req {boost::beast::http::verb::get, "/", 11};

    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, async_write(_, _))
        .WillOnce([](const boost::beast::http::request<boost::beast::http::string_body>&,
                     boost::system::error_code&) -> boost::asio::awaitable<void>
                  { throw std::runtime_error("Test exception"); });

    boost::system::error_code ec;
    boost::asio::co_spawn(
        *m_ioContext,
        [&]() -> boost::asio::awaitable<void> { co_await m_socket->AsyncWrite(req, ec); },
        boost::asio::detached);

    m_ioContext->run();
    EXPECT_TRUE(ec);
}

TEST_F(HttpSocketTest, ReadSuccess)
{
    boost::beast::http::response<boost::beast::http::dynamic_body> res;
    res.result(boost::beast::http::status::ok);

    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, read(_, _, _))
        .WillOnce([](boost::beast::flat_buffer&,
                     boost::beast::http::response<boost::beast::http::dynamic_body>&,
                     boost::system::error_code& ec) { ec = boost::system::error_code {}; });

    boost::system::error_code ec;
    m_socket->Read(res, ec);

    EXPECT_FALSE(ec);
    EXPECT_EQ(res.result(), boost::beast::http::status::ok);
}

TEST_F(HttpSocketTest, ReadFailure)
{
    boost::beast::http::response<boost::beast::http::dynamic_body> res;
    res.result(boost::beast::http::status::ok);

    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, read(_, _, _))
        .WillOnce([](boost::beast::flat_buffer&,
                     boost::beast::http::response<boost::beast::http::dynamic_body>&,
                     boost::system::error_code& ec) { ec = boost::asio::error::connection_refused; });

    boost::system::error_code ec;
    m_socket->Read(res, ec);

    EXPECT_TRUE(ec);
    EXPECT_EQ(ec, boost::asio::error::connection_refused);
}

TEST_F(HttpSocketTest, ReadException)
{
    boost::beast::http::response<boost::beast::http::dynamic_body> res;
    res.result(boost::beast::http::status::ok);

    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, read(_, _, _))
        .WillOnce([](boost::beast::flat_buffer&,
                     boost::beast::http::response<boost::beast::http::dynamic_body>&,
                     boost::system::error_code&) { throw std::runtime_error("Test exception"); });

    boost::system::error_code ec;
    m_socket->Read(res, ec);

    EXPECT_FALSE(ec);
}

TEST_F(HttpSocketTest, AsyncReadSuccess)
{
    boost::beast::http::response<boost::beast::http::dynamic_body> res;
    res.result(boost::beast::http::status::ok);

    EXPECT_CALL(*m_mockHelper, expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS))).Times(1);
    EXPECT_CALL(*m_mockHelper, async_read(testing::_, testing::_, testing::_))
        .WillOnce(
            [](boost::beast::flat_buffer&,
               boost::beast::http::response<boost::beast::http::dynamic_body>&,
               boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                ec = boost::system::error_code {};
                co_return;
            });

    boost::system::error_code result_ec;
    boost::asio::co_spawn(
        *m_ioContext,
        [&]() -> boost::asio::awaitable<void> { co_await m_socket->AsyncRead(res, result_ec); },
        boost::asio::detached);

    m_ioContext->run();

    EXPECT_FALSE(result_ec);
    EXPECT_EQ(res.result(), boost::beast::http::status::ok);
}

TEST_F(HttpSocketTest, AsyncReadFailure)
{
    boost::beast::http::response<boost::beast::http::dynamic_body> res;
    res.result(boost::beast::http::status::ok);

    EXPECT_CALL(*m_mockHelper, expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS))).Times(1);
    EXPECT_CALL(*m_mockHelper, async_read(testing::_, testing::_, testing::_))
        .WillOnce(
            [](boost::beast::flat_buffer&,
               boost::beast::http::response<boost::beast::http::dynamic_body>&,
               boost::system::error_code& ec) -> boost::asio::awaitable<void>
            {
                ec = boost::asio::error::connection_refused;
                co_return;
            });

    boost::system::error_code result_ec;
    boost::asio::co_spawn(
        *m_ioContext,
        [&]() -> boost::asio::awaitable<void> { co_await m_socket->AsyncRead(res, result_ec); },
        boost::asio::detached);

    m_ioContext->run();

    EXPECT_TRUE(result_ec);
}

TEST_F(HttpSocketTest, AsyncReadException)
{
    boost::beast::http::response<boost::beast::http::dynamic_body> res;
    res.result(boost::beast::http::status::ok);

    EXPECT_CALL(*m_mockHelper, expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS))).Times(1);
    EXPECT_CALL(*m_mockHelper, async_read(testing::_, testing::_, testing::_))
        .WillOnce([](boost::beast::flat_buffer&,
                     boost::beast::http::response<boost::beast::http::dynamic_body>&,
                     boost::system::error_code&) -> boost::asio::awaitable<void>
                  { throw std::runtime_error("Test exception"); });

    boost::system::error_code result_ec;
    boost::asio::co_spawn(
        *m_ioContext,
        [&]() -> boost::asio::awaitable<void> { co_await m_socket->AsyncRead(res, result_ec); },
        boost::asio::detached);

    m_ioContext->run();

    EXPECT_TRUE(result_ec);
}

TEST_F(HttpSocketTest, CloseSocket)
{
    EXPECT_CALL(*m_mockHelper, expires_after(_)).Times(1);
    EXPECT_CALL(*m_mockHelper, connect(_, _)).Times(1);
    EXPECT_CALL(*m_mockHelper, close()).Times(1);

    boost::system::error_code ec;
    m_socket->Connect(dummyResults, ec);
    EXPECT_NO_THROW(m_socket->Close());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines,cppcoreguidelines-avoid-reference-coroutine-parameters,
// clang-diagnostic-unused-result)
