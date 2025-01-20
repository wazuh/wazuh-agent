#include <gmock/gmock.h>

#include <icert_store_utils_win.hpp>

class MockCertStoreUtils : public https_socket_verify_utils::ICertificateStoreUtilsWin
{
public:
    MOCK_METHOD(PCCERT_CONTEXT,
                CreateCertificateContext,
                (DWORD dwCertEncodingType, const BYTE* pbCertEncoded, DWORD cbCertEncoded),
                (const, override));

    MOCK_METHOD(HCERTSTORE, OpenSystemStore, (HCRYPTPROV_LEGACY hProv, LPCSTR szSubsystemProtocol), (const, override));

    MOCK_METHOD(BOOL, FreeCertificateContext, (PCCERT_CONTEXT pCertContext), (const, override));

    MOCK_METHOD(BOOL,
                GetCertificateChain,
                (HCERTCHAINENGINE hChainEngine,
                 PCCERT_CONTEXT pCertContext,
                 LPFILETIME pTime,
                 HCERTSTORE hAdditionalStore,
                 PCERT_CHAIN_PARA pChainPara,
                 DWORD dwFlags,
                 LPVOID pvReserved,
                 PCCERT_CHAIN_CONTEXT* ppChainContext),
                (const, override));

    MOCK_METHOD(BOOL,
                VerifyCertificateChainPolicy,
                (LPCSTR pszPolicyOID,
                 PCCERT_CHAIN_CONTEXT pChainContext,
                 PCERT_CHAIN_POLICY_PARA pPolicyPara,
                 PCERT_CHAIN_POLICY_STATUS pPolicyStatus),
                (const, override));

    MOCK_METHOD(DWORD,
                GetNameString,
                (PCCERT_CONTEXT pCertContext,
                 DWORD dwType,
                 DWORD dwFlags,
                 void* pvTypePara,
                 LPSTR pszNameString,
                 DWORD cchNameString),
                (const, override));

    MOCK_METHOD(void, FreeCertificateChain, (PCCERT_CHAIN_CONTEXT pChainContext), (const, override));

    MOCK_METHOD(BOOL, CloseStore, (HCERTSTORE hCertStore, DWORD dwFlags), (const, override));
};
