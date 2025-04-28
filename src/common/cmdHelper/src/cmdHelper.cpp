#include <cmdHelper.hpp>

#include <fileSmartDeleter.hpp>
#include <logger.hpp>

#include <boost/process.hpp>

#include <cstdio>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    std::vector<std::string> TokenizeCommand(const std::string& command)
    {
        std::vector<std::string> result;

        for (auto token : command | std::views::split(' ') | std::views::filter([](auto&& r) { return !r.empty(); }))
        {
            result.emplace_back(token.begin(), token.end());
        }

        if (result.empty())
        {
            throw std::runtime_error("Empty command string");
        }

        return result;
    }

    std::string GetStreamOutput(boost::process::pipe& processPipe)
    {
        std::string line;
        std::stringstream output;
        boost::process::ipstream stream(processPipe);
        while (stream && std::getline(stream, line))
        {
            output << line << '\n';
        }
        return output.str();
    }
} // namespace

namespace Utils
{
    std::string PipeOpen(const std::string& cmd, const size_t bufferSize)
    {
        const std::unique_ptr<FILE, FileSmartDeleter> file {popen(cmd.c_str(), "r")};
        std::vector<char> buffer(bufferSize);
        std::string result;

        if (file)
        {
            while (fgets(buffer.data(), static_cast<int>(bufferSize), file.get()))
            {
                result += buffer.data();
            }
        }

        return result;
    }

    ExecResult Exec(const std::string& cmd)
    {
        try
        {
            // Tokenize to separate the command and its arguments
            const auto args = TokenizeCommand(cmd);
            const auto exePath = boost::process::search_path(args[0]);
            if (exePath.empty())
            {
                throw std::runtime_error("Executable not found in PATH: " + args[0]);
            }

            boost::process::pipe stdOutPipe;
            boost::process::pipe stdErrPipe;
            const std::vector<std::string> execArgs(args.begin() + 1, args.end());

            // Launch the process with args and capture output
            boost::process::child process(exePath,
                                          boost::process::args = execArgs,
                                          boost::process::std_out > stdOutPipe,
                                          boost::process::std_err > stdErrPipe);

            const auto output = GetStreamOutput(stdOutPipe);
            const auto error = GetStreamOutput(stdErrPipe);
            process.wait();

            return {.StdOut = output, .StdErr = error, .ExitCode = process.exit_code()};
        }
        // NOLINTBEGIN(bugprone-empty-catch)
        catch (const std::exception&)
        {
            // log? throw?
        }
        // NOLINTEND(bugprone-empty-catch)

        return {};
    }
} // namespace Utils
