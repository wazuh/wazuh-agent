#pragma once

#include "icert_store_utils_mac.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

#include <string>
#include <vector>

namespace https_socket_verify_utils
{
    /// @brief A wrapper class for managing certificate utilities and operations.
    ///
    /// This class implements the ICertificateStoreUtilsMac interface and provides methods to interact with
    /// certificates and SSL/TLS trust objects using the Core Foundation and Security frameworks.
    class CertificateStoreUtilsWrapperMac : public ICertificateStoreUtilsMac
    {
    public:
        /// @copydoc ICertificateStoreUtilsMac::CreateCertificate
        SecCertificateRef CreateCertificate(CFDataRef certData) const override
        {
            return SecCertificateCreateWithData(nullptr, certData);
        }

        /// @copydoc ICertificateStoreUtilsMac::CreateTrustObject
        OSStatus CreateTrustObject(CFArrayRef certs, SecPolicyRef policy, SecTrustRef* trust) const override
        {
            return SecTrustCreateWithCertificates(certs, policy, trust);
        }

        /// @copydoc ICertificateStoreUtilsMac::EvaluateTrust
        bool EvaluateTrust(SecTrustRef trust, CFErrorRef* error) const override
        {
            return SecTrustEvaluateWithError(trust, error);
        }

        /// @copydoc ICertificateStoreUtilsMac::CreateCFData
        CFDataRef CreateCFData(const unsigned char* certData, int certLen) const override
        {
            return CFDataCreate(kCFAllocatorDefault, certData, certLen);
        }

        /// @copydoc ICertificateStoreUtilsMac::CreateCertArray
        CFArrayRef CreateCertArray(const void* certArrayValues[], size_t count) const override
        {
            return CFArrayCreate(
                kCFAllocatorDefault, certArrayValues, static_cast<CFIndex>(count), &kCFTypeArrayCallBacks);
        }

        /// @copydoc ICertificateStoreUtilsMac::CopyErrorDescription
        CFStringRef CopyErrorDescription(CFErrorRef error) const override
        {
            return CFErrorCopyDescription(error);
        }

        /// @copydoc ICertificateStoreUtilsMac::CopySubjectSummary
        CFStringRef CopySubjectSummary(SecCertificateRef cert) const override
        {
            return SecCertificateCopySubjectSummary(cert);
        }

        /// @copydoc ICertificateStoreUtilsMac::GetStringCFString
        std::string GetStringCFString(CFStringRef cfString) const override
        {
            if (!cfString)
            {
                return ""; // Return an empty string if the CFStringRef is null.
            }

            // Get the length of the CFString in characters
            CFIndex length = CFStringGetLength(cfString);
            CFIndex maxBufferSize =
                length * 3 + 1; // Estimate the maximum size, UTF-8 could use up to 3 bytes per character
            std::vector<char> buffer(
                static_cast<size_t>(maxBufferSize)); // Use a dynamic vector to handle large strings

            // Convert the CFStringRef to a byte array (UTF-8)
            if (CFStringGetCString(cfString, buffer.data(), static_cast<CFIndex>(buffer.size()), kCFStringEncodingUTF8))
            {
                return std::string(buffer.data()); // Convert the buffer to std::string and return it
            }

            return ""; // If the conversion fails, return an empty string
        }

        /// @copydoc ICertificateStoreUtilsMac::ReleaseCFObject
        void ReleaseCFObject(CFTypeRef cfObject) const override
        {
            if (cfObject)
            {
                CFRelease(cfObject);
            }
        }

        /// @copydoc ICertificateStoreUtilsMac::CreateSSLPolicy
        SecPolicyRef CreateSSLPolicy(bool server, const std::string& hostname) const override
        {
            // Convert std::string to CFStringRef
            CFStringRef cfHostname =
                hostname.empty()
                    ? nullptr
                    : CFStringCreateWithCString(kCFAllocatorDefault, hostname.c_str(), kCFStringEncodingUTF8);

            // Call SecPolicyCreateSSL with the CFStringRef hostname
            SecPolicyRef policy = SecPolicyCreateSSL(server, cfHostname);

            // Release the CFStringRef if it was created
            if (cfHostname)
            {
                CFRelease(cfHostname);
            }

            return policy;
        }
    };

} // namespace https_socket_verify_utils
