#include <storage.hpp>

#include <logger.hpp>

#include <column.hpp>
#include <persistence.hpp>
#include <persistence_factory.hpp>

namespace
{
    // column names
    const std::string ROW_ID_COLUMN_NAME = "rowid";
    const std::string MODULE_NAME_COLUMN_NAME = "module_name";
    const std::string MODULE_TYPE_COLUMN_NAME = "module_type";
    const std::string METADATA_COLUMN_NAME = "metadata";
    const std::string MESSAGE_COLUMN_NAME = "message";

    nlohmann::json ProcessRequest(const std::vector<Row>& rows, size_t maxSize = 0)
    {
        nlohmann::json messages = nlohmann::json::array();
        size_t sizeAccum = 0;

        for (const auto& row : rows)
        {
            std::string moduleNameString = row[0].Value;
            std::string moduleTypeString = row[1].Value;
            std::string metadataString = row[2].Value;
            std::string dataString = row[3].Value;

            nlohmann::json outputJson = {{"moduleName", ""}, {"moduleType", ""}, {"metadata", ""}, {"data", {}}};

            if (!dataString.empty())
            {
                outputJson["data"] = nlohmann::json::parse(dataString);
            }

            if (!metadataString.empty())
            {
                outputJson["metadata"] = metadataString;
            }

            if (!moduleNameString.empty())
            {
                outputJson["moduleName"] = moduleNameString;
            }

            if (!moduleTypeString.empty())
            {
                outputJson["moduleType"] = moduleTypeString;
            }

            messages.push_back(outputJson);
            if (maxSize)
            {
                size_t messageSize = moduleNameString.size() + moduleTypeString.size() + metadataString.size() +
                                     outputJson["data"].dump().size();
                if (sizeAccum + messageSize >= maxSize)
                {
                    break;
                }
                sizeAccum += messageSize;
            }
        }

        return messages;
    }
} // namespace

Storage::Storage(const std::string& dbFilePath, const std::vector<std::string>& tableNames)
{
    try
    {
        m_db = PersistenceFactory::CreatePersistence(PersistenceFactory::PersistenceType::SQLITE3, dbFilePath);

        for (const auto& table : tableNames)
        {
            if (!m_db->TableExists(table))
            {
                CreateTable(table);
            }
        }
    }
    catch (const std::exception&)
    {
        throw std::runtime_error(std::string("Cannot open database: " + dbFilePath));
    }
}

Storage::~Storage() = default;

void Storage::CreateTable(const std::string& tableName)
{
    try
    {
        Keys columns;
        columns.emplace_back(MODULE_NAME_COLUMN_NAME, ColumnType::TEXT, false, false);
        columns.emplace_back(MODULE_TYPE_COLUMN_NAME, ColumnType::TEXT, false, false);
        columns.emplace_back(METADATA_COLUMN_NAME, ColumnType::TEXT, false, false);
        columns.emplace_back(MESSAGE_COLUMN_NAME, ColumnType::TEXT, true, false);

        m_db->CreateTable(tableName, columns);
    }
    catch (const std::exception& e)
    {
        LogError("Error creating table: {}.", e.what());
        throw;
    }
}

int Storage::Store(const nlohmann::json& message,
                   const std::string& tableName,
                   const std::string& moduleName,
                   const std::string& moduleType,
                   const std::string& metadata)
{
    Row fields;
    fields.emplace_back(MODULE_NAME_COLUMN_NAME, ColumnType::TEXT, moduleName);
    fields.emplace_back(MODULE_TYPE_COLUMN_NAME, ColumnType::TEXT, moduleType);
    fields.emplace_back(METADATA_COLUMN_NAME, ColumnType::TEXT, metadata);

    int result = 0;

    std::unique_lock<std::mutex> lock(m_mutex);

    auto transaction = m_db->BeginTransaction();

    if (message.is_array())
    {
        for (const auto& singleMessageData : message)
        {
            fields.emplace_back(MESSAGE_COLUMN_NAME, ColumnType::TEXT, singleMessageData.dump());

            try
            {
                m_db->Insert(tableName, fields);
                result++;
            }
            catch (const std::exception& e)
            {
                LogError("Error during Store operation: {}.", e.what());
            }
            fields.pop_back();
        }
    }
    else
    {
        fields.emplace_back(MESSAGE_COLUMN_NAME, ColumnType::TEXT, message.dump());

        try
        {
            m_db->Insert(tableName, fields);
            result++;
        }
        catch (const std::exception& e)
        {
            LogError("Error during Store operation: {}.", e.what());
        }
    }

    m_db->CommitTransaction(transaction);

    return result;
}

