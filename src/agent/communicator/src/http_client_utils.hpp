#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system.hpp>

#include <fstream>
#include <ios>
#include <string>

namespace http_client_utils
{
    /// @brief Reads a response from a socket and writes it to a file
    /// @param socket The socket to read from
    /// @param res The response to use
    /// @param dstFilePath The path to the file to write to
    template<typename SocketType>
    void ReadToFile(SocketType& socket,
                    boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                    const std::string& dstFilePath)
    {
        boost::beast::http::response_parser<boost::beast::http::dynamic_body> res_parser;

        res_parser.body_limit(std::numeric_limits<std::uint64_t>::max());
        boost::beast::flat_buffer buffer;
        boost::system::error_code error;

        boost::beast::http::read_header(socket, buffer, res_parser, error);

        if (error && error != boost::beast::http::error::need_buffer)
        {
            throw boost::system::system_error(error);
        }

        unsigned int statusCode = res_parser.get().result_int();
        if (statusCode != 200)
        {
            return;
        }

        std::ofstream file(dstFilePath, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("The file could not be opened for writing: " + dstFilePath);
        }

        while (!res_parser.is_done())
        {
            boost::beast::http::read(socket, buffer, res_parser, error);

            if (error && error != boost::beast::http::error::need_buffer && error != boost::asio::error::eof)
            {
                file.close();
                throw boost::system::system_error(error);
            }

            auto bodyData = res_parser.get().body().data();

            for (auto const& bufferSeq : bodyData)
            {
                std::streamsize chunkSize = static_cast<std::streamsize>(bufferSeq.size());
                file.write(static_cast<const char*>(bufferSeq.data()), chunkSize);
            }

            res_parser.get().body().consume(res_parser.get().body().size());
        }

        res = res_parser.release();
        file.close();
    }
} // namespace http_client_utils
