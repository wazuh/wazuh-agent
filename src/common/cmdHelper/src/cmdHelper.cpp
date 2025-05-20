#include <cmdHelper.hpp>

#include <fileSmartDeleter.hpp>
#include <logger.hpp>

#include <boost/process.hpp>

#include <cstdio>
#include <filesystem>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
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

    std::vector<std::string> TokenizeCommand(const std::string& command)
    {
        std::vector<std::string> result;
        std::string accum;
        bool quoting = false;

        const size_t len = command.length();
        size_t i = 0;

        while (i < len)
        {
            const char c = command[i];

            if (c == ' ')
            {
                if (quoting)
                {
                    accum += c;
                }
                else
                {
                    if (!accum.empty())
                    {
                        result.push_back(std::move(accum));
                        accum.clear();
                    }
                }
                ++i;
            }
            else if (c == '"')
            {
                ++i; // Skip the quote
                quoting = !quoting;
            }
            else if (c == '\\')
            {
                ++i;
                if (i < len)
                {
                    if (!quoting && command[i] != '\\' && command[i] != '"')
                    {
                        // Outside quotes, preserve the backslash and the character
                        accum += '\\';
                    }
                    accum += command[i];
                    ++i;
                }
                else
                {
                    // Trailing backslash
                    accum += '\\';
                }
            }
            else
            {
                accum += c;
                ++i;
            }
        }

        if (!accum.empty())
        {
            result.push_back(std::move(accum));
        }

        if (result.empty())
        {
            throw std::runtime_error("Empty command string");
        }

        return result;
    }

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

    std::optional<ExecResult> Exec(const std::string& cmd)
    {
        try
        {
            // Tokenize to separate the command and its arguments
            const auto args = TokenizeCommand(cmd);
            const auto exePath = [&]()
            {
                const std::string& exeCandidate = args[0];

                if (exeCandidate.starts_with('/'))
                {
                    if (!std::filesystem::exists(exeCandidate))
                    {
                        throw std::runtime_error("Executable not found at: " + exeCandidate);
                    }
                    return exeCandidate;
                }

                const auto foundPath = boost::process::search_path(exeCandidate);
                if (foundPath.empty())
                {
                    throw std::runtime_error("Executable not found in PATH: " + exeCandidate);
                }
                return foundPath.string();
            }();

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

            ExecResult result;
            result.StdOut = output;
            result.StdErr = error;
            result.ExitCode = process.exit_code();
            return std::make_optional(result);
        }
        catch (const std::exception& e)
        {
            LogDebug("Error executing command: {}", e.what());
            return std::nullopt;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }
} // namespace Utils
