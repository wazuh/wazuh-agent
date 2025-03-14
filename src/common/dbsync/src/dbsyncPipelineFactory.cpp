#include "dbsyncPipelineFactory.h"
#include "db_exception.h"
#include "dbsync_implementation.h"
#include "pipelineNodesImp.hpp"
#include <utility>

namespace DbSync
{
    class Pipeline final : public IPipeline
    {
    public:
        /// @brief Constructor
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param tables Tables to be created in the transaction.
        /// @param threadNumber   Number of worker threads for processing data. If 0 hardware concurrency
        ///                       value will be used.
        /// @param maxQueueSize   Max data number to hold/queue to be processed.
        /// @param callback callback
        Pipeline(const DBSYNC_HANDLE handle,
                 const nlohmann::json& tables,
                 const unsigned int threadNumber,
                 const unsigned int maxQueueSize,
                 ResultCallback callback)
            : m_handle {handle}
            , m_txnContext {DBSyncImplementation::instance().createTransaction(handle, tables)}
            , m_maxQueueSize {maxQueueSize}
            , m_callback {std::move(callback)}
            , m_spDispatchNode {maxQueueSize ? GetDispatchNode(threadNumber) : nullptr}
        {
            if (!m_callback || !m_handle || !m_txnContext)
            {
                throw dbsync_error {INVALID_PARAMETERS};
            }
        }

        /// @brief Destructor
        ~Pipeline() override
        {
            if (m_spDispatchNode)
            {
                try
                {
                    m_spDispatchNode->rundown();
                }
                catch (...) // NOLINT(bugprone-empty-catch)
                {
                }
            }

            try
            {
                DBSyncImplementation::instance().closeTransaction(m_handle, m_txnContext);
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
            }
        }

        /// @brief Default copy constructor
        Pipeline(const Pipeline&) = default;

        /// @brief Default copy operator
        Pipeline& operator=(const Pipeline&) = default;

        /// @brief Default move constructor
        Pipeline(Pipeline&&) = default;

        /// @brief Default move operator
        Pipeline& operator=(Pipeline&&) = default;

        /// @copydoc IPipeline::syncRow
        void syncRow(const nlohmann::json& value) override
        {
            try
            {
                DBSyncImplementation::instance().syncRowData(
                    m_handle,
                    m_txnContext,
                    value,
                    [this](ReturnTypeCallback resType, const nlohmann::json& resValue)
                    { this->PushResult(SyncResult {resType, resValue}); });
            }
            catch (const DbSync::max_rows_error&)
            {
                PushResult(SyncResult {MAX_ROWS, value});
            }
            catch (const std::exception& ex)
            {
                SyncResult result;
                result.first = DB_ERROR;
                result.second = value;
                result.second["exception"] = ex.what();
                PushResult(result);
            }
        }

        /// @copydoc IPipeline::getDeleted
        void getDeleted(const ResultCallback& callback) override
        {
            if (m_spDispatchNode)
            {
                m_spDispatchNode->rundown();
            }

            DBSyncImplementation::instance().getDeleted(m_handle, m_txnContext, callback);
        }

    private:
        using SyncResult = std::pair<ReturnTypeCallback, nlohmann::json>;
        using DispatchCallbackNode = Utils::ReadNode<SyncResult>;

        /// @brief Gets the dispatch node
        /// @param threadNumber Number of threads
        /// @return std::shared_ptr<DispatchCallbackNode>
        std::shared_ptr<DispatchCallbackNode> GetDispatchNode(const unsigned int threadNumber)
        {
            return std::make_shared<DispatchCallbackNode>(
                [this](auto&& pH1) { DispatchResult(std::forward<decltype(pH1)>(pH1)); },
                threadNumber ? threadNumber : std::thread::hardware_concurrency());
        }

        /// @brief Pushes the result
        /// @param result
        void PushResult(const SyncResult& result)
        {
            const auto async {m_spDispatchNode && m_spDispatchNode->size() < m_maxQueueSize};

            if (async)
            {
                m_spDispatchNode->receive(result);
            }
            else
            {
                DispatchResult(result);
            }
        }

        /// @brief Dispatches the result
        /// @param result
        void DispatchResult(const SyncResult& result)
        {
            const auto& value {result.second};

            if (!value.empty())
            {
                m_callback(result.first, value);
            }
        }

        DBSYNC_HANDLE m_handle;
        TXN_HANDLE m_txnContext;
        unsigned int m_maxQueueSize;
        ResultCallback m_callback;
        std::shared_ptr<DispatchCallbackNode> m_spDispatchNode;
    };

    PipelineFactory& PipelineFactory::instance() noexcept
    {
        static PipelineFactory s_instance;
        return s_instance;
    }

    void PipelineFactory::release() noexcept
    {
        const std::lock_guard<std::mutex> lock {m_contextsMutex};
        m_contexts.clear();
    }

    PipelineCtxHandle PipelineFactory::create(const DBSYNC_HANDLE handle,
                                              const nlohmann::json& tables,
                                              const unsigned int threadNumber,
                                              const unsigned int maxQueueSize,
                                              const ResultCallback& callback)
    {
        const auto spContext {std::make_shared<Pipeline>(handle, tables, threadNumber, maxQueueSize, callback)};
        const auto ret {spContext.get()};
        const std::lock_guard<std::mutex> lock {m_contextsMutex};
        m_contexts.emplace(ret, spContext);
        return ret;
    }

    const std::shared_ptr<IPipeline>& PipelineFactory::pipeline(const PipelineCtxHandle handle)
    {
        const std::lock_guard<std::mutex> lock {m_contextsMutex};
        const auto& it {m_contexts.find(handle)};

        if (it == m_contexts.end())
        {
            throw dbsync_error {INVALID_HANDLE};
        }

        return it->second;
    }

    void PipelineFactory::destroy(const PipelineCtxHandle handle)
    {
        const std::lock_guard<std::mutex> lock {m_contextsMutex};
        const auto& it {m_contexts.find(handle)};

        if (it == m_contexts.end())
        {
            throw dbsync_error {INVALID_HANDLE};
        }

        m_contexts.erase(it);
    }
} // namespace DbSync
