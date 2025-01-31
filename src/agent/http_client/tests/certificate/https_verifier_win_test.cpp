#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// clang-format off
#include <windows.h>
#include <wincrypt.h>
// clang-format on

#include "../mocks/mock_cert_store_utils_win.hpp"
#include "../mocks/mock_x509_utils.hpp"
#include "https_verifier_win.hpp"
#include <boost/asio/ssl.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

using namespace https_socket_verify_utils;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

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

TEST_F(HttpsVerifierTest, VerifyCreateCertStoreOpenSystemStoreFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _))
        .WillOnce(Return(reinterpret_cast<PCCERT_CONTEXT>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateContext(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, OpenSystemStore(_, _)).WillOnce(Return(nullptr));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyVerifyCertificateChainGetCertificateChainFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _))
        .WillOnce(Return(reinterpret_cast<PCCERT_CONTEXT>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateContext(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, OpenSystemStore(_, _)).WillOnce(Return(reinterpret_cast<HCERTSTORE>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CloseStore(_, _)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, GetCertificateChain(_, _, _, _, _, _, _, _)).WillOnce(Return(false));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateChainPolicyVerifyCertificateChainPolicyFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _))
        .WillOnce(Return(reinterpret_cast<PCCERT_CONTEXT>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateContext(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, OpenSystemStore(_, _)).WillOnce(Return(reinterpret_cast<HCERTSTORE>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CloseStore(_, _)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, GetCertificateChain(_, _, _, _, _, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<7>(reinterpret_cast<PCCERT_CHAIN_CONTEXT>(0x1)), Return(true)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateChain(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, VerifyCertificateChainPolicy(_, _, _, _)).WillOnce(Return(false));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateChainPolicyVerifyCertificateChainPolicyError)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _))
        .WillOnce(Return(reinterpret_cast<PCCERT_CONTEXT>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateContext(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, OpenSystemStore(_, _)).WillOnce(Return(reinterpret_cast<HCERTSTORE>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CloseStore(_, _)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, GetCertificateChain(_, _, _, _, _, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<7>(reinterpret_cast<PCCERT_CHAIN_CONTEXT>(0x1)), Return(true)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateChain(_)).Times(1);
    CERT_CHAIN_POLICY_STATUS mockPolicyStatus = {sizeof(CERT_CHAIN_POLICY_STATUS)};
    mockPolicyStatus.dwError = static_cast<DWORD>(CERT_E_UNTRUSTEDROOT);

    EXPECT_CALL(*mockCertStorePtr, VerifyCertificateChainPolicy(_, _, _, _))
        .WillOnce(testing::DoAll(testing::WithArg<3>(
                                     [&](PCERT_CHAIN_POLICY_STATUS status)
                                     {
                                         if (status)
                                         {
                                             *status = mockPolicyStatus;
                                         }
                                     }),
                                 Return(true)));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameGetNameStringSubjectFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _))
        .WillOnce(Return(reinterpret_cast<PCCERT_CONTEXT>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateContext(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, OpenSystemStore(_, _)).WillOnce(Return(reinterpret_cast<HCERTSTORE>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CloseStore(_, _)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, GetCertificateChain(_, _, _, _, _, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<7>(reinterpret_cast<PCCERT_CHAIN_CONTEXT>(0x1)), Return(true)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateChain(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, VerifyCertificateChainPolicy(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockCertStorePtr, GetNameString(_, _, _, _, _, _)).WillOnce(Return(0));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameGetNameNameStringTooSmall)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _))
        .WillOnce(Return(reinterpret_cast<PCCERT_CONTEXT>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateContext(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, OpenSystemStore(_, _)).WillOnce(Return(reinterpret_cast<HCERTSTORE>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CloseStore(_, _)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, GetCertificateChain(_, _, _, _, _, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<7>(reinterpret_cast<PCCERT_CHAIN_CONTEXT>(0x1)), Return(true)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateChain(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, VerifyCertificateChainPolicy(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockCertStorePtr, GetNameString(_, _, _, _, _, _)).WillOnce(Return(1));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameStringSizeMismatch)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _))
        .WillOnce(Return(reinterpret_cast<PCCERT_CONTEXT>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateContext(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, OpenSystemStore(_, _)).WillOnce(Return(reinterpret_cast<HCERTSTORE>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CloseStore(_, _)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, GetCertificateChain(_, _, _, _, _, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<7>(reinterpret_cast<PCCERT_CHAIN_CONTEXT>(0x1)), Return(true)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateChain(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, VerifyCertificateChainPolicy(_, _, _, _)).WillOnce(Return(true));

    const DWORD nameSize = 10;
    EXPECT_CALL(*mockCertStorePtr, GetNameString(_, _, _, _, _, _))
        .WillOnce(Return(nameSize))
        .WillOnce(Return(nameSize - 1));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameMismatch)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _))
        .WillOnce(Return(reinterpret_cast<PCCERT_CONTEXT>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateContext(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, OpenSystemStore(_, _)).WillOnce(Return(reinterpret_cast<HCERTSTORE>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CloseStore(_, _)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, GetCertificateChain(_, _, _, _, _, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<7>(reinterpret_cast<PCCERT_CHAIN_CONTEXT>(0x1)), Return(true)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateChain(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, VerifyCertificateChainPolicy(_, _, _, _)).WillOnce(Return(true));

    const DWORD nameSize = 10;
    const std::string certCN = "wrong-host";
    EXPECT_CALL(*mockCertStorePtr, GetNameString(_, _, _, _, _, _))
        .WillOnce(Return(nameSize))
        .WillOnce(testing::DoAll(testing::WithArg<4>(
                                     [&certCN](LPSTR pszNameString)
                                     {
                                         if (pszNameString)
                                         {
                                             std::copy(certCN.begin(), certCN.end(), pszNameString);
                                             pszNameString[certCN.size()] = '\0';
                                         }
                                     }),
                                 Return(nameSize)));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameMatch)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _))
        .WillOnce(Return(reinterpret_cast<PCCERT_CONTEXT>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateContext(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, OpenSystemStore(_, _)).WillOnce(Return(reinterpret_cast<HCERTSTORE>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CloseStore(_, _)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, GetCertificateChain(_, _, _, _, _, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<7>(reinterpret_cast<PCCERT_CHAIN_CONTEXT>(0x1)), Return(true)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateChain(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, VerifyCertificateChainPolicy(_, _, _, _)).WillOnce(Return(true));

    const DWORD nameSize = 12;
    const std::string certCN = "example.com";
    EXPECT_CALL(*mockCertStorePtr, GetNameString(_, _, _, _, _, _))
        .WillOnce(Return(nameSize))
        .WillOnce(testing::DoAll(testing::WithArg<4>(
                                     [&certCN](LPSTR pszNameString)
                                     {
                                         if (pszNameString)
                                         {
                                             std::copy(certCN.begin(), certCN.end(), pszNameString);
                                             pszNameString[certCN.size()] = '\0';
                                         }
                                     }),
                                 Return(nameSize)));

    EXPECT_TRUE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTestModeCertificate, VerifyCertificateModeSuccess)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificateContext(_, _, _))
        .WillOnce(Return(reinterpret_cast<PCCERT_CONTEXT>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateContext(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, OpenSystemStore(_, _)).WillOnce(Return(reinterpret_cast<HCERTSTORE>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CloseStore(_, _)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, GetCertificateChain(_, _, _, _, _, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<7>(reinterpret_cast<PCCERT_CHAIN_CONTEXT>(0x1)), Return(true)));
    EXPECT_CALL(*mockCertStorePtr, FreeCertificateChain(_)).Times(1);
    EXPECT_CALL(*mockCertStorePtr, VerifyCertificateChainPolicy(_, _, _, _)).WillOnce(Return(true));

    EXPECT_TRUE(verifier->Verify(ctx));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
