#pragma once

#include <filesystem>

class TempFile {
public:
    TempFile(std::string path, const std::string& str = "") :
        m_path(std::move(path)),
        m_stream(m_path)
    {
        if (!str.empty()) {
            Write(str);
        }
    }

    void Write(const std::string& str) {
        m_stream.write(str.data(), static_cast<long>(str.size()));
        m_stream.flush();
    }

    void Truncate() {
        std::filesystem::resize_file(m_path, 0);
        m_stream.seekp(0);
    }

    ~TempFile() {
        std::error_code ec;
        std::filesystem::remove(m_path, ec);
    }

    const std::string& Path() const {
        return m_path;
    }

private:
    std::string m_path;
    std::ofstream m_stream;
};
