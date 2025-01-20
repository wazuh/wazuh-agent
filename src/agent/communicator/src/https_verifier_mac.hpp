#pragma once

#include "icert_store_utils_mac.hpp"
#include "ix509_utils.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <boost/asio/ssl.hpp>

#include <memory>
#include <string>

namespace https_socket_verify_utils
{
    using CFDataPtr = std::unique_ptr<const __CFData, std::function<void(CFTypeRef)>>;
    using CFArrayPtr = std::unique_ptr<const __CFArray, std::function<void(CFTypeRef)>>;
    using CFStringPtr = std::unique_ptr<const __CFString, std::function<void(CFTypeRef)>>;
    using SecTrustPtr = std::unique_ptr<__SecTrust, std::function<void(CFTypeRef)>>;
    using CFErrorPtr = std::unique_ptr<__CFError, std::function<void(CFTypeRef)>>;
    using SecCertificatePtr = std::unique_ptr<__SecCertificate, std::function<void(CFTypeRef)>>;
    using SecPolicyPtr = std::unique_ptr<__SecPolicy, std::function<void(CFTypeRef)>>;

    class HttpsVerifier
    {
    public:
        /// @brief Constructor to initialize the verifier object.
        /// @param mode The verification mode to use
        /// @param host The hostname to verify against
        /// @param x509Utils The x509 utilities object to use
        /// @param certStoreUtils The certificate store utilities object to use
        HttpsVerifier(const std::string& mode,
                      const std::string& host,
                      std::unique_ptr<ICertificateX509Utils>& x509Utils,
                      std::unique_ptr<ICertificateStoreUtilsMac>& certStoreUtils)
            : m_mode(mode)
            , m_host(host)
            , m_x509Utils(std::move(x509Utils))
            , m_certStoreUtils(std::move(certStoreUtils))
        {
            m_deleter = [this](CFTypeRef obj)
            {
                if (obj)
                {
                    m_certStoreUtils->ReleaseCFObject(obj);
                }
            };
        }

        /// @brief Verifies the certificate of the HTTPS connection
        /// @param ctx The verification context
        /// @return True if the certificate is valid, false otherwise
        bool Verify(boost::asio::ssl::verify_context& ctx);

    private:
        /// @brief Extracts the certificate from the verification context
        /// @param ctx The verification context
        /// @param certData The extracted certificate
        /// @return True if the certificate was extracted successfully, false otherwise
        bool ExtractCertificate(boost::asio::ssl::verify_context& ctx, CFDataPtr& certData);

        /// @brief Creates a trust object from the certificate
        /// @param certData The certificate data
        /// @param trust The created trust object
        /// @return True if the trust object was created successfully, false otherwise
        bool CreateTrustObject(const CFDataPtr& certData, SecTrustPtr& trust);

        /// @brief Evaluates the trust status of the trust object
        /// @param trust The trust object to evaluate
        /// @return True if the trust evaluation succeeded, false otherwise
        bool EvaluateTrust(const SecTrustPtr& trust);

        /// @brief Validates the hostname of the server certificate
        /// @param cert The server certificate
        /// @return True if the hostname is valid, false otherwise
        bool ValidateHostname(const SecCertificatePtr& cert);

        /// @brief The verification mode to use
        std::string m_mode;

        /// @brief The hostname to verify against
        std::string m_host;

        /// @brief The x509 utilities object to use
        std::unique_ptr<ICertificateX509Utils> m_x509Utils;

        /// @brief The certificate store utilities object to use
        std::unique_ptr<ICertificateStoreUtilsMac> m_certStoreUtils;

        /// @brief The deleter function to release CFTypeRef objects
        std::function<void(CFTypeRef)> m_deleter;
    };
} // namespace https_socket_verify_utils
