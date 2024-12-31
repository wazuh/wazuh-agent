#pragma once

#include <boost/asio/ssl.hpp>

#include <string>

namespace https_socket_verify_utils
{
    /// @brief Verifies the certificate of the HTTPS connection
    /// @param preverified The result of the pre-verification
    /// @param ctx The verification context
    /// @param mode The verification mode to use
    /// @param host The hostname to verify against
    /// @return True if the certificate is valid, false otherwise
    bool VerifyCertificate(bool preverified,
                           boost::asio::ssl::verify_context& ctx,
                           const std::string& mode,
                           const std::string& host);
} // namespace https_socket_verify_utils
