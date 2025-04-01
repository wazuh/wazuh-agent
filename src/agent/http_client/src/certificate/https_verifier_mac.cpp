#include "https_verifier_mac.hpp"
#include "cert_store_utils_mac.hpp"
#include "icert_store_utils_mac.hpp"
#include "ix509_utils.hpp"
#include "x509_utils.hpp"
#include <logger.hpp>

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <boost/asio/ssl.hpp>

#include <memory>
#include <string>

namespace https_socket_verify_utils
{
    bool HttpsVerifierMac::Verify(boost::asio::ssl::verify_context& ctx)
    {
        if (!m_x509Utils || !m_certStoreUtils)
        {
            LogError("Invalid utils pointers");
            return false;
        }

        CFDataPtr certData;
        if (!ExtractCertificate(ctx, certData))
        {
            return false;
        }

        SecTrustPtr trust;
        if (!CreateTrustObject(certData, trust))
        {
            return false;
        }

        if (!EvaluateTrust(trust))
        {
            return false;
        }

        if (m_mode == "full")
        {
            const SecCertificatePtr serverCert(m_certStoreUtils->CreateCertificate(certData.get()), m_deleter);
            if (!serverCert || !ValidateHostname(serverCert))
            {
                return false;
            }
        }

        return true;
    }

    bool HttpsVerifierMac::ExtractCertificate(boost::asio::ssl::verify_context& ctx, CFDataPtr& certData)
    {
        STACK_OF(X509)* certChain = m_x509Utils->GetCertChain(ctx.native_handle());
        if (!certChain || m_x509Utils->GetCertificateCount(certChain) == 0)
        {
            LogError("No certificates in the chain.");
            return false;
        }

        X509* cert = m_x509Utils->GetCertificateFromChain(certChain, 0);
        if (!cert)
        {
            LogError("The server certificate could not be obtained.");
            return false;
        }

        unsigned char* certRawData = nullptr;
        const int certLen = m_x509Utils->EncodeCertificateToDER(cert, &certRawData);
        if (certLen <= 0)
        {
            LogError("Failed to encode certificate to DER.");
            return false;
        }

        certData = CFDataPtr(m_certStoreUtils->CreateCFData(certRawData, certLen), m_deleter);
        OPENSSL_free(certRawData);

        return certData != nullptr;
    }

    bool HttpsVerifierMac::CreateTrustObject(const CFDataPtr& certData, SecTrustPtr& trust)
    {
        const SecCertificatePtr serverCert(m_certStoreUtils->CreateCertificate(certData.get()), m_deleter);
        if (!serverCert)
        {
            LogError("Failed to create SecCertificateRef.");
            return false;
        }

        const void* certArrayValues[] = {serverCert.get()};

        const CFArrayPtr certArray(m_certStoreUtils->CreateCertArray(certArrayValues, 1), m_deleter);
        const SecPolicyPtr policy(m_certStoreUtils->CreateSSLPolicy(true, ""), m_deleter);

        SecTrustRef rawTrust = nullptr;
        const OSStatus status = m_certStoreUtils->CreateTrustObject(certArray.get(), policy.get(), &rawTrust);

        if (status != errSecSuccess)
        {
            m_deleter(rawTrust);
            LogError("Failed to create trust object.");
            return false;
        }

        trust = SecTrustPtr(rawTrust, m_deleter);
        return true;
    }

    bool HttpsVerifierMac::EvaluateTrust(const SecTrustPtr& trust)
    {
        CFErrorRef errorRef = nullptr;
        const bool trustResult = m_certStoreUtils->EvaluateTrust(trust.get(), &errorRef);

        if (!trustResult && errorRef)
        {
            const CFErrorPtr error(const_cast<__CFError*>(errorRef), m_deleter);

            const CFStringPtr errorDesc(m_certStoreUtils->CopyErrorDescription(errorRef), m_deleter);
            std::string errorString = m_certStoreUtils->GetStringCFString(errorDesc.get());
            LogError("Trust evaluation failed: {}", errorString);
        }

        return trustResult;
    }

    bool HttpsVerifierMac::ValidateHostname(const SecCertificatePtr& serverCert)
    {
        const CFStringPtr sanString(m_certStoreUtils->CopySubjectSummary(serverCert.get()), m_deleter);
        if (!sanString)
        {
            LogError("Failed to retrieve SAN or CN for hostname validation.");
            return false;
        }

        bool hostnameMatches = false;
        std::string sanStringStr = m_certStoreUtils->GetStringCFString(sanString.get());
        if (!sanStringStr.empty())
        {
            hostnameMatches = (m_host == sanStringStr);
        }

        if (!hostnameMatches)
        {
            LogError("The hostname '{}' does not match the certificate's SAN or CN '{}'.", m_host, sanStringStr);
        }

        return hostnameMatches;
    }

} // namespace https_socket_verify_utils
