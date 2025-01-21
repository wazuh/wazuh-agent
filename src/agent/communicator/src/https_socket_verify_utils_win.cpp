#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// clang-format off
#include <windows.h>
#include <wincrypt.h>
// clang-format on

#include "cert_store_utils_win.hpp"
#include "https_socket_verify_utils.hpp"
#include "https_verifier_win.hpp"
#include "icert_store_utils_win.hpp"
#include "ix509_utils.hpp"
#include "x509_utils.hpp"

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
        std::unique_ptr<ICertificateX509Utils> x509Utils = std::make_unique<CertificateX509UtilsWrapper>();
        std::unique_ptr<ICertificateStoreUtilsWin> certStoreUtils = std::make_unique<CertificateStoreUtilsWrapperWin>();

        HttpsVerifierWin verifier(mode, host, x509Utils, certStoreUtils);

        return verifier.Verify(ctx);
    }
} // namespace https_socket_verify_utils
