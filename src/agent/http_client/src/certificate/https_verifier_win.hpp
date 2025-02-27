#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// clang-format off
#include <windows.h>
#include <wincrypt.h>
// clang-format on

#include "icert_store_utils_win.hpp"
#include "ix509_utils.hpp"

#include <boost/asio/ssl.hpp>

#include <memory>
#include <string>

namespace https_socket_verify_utils
{
    using CertContextPtr = std::unique_ptr<const CERT_CONTEXT, std::function<void(const CERT_CONTEXT*)>>;
    using CertStorePtr = std::unique_ptr<void, std::function<void(HCERTSTORE)>>;
    using CertChainContextPtr =
        std::unique_ptr<const CERT_CHAIN_CONTEXT, std::function<void(const CERT_CHAIN_CONTEXT*)>>;

    class HttpsVerifierWin
    {
    public:
        /// @brief Constructor to initialize the verifier object.
        /// @param mode The verification mode to use
        /// @param host The hostname to verify against
        /// @param x509Utils The x509 utilities object to use
        /// @param certStoreUtils The certificate store utilities object to use
        HttpsVerifierWin(const std::string& mode,
                         const std::string& host,
                         std::unique_ptr<ICertificateX509Utils>& x509Utils,
                         std::unique_ptr<ICertificateStoreUtilsWin>& certStoreUtils)
            : m_mode(mode)
            , m_host(host)
            , m_x509Utils(std::move(x509Utils))
            , m_certStoreUtils(std::move(certStoreUtils))
        {
            if (!m_x509Utils || !m_certStoreUtils)
            {
                throw std::invalid_argument("Invalid utils pointers");
            }

            m_certContextDeleter = [this](const CERT_CONTEXT* ctx)
            {
                if (ctx)
                {
                    m_certStoreUtils->FreeCertificateContext(ctx);
                }
            };
            m_certStoreDeleter = [this](HCERTSTORE store)
            {
                if (store)
                {
                    m_certStoreUtils->CloseStore(store, 0);
                }
            };
            m_chainContextDeleter = [this](const CERT_CHAIN_CONTEXT* ctx)
            {
                if (ctx)
                {
                    m_certStoreUtils->FreeCertificateChain(ctx);
                }
            };
        }

        /// @brief Verifies the certificate of the HTTPS connection
        /// @param ctx The verification context
        /// @return True if the certificate is valid, false otherwise
        bool Verify(boost::asio::ssl::verify_context& ctx);

    private:
        /// @brief Extracts the server certificate from the context
        /// @param ctx The verification context
        /// @param certCtx The extracted certificate
        /// @return True if the certificate was extracted, false otherwise
        bool ExtractCertificate(boost::asio::ssl::verify_context& ctx, CertContextPtr& certCtx);

        /// @brief Creates a certificate store connections to the Windows certificate store
        /// @param store The created certificate store
        /// @return True if the store was created, false otherwise
        bool CreateCertStore(CertStorePtr& store);

        /// @brief Verifies the certificate chain
        /// @param certCtx The certificate to verify
        /// @param store The certificate store to use
        /// @param chainCtx The certificate chain
        /// @return True if the chain was verified, false otherwise
        bool
        VerifyCertificateChain(const CertContextPtr& certCtx, const CertStorePtr& store, CertChainContextPtr& chainCtx);

        /// @brief Validates the chain policy
        /// @param chainCtx The certificate chain
        /// @return True if the chain policy was validated, false otherwise
        bool ValidateChainPolicy(const CertChainContextPtr& chainCtx);

        /// @brief Validates the hostname
        /// @param certCtx The certificate to validate
        /// @return True if the hostname was validated, false otherwise
        bool ValidateHostname(const CertContextPtr& certCtx);

        /// @brief The mode to use for the verification
        std::string m_mode;

        /// @brief The hostname to verify
        std::string m_host;

        /// @brief The x509 utilities object
        std::unique_ptr<ICertificateX509Utils> m_x509Utils;

        /// @brief The certificate store utilities object
        std::unique_ptr<ICertificateStoreUtilsWin> m_certStoreUtils;

        /// @brief The certificate context deleter
        std::function<void(const CERT_CONTEXT*)> m_certContextDeleter;

        /// @brief The certificate store deleter
        std::function<void(HCERTSTORE)> m_certStoreDeleter;

        /// @brief The certificate chain context deleter
        std::function<void(const CERT_CHAIN_CONTEXT*)> m_chainContextDeleter;
    };
} // namespace https_socket_verify_utils
