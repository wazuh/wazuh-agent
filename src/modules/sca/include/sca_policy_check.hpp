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
    CommandRuleEvaluator(PolicyEvaluationContext ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper);

    RuleResult Evaluate() override;
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
    ProcessRuleEvaluator(PolicyEvaluationContext ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper);

    RuleResult Evaluate() override;
};
