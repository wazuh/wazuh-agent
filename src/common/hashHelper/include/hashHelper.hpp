#pragma once

#include <memory>
#include <openssl/evp.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace Utils
{
    /// @brief Hash type
    enum class HashType
    {
        Sha1,
        Sha256,
    };

    /// @brief Class to calculate hash
    class HashData final
    {
    public:
        /// @brief Constructor
        /// @param hashType Hash type
        HashData(const HashType hashType = HashType::Sha1)
            : m_spCtx {createContext()}
        {
            initializeContext(hashType, m_spCtx);
        }

        /// @brief Destructor
        ~HashData() = default;

        /// @brief Update hash
        /// @param data Data to update
        /// @param size Size of data
        void update(const void* data, const size_t size)
        {
            const auto ret {EVP_DigestUpdate(m_spCtx.get(), data, size)};

            if (!ret)
            {
                throw std::runtime_error {"Error getting digest final."};
            }
        }

        /// @brief Get hash
        /// @return Digest vector
        std::vector<unsigned char> hash()
        {
            unsigned char digest[EVP_MAX_MD_SIZE] {0};
            unsigned int digestSize {0};
            const auto ret {EVP_DigestFinal_ex(m_spCtx.get(), digest, &digestSize)};

            if (!ret)
            {
                throw std::runtime_error {"Error getting digest final."};
            }

            return {digest, digest + digestSize};
        }

    private:
        /// @brief Deleter for EVP_MD_CTX
        struct EvpContextDeleter final
        {
            void operator()(EVP_MD_CTX* ctx)
            {
                EVP_MD_CTX_destroy(ctx);
            }
        };

        /// @brief Create EVP_MD_CTX
        /// @return EVP_MD_CTX
        static EVP_MD_CTX* createContext()
        {
            auto ctx {EVP_MD_CTX_create()};

            if (!ctx)
            {
                throw std::runtime_error {"Error creating EVP_MD_CTX."};
            }

            return ctx;
        }

        /// @brief Initialize EVP_MD_CTX
        /// @param hashType Hash type
        /// @param spCtx EVP_MD_CTX
        static void initializeContext(const HashType hashType, std::unique_ptr<EVP_MD_CTX, EvpContextDeleter>& spCtx)
        {
            static auto cryptoInitialized {false};

            if (!cryptoInitialized)
            {
                OPENSSL_init_crypto(OPENSSL_INIT_ADD_ALL_CIPHERS | OPENSSL_INIT_ADD_ALL_DIGESTS |
                                        OPENSSL_INIT_LOAD_CONFIG | OPENSSL_INIT_NO_ATEXIT,
                                    nullptr);
                cryptoInitialized = true;
            }

            auto ret {0};

            switch (hashType)
            {
                case HashType::Sha1: ret = EVP_DigestInit(spCtx.get(), EVP_sha1()); break;

                case HashType::Sha256: ret = EVP_DigestInit(spCtx.get(), EVP_sha256()); break;
            }

            if (!ret)
            {
                throw std::runtime_error {"Error initializing EVP_MD_CTX."};
            }
        }

        std::unique_ptr<EVP_MD_CTX, EvpContextDeleter> m_spCtx;
    };

    /// @brief Function to calculate the hash of a file.
    /// @param filepath Path to the file.
    /// @return std::vector<unsigned char> Digest vector.
    std::vector<unsigned char> hashFile(const std::string& filepath);
} // namespace Utils
