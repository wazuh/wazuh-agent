#include "communicator.hpp"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

namespace communicator
{

    Communicator::Communicator()
    {

    }

    int Communicator::SendAuthenticationRequest()
    {
        json bodyJson = {{communicator::uuidKey, communicator::kUUID},
                         {communicator::passwordKey, communicator::kPASSWORD}};
        http::response<http::dynamic_body> res = sendHttpRequest(http::verb::post, "/authentication", "", bodyJson.dump());
        m_token = beast::buffers_to_string(res.body().data());
        return res.result_int();
    }

    int Communicator::SendRegistrationRequest()
    {
        // TO DO: Create uuid to send to the server.
        json bodyJson = {{communicator::uuidKey, communicator::kUUID},
                         {communicator::nameKey, communicator::kNAME},
                         {communicator::ipKey, communicator::kIP}};
        http::response<http::dynamic_body> res = sendHttpRequest(http::verb::post, "/agents", m_token, bodyJson.dump());
        return res.result_int();
    }

    http::response<http::dynamic_body> Communicator::sendHttpRequest(http::verb method,
                                                                     const std::string& url,
                                                                     const std::string& token,
                                                                     const std::string& body)
    {
        http::response<http::dynamic_body> res;
        try
        {
            net::io_context io_context;
            tcp::resolver resolver(io_context);
            auto const results = resolver.resolve("localhost", "8080");

            tcp::socket socket(io_context);
            boost::asio::connect(socket, results.begin(), results.end());

            http::request<http::string_body> req {method, url, 11};
            req.set(http::field::host, "localhost");
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req.set(http::field::accept, "application/json");

            if (!token.empty())
            {
                req.set(http::field::authorization, "Bearer " + token);
            }

            if (!body.empty())
            {
                req.set(http::field::content_type, "application/json");
                req.body() = body;
                req.prepare_payload();
            }

            http::write(socket, req);

            beast::flat_buffer buffer;

            http::read(socket, buffer, res);

            std::cout << "Response code: " << res.result_int() << std::endl;
            std::cout << "Response body: " << beast::buffers_to_string(res.body().data()) << std::endl;
        }
        catch (std::exception const& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        return res;
    }

} // namespace communicator
