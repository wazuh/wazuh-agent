#include "https_socket_verify_utils.hpp"
#include "https_verifier_mac.hpp"
#include "icert_store_utils_mac.hpp"
#include "ix509_utils.hpp"
#include <CoreFoundation/CoreFoundation.h>
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

class MockCertStoreUtils : public ICertificateStoreUtilsMac
{
public:
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
    MockX509Utils* mockX509Ptr;
    MockCertStoreUtils* mockCertStorePtr;
    std::unique_ptr<HttpsVerifier> verifier;
    MockVerifyContext ctx;

    void SetUp() override
    {
        mockX509Ptr = new MockX509Utils();
        std::unique_ptr<ICertificateX509Utils> x509Ptr(mockX509Ptr);

        mockCertStorePtr = new MockCertStoreUtils();
        std::unique_ptr<ICertificateStoreUtilsMac> certStorePtr(mockCertStorePtr);
        verifier = std::make_unique<HttpsVerifier>("full", "example.com", x509Ptr, certStorePtr);
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
    std::unique_ptr<HttpsVerifier> verifier;
    MockVerifyContext ctx;

    void SetUp() override
    {
        mockX509Ptr = new MockX509Utils();
        std::unique_ptr<ICertificateX509Utils> x509Ptr(mockX509Ptr);

        mockCertStorePtr = new MockCertStoreUtils();
        std::unique_ptr<ICertificateStoreUtilsMac> certStorePtr(mockCertStorePtr);
        verifier = std::make_unique<HttpsVerifier>("certificate", "example.com", x509Ptr, certStorePtr);
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

TEST_F(HttpsVerifierTest, VerifyExtractCertificateCreateCFDataFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(nullptr));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyCreateTrustObjectCreateCertificateFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificate(_)).WillOnce(Return(nullptr));
    EXPECT_CALL(*mockCertStorePtr, ReleaseCFObject(_));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyCreateTrustObjectCreateTrustObjectFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificate(_)).WillOnce(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateTrustObject(_, _, _)).WillOnce(Return(errSecInternalError));
    EXPECT_CALL(*mockCertStorePtr, ReleaseCFObject(_)).Times(4);

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyEvaluateTrustFailsNoErrorRef)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x2)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x3)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificate(_)).WillOnce(Return(reinterpret_cast<SecCertificateRef>(0x4)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x5)));
    EXPECT_CALL(*mockCertStorePtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x6)));
    EXPECT_CALL(*mockCertStorePtr, CreateTrustObject(_, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(reinterpret_cast<SecTrustRef>(0x7)), Return(errSecSuccess)));
    EXPECT_CALL(*mockCertStorePtr, EvaluateTrust(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*mockCertStorePtr, ReleaseCFObject(_)).Times(5);

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyEvaluateTrustFailsWithErrorRef)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x11)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificate(_)).WillOnce(Return(reinterpret_cast<SecCertificateRef>(0x12)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x13)));
    EXPECT_CALL(*mockCertStorePtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x14)));
    EXPECT_CALL(*mockCertStorePtr, CreateTrustObject(_, _, _))
        .WillOnce(
            testing::DoAll(testing::SetArgPointee<2>(reinterpret_cast<SecTrustRef>(0x15)), Return(errSecSuccess)));
    EXPECT_CALL(*mockCertStorePtr, EvaluateTrust(_, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(reinterpret_cast<CFErrorRef>(0x16)), Return(false)));

    EXPECT_CALL(*mockCertStorePtr, CopyErrorDescription(_)).WillOnce(Return(reinterpret_cast<CFStringRef>(0x17)));

    EXPECT_CALL(*mockCertStorePtr, GetStringCFString(_)).WillOnce(Return("Simulated error description"));
    EXPECT_CALL(*mockCertStorePtr, ReleaseCFObject(_)).Times(7);

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyCreateCertificateFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificate(_))
        .WillOnce(Return(reinterpret_cast<SecCertificateRef>(0x1)))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(*mockCertStorePtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateTrustObject(_, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(reinterpret_cast<SecTrustRef>(0x7)), Return(errSecSuccess)));
    EXPECT_CALL(*mockCertStorePtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockCertStorePtr, ReleaseCFObject(_)).Times(5);

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameCopySubjectSummaryFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificate(_))
        .WillRepeatedly(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateTrustObject(_, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(reinterpret_cast<SecTrustRef>(0x7)), Return(errSecSuccess)));
    EXPECT_CALL(*mockCertStorePtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockCertStorePtr, ReleaseCFObject(_)).Times(6);
    EXPECT_CALL(*mockCertStorePtr, CopySubjectSummary(_)).WillOnce(Return(nullptr));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameCNEmpty)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificate(_))
        .WillRepeatedly(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateTrustObject(_, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(reinterpret_cast<SecTrustRef>(0x7)), Return(errSecSuccess)));
    EXPECT_CALL(*mockCertStorePtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockCertStorePtr, ReleaseCFObject(_)).Times(7);
    EXPECT_CALL(*mockCertStorePtr, CopySubjectSummary(_)).WillOnce(Return(reinterpret_cast<CFStringRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, GetStringCFString(_)).WillOnce(Return(""));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameCNNotMatch)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificate(_))
        .WillRepeatedly(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateTrustObject(_, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(reinterpret_cast<SecTrustRef>(0x7)), Return(errSecSuccess)));
    EXPECT_CALL(*mockCertStorePtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockCertStorePtr, ReleaseCFObject(_)).Times(7);
    EXPECT_CALL(*mockCertStorePtr, CopySubjectSummary(_)).WillOnce(Return(reinterpret_cast<CFStringRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, GetStringCFString(_)).WillOnce(Return("noexample.com"));

    EXPECT_FALSE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTest, VerifyValidateHostnameSuccess)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificate(_))
        .WillRepeatedly(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateTrustObject(_, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(reinterpret_cast<SecTrustRef>(0x7)), Return(errSecSuccess)));
    EXPECT_CALL(*mockCertStorePtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockCertStorePtr, ReleaseCFObject(_)).Times(7);
    EXPECT_CALL(*mockCertStorePtr, CopySubjectSummary(_)).WillOnce(Return(reinterpret_cast<CFStringRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, GetStringCFString(_)).WillOnce(Return("example.com"));

    EXPECT_TRUE(verifier->Verify(ctx));
}

TEST_F(HttpsVerifierTestModeCertificate, VerifyCreateCertificateFails)
{
    EXPECT_CALL(*mockX509Ptr, GetCertChain(_)).WillOnce(Return(reinterpret_cast<STACK_OF(X509)*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, GetCertificateCount(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockX509Ptr, GetCertificateFromChain(_, _)).WillOnce(Return(reinterpret_cast<X509*>(0x1)));
    EXPECT_CALL(*mockX509Ptr, EncodeCertificateToDER(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockCertStorePtr, CreateCFData(_, _)).WillOnce(Return(reinterpret_cast<CFDataRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertificate(_)).WillOnce(Return(reinterpret_cast<SecCertificateRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateCertArray(_, _)).WillOnce(Return(reinterpret_cast<CFArrayRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateSSLPolicy(_, _)).WillOnce(Return(reinterpret_cast<SecPolicyRef>(0x1)));
    EXPECT_CALL(*mockCertStorePtr, CreateTrustObject(_, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(reinterpret_cast<SecTrustRef>(0x7)), Return(errSecSuccess)));
    EXPECT_CALL(*mockCertStorePtr, EvaluateTrust(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*mockCertStorePtr, ReleaseCFObject(_)).Times(5);

    EXPECT_TRUE(verifier->Verify(ctx));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
