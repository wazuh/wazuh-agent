#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// clang-format off
#include <windows.h>
#include <wincrypt.h>
// clang-format on

#include "icert_store_utils_win.hpp"

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

        /// @copydoc ICertificateStoreUtilsWin::OpenSystemStore
        HCERTSTORE OpenSystemStore(HCRYPTPROV_LEGACY hProv, LPCSTR szSubsystemProtocol) const override
        {
            return CertOpenSystemStoreA(hProv, szSubsystemProtocol);
        }

        /// @copydoc ICertificateStoreUtilsWin::FreeCertificateContext
        BOOL FreeCertificateContext(PCCERT_CONTEXT pCertContext) const override
        {
            return CertFreeCertificateContext(pCertContext);
        }

        /// @copydoc ICertificateStoreUtilsWin::GetCertificateChain
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

        /// @copydoc ICertificateStoreUtilsWin::VerifyCertificateChainPolicy
        BOOL VerifyCertificateChainPolicy(LPCSTR pszPolicyOID,
                                          PCCERT_CHAIN_CONTEXT pChainContext,
                                          PCERT_CHAIN_POLICY_PARA pPolicyPara,
                                          PCERT_CHAIN_POLICY_STATUS pPolicyStatus) const override
        {
            return CertVerifyCertificateChainPolicy(pszPolicyOID, pChainContext, pPolicyPara, pPolicyStatus);
        }

        /// @copydoc ICertificateStoreUtilsWin::GetNameString
        DWORD GetNameString(PCCERT_CONTEXT pCertContext,
                            DWORD dwType,
                            DWORD dwFlags,
                            void* pvTypePara,
                            LPSTR pszNameString,
                            DWORD cchNameString) const override
        {
            return CertGetNameStringA(pCertContext, dwType, dwFlags, pvTypePara, pszNameString, cchNameString);
        }

        /// @copydoc ICertificateStoreUtilsWin::FreeCertificateChain
        void FreeCertificateChain(PCCERT_CHAIN_CONTEXT pChainContext) const override
        {
            CertFreeCertificateChain(pChainContext);
        }

        /// @copydoc ICertificateStoreUtilsWin::CloseStore
        BOOL CloseStore(HCERTSTORE hCertStore, DWORD dwFlags) const override
        {
            return CertCloseStore(hCertStore, dwFlags);
        }
    };

} // namespace https_socket_verify_utils
