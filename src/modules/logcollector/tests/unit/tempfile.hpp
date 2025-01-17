#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>

class TempFile {
public:
    TempFile(std::string path, const std::string& str = "") :
        m_path(std::move(path))
    {
        m_stream.open(m_path, std::ios::out | std::ios::binary);

        if (!str.empty()) {
            Write(str);
        }
    }

    void Write(const std::string& str) {
        m_stream.write(str.data(), static_cast<long>(str.size()));
        m_stream.flush();
    }

    void Truncate() {
        // Close and reopen the file to ensure Windows allows the resize
        m_stream.close();
        std::filesystem::resize_file(m_path, 0);
        m_stream.open(m_path, std::ios::out | std::ios::binary | std::ios::trunc);
    }

    ~TempFile() {
        m_stream.close();

        std::error_code ec;
        if (!std::filesystem::remove(m_path, ec))
        {
            auto error_msg = std::system_category().message(ec.value());
        }
    }

    const std::string& Path() const {
        return m_path;
    }

private:
    std::string m_path;
    std::ofstream m_stream;
};
