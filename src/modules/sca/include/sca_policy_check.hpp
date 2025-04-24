#pragma once

#include <ifile_io_utils.hpp>
#include <ifilesystem_wrapper.hpp>
#include <sca_utils.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <string>

enum class RuleResult
{
    Invalid = -1,
    Found,
    NotFound
};

enum class ConditionType
{
    All,
    Any,
    None
};

struct PolicyEvaluationContext
{
    std::string rule;
    std::optional<std::string> pattern;
};

class IRuleEvaluator
{
public:
    virtual ~IRuleEvaluator() = default;

    virtual RuleResult Evaluate() = 0;
};

class RuleEvaluator : public IRuleEvaluator
{
public:
    RuleEvaluator(PolicyEvaluationContext ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper);

    const PolicyEvaluationContext& GetContext() const;

protected:
    std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper = nullptr;
    PolicyEvaluationContext m_ctx = {};
};

class FileRuleEvaluator : public RuleEvaluator
{
public:
    FileRuleEvaluator(PolicyEvaluationContext ctx,
                      std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                      std::unique_ptr<IFileIOUtils> fileUtils);

    RuleResult Evaluate() override;

private:
    RuleResult CheckFileForContents();

    RuleResult CheckFileExistence();

    std::unique_ptr<IFileIOUtils> m_fileUtils = nullptr;
};

class CommandRuleEvaluator : public RuleEvaluator
{
public:
    using CommandExecFunc = std::function<std::string(const std::string&)>;

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
    std::unique_ptr<IFileIOUtils> m_fileUtils = nullptr;
};

class ProcessRuleEvaluator : public RuleEvaluator
{
public:
    using GetProcessesFunc = std::function<std::vector<std::string>()>;

    ProcessRuleEvaluator(PolicyEvaluationContext ctx,
                         std::unique_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                         GetProcessesFunc getProcesses = nullptr);

    RuleResult Evaluate() override;

private:
    GetProcessesFunc m_getProcesses = nullptr;
};

class RegistryRuleEvaluator : public RuleEvaluator
{
public:
    RegistryRuleEvaluator(PolicyEvaluationContext ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper);

    RuleResult Evaluate() override;
};
