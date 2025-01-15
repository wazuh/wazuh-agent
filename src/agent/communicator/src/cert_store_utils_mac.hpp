#pragma once

#include <icert_store_utils_mac.hpp>

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
        /// @brief Creates a SecCertificateRef from the provided certificate data.
        ///
        /// @param certData The raw certificate data in CFDataRef format.
        /// @return A SecCertificateRef object created from the data.
        SecCertificateRef CreateCertificate(CFDataRef certData) const override
        {
            return SecCertificateCreateWithData(nullptr, certData);
        }

        /// @brief Creates a SecTrustRef object with the provided certificates and policy.
        ///
        /// @param certs The array of certificates in CFArrayRef format.
        /// @param policy The policy defining how the trust is evaluated.
        /// @param trust The resulting SecTrustRef object.
        /// @return An OSStatus indicating the success or failure of the trust object creation.
        OSStatus CreateTrustObject(CFArrayRef certs, SecPolicyRef policy, SecTrustRef* trust) const override
        {
            return SecTrustCreateWithCertificates(certs, policy, trust);
        }

        /// @brief Evaluates the trust status of a SecTrustRef object.
        ///
        /// @param trust The SecTrustRef object to evaluate.
        /// @param error A pointer to a CFErrorRef to capture any error during the evaluation.
        /// @return `true` if the trust evaluation succeeded, otherwise `false`.
        bool EvaluateTrust(SecTrustRef trust, CFErrorRef* error) const override
        {
            return SecTrustEvaluateWithError(trust, error);
        }

        /// @brief Creates a CFDataRef from the raw certificate data.
        ///
        /// @param certData The raw certificate data as a byte array.
        /// @param certLen The length of the certificate data.
        /// @return A CFDataRef object containing the certificate data.
        CFDataRef CreateCFData(const unsigned char* certData, int certLen) const override
        {
            return CFDataCreate(kCFAllocatorDefault, certData, certLen);
        }

        /// @brief Creates a CFArrayRef from an array of certificate values.
        ///
        /// @param certArrayValues The array of certificate values.
        /// @param count The number of elements in the array.
        /// @return A CFArrayRef object containing the certificates.
        CFArrayRef CreateCertArray(const void* certArrayValues[], size_t count) const override
        {
            return CFArrayCreate(
                kCFAllocatorDefault, certArrayValues, static_cast<CFIndex>(count), &kCFTypeArrayCallBacks);
        }

        /// @brief Copies the error description from a CFErrorRef.
        ///
        /// @param error The CFErrorRef object containing the error.
        /// @return A CFStringRef containing the error description.
        CFStringRef CopyErrorDescription(CFErrorRef error) const override
        {
            return CFErrorCopyDescription(error);
        }

        /// @brief Retrieves the subject summary (e.g., SAN or CN) of the given certificate.
        ///
        /// @param cert The SecCertificateRef object representing the certificate.
        /// @return A CFStringRef containing the subject summary of the certificate.
        CFStringRef CopySubjectSummary(SecCertificateRef cert) const override
        {
            return SecCertificateCopySubjectSummary(cert);
        }

        /// @brief Converts a CFStringRef to a std::string.
        ///
        /// @param cfString The CFStringRef to convert to std::string.
        /// @return A std::string containing the UTF-8 encoded string.
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

        /// @brief Releases a CFTypeRef object.
        ///
        /// @param cfObject The CFTypeRef object to release.
        /// @return void
        void ReleaseCFObject(CFTypeRef cfObject) const override
        {
            if (cfObject)
            {
                CFRelease(cfObject);
            }
        }

        /// @brief Creates an SSL policy for the specified hostname.
        ///
        /// @param server A boolean indicating whether the policy is for a server (true) or a client (false).
        /// @param hostname The hostname to associate with the policy. If empty, no hostname is used.
        /// @return A SecPolicyRef representing the SSL policy for the specified hostname.
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
