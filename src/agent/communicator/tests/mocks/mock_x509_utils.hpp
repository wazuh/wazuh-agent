#include <gmock/gmock.h>

#include "ix509_utils.hpp"

class MockX509Utils : public https_socket_verify_utils::ICertificateX509Utils
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
