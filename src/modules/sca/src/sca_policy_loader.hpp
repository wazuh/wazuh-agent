#pragma once

#include <isca_policy_loader.hpp>

#include <ifilesystem_wrapper.hpp>

#include <memory>

class SCAPolicyLoader : public ISCAPolicyLoader
{
public:
    SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr);

    ~SCAPolicyLoader() = default;

private:
    std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;
};
