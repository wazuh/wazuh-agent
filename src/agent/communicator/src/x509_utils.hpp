#pragma once

#include "ix509_utils.hpp"

#include <boost/asio/ssl.hpp>

#include <string>

namespace https_socket_verify_utils
{
    /// @brief A wrapper class for managing certificate utilities and operations.
    ///
    /// This class implements the ICertificateX509Utils interface and provides methods to interact with
    /// certificates and SSL/TLS trust objects using the OpenSSL frameworks.
    class CertificateX509UtilsWrapper : public ICertificateX509Utils
    {
    public:
        /// @brief Retrieves the certificate chain from the given X509_STORE_CTX.
        ///
        /// @param ctx The X509_STORE_CTX context containing the certificate chain.
        /// @return A STACK_OF(X509) containing the certificate chain.
        STACK_OF(X509) * GetCertChain(X509_STORE_CTX* ctx) const override
        {
            return X509_STORE_CTX_get_chain(ctx);
        }

        /// @brief Retrieves a certificate from the certificate chain at the specified index.
        ///
        /// @param chain The certificate chain (STACK_OF(X509)).
        /// @param index The index of the certificate to retrieve.
        /// @return The X509 certificate at the specified index.
        X509* GetCertificateFromChain(STACK_OF(X509) * chain, int index) const override
        {
            return sk_X509_value(chain, index);
        }

        /// @brief Retrieves the number of certificates in the given certificate chain.
        ///
        /// @param chain The certificate chain (STACK_OF(X509)).
        /// @return The number of certificates in the chain.
        int GetCertificateCount(STACK_OF(X509) * chain) const override
        {
            return sk_X509_num(chain);
        }

        /// @brief Encodes a certificate into DER format.
        ///
        /// @param cert The X509 certificate to encode.
        /// @param certData A pointer to the encoded certificate data.
        /// @return The length of the encoded certificate.
        int EncodeCertificateToDER(X509* cert, unsigned char** certData) const override
        {
            return i2d_X509(cert, certData);
        }

        /// @brief Retrieves an extension from the given X509 certificate.
        ///
        /// @param x The X509 certificate to retrieve the extension from.
        /// @param nid The NID of the extension to retrieve.
        /// @param crit A pointer to a boolean indicating whether the extension is critical.
        /// @param idx A pointer to the index of the extension in the certificate.
        /// @return The extension, or NULL if it could not be retrieved.
        void* GetExtD2I(X509* x, int nid, int* crit, int* idx) const override
        {
            return X509_get_ext_d2i(x, nid, crit, idx);
        }

        /// @brief Retrieves the data from the given ASN1_STRING.
        ///
        /// @param str The ASN1_STRING to retrieve the data from.
        /// @return A pointer to the data stored in the string.
        const unsigned char* GetASN1StringData(const ASN1_STRING* str) const override
        {
            return ASN1_STRING_get0_data(str);
        }

        /// @brief Frees the memory allocated for GENERAL_NAMES.
        ///
        /// @param genNames The GENERAL_NAMES object to free.
        void FreeGeneralNames(GENERAL_NAMES* genNames) const override
        {
            GENERAL_NAMES_free(genNames);
        }
    };

} // namespace https_socket_verify_utils
