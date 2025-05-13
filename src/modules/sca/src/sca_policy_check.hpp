#pragma once

#include <ifile_io_utils.hpp>
#include <ifilesystem_wrapper.hpp>
#include <sysInfoInterface.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

enum class RuleResult
{
    Invalid = -1,
    Found,
    NotFound
};

struct PolicyEvaluationContext
{
    std::string rule = {};
    std::optional<std::string> pattern = std::nullopt;
    bool isNegated = false;
};

class IRuleEvaluator
{
public:
    virtual ~IRuleEvaluator() = default;

    virtual RuleResult Evaluate() = 0;

    virtual const PolicyEvaluationContext& GetContext() const = 0;
};

class RuleEvaluator : public IRuleEvaluator
{
public:
    RuleEvaluator(PolicyEvaluationContext ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper);

    const PolicyEvaluationContext& GetContext() const override;

protected:
    std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper = nullptr;
    PolicyEvaluationContext m_ctx = {};
};

class FileRuleEvaluator : public RuleEvaluator
{
public:
    FileRuleEvaluator(PolicyEvaluationContext ctx,
                      std::unique_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                      std::unique_ptr<IFileIOUtils> fileUtils = nullptr);

    RuleResult Evaluate() override;

private:
    RuleResult CheckFileForContents();

    RuleResult CheckFileExistence();

    std::unique_ptr<IFileIOUtils> m_fileUtils = nullptr;
};

class CommandRuleEvaluator : public RuleEvaluator
{
public:
    /// @brief Function that takes a command and returns the output and error as a pair of strings.
    using CommandExecFunc = std::function<std::pair<std::string, std::string>(const std::string&)>;

    CommandRuleEvaluator(PolicyEvaluationContext ctx,
                         std::unique_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                         CommandExecFunc commandExecFunc = nullptr);

    RuleResult Evaluate() override;

private:
    CommandExecFunc m_commandExecFunc = nullptr;
};

class DirRuleEvaluator : public RuleEvaluator
{
public:
    DirRuleEvaluator(PolicyEvaluationContext ctx,
                     std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                     std::unique_ptr<IFileIOUtils> fileUtils);

    RuleResult Evaluate() override;

private:
    RuleResult CheckDirectoryForContents();

    RuleResult CheckDirectoryExistence();

    std::unique_ptr<IFileIOUtils> m_fileUtils = nullptr;
};

class ProcessRuleEvaluator : public RuleEvaluator
{
public:
    using GetProcessesFunc = std::function<std::vector<std::string>()>;

    ProcessRuleEvaluator(PolicyEvaluationContext ctx,
                         std::unique_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                         std::unique_ptr<ISysInfo> sysInfo = nullptr,
                         GetProcessesFunc getProcesses = nullptr);

    RuleResult Evaluate() override;

private:
    std::unique_ptr<ISysInfo> m_sysInfo = nullptr;
    GetProcessesFunc m_getProcesses = nullptr;
};

class RegistryRuleEvaluator : public RuleEvaluator
{
public:
    using IsValidRegistryKeyFunc = std::function<bool(const std::string& rootKey)>;
    using GetRegistryValuesFunc = std::function<std::vector<std::string>(const std::string& rootKey)>;
    using GetRegistryKeyValueFunc = std::function<std::string(const std::string& rootKey, const std::string& key)>;

    RegistryRuleEvaluator(PolicyEvaluationContext ctx,
                          IsValidRegistryKeyFunc isValidRegistryKey = nullptr,
                          GetRegistryValuesFunc getRegistryValues = nullptr,
                          GetRegistryKeyValueFunc getRegistryKeyValue = nullptr);

    RuleResult Evaluate() override;

private:
    RuleResult CheckRegistryForContents();

    RuleResult CheckRegistryExistence();

    IsValidRegistryKeyFunc m_isValidRegistryKey = nullptr;
    GetRegistryValuesFunc m_getRegistryValues = nullptr;
    GetRegistryKeyValueFunc m_getRegistryKeyValue = nullptr;
};

class RuleEvaluatorFactory
{
public:
    static std::unique_ptr<IRuleEvaluator>
    CreateEvaluator(const std::string& input,
                    std::unique_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                    std::unique_ptr<IFileIOUtils> fileUtils = nullptr,
                    std::unique_ptr<ISysInfo> sysInfo = nullptr);
};
