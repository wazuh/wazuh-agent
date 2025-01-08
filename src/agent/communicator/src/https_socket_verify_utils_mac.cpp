#include "certificate_utils.hpp"
#include <https_socket_verify_utils.hpp>
#include <icertificate_utils.hpp>
#include <logger.hpp>

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <boost/asio/ssl.hpp>

#include <string>

namespace https_socket_verify_utils
{
    class HttpsVerifier
    {
    public:
        HttpsVerifier(const std::string& mode, const std::string& host, std::unique_ptr<ICertificateUtils>& utils)
            : m_mode(mode)
            , m_host(host)
            , m_utils(utils)
        {
        }

        bool Verify(boost::asio::ssl::verify_context& ctx);

    private:
        bool ExtractCertificate(boost::asio::ssl::verify_context& ctx, CFDataRef& certData);
        bool CreateTrustObject(CFDataRef certData, SecTrustRef& trust);
        bool EvaluateTrust(SecTrustRef trust);
        bool ValidateHostname(SecCertificateRef cert);

        std::string m_mode;
        std::string m_host;
        std::unique_ptr<ICertificateUtils>& m_utils;
    };

    bool VerifyCertificate([[maybe_unused]] bool preverified,
                           boost::asio::ssl::verify_context& ctx,
                           const std::string& mode,
                           const std::string& host)
    {
        std::unique_ptr<ICertificateUtils> certUtils = std::make_unique<CertificateUtilsWrapper>();

        HttpsVerifier verifier(mode, host, certUtils);

        return verifier.Verify(ctx);
    }

    bool HttpsVerifier::Verify(boost::asio::ssl::verify_context& ctx)
    {
        CFDataRef certData = nullptr;
        if (!ExtractCertificate(ctx, certData))
        {
            return false;
        }

        SecTrustRef trust = nullptr;
        if (!CreateTrustObject(certData, trust))
        {
            return false;
        }

        if (!EvaluateTrust(trust))
        {
            m_utils->ReleaseCFObject(certData);
            m_utils->ReleaseCFObject(trust);
            return false;
        }

        SecCertificateRef serverCert = m_utils->CreateCertificate(certData);
        if (m_mode == "full" && !ValidateHostname(serverCert))
        {
            m_utils->ReleaseCFObject(certData);
            m_utils->ReleaseCFObject(serverCert);
            m_utils->ReleaseCFObject(trust);
            return false;
        }

        m_utils->ReleaseCFObject(certData);
        m_utils->ReleaseCFObject(serverCert);
        m_utils->ReleaseCFObject(trust);
        return true;
    }

    bool HttpsVerifier::ExtractCertificate(boost::asio::ssl::verify_context& ctx, CFDataRef& certData)
    {
        STACK_OF(X509)* certChain = m_utils->GetCertChain(ctx.native_handle());
        if (!certChain || m_utils->GetCertificateCount(certChain) == 0)
        {
            LogError("No certificates in the chain.");
            return false;
        }

        X509* cert = m_utils->GetCertificateFromChain(certChain, 0);
        if (!cert)
        {
            LogError("The server certificate could not be obtained.");
            return false;
        }

        unsigned char* certRawData = nullptr;
        const int certLen = m_utils->EncodeCertificateToDER(cert, &certRawData);
        if (certLen <= 0)
        {
            LogError("Failed to encode certificate to DER.");
            return false;
        }

        certData = m_utils->CreateCFData(certRawData, certLen);
        OPENSSL_free(certRawData);

        return certData != nullptr;
    }

    bool HttpsVerifier::CreateTrustObject(CFDataRef certData, SecTrustRef& trust)
    {
        SecCertificateRef serverCert = m_utils->CreateCertificate(certData);

        if (!serverCert)
        {
            LogError("Failed to create SecCertificateRef.");
            return false;
        }

        const void* certArrayValues[] = {serverCert};

        CFArrayRef certArray = m_utils->CreateCertArray(certArrayValues, 1);
        SecPolicyRef policy = m_utils->CreateSSLPolicy(true, "");

        OSStatus status = m_utils->CreateTrustObject(certArray, policy, &trust);
        m_utils->ReleaseCFObject(certArray);
        m_utils->ReleaseCFObject(policy);
        m_utils->ReleaseCFObject(serverCert);

        return (status == errSecSuccess && trust != nullptr);
    }

    bool HttpsVerifier::EvaluateTrust(SecTrustRef trust)
    {
        CFErrorRef error = nullptr;
        const bool trustResult = m_utils->EvaluateTrust(trust, &error);
        if (!trustResult && error)
        {
            CFStringRef errorDesc = m_utils->CopyErrorDescription(error);
            std::string errorString = m_utils->GetStringCFString(errorDesc);
            LogError("Trust evaluation failed: {}", errorString);
            m_utils->ReleaseCFObject(errorDesc);
            m_utils->ReleaseCFObject(error);
        }
        return trustResult;
    }

    bool HttpsVerifier::ValidateHostname(SecCertificateRef serverCert)
    {
        CFStringRef sanString = SecCertificateCopySubjectSummary(serverCert);
        if (!sanString)
        {
            LogError("Failed to retrieve SAN or CN for hostname validation.");
            return false;
        }

        bool hostnameMatches = false;
        std::string sanStringStr = m_utils->GetStringCFString(sanString);
        if (!sanStringStr.empty())
        {
            hostnameMatches = (m_host == sanStringStr);
        }

        m_utils->ReleaseCFObject(sanString);
        if (!hostnameMatches)
        {
            LogError("The hostname '{}' does not match the certificate's SAN or CN '{}'.", m_host, sanStringStr);
        }

        return hostnameMatches;
    }
} // namespace https_socket_verify_utils