int Storage::RemoveMultiple(int n,
                            const std::string& tableName,
                            const std::string& moduleName,
                            const std::string& moduleType)
{
    Criteria filters;
    if (!moduleName.empty())
        filters.emplace_back(MODULE_NAME_COLUMN_NAME, ColumnType::TEXT, moduleName);
    if (!moduleType.empty())
        filters.emplace_back(MODULE_TYPE_COLUMN_NAME, ColumnType::TEXT, moduleType);

    int result = 0;

    std::unique_lock<std::mutex> lock(m_mutex);

    auto transaction = m_db->BeginTransaction();

    try
    {
        Names columns;
        columns.emplace_back(ROW_ID_COLUMN_NAME, ColumnType::INTEGER);

        // Select first n messages
        const auto results =
            m_db->Select(tableName, columns, filters, LogicalOperator::AND, columns, OrderType::ASC, n);

        if (!results.empty())
        {
            filters.clear();

            for (const auto& row : results)
            {
                filters.emplace_back(ROW_ID_COLUMN_NAME, ColumnType::INTEGER, row[0].Value);

                try
                {
                    // Remove selected message
                    m_db->Remove(tableName, filters, LogicalOperator::AND);
                    result++;
                }
                catch (const std::exception& e)
                {
                    LogError("Error during Remove operation: {}.", e.what());
                }
                filters.pop_back();
            }
        }
    }
    catch (const std::exception& e)
    {
        LogError("Error during RemoveMultiple operation: {}.", e.what());
    }

    m_db->CommitTransaction(transaction);

    return result;
}

nlohmann::json Storage::RetrieveMultiple(int n,
                                         const std::string& tableName,
                                         const std::string& moduleName,
                                         const std::string& moduleType)
{
    Names columns;
    columns.emplace_back(MODULE_NAME_COLUMN_NAME, ColumnType::TEXT);
    columns.emplace_back(MODULE_TYPE_COLUMN_NAME, ColumnType::TEXT);
    columns.emplace_back(METADATA_COLUMN_NAME, ColumnType::TEXT);
    columns.emplace_back(MESSAGE_COLUMN_NAME, ColumnType::TEXT);

    Criteria filters;
    if (!moduleName.empty())
        filters.emplace_back(MODULE_NAME_COLUMN_NAME, ColumnType::TEXT, moduleName);
    if (!moduleType.empty())
        filters.emplace_back(MODULE_TYPE_COLUMN_NAME, ColumnType::TEXT, moduleType);

    Names orderColumns;
    orderColumns.emplace_back(ROW_ID_COLUMN_NAME, ColumnType::INTEGER);

    try
    {
        const auto results =
            m_db->Select(tableName, columns, filters, LogicalOperator::AND, orderColumns, OrderType::ASC, n);

        return ProcessRequest(results);
    }
    catch (const std::exception& e)
    {
        LogError("Error during RetrieveMultiple operation: {}.", e.what());
        return {};
    }
}

nlohmann::json Storage::RetrieveBySize(size_t n,
                                       const std::string& tableName,
                                       const std::string& moduleName,
                                       const std::string& moduleType)
{
    Names columns;
    columns.emplace_back(MODULE_NAME_COLUMN_NAME, ColumnType::TEXT);
    columns.emplace_back(MODULE_TYPE_COLUMN_NAME, ColumnType::TEXT);
    columns.emplace_back(METADATA_COLUMN_NAME, ColumnType::TEXT);
    columns.emplace_back(MESSAGE_COLUMN_NAME, ColumnType::TEXT);

    Criteria filters;
    if (!moduleName.empty())
        filters.emplace_back(MODULE_NAME_COLUMN_NAME, ColumnType::TEXT, moduleName);
    if (!moduleType.empty())
        filters.emplace_back(MODULE_TYPE_COLUMN_NAME, ColumnType::TEXT, moduleType);

    Names orderColumns;
    orderColumns.emplace_back(ROW_ID_COLUMN_NAME, ColumnType::INTEGER);

    try
    {
        const auto results =
            m_db->Select(tableName, columns, filters, LogicalOperator::AND, orderColumns, OrderType::ASC);

        return ProcessRequest(results, n);
    }
    catch (const std::exception& e)
    {
        LogError("Error during RetrieveBySize operation: {}.", e.what());
        return {};
    }
}

int Storage::GetElementCount(const std::string& tableName, const std::string& moduleName, const std::string& moduleType)
{
    Criteria filters;
    if (!moduleName.empty())
        filters.emplace_back(MODULE_NAME_COLUMN_NAME, ColumnType::TEXT, moduleName);
    if (!moduleType.empty())
        filters.emplace_back(MODULE_TYPE_COLUMN_NAME, ColumnType::TEXT, moduleType);

    int count = 0;

    try
    {
        count = m_db->GetCount(tableName, filters, LogicalOperator::AND);
    }
    catch (const std::exception& e)
    {
        LogError("Error during GetElementCount operation: {}.", e.what());
    }

    return count;
}

size_t Storage::GetElementsStoredSize(const std::string& tableName,
                                      const std::string& moduleName,
                                      const std::string& moduleType)
{
    Criteria filters;
    if (!moduleName.empty())
        filters.emplace_back(MODULE_NAME_COLUMN_NAME, ColumnType::TEXT, moduleName);
    if (!moduleType.empty())
        filters.emplace_back(MODULE_TYPE_COLUMN_NAME, ColumnType::TEXT, moduleType);

    size_t count = 0;

    try
    {
        Names columns;
        columns.emplace_back(MODULE_NAME_COLUMN_NAME, ColumnType::TEXT);
        columns.emplace_back(MODULE_TYPE_COLUMN_NAME, ColumnType::TEXT);
        columns.emplace_back(METADATA_COLUMN_NAME, ColumnType::TEXT);
        columns.emplace_back(MESSAGE_COLUMN_NAME, ColumnType::TEXT);

        count = m_db->GetSize(tableName, columns, filters, LogicalOperator::AND);
    }
    catch (const std::exception& e)
    {
        LogError("Error during GetElementsStoredSize operation: {}.", e.what());
    }

    return count;
}
