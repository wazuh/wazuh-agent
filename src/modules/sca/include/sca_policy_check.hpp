#pragma once

#include <cmdHelper.hpp>
#include <filesystem_wrapper.hpp>

#include <boost/asio/awaitable.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct PolicyEvaluationContext
{
    std::optional<std::string> command;
    std::optional<std::string> pattern;
    std::optional<std::string> directory;
    std::optional<std::vector<std::string>> paths;
};

enum class RuleResult
{
    Found,
    NotFound,
    Invalid
};

class IRuleEvaluator
{
public:
    virtual ~IRuleEvaluator() = default;

    virtual RuleResult Evaluate(const PolicyEvaluationContext& ctx) = 0;

    /// @brief Runs the file check
    /// @return Awaitable void
    virtual boost::asio::awaitable<void> Run() = 0;
};

class FileRuleEvaluator : public IRuleEvaluator
{
public:
    FileRuleEvaluator(std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
        : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                                : std::make_unique<file_system::FileSystemWrapper>())
    {
    }

    RuleResult Evaluate(const PolicyEvaluationContext& ctx) override
    {
        if (ctx.pattern)
        {
            return CheckFileListForContents(ctx);
        }
        return CheckFileListForExistence(ctx);
    }

private:
    RuleResult CheckFileListForContents(const PolicyEvaluationContext& ctx)
    {
        if (ctx.paths)
        {
            for (const auto& path : *ctx.paths)
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

    RuleResult CheckFileListForExistence(const PolicyEvaluationContext& ctx)
    {
        if (ctx.paths)
        {
            for (const auto& path : *ctx.paths)
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

    std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper;
};

class CommandRuleEvaluator : public IRuleEvaluator
{
public:
    RuleResult Evaluate(const PolicyEvaluationContext& ctx) override
    {
        if (ctx.command && ctx.pattern)
        {
            const auto output = Utils::Exec(*ctx.command);
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

class DirRuleEvaluator : public IRuleEvaluator
{
public:
    DirRuleEvaluator(std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
        : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                                : std::make_unique<file_system::FileSystemWrapper>())
    {
    }

    RuleResult Evaluate(const PolicyEvaluationContext& ctx) override
    {
        // if there's no "file" pattern in pattern, just check that the list of paths exists (directories)
        // TODO check that there's a "file" pattern or not
        if (!ctx.pattern)
        {
            if (ctx.paths)
            {
                for (const auto& path : *ctx.paths)
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
        if (ctx.paths)
        {
            for ([[maybe_unused]] const auto& path : *ctx.paths)
            {
            }
            return RuleResult::Found;
        }
        return RuleResult::Invalid;
    }

private:
    std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper;
};

class ProcessRuleEvaluator : public IRuleEvaluator
{
public:
    RuleResult Evaluate([[maybe_unused]] const PolicyEvaluationContext& ctx) override
    {
        // get list of running processes
        // check if pattern matches
        return RuleResult::Invalid;
    }
};
