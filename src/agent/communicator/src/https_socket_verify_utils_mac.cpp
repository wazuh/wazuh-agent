#include "certificate_utils.hpp"
#include <https_socket_verify_utils.hpp>
#include <https_verifier_mac.hpp>
#include <icertificate_utils.hpp>

#include <boost/asio/ssl.hpp>

#include <memory>
#include <string>

namespace https_socket_verify_utils
{
    bool VerifyCertificate([[maybe_unused]] bool preverified,
                           boost::asio::ssl::verify_context& ctx,
                           const std::string& mode,
                           const std::string& host)
    {
        std::unique_ptr<ICertificateUtils> certUtils = std::make_unique<CertificateUtilsWrapper>();

        HttpsVerifier verifier(mode, host, certUtils);

        return verifier.Verify(ctx);
    }
} // namespace https_socket_verify_utils
