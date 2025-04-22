#pragma once

#include <ifilesystem_wrapper.hpp>
#include <sca_utils.hpp>

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

class CheckConditionEvaluator
{
public:
    static CheckConditionEvaluator fromString(const std::string& str);

    explicit CheckConditionEvaluator(ConditionType type);

    void addResult(bool passed);

    bool result() const;

private:
    ConditionType m_type;
    int m_totalRules {0};
    int m_passedRules {0};
    std::optional<bool> m_result;
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
    RuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper);

    const PolicyEvaluationContext& GetContext() const;

protected:
    std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper = nullptr;
    PolicyEvaluationContext m_ctx = {};
};

class FileRuleEvaluator : public RuleEvaluator
{
public:
    FileRuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper);

    RuleResult Evaluate() override;

private:
    RuleResult CheckFileForContents();

    RuleResult CheckFileExistence();
};

class CommandRuleEvaluator : public RuleEvaluator
{
public:
    CommandRuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper);

    RuleResult Evaluate() override;
};

class DirRuleEvaluator : public RuleEvaluator
{
public:
    DirRuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper);

    RuleResult Evaluate() override;
};

class ProcessRuleEvaluator : public RuleEvaluator
{
public:
    ProcessRuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper);

    RuleResult Evaluate() override;
};
