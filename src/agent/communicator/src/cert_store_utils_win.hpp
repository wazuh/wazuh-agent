#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// clang-format off
#include <windows.h>
#include <wincrypt.h>
// clang-format on

#include <icert_store_utils_win.hpp>

#include <string>

namespace https_socket_verify_utils
{
    /// @brief A wrapper class for managing certificate utilities and operations for Windows.
    ///
    /// This class implements the ICertificateStoreUtilsWin interface and provides methods to interact with
    /// certificates and SSL/TLS trust objects using the Wincrypt frameworks.
    class CertificateStoreUtilsWrapperWin : public ICertificateStoreUtilsWin
    {
    public:
        /// @brief Creates a certificate context from the given encoded certificate data.
        ///
        /// @param dwCertEncodingType The encoding type of the certificate.
        /// @param pbCertEncoded The encoded certificate data.
        /// @param cbCertEncoded The length of the encoded certificate data in bytes.
        /// @return A PCCERT_CONTEXT containing the created certificate context.
        PCCERT_CONTEXT CreateCertificateContext(DWORD dwCertEncodingType,
                                                const BYTE* pbCertEncoded,
                                                DWORD cbCertEncoded) const override
        {
            return CertCreateCertificateContext(dwCertEncodingType, pbCertEncoded, cbCertEncoded);
        }

        /// @brief Opens a system certificate store.
        ///
        /// @param hProv The provider handle that specifies the cryptographic provider.
        /// @param szSubsystemProtocol The subsystem protocol to use to open the store.
        /// @return A HCERTSTORE object representing the opened system store.
        HCERTSTORE OpenSystemStore(HCRYPTPROV_LEGACY hProv, LPCSTR szSubsystemProtocol) const override
        {
            return CertOpenSystemStoreA(hProv, szSubsystemProtocol);
        }

        /// @brief Frees a certificate context.
        ///
        /// @param pCertContext The PCCERT_CONTEXT object to free.
        /// @return TRUE if the context was successfully freed, FALSE otherwise.
        BOOL FreeCertificateContext(PCCERT_CONTEXT pCertContext) const override
        {
            return CertFreeCertificateContext(pCertContext);
        }

        /// @brief Retrieves a certificate chain from the given parameters.
        ///
        /// @param hChainEngine The HCERTCHAINENGINE object to use for building the chain.
        /// @param pCertContext The PCCERT_CONTEXT object for the certificate to build the chain from.
        /// @param pTime The time for which to build the chain; may be NULL.
        /// @param hAdditionalStore An additional store to use when building the chain; may be NULL.
        /// @param pChainPara The chain parameters to use when building the chain.
        /// @param dwFlags The flags to use when building the chain.
        /// @param pvReserved Reserved; must be NULL.
        /// @param ppChainContext A pointer to the PCCERT_CHAIN_CONTEXT object that will contain the built chain.
        /// @return TRUE if the chain was successfully built, FALSE otherwise.
        BOOL GetCertificateChain(HCERTCHAINENGINE hChainEngine,
                                 PCCERT_CONTEXT pCertContext,
                                 LPFILETIME pTime,
                                 HCERTSTORE hAdditionalStore,
                                 PCERT_CHAIN_PARA pChainPara,
                                 DWORD dwFlags,
                                 LPVOID pvReserved,
                                 PCCERT_CHAIN_CONTEXT* ppChainContext) const override
        {
            return CertGetCertificateChain(
                hChainEngine, pCertContext, pTime, hAdditionalStore, pChainPara, dwFlags, pvReserved, ppChainContext);
        }

        /// @brief Verifies the certificate chain policy.
        ///
        /// @param pszPolicyOID A string that contains the object identifier (OID) of the policy to verify.
        /// @param pChainContext A pointer to the certificate chain context to be verified.
        /// @param pPolicyPara A pointer to the policy parameters to use for verification.
        /// @param pPolicyStatus A pointer to the policy status structure that receives the verification results.
        /// @return TRUE if the policy verification was successful, FALSE otherwise.
        BOOL VerifyCertificateChainPolicy(LPCSTR pszPolicyOID,
                                          PCCERT_CHAIN_CONTEXT pChainContext,
                                          PCERT_CHAIN_POLICY_PARA pPolicyPara,
                                          PCERT_CHAIN_POLICY_STATUS pPolicyStatus) const override
        {
            return CertVerifyCertificateChainPolicy(pszPolicyOID, pChainContext, pPolicyPara, pPolicyStatus);
        }

        /// @brief Retrieves a string that represents the subject or issuer of a certificate.
        ///
        /// @param pCertContext A pointer to the certificate context that contains the subject or issuer to be
        /// retrieved.
        /// @param dwType The type of string to retrieve.
        /// @param dwFlags The flags to use when retrieving the string.
        /// @param pvTypePara A pointer to a value that is specific to the type of string being retrieved.
        /// @param pszNameString A pointer to a buffer that receives the retrieved string.
        /// @param cchNameString The size of the buffer pointed to by the pszNameString parameter.
        /// @return The number of bytes that were copied to the buffer pointed to by the pszNameString parameter.
        DWORD GetNameString(PCCERT_CONTEXT pCertContext,
                            DWORD dwType,
                            DWORD dwFlags,
                            void* pvTypePara,
                            LPSTR pszNameString,
                            DWORD cchNameString) const override
        {
            return CertGetNameStringA(pCertContext, dwType, dwFlags, pvTypePara, pszNameString, cchNameString);
        }

        /// @brief Frees a certificate chain context that was previously allocated with CertGetCertificateChain.
        ///
        /// @param pChainContext A pointer to the certificate chain context to be freed.
        void FreeCertificateChain(PCCERT_CHAIN_CONTEXT pChainContext) const override
        {
            CertFreeCertificateChain(pChainContext);
        }

        /// @brief Closes a certificate store handle.
        ///
        /// @param hCertStore A handle to the certificate store to be closed.
        /// @param dwFlags The flags that specify how the store should be closed.
        /// @return TRUE if the store was successfully closed, FALSE otherwise.
        BOOL CloseStore(HCERTSTORE hCertStore, DWORD dwFlags) const override
        {
            return CertCloseStore(hCertStore, dwFlags);
        }
    };

} // namespace https_socket_verify_utils
