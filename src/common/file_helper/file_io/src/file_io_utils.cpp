#include <file_io_utils.hpp>
#include <file_io_wrapper.hpp>

#include <fstream>
#include <sstream>

namespace file_io
{
    FileIOUtils::FileIOUtils(std::shared_ptr<IFileIOWrapper> fileIOWrapper)
    : m_fileIOWrapper(fileIOWrapper ? std::move(fileIOWrapper)
                                    : std::make_shared<FileIOWrapper>())
    {

    }

    void FileIOUtils::readLineByLine(const std::filesystem::path& filePath,
                                const std::function<bool(const std::string&)>& callback) const
    {
        std::ifstream file = m_fileIOWrapper->create_ifstream(filePath.string());

        if (!m_fileIOWrapper->is_open(file))
        {
            throw std::runtime_error("Could not open file");
        }

        std::string line;

        while (m_fileIOWrapper->get_line(file, line))
        {
            if (!callback(line))
            {
                break;
            }
        }
    }

    std::string FileIOUtils::getFileContent(const std::string& filePath) const
    {
        std::stringstream content;
        const auto file = m_fileIOWrapper->create_ifstream(filePath);

        if (m_fileIOWrapper->is_open(file))
        {
            content << m_fileIOWrapper->get_rdbuf(file);
        }

        return content.str();
    }

    std::vector<char> FileIOUtils::getBinaryContent(const std::string& filePath) const
    {
        std::streamoff size {0};
        std::unique_ptr<char[]> spBuffer;
        std::ifstream file {filePath, std::ios_base::binary};

        if (file.is_open())
        {
            // Get pointer to associated buffer object
            auto* buffer = file.rdbuf();

            if (nullptr != buffer)
            {
                // Get file size using buffer's members
                size = buffer->pubseekoff(0, file.end, file.in);
                if (size < 0)
                {
                    return std::vector<char> {};
                }
                buffer->pubseekpos(0, file.in);
                // Allocate memory to contain file data
                auto size_t_size = static_cast<std::size_t>(size);
                spBuffer = std::make_unique<char[]>(size_t_size);
                // Get file data
                buffer->sgetn(spBuffer.get(), static_cast<std::streamsize>(size_t_size));
            }
        }

        return std::vector<char> {spBuffer.get(), spBuffer.get() + static_cast<std::size_t>(size)};
    }
}; // namespace file_io
