#include <gmock/gmock.h>

#include <icert_store_utils_mac.hpp>

class MockCertStoreUtils : public https_socket_verify_utils::ICertificateStoreUtilsMac
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
