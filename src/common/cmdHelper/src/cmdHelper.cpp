#include <cmdHelper.hpp>

#include <fileSmartDeleter.hpp>
#include <logger.hpp>

#include <boost/process.hpp>

#include <cstdio>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    std::vector<std::string> TokenizeCommand(const std::string& command)
    {
        std::vector<std::string> result;
        std::string current;
        bool inQuotes = false;
        char quoteChar = '\0';

        for (size_t i = 0; i < command.length(); ++i)
        {
            const char c = command[i];

            if ((c == '"' || c == '\''))
            {
                size_t backslashCount = 0;
                for (size_t j = i; j > 0 && command[j - 1] == '\\'; --j)
                {
                    ++backslashCount;
                }
                const bool escaped = (backslashCount % 2 == 1);

                if (!escaped)
                {
                    if (!inQuotes)
                    {
                        inQuotes = true;
                        quoteChar = c;
                    }
                    else if (quoteChar == c)
                    {
                        inQuotes = false;
                    }
                    else
                    {
                        current += c;
                    }
                    continue;
                }
            }

            if (std::isspace(c) && !inQuotes)
            {
                if (!current.empty())
                {
                    result.push_back(current);
                    current.clear();
                }
            }
            else
            {
                current += c;
            }
        }

        if (inQuotes)
        {
            throw std::runtime_error("Unclosed quote in command");
        }

        if (!current.empty())
        {
            result.push_back(current);
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
        catch (const std::exception& e)
        {
            return {.StdOut = "", .StdErr = e.what(), .ExitCode = 1};
        }

        return {.StdOut = "", .StdErr = "", .ExitCode = 1};
    }
} // namespace Utils
