#pragma once

#include <cmdHelper.hpp>
#include <filesystem_wrapper.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>

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

    virtual RuleResult Evaluate() = 0;

    /// @brief Runs the file check
    /// @return Awaitable void
    virtual boost::asio::awaitable<void> Run() = 0;
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

    boost::asio::awaitable<void> Run() override
    {
        // while keep running evaluate then async wait a timer sleep

        while (m_running)
        {
            // evaluate
            const auto result = Evaluate();
            if (result == RuleResult::Found)
            {
                // do something
            }
            else if (result == RuleResult::NotFound)
            {
                // do something else
            }
            else if (result == RuleResult::Invalid)
            {
                // handle invalid case
            }
            // sleep for a while
            auto executor = co_await boost::asio::this_coro::executor;
            boost::asio::steady_timer timer(executor);
            timer.expires_after(std::chrono::seconds(5));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }
        co_return;
    }

protected:
    std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper = nullptr;
    PolicyEvaluationContext m_ctx = {};

private:
    // keep runing atomic flag
    std::atomic<bool> m_running {true};
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
