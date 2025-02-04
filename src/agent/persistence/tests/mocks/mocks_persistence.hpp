#include <gmock/gmock.h>

#include <persistence.hpp>

#include <string>
#include <vector>

class MockPersistence : public Persistence
{
public:
    MOCK_METHOD(bool, TableExists, (const std::string& tableName), (override));
    MOCK_METHOD(void, CreateTable, (const std::string& tableName, const column::Keys& cols), (override));
    MOCK_METHOD(void, Insert, (const std::string& tableName, const column::Row& cols), (override));
    MOCK_METHOD(void,
                Update,
                (const std::string& tableName,
                 const column::Row& fields,
                 const column::Criteria& selCriteria,
                 column::LogicalOperator logOp),
                (override));
    MOCK_METHOD(void,
                Remove,
                (const std::string& tableName, const column::Criteria& selCriteria, column::LogicalOperator logOp),
                (override));
    MOCK_METHOD(void, DropTable, (const std::string& tableName), (override));
    MOCK_METHOD(std::vector<column::Row>,
                Select,
                (const std::string& tableName,
                 const column::Names& fields,
                 const column::Criteria& selCriteria,
                 column::LogicalOperator logOp,
                 const column::Names& orderBy,
                 column::OrderType orderType,
                 int limit),
                (override));
    MOCK_METHOD(int,
                GetCount,
                (const std::string& tableName, const column::Criteria& selCriteria, column::LogicalOperator logOp),
                (override));
    MOCK_METHOD(size_t,
                GetSize,
                (const std::string& tableName,
                 const column::Names& fields,
                 const column::Criteria& selCriteria,
                 column::LogicalOperator logOp),
                (override));
    MOCK_METHOD(TransactionId, BeginTransaction, (), (override));
    MOCK_METHOD(void, CommitTransaction, (TransactionId transactionId), (override));
    MOCK_METHOD(void, RollbackTransaction, (TransactionId transactionId), (override));
};
