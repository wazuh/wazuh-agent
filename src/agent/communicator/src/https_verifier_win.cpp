#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// clang-format off
#include <windows.h>
#include <wincrypt.h>
// clang-format on

#include "https_verifier_win.hpp"
#include "cert_store_utils_win.hpp"
#include "icert_store_utils_win.hpp"
#include "ix509_utils.hpp"
#include "x509_utils.hpp"
#include <logger.hpp>

#include <boost/asio/ssl.hpp>

#include <memory>
#include <string>

namespace https_socket_verify_utils
{
    bool HttpsVerifierWin::Verify(boost::asio::ssl::verify_context& ctx)
    {
        if (!m_x509Utils || !m_certStoreUtils)
        {
            LogError("Invalid utils pointers");
            return false;
        }

        CertContextPtr certCtx;
        if (!ExtractCertificate(ctx, certCtx))
        {
            return false;
        }

        CertStorePtr store;
        if (!CreateCertStore(store))
        {
            return false;
        }

        CertChainContextPtr chainCtx;
        if (!VerifyCertificateChain(certCtx, store, chainCtx))
        {
            return false;
        }

        if (!ValidateChainPolicy(chainCtx))
        {
            return false;
        }

        if (m_mode == "full" && !ValidateHostname(certCtx))
        {
            return false;
        }

        return true;
    }

    bool HttpsVerifierWin::ExtractCertificate(boost::asio::ssl::verify_context& ctx, CertContextPtr& certCtx)
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

        unsigned char* der = nullptr;
        const int derLen = m_x509Utils->EncodeCertificateToDER(cert, &der);
        if (derLen <= 0)
        {
            LogError("Certificate conversion to DER failed.");
            return false;
        }

        PCCERT_CONTEXT rawCertCtx =
            m_certStoreUtils->CreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, der, derLen);
        OPENSSL_free(der);

        if (!rawCertCtx)
        {
            LogError("The certificate context could not be created.");
            return false;
        }

        certCtx = CertContextPtr(rawCertCtx, m_certContextDeleter);
        return true;
    }

    bool HttpsVerifierWin::CreateCertStore(CertStorePtr& store)
    {
        HCERTSTORE rawStore = m_certStoreUtils->OpenSystemStore(NULL, "ROOT");
        if (!rawStore)
        {
            LogError("The Windows certificate store could not be opened.");
            return false;
        }

        store = CertStorePtr(rawStore, m_certStoreDeleter);
        return true;
    }

    bool HttpsVerifierWin::VerifyCertificateChain(const CertContextPtr& certCtx,
                                                  const CertStorePtr& store,
                                                  CertChainContextPtr& chainCtx)
    {
        if (!certCtx || !store)
        {
            LogError("Invalid certificate context or store");
            return false;
        }

        CERT_CHAIN_PARA chainPara = {sizeof(CERT_CHAIN_PARA)};
        PCCERT_CHAIN_CONTEXT rawChainCtx = nullptr;

        if (!m_certStoreUtils->GetCertificateChain(
                NULL, certCtx.get(), NULL, store.get(), &chainPara, 0, NULL, &rawChainCtx))
        {
            LogError("The certificate chain could not be verified.");
            return false;
        }

        chainCtx = CertChainContextPtr(rawChainCtx, m_chainContextDeleter);
        return true;
    }

    bool HttpsVerifierWin::ValidateChainPolicy(const CertChainContextPtr& chainCtx)
    {
        CERT_CHAIN_POLICY_PARA policyPara = {sizeof(CERT_CHAIN_POLICY_PARA)};
        CERT_CHAIN_POLICY_STATUS policyStatus = {sizeof(CERT_CHAIN_POLICY_STATUS)};
        policyPara.dwFlags = 0;

        if (!m_certStoreUtils->VerifyCertificateChainPolicy(
                CERT_CHAIN_POLICY_BASE, chainCtx.get(), &policyPara, &policyStatus))
        {
            LogError("Error verifying certificate chain policy.");
            return false;
        }

        if (policyStatus.dwError != 0)
        {
            LogError("Certification policy error: {}", policyStatus.dwError);
            return false;
        }

        return true;
    }

    bool HttpsVerifierWin::ValidateHostname(const CertContextPtr& certCtx)
    {
        if (!certCtx)
        {
            LogError("Invalid certificate context");
            return false;
        }

        const DWORD subjectSize =
            m_certStoreUtils->GetNameString(certCtx.get(), CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, NULL, 0);

        if (subjectSize <= 1) // Considering the null terminator
        {
            LogError("The name of the subject of the certificate could not be obtained.");
            return false;
        }

        std::string subjectName(subjectSize, '\0');
        const DWORD actualSize = m_certStoreUtils->GetNameString(
            certCtx.get(), CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, subjectName.data(), subjectSize);

        if (actualSize != subjectSize)
        {
            LogError("Failed to get the complete subject name");
            return false;
        }

        subjectName.resize(subjectSize - 1); // Remove null terminator

        if (m_host != subjectName)
        {
            LogError("The host name '{}' does not match the certificate's CN '{}'.", m_host, subjectName);
            return false;
        }

        return true;
    }

} // namespace https_socket_verify_utils
