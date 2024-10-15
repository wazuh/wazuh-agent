#pragma once

#include <filesystem>
using namespace std;

class TempFile {
public:
    TempFile(string path) :
        m_path(std::move(path)),
        m_stream(m_path) { }

    void write(const string& str) {
        m_stream.write(str.data(), static_cast<long>(str.size()));
    }

    ~TempFile() {
        error_code ec;
        filesystem::remove(m_path, ec);
    }

    const std::string& Path() const {
        return m_path;
    }

private:
    string m_path;
    ofstream m_stream;
};
