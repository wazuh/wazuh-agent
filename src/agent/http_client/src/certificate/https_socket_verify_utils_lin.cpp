#include "https_socket_verify_utils.hpp"
#include <boost/asio/ssl.hpp>

#include <string>

namespace https_socket_verify_utils
{
    bool VerifyCertificate(bool preverified,
                           boost::asio::ssl::verify_context& ctx,
                           const std::string& mode,
                           const std::string& host)
    {
        if (mode == "certificate")
        {
            return preverified;
        }
        else if (mode == "full")
        {
            const boost::asio::ssl::rfc2818_verification verifier(host);
            return verifier(preverified, ctx);
        }
        return false;
    }
} // namespace https_socket_verify_utils
