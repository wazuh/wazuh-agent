#pragma once

#include <cmdHelper.hpp>
#include <filesystem_wrapper.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct PolicyEvaluationContext
{
    std::string condition;
    std::optional<std::string> command;
    std::optional<std::string> pattern;
    std::optional<std::string> directory;
    std::optional<std::vector<std::string>> paths;
};

enum class RuleResult
{
    Invalid = -1,
    Found,
    NotFound
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
