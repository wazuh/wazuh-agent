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
        /// @copydoc ICertificateX509Utils::GetCertChain
        CertChain* GetCertChain(X509_STORE_CTX* ctx) const override
        {
            return X509_STORE_CTX_get_chain(ctx);
        }

        /// @copydoc ICertificateX509Utils::GetCertificateFromChain
        X509* GetCertificateFromChain(STACK_OF(X509) * chain, int index) const override
        {
            return sk_X509_value(chain, index);
        }

        /// @copydoc ICertificateX509Utils::GetCertificateCount
        int GetCertificateCount(STACK_OF(X509) * chain) const override
        {
            return sk_X509_num(chain);
        }

        /// @copydoc ICertificateX509Utils::EncodeCertificateToDER
        int EncodeCertificateToDER(X509* cert, unsigned char** certData) const override
        {
            return i2d_X509(cert, certData);
        }

        /// @copydoc ICertificateX509Utils::GetExtD2I
        void* GetExtD2I(X509* x, int nid, int* crit, int* idx) const override
        {
            return X509_get_ext_d2i(x, nid, crit, idx);
        }

        /// @copydoc ICertificateX509Utils::GetASN1StringData
        const unsigned char* GetASN1StringData(const ASN1_STRING* str) const override
        {
            return ASN1_STRING_get0_data(str);
        }

        /// @copydoc ICertificateX509Utils::FreeGeneralNames
        void FreeGeneralNames(GENERAL_NAMES* genNames) const override
        {
            GENERAL_NAMES_free(genNames);
        }
    };

} // namespace https_socket_verify_utils
