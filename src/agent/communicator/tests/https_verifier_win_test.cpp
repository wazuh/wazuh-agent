#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// NOLINTNEXTLINE(clang-format)
#include <wincrypt.h>
#include <windows.h>

#include "https_socket_verify_utils.hpp"
#include "https_verifier_win.hpp"
#include "icert_store_utils_win.hpp"
#include "ix509_utils.hpp"
#include <boost/asio/ssl.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

using namespace https_socket_verify_utils;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class MockX509Utils : public ICertificateX509Utils
{
public:
    MOCK_METHOD(STACK_OF(X509) *, GetCertChain, (X509_STORE_CTX*), (const, override));
    MOCK_METHOD(X509*, GetCertificateFromChain, (STACK_OF(X509) *, int), (const, override));
    MOCK_METHOD(int, GetCertificateCount, (STACK_OF(X509) *), (const, override));
    MOCK_METHOD(int, EncodeCertificateToDER, (X509*, unsigned char**), (const, override));
    MOCK_METHOD(void*, GetExtD2I, (X509 * x, int nid, int* crit, int* idx), (const, override));
    MOCK_METHOD(const unsigned char*, GetASN1StringData, (const ASN1_STRING* str), (const, override));
    MOCK_METHOD(void, FreeGeneralNames, (GENERAL_NAMES * genNames), (const, override));
};

class MockCertStoreUtils : public ICertificateStoreUtilsWin
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

class MockVerifyContext : public boost::asio::ssl::verify_context
{
public:
    MockVerifyContext()
        : verify_context(nullptr)
    {
    }

    void SetCertificate(X509* cert)
    {
        m_cert = cert;
    }

    X509* native_handle()
    {
        return m_cert;
    }

private:
    X509* m_cert;
};

class HttpsVerifierTest : public ::testing::Test
{
protected:
    MockX509Utils* mockX509Ptr;
    MockCertStoreUtils* mockCertStorePtr;
    std::unique_ptr<HttpsVerifierWin> verifier;
    MockVerifyContext ctx;

    void SetUp() override
    {
        mockX509Ptr = new MockX509Utils();
        std::unique_ptr<ICertificateX509Utils> x509Ptr(mockX509Ptr);

        mockCertStorePtr = new MockCertStoreUtils();
        std::unique_ptr<ICertificateStoreUtilsWin> certStorePtr(mockCertStorePtr);
        verifier = std::make_unique<HttpsVerifierWin>("full", "example.com", x509Ptr, certStorePtr);
    }

    void TearDown() override
    {
        verifier.reset();
    }
};

class HttpsVerifierTestModeCertificate : public ::testing::Test
{
protected:
    MockX509Utils* mockX509Ptr;
    MockCertStoreUtils* mockCertStorePtr;
    std::unique_ptr<HttpsVerifierWin> verifier;
    MockVerifyContext ctx;

    void SetUp() override
    {
        mockX509Ptr = new MockX509Utils();
        std::unique_ptr<ICertificateX509Utils> x509Ptr(mockX509Ptr);

        mockCertStorePtr = new MockCertStoreUtils();
        std::unique_ptr<ICertificateStoreUtilsWin> certStorePtr(mockCertStorePtr);
        verifier = std::make_unique<HttpsVerifierWin>("certificate", "example.com", x509Ptr, certStorePtr);
    }

    void TearDown() override
    {
        verifier.reset();
    }
};

TEST_F(HttpsVerifierTest, VerifyExtractCertificateNoCertificates)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(0));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyExtractCertificateCertificatesNotObtained)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(nullptr));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyExtractCertificateEncodeFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(-1));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyExtractCertificateCreateCertificateContextFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _)).WillOnce(Return(nullptr));

    EXPECT_FALSE(verifier->Verify(ctx));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
