#include "https_socket_verify_utils.hpp"
#include "https_verifier_mac.hpp"
#include "icertificate_utils.hpp"
#include <CoreFoundation/CoreFoundation.h>
#include <boost/asio/ssl.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

using namespace https_socket_verify_utils;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class MockCertificateUtils : public ICertificateUtils
{
public:
    MOCK_METHOD(STACK_OF(X509) *, GetCertChain, (X509_STORE_CTX*), (const, override));
    MOCK_METHOD(X509*, GetCertificateFromChain, (STACK_OF(X509) *, int), (const, override));
    MOCK_METHOD(int, GetCertificateCount, (STACK_OF(X509) *), (const, override));
    MOCK_METHOD(int, EncodeCertificateToDER, (X509*, unsigned char**), (const, override));
    MOCK_METHOD(SecCertificateRef, CreateCertificate, (CFDataRef), (const, override));
    MOCK_METHOD(OSStatus, CreateTrustObject, (CFArrayRef, SecPolicyRef, SecTrustRef*), (const, override));
    MOCK_METHOD(bool, EvaluateTrust, (SecTrustRef, CFErrorRef*), (const, override));
    MOCK_METHOD(CFDataRef, CreateCFData, (const unsigned char*, int), (const, override));
    MOCK_METHOD(CFArrayRef, CreateCertArray, (const void**, size_t), (const, override));
    MOCK_METHOD(CFStringRef, CopyErrorDescription, (CFErrorRef), (const, override));
    MOCK_METHOD(CFStringRef, CopySubjectSummary, (SecCertificateRef), (const, override));
    MOCK_METHOD(std::string, GetStringCFString, (CFStringRef), (const, override));
    MOCK_METHOD(void, ReleaseCFObject, (CFTypeRef), (const, override));
    MOCK_METHOD(SecPolicyRef, CreateSSLPolicy, (bool, const std::string&), (const, override));
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
    MockCertificateUtils* mockPtr;
    std::unique_ptr<HttpsVerifier> verifier;
    MockVerifyContext ctx;

    void SetUp() override
    {
        mockPtr = new MockCertificateUtils();
        std::unique_ptr<ICertificateUtils> utilsPtr(mockPtr);
        verifier = std::make_unique<HttpsVerifier>("full", "example.com", utilsPtr);
    }

    void TearDown() override
    {
        verifier.reset();
    }
};

class HttpsVerifierTestModeCertificate : public ::testing::Test
{
protected:
    MockCertificateUtils* mockPtr;
    std::unique_ptr<HttpsVerifier> verifier;
    MockVerifyContext ctx;

    void SetUp() override
    {
        mockPtr = new MockCertificateUtils();
        std::unique_ptr<ICertificateUtils> utilsPtr(mockPtr);
        verifier = std::make_unique<HttpsVerifier>("certificate", "example.com", utilsPtr);
    }

    void TearDown() override
    {
        verifier.reset();
    }
};

TEST_F(HttpsVerifierTest, VerifyExtractCertificateNoCertificates)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(0));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyExtractCertificateCertificatesNotObtained)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(nullptr));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyExtractCertificateEncodeFails)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(-1));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyExtractCertificateCreateCFDataFails)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(nullptr));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyCreateTrustObjectCreateCertificateFails)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertificate(_)).WillOnce(Return(nullptr));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyCreateTrustObjectCreateTrustObjectFails)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertificate(_)).WillOnce(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateTrustObject(_, _, _)).WillOnce(Return(errSecInternalError));
    EXPECT_CALL(*mockPtr, ReleaseCFObject(_));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyEvaluateTrustFailsNoErrorRef)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertificate(_)).WillOnce(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateTrustObject(_, _, _)).WillOnce(Return(errSecSuccess));
    EXPECT_CALL(*mockPtr, EvaluateTrust(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*mockPtr, ReleaseCFObject(_));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyEvaluateTrustFailsWithErrorRef)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertificate(_)).WillOnce(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateTrustObject(_, _, _)).WillOnce(Return(errSecSuccess));

    CFErrorRef mockErrorRef = reinterpret_cast<CFErrorRef>(0x1);
    EXPECT_CALL(*mockPtr, EvaluateTrust(_, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(mockErrorRef), Return(false)));

    CFStringRef mockErrorDesc =
        CFStringCreateWithCString(nullptr, "Simulated error description", kCFStringEncodingUTF8);
    ASSERT_NE(mockErrorDesc, nullptr);
    EXPECT_CALL(*mockPtr, CopyErrorDescription(_)).WillOnce(Return(mockErrorDesc));

    EXPECT_CALL(*mockPtr, GetStringCFString(_)).WillOnce(Return("Simulated error description"));
    EXPECT_CALL(*mockPtr, ReleaseCFObject(_));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyCreateCertificateFails)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertificate(_))
        .WillOnce(Return(reinterpret_cast<SecCertificateRef>(0x1)))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(*mockPtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateTrustObject(_, _, _)).WillOnce(Return(errSecSuccess));
    EXPECT_CALL(*mockPtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockPtr, ReleaseCFObject(_));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameCopySubjectSummaryFails)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertificate(_)).WillRepeatedly(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateTrustObject(_, _, _)).WillOnce(Return(errSecSuccess));
    EXPECT_CALL(*mockPtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockPtr, ReleaseCFObject(_));
    EXPECT_CALL(*mockPtr, CopySubjectSummary(_)).WillOnce(Return(nullptr));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameCNEmpty)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertificate(_)).WillRepeatedly(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateTrustObject(_, _, _)).WillOnce(Return(errSecSuccess));
    EXPECT_CALL(*mockPtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockPtr, ReleaseCFObject(_));
    EXPECT_CALL(*mockPtr, CopySubjectSummary(_)).WillOnce(Return(reinterpret_cast<CFStringRef>(0x1)));
    EXPECT_CALL(*mockPtr, GetStringCFString(_)).WillOnce(Return(""));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameCNNotMatch)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertificate(_)).WillRepeatedly(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateTrustObject(_, _, _)).WillOnce(Return(errSecSuccess));
    EXPECT_CALL(*mockPtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockPtr, ReleaseCFObject(_));
    EXPECT_CALL(*mockPtr, CopySubjectSummary(_)).WillOnce(Return(reinterpret_cast<CFStringRef>(0x1)));
    EXPECT_CALL(*mockPtr, GetStringCFString(_)).WillOnce(Return("noexample.com"));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameSuccess)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertificate(_)).WillRepeatedly(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateTrustObject(_, _, _)).WillOnce(Return(errSecSuccess));
    EXPECT_CALL(*mockPtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockPtr, ReleaseCFObject(_));
    EXPECT_CALL(*mockPtr, CopySubjectSummary(_)).WillOnce(Return(reinterpret_cast<CFStringRef>(0x1)));
    EXPECT_CALL(*mockPtr, GetStringCFString(_)).WillOnce(Return("example.com"));

    EXPECT_TRUE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTestModeCertificate, VerifyCreateCertificateFails)
{
    EXPECT_CALL(*mockPtr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockPtr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockPtr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockPtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertificate(_)).WillOnce(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockPtr, CreateTrustObject(_, _, _)).WillOnce(Return(errSecSuccess));
    EXPECT_CALL(*mockPtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockPtr, ReleaseCFObject(_));

    EXPECT_TRUE(verifier->Verify(ctx));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
