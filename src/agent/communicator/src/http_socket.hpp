#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <fstream>

namespace http_client
{
    class HttpSocket : public IHttpSocket
    {
    public:
        HttpSocket(const boost::asio::any_io_executor& io_context)
            : m_socket(io_context)
        {
        }

        void Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) override
        {
            boost::asio::connect(m_socket, endpoints);
        }

        boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                  boost::system::error_code& code) override
        {
            co_await boost::asio::async_connect(
                m_socket, endpoints, boost::asio::redirect_error(boost::asio::use_awaitable, code));
        }

        void Write(const boost::beast::http::request<boost::beast::http::string_body>& req) override
        {
            boost::beast::http::write(m_socket, req);
        }

        boost::asio::awaitable<void> AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                                boost::beast::error_code& ec) override
        {
            co_await boost::beast::http::async_write(
                m_socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res) override
        {
            boost::beast::flat_buffer buffer;
            boost::beast::http::read(m_socket, buffer, res);
        }

        void ReadToFile(boost::beast::http::response_parser<boost::beast::http::dynamic_body>& res,
                        const std::string& output_file) override
        {
            res.body_limit(std::numeric_limits<std::uint64_t>::max());
            boost::beast::flat_buffer buffer;
            boost::system::error_code error;

            boost::beast::http::read_header(m_socket, buffer, res, error);

            if (error && error != boost::beast::http::error::need_buffer)
            {
                throw boost::system::system_error(error);
            }

            unsigned int status_code = res.get().result_int();
            if (status_code != 200)
            {
                return;
            }

            std::ofstream file(output_file, std::ios::binary);
            if (!file)
            {
                throw std::runtime_error("The file could not be opened for writing: " + output_file);
            }

            while (!res.is_done())
            {
                boost::beast::http::read(m_socket, buffer, res, error);

                if (error && error != boost::beast::http::error::need_buffer && error != boost::asio::error::eof)
                {
                    file.close();
                    throw boost::system::system_error(error);
                }

                auto body_data = res.get().body().data();

                for (auto const& buffer_seq : body_data)
                {
                    std::streamsize chunk_size = static_cast<std::streamsize>(buffer_seq.size());
                    file.write(static_cast<const char*>(buffer_seq.data()), chunk_size);
                }

                res.get().body().consume(res.get().body().size());
            }

            file.close();
        }

        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::beast::error_code& ec) override
        {
            boost::beast::flat_buffer buffer;
            co_await boost::beast::http::async_read(
                m_socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        void Close() override
        {
            m_socket.close();
        }

    private:
        boost::asio::ip::tcp::socket m_socket;
    };
} // namespace http_client
