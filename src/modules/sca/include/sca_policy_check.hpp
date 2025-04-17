#pragma once

#include <cmdHelper.hpp>
#include <filesystem_wrapper.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using PolicyVariables = std::unordered_map<std::string, std::vector<std::string>>;

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
    ConditionType condition;
    PolicyVariables variables;
    std::string rule;

    std::optional<std::string> command;
    std::optional<std::string> pattern;
    std::optional<std::string> directory;
    std::optional<std::vector<std::string>> paths;
};

class CheckConditionEvaluator
{
public:
    static CheckConditionEvaluator fromString(const std::string& str)
    {
        if (str == "all")
        {
            return CheckConditionEvaluator {ConditionType::All};
        }
        if (str == "any")
        {
            return CheckConditionEvaluator {ConditionType::Any};
        }
        if (str == "none")
        {
            return CheckConditionEvaluator {ConditionType::None};
        }
        throw std::invalid_argument("Invalid condition type: " + str);
    }

    explicit CheckConditionEvaluator(ConditionType type)
        : m_type {type}
    {
    }

    void addResult(bool passed)
    {
        if (m_result.has_value())
        {
            return;
        }

        ++m_totalRules;
        m_passedRules += passed;

        switch (m_type)
        {
            case ConditionType::All:
                if (!passed)
                {
                    m_result = false;
                }
                break;
            case ConditionType::Any:
                if (passed)
                {
                    m_result = true;
                }
                break;
            case ConditionType::None:
                if (passed)
                {
                    m_result = false;
                }
                break;
        }
    }

    bool result() const
    {
        if (m_result.has_value())
        {
            return *m_result;
        }

        switch (m_type)
        {
            case ConditionType::All: return m_totalRules > 0 && m_passedRules == m_totalRules;
            case ConditionType::Any: return m_passedRules > 0;
            case ConditionType::None: return m_passedRules == 0;
        }

        return false;
    }

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
    RuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
        : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                                : std::make_unique<file_system::FileSystemWrapper>())
        , m_ctx(ctx)
    {
    }

    const PolicyEvaluationContext& GetContext() const
    {
        return m_ctx;
    }

protected:
    std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper = nullptr;
    PolicyEvaluationContext m_ctx = {};
};

class FileRuleEvaluator : public RuleEvaluator
{
public:
    FileRuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
        : RuleEvaluator(ctx, std::move(fileSystemWrapper))
    {
    }

    RuleResult Evaluate() override
    {
        if (m_ctx.pattern)
        {
            return CheckFileListForContents();
        }
        return CheckFileListForExistence();
    }

private:
    RuleResult CheckFileListForContents()
    {
        if (m_ctx.paths)
        {
            for (const auto& path : *m_ctx.paths)
            {
                if (m_fileSystemWrapper->exists(path))
                {
                    // Check file contents against the pattern
                    // Placeholder for actual content check logic
                    return RuleResult::Found;
                }
            }
            return RuleResult::NotFound;
        }
        return RuleResult::Invalid;
    }

    RuleResult CheckFileListForExistence()
    {
        if (m_ctx.paths)
        {
            for (const auto& path : *m_ctx.paths)
            {
                if (!m_fileSystemWrapper->exists(path))
                {
                    return RuleResult::NotFound;
                }
            }
            return RuleResult::Found;
        }
        return RuleResult::Invalid;
    }
};

class CommandRuleEvaluator : public RuleEvaluator
{
public:
    CommandRuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
        : RuleEvaluator(ctx, std::move(fileSystemWrapper))
    {
    }

    RuleResult Evaluate() override
    {
        if (m_ctx.command && m_ctx.pattern)
        {
            const auto output = Utils::Exec(*m_ctx.command);
            if (!output.empty())
            {
                // check pattern against output
                // Placeholder for actual pattern check logic
                return RuleResult::Found;
            }
            return RuleResult::NotFound;
        }
        return RuleResult::Invalid;
    }
};

class DirRuleEvaluator : public RuleEvaluator
{
public:
    DirRuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
        : RuleEvaluator(ctx, std::move(fileSystemWrapper))
    {
    }

    RuleResult Evaluate() override
    {
        // if there's no "file" pattern in pattern, just check that the list of paths exists (directories)
        // TODO check that there's a "file" pattern or not
        if (!m_ctx.pattern)
        {
            if (m_ctx.paths)
            {
                for (const auto& path : *m_ctx.paths)
                {
                    if (!m_fileSystemWrapper->exists(path) || !m_fileSystemWrapper->is_directory(path))
                    {
                        return RuleResult::NotFound;
                    }
                }
                return RuleResult::Found;
            }
        }
        // if there's a pattern, check that the list of paths exists (directories) and that the pattern exists in the
        // directory
        if (m_ctx.paths)
        {
            for ([[maybe_unused]] const auto& path : *m_ctx.paths)
            {
            }
            return RuleResult::Found;
        }
        return RuleResult::Invalid;
    }
};

class ProcessRuleEvaluator : public RuleEvaluator
{
public:
    ProcessRuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
        : RuleEvaluator(ctx, std::move(fileSystemWrapper))
    {
    }

    RuleResult Evaluate() override
    {
        // get list of running processes
        // check if pattern matches
        return RuleResult::Invalid;
    }
};
