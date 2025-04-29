#include <cmdHelper.hpp>

#include <fileSmartDeleter.hpp>
#include <logger.hpp>

#include <boost/process.hpp>

#include <cstdio>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
    std::vector<std::string> TokenizeCommand(const std::string& command)
    {
        std::vector<std::string> result;
        std::istringstream stream(command);
        std::string token;
        while (stream >> token)
        {
            result.push_back(token);
        }

        if (result.empty())
        {
            throw std::runtime_error("Empty command string");
        }

        return result;
    }

    std::string GetStreamOutput(std::istream& stream)
    {
        std::string line;
        std::stringstream output;
        while (std::getline(stream, line))
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
            while (fgets(buffer.data(), static_cast<int>(buffer.size()), file.get()))
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

            boost::process::ipstream stdoutStream;
            boost::process::ipstream stderrStream;
            const std::vector<std::string> execArgs(args.begin() + 1, args.end());

            boost::process::child process(exePath,
                                          boost::process::args = execArgs,
                                          boost::process::std_out > stdoutStream,
                                          boost::process::std_err > stderrStream);

            const auto output = GetStreamOutput(stdoutStream);
            const auto error = GetStreamOutput(stderrStream);
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
