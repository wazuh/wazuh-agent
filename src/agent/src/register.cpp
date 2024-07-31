#include "register.hpp"
#include "configuration_parser/include/configuration_parser.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace uuids = boost::uuids;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

namespace registration
{
    http::response<http::dynamic_body> sendRequest(const std::string& managerIp,
                                                   const std::string& port,
                                                   const http::verb method,
                                                   const std::string& url,
                                                   const std::string& token,
                                                   const std::string& body,
                                                   const std::string& user_pass)
    {
        http::response<http::dynamic_body> res;
        try
        {
            boost::asio::io_context io_context;
            tcp::resolver resolver(io_context);
            auto const results = resolver.resolve(managerIp, port);

            tcp::socket socket(io_context);
            boost::asio::connect(socket, results.begin(), results.end());

            http::request<http::string_body> req {method, url, 11};
            req.set(http::field::host, managerIp);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req.set(http::field::accept, "application/json");

            if (!token.empty())
            {
                req.set(http::field::authorization, "Bearer " + token);
            }

            if (!user_pass.empty())
            {
                req.set(http::field::authorization, "Basic " + user_pass);
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
            res.result(http::status::internal_server_error);
            beast::ostream(res.body()) << "Internal server error: " << e.what();
            res.prepare_payload();
        }
        return res;
    }

    std::pair<int, std::string>
    SendAuthenticationRequest(const std::string& managerIp, const std::string& port, const std::string& user_pass)
    {
        http::response<http::dynamic_body> res =
            sendRequest(managerIp, port, http::verb::post, "/authenticate", "", "", user_pass);
        const auto token = beast::buffers_to_string(res.body().data());

        return {res.result_int(), token};
    }

    int SendRegistrationRequest(const std::string& managerIp,
                                const std::string& port,
                                const std::string& token,
                                const std::string& uuid,
                                const std::optional<std::string>& name,
                                const std::optional<std::string>& ip)
    {
        json bodyJson = {{"uuid", uuid}};

        if (name.has_value())
        {
            bodyJson["name"] = name.value();
        }

        if (ip.has_value())
        {
            bodyJson["ip"] = ip.value();
        }

        http::response<http::dynamic_body> res =
            sendRequest(managerIp, port, http::verb::post, "/agents", token, bodyJson.dump(), "");
        return res.result_int();
    }

    std::string GenerateUuid()
    {
        return to_string(uuids::random_generator()());
    }

} // namespace registration

bool RegisterAgent(const std::string& user,
                   const std::string& password,
                   const std::optional<std::string>& name,
                   const std::optional<std::string>& ip)
{
    const auto uuid = registration::GenerateUuid();

    const auto configurationParser = std::make_shared<configuration::ConfigurationParser>();
    const auto managerIp = configurationParser->GetConfig<std::string>("agent", "manager_ip");
    const auto port = configurationParser->GetConfig<std::string>("agent", "port");

    auto [authResultCode, token] = registration::SendAuthenticationRequest(managerIp, port, user + ":" + password);
    if (authResultCode != 200)
    {
        std::cout << "Authentication error: " << authResultCode << std::endl;
        return false;
    }

    if (int registrationResultCode = registration::SendRegistrationRequest(managerIp, port, token, uuid, name, ip);
        registrationResultCode != 200)
    {
        std::cout << "Registration error: " << registrationResultCode << std::endl;
        return false;
    }

    return true;
}
