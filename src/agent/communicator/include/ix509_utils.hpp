#pragma once

#include <boost/asio/ssl.hpp>

#include <string>

namespace https_socket_verify_utils
{
    /// @brief Interface for certificate utility functions.
    ///
    /// This interface defines methods for handling certificate chains, encoding certificates,
    /// creating trust objects, evaluating trust, and working with X509 and SSL/TLS certificate verification.
    class ICertificateX509Utils
    {
    public:
        /// @brief Virtual destructor.
        virtual ~ICertificateX509Utils() = default;

        /// @brief Retrieves the certificate chain from the given context.
        ///
        /// @param ctx The X509_STORE_CTX context from which to extract the certificate chain.
        /// @return A STACK_OF(X509) object containing the certificate chain.
        virtual STACK_OF(X509) * GetCertChain(X509_STORE_CTX* ctx) const = 0;

        /// @brief Retrieves a certificate from the given certificate chain at the specified index.
        ///
        /// @param chain The certificate chain (STACK_OF(X509)).
        /// @param index The index of the certificate to retrieve.
        /// @return The X509 certificate at the specified index in the chain.
        virtual X509* GetCertificateFromChain(STACK_OF(X509) * chain, int index) const = 0;

        /// @brief Retrieves the count of certificates in the given certificate chain.
        ///
        /// @param chain The certificate chain (STACK_OF(X509)).
        /// @return The number of certificates in the chain.
        virtual int GetCertificateCount(STACK_OF(X509) * chain) const = 0;

        /// @brief Encodes the given certificate into DER format.
        ///
        /// @param cert The X509 certificate to encode.
        /// @param certData A pointer to the pointer where the encoded data will be stored.
        /// @return The length of the encoded certificate.
        virtual int EncodeCertificateToDER(X509* cert, unsigned char** certData) const = 0;

        /// @brief Retrieves an extension from the given X509 certificate.
        ///
        /// @param x The X509 certificate to retrieve the extension from.
        /// @param nid The NID of the extension to retrieve.
        /// @param crit A pointer to a boolean indicating whether the extension is critical.
        /// @param idx A pointer to the index of the extension in the certificate.
        /// @return The extension, or NULL if it could not be retrieved.
        virtual void* GetExtD2I(X509* x, int nid, int* crit, int* idx) const = 0;

        /// @brief Retrieves the data from the given ASN1_STRING.
        ///
        /// @param str The ASN1_STRING to retrieve the data from.
        /// @return A pointer to the data stored in the string.
        virtual const unsigned char* GetASN1StringData(const ASN1_STRING* str) const = 0;

        /// @brief Frees the memory allocated for GENERAL_NAMES.
        ///
        /// @param genNames The GENERAL_NAMES object to free.
        virtual void FreeGeneralNames(GENERAL_NAMES* genNames) const = 0;
    };

} // namespace https_socket_verify_utils
