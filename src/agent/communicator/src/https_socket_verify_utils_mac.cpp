#include <https_socket_verify_utils.hpp>
#include <logger.hpp>

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <boost/asio/ssl.hpp>

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

        unsigned char* certData = nullptr;
        const int certLen = i2d_X509(cert, &certData);
        if (certLen <= 0)
        {
            LogError("Failed to encode certificate to DER.");
            return false;
        }

        CFDataRef cfCertData = CFDataCreate(kCFAllocatorDefault, certData, certLen);
        OPENSSL_free(certData);
        if (!cfCertData)
        {
            LogError("Failed to create CFData for certificate.");
            return false;
        }

        SecCertificateRef serverCert = SecCertificateCreateWithData(nullptr, cfCertData);
        CFRelease(cfCertData);
        if (!serverCert)
        {
            LogError("Failed to create SecCertificateRef.");
            return false;
        }

        const void* certArrayValues[] = {serverCert};

        CFArrayRef certArray = CFArrayCreate(kCFAllocatorDefault, certArrayValues, 1, &kCFTypeArrayCallBacks);
        SecPolicyRef policy = SecPolicyCreateSSL(true, nullptr);
        SecTrustRef trust = nullptr;

        OSStatus status = SecTrustCreateWithCertificates(certArray, policy, &trust);
        CFRelease(certArray);
        CFRelease(policy);

        if (status != errSecSuccess || !trust)
        {
            LogError("Failed to create SecTrust object.");
            CFRelease(serverCert);
            return false;
        }

        // Evaluate certificate trust using SecTrustEvaluateWithError
        CFErrorRef error = nullptr;

        const bool trustResult = SecTrustEvaluateWithError(trust, &error);
        if (!trustResult)
        {
            if (error)
            {
                CFStringRef errorDesc = CFErrorCopyDescription(error);
                LogError("Trust evaluation failed: {}", CFStringGetCStringPtr(errorDesc, kCFStringEncodingUTF8));
                CFRelease(errorDesc);
                CFRelease(error);
            }
            CFRelease(trust);
            CFRelease(serverCert);
            return false;
        }

        // Validate the hostname if the mode is 'full'.
        if (mode == "full")
        {
            CFStringRef sanString = SecCertificateCopySubjectSummary(serverCert);
            if (!sanString)
            {
                LogError("Failed to retrieve SAN or CN for hostname validation.");
                CFRelease(trust);
                CFRelease(serverCert);
                return false;
            }

            bool hostnameMatches = false;
            char buffer[256];
            if (CFStringGetCString(sanString, buffer, sizeof(buffer), kCFStringEncodingUTF8))
            {
                hostnameMatches = (host == buffer);
            }

            CFRelease(sanString);
            if (!hostnameMatches)
            {
                LogError("The hostname does not match the certificate's SAN or CN.");
                CFRelease(trust);
                CFRelease(serverCert);
                return false;
            }
        }

        // Free resources
        CFRelease(trust);
        CFRelease(serverCert);

        return true;
    }

} // namespace https_socket_verify_utils
