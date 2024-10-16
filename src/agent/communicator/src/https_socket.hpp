#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

namespace http_client
{
    class HttpsSocket : public IHttpSocket
    {
    public:
        HttpsSocket(const boost::asio::any_io_executor& io_context)
            : m_ctx(boost::asio::ssl::context::sslv23)
            , m_ssl_socket(io_context, m_ctx)
        {
            m_ctx.set_verify_mode(boost::asio::ssl::verify_peer);
        }

        void Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) override
        {
            boost::asio::connect(m_ssl_socket.next_layer(), endpoints.begin(), endpoints.end());

            m_ssl_socket.handshake(boost::asio::ssl::stream_base::client);
        }

        boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                  boost::system::error_code& code) override
        {
            co_await boost::asio::async_connect(
                m_ssl_socket.lowest_layer(), endpoints, boost::asio::redirect_error(boost::asio::use_awaitable, code));

            co_await m_ssl_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                                  boost::asio::redirect_error(boost::asio::use_awaitable, code));
        }

        void Write(const boost::beast::http::request<boost::beast::http::string_body>& req) override
        {
            boost::beast::http::write(m_ssl_socket, req);
        }

        boost::asio::awaitable<void> AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                                boost::beast::error_code& ec) override
        {
            co_await boost::beast::http::async_write(
                m_ssl_socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res) override
        {
            boost::beast::flat_buffer buffer;
            boost::beast::http::read(m_ssl_socket, buffer, res);
        }

        void ReadToFile(boost::beast::http::response_parser<boost::beast::http::dynamic_body>& res,
                        const std::string& dstFilePath) override
        {
            http_client_utils::ReadToFile(m_ssl_socket, res, dstFilePath);
        }

        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::beast::error_code& ec) override
        {
            boost::beast::flat_buffer buffer;
            co_await boost::beast::http::async_read(
                m_ssl_socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        void Close() override
        {
            m_ssl_socket.shutdown();
        }

    private:
        boost::asio::ssl::context m_ctx;
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_ssl_socket;
    };
} // namespace http_client
