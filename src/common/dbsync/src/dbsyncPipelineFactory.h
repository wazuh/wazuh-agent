#pragma once

#include "commonDefs.h"
#include "dbengine.h"
#include <functional>
#include <map>
#include <memory>
#include <mutex>

namespace DbSync
{
    using TxnContext = void*;
    using PipelineCtxHandle = void*;

    /// @brief Pipeline interface
    struct IPipeline
    {
        /// @brief Default destructor
        virtual ~IPipeline() = default;

        /// @brief Syncs a row
        /// @param syncJson Sync JSON
        virtual void syncRow(const nlohmann::json& syncJson) = 0;

        /// @brief Gets deleted rows
        /// @param callback callback
        virtual void getDeleted(const ResultCallback callback) = 0;
    };

    /// @brief Pipeline factory
    class PipelineFactory final
    {
    public:
        /// @brief Singleton
        static PipelineFactory& instance() noexcept;

        /// @brief Releases all the pipelines
        void release() noexcept;

        /// @brief Creates a pipeline
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param tables Tables to be created in the transaction.
        /// @param threadNumber   Number of worker threads for processing data. If 0 hardware concurrency
        ///                       value will be used.
        /// @param maxQueueSize   Max data number to hold/queue to be processed.
        /// @param callback callback
        /// @return PipelineCtxHandle
        PipelineCtxHandle create(const DBSYNC_HANDLE handle,
                                 const nlohmann::json& tables,
                                 const unsigned int threadNumber,
                                 const unsigned int maxQueueSize,
                                 const ResultCallback callback);

        /// @brief Gets the pipeline
        /// @param handle PipelineCtxHandle
        /// @return std::shared_ptr<IPipeline>
        const std::shared_ptr<IPipeline>& pipeline(const PipelineCtxHandle handle);

        /// @brief Destroys the pipeline
        /// @param handle PipelineCtxHandle
        void destroy(const PipelineCtxHandle handle);

    private:
        /// @brief Delete copy constructor
        PipelineFactory(const PipelineFactory&) = delete;

        /// @brief Delete copy operator
        PipelineFactory& operator=(const PipelineFactory&) = delete;

        /// @brief Default constructor
        PipelineFactory() = default;

        /// @brief Default destructor
        ~PipelineFactory() = default;

        std::map<PipelineCtxHandle, std::shared_ptr<IPipeline>> m_contexts;
        std::mutex m_contextsMutex;
    };

} // namespace DbSync
