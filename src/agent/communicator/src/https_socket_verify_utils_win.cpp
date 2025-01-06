#include <https_socket_verify_utils.hpp>
#include <logger.hpp>

#include <boost/asio/ssl.hpp>
#include <wincrypt.h>
#include <windows.h>

namespace https_socket_verify_utils
{
    bool VerifyCertificate([[maybe_unused]] bool preverified,
                           boost::asio::ssl::verify_context& ctx,
                           const std::string& mode,
                           const std::string& host)
    {
        STACK_OF(X509)* certChain = X509_STORE_CTX_get_chain(ctx.native_handle());
        if (!certChain || sk_X509_num(certChain) == 0)
        {
            LogError("No certificates in the chain.");
            return false;
        }

        X509* cert = sk_X509_value(certChain, 0);
        if (!cert)
        {
            LogError("The server certificate could not be obtained.");
            return false;
        }

        // Convert certificate to DER format
        unsigned char* der = nullptr;
        int derLen = i2d_X509(cert, &der);
        if (derLen <= 0)
        {
            LogError("Certificate conversion to DER failed.");
            return false;
        }

        // Create the certificate context
        PCCERT_CONTEXT certCtx = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, der, derLen);
        OPENSSL_free(der);

        if (!certCtx)
        {
            LogError("The certificate context could not be created.");
            return false;
        }

        // Open the Windows certificate store
        HCERTSTORE hStore = CertOpenSystemStoreA(NULL, "ROOT");
        if (!hStore)
        {
            LogError("The Windows certificate store could not be opened.");
            CertFreeCertificateContext(certCtx);
            return false;
        }

        // Create the certificate chain context
        CERT_CHAIN_PARA chainPara = {sizeof(CERT_CHAIN_PARA)};
        PCCERT_CHAIN_CONTEXT chainCtx = nullptr;

        bool result = CertGetCertificateChain(NULL, certCtx, NULL, hStore, &chainPara, 0, NULL, &chainCtx);

        if (!result || !chainCtx)
        {
            LogError("The certificate chain could not be verified.");
        }

        // Validate the SSL policy of the chain
        CERT_CHAIN_POLICY_PARA policyPara = {sizeof(CERT_CHAIN_POLICY_PARA)};
        CERT_CHAIN_POLICY_STATUS policyStatus = {sizeof(CERT_CHAIN_POLICY_STATUS)};
        policyPara.dwFlags = 0;

        if (!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_BASE, chainCtx, &policyPara, &policyStatus))
        {
            LogError("Error verifying certificate chain policy.");
            result = false;
        }
        else if (policyStatus.dwError != 0)
        {
            LogError("Certification policy error: {}", policyStatus.dwError);
            result = false;
        }

        if (result && mode == "full")
        {
            // Obtain SAN from the certificate
            STACK_OF(GENERAL_NAME)* sanNames =
                static_cast<STACK_OF(GENERAL_NAME)*>(X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL));
            bool hostValidated = false;

            if (sanNames)
            {
                int sanCount = sk_GENERAL_NAME_num(sanNames);
                for (int i = 0; i < sanCount; ++i)
                {
                    const GENERAL_NAME* san = sk_GENERAL_NAME_value(sanNames, i);
                    if (san->type == GEN_DNS)
                    {
                        const char* dnsName = reinterpret_cast<const char*>(ASN1_STRING_get0_data(san->d.dNSName));
                        if (host == dnsName)
                        {
                            hostValidated = true;
                            break;
                        }
                    }
                }
                GENERAL_NAMES_free(sanNames);
            }

            // If no SAN was found, check the CN
            if (!hostValidated)
            {
                DWORD subjectSize = CertGetNameStringA(certCtx, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, NULL, 0);

                if (subjectSize > 0)
                {
                    std::string subjectName(subjectSize, '\0');
                    CertGetNameStringA(
                        certCtx, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, subjectName.data(), subjectSize);

                    subjectName.pop_back();

                    if (host == subjectName)
                    {
                        hostValidated = true;
                    }
                    else
                    {
                        LogError("The host name does not match the CN of the certificate.");
                    }
                }
                else
                {
                    LogError("The name of the subject of the certificate could not be obtained.");
                }
            }

            if (!hostValidated)
            {
                LogError("The host name does not match the SAN or the CN.");
                result = false;
            }
        }

        // Free resources
        if (chainCtx)
        {
            CertFreeCertificateChain(chainCtx);
        }
        CertCloseStore(hStore, 0);
        CertFreeCertificateContext(certCtx);

        return result;
    }

} // namespace https_socket_verify_utils
