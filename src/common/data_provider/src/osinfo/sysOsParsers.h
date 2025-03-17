#pragma once

#include <istream>
#include <nlohmann/json.hpp>

/// @brief Interface for OS information
struct ISysOsParser
{
    /// @brief Default destructor
    virtual ~ISysOsParser() = default;

    /// @brief Parses the OS information
    /// @return True if successful
    virtual bool parseFile(std::istream& /*in*/, nlohmann::json& /*output*/)
    {
        return false;
    }

    /// @brief Parses the OS uname
    /// @return True if successful
    virtual bool parseUname(const std::string& /*in*/, nlohmann::json& /*output*/)
    {
        return false;
    }
};

/// @brief Class for Unix OS information
class UnixOsParser : public ISysOsParser
{
public:
    /// @brief Default constructor
    UnixOsParser() = default;

    /// @brief Default destructor
    ~UnixOsParser() = default;

    /// @brief Parses the OS information
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseFile(std::istream& in, nlohmann::json& output) override;
};

/// @brief Class for Ubuntu OS information
class UbuntuOsParser : public ISysOsParser
{
public:
    /// @brief Default constructor
    UbuntuOsParser() = default;

    /// @brief Default destructor
    ~UbuntuOsParser() = default;

    /// @brief Parses the OS information
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseFile(std::istream& in, nlohmann::json& output) override;
};

/// @brief Class for Centos OS information
class CentosOsParser : public ISysOsParser
{
public:
    /// @brief Default constructor
    CentosOsParser() = default;

    /// @brief Default destructor
    ~CentosOsParser() = default;

    /// @brief Parses the OS information
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseFile(std::istream& in, nlohmann::json& output) override;
};

/// @brief Class for BSD OS information
class BSDOsParser : public ISysOsParser
{
public:
    /// @brief Default constructor
    BSDOsParser() = default;

    /// @brief Default destructor
    ~BSDOsParser() = default;

    /// @brief Parses the OS uname
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseUname(const std::string& in, nlohmann::json& output) override;
};

/// @brief Class for RedHat OS information
class RedHatOsParser : public ISysOsParser
{
public:
    /// @brief Default constructor
    RedHatOsParser() = default;

    /// @brief Default destructor
    ~RedHatOsParser() = default;

    /// @brief Parses the OS information
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseFile(std::istream& in, nlohmann::json& output) override;
};

/// @brief Class for Debian OS information
class DebianOsParser : public ISysOsParser
{
public:
    /// @brief Default constructor
    DebianOsParser() = default;

    /// @brief Default destructor
    ~DebianOsParser() = default;

    /// @brief Parses the OS information
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseFile(std::istream& in, nlohmann::json& output) override;
};

/// @brief Class for Slackware OS information
class SlackwareOsParser : public ISysOsParser
{
public:
    /// @brief Default constructor
    SlackwareOsParser() = default;

    /// @brief Default destructor
    ~SlackwareOsParser() = default;

    /// @brief Parses the OS information
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseFile(std::istream& in, nlohmann::json& output) override;
};

/// @brief Class for Gentoo OS information
class GentooOsParser : public ISysOsParser
{
public:
    /// @brief Default constructor
    GentooOsParser() = default;

    /// @brief Default destructor
    ~GentooOsParser() = default;

    /// @brief Parses the OS information
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseFile(std::istream& in, nlohmann::json& output) override;
};

/// @brief Class for SuSE OS information
class SuSEOsParser : public ISysOsParser
{
public:
    /// @brief Default constructor
    SuSEOsParser() = default;

    /// @brief Default destructor
    ~SuSEOsParser() = default;

    /// @brief Parses the OS information
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseFile(std::istream& in, nlohmann::json& output) override;
};

/// @brief Class for Fedora OS information
class FedoraOsParser : public ISysOsParser
{
public:
    /// @brief Default constructor
    FedoraOsParser() = default;

    /// @brief Default destructor
    ~FedoraOsParser() = default;

    /// @brief Parses the OS information
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseFile(std::istream& in, nlohmann::json& output) override;
};

/// @brief Class for Mac OS information
class MacOsParser
{
public:
    /// @brief Default constructor
    MacOsParser() = default;

    /// @brief Default destructor
    ~MacOsParser() = default;

    /// @brief Parses the OS information
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseSwVersion(const std::string& in, nlohmann::json& output);

    /// @brief Parses the OS system profiler
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseSystemProfiler(const std::string& in, nlohmann::json& output);

    /// @brief Parses the OS uname
    /// @param in Input
    /// @param output Output
    /// @return True if successful
    bool parseUname(const std::string& in, nlohmann::json& output);
};

/// @brief Factory class for OS parsers
class FactorySysOsParser final
{
public:
    /// @brief Creates an OS parser
    /// @param platform Platform
    /// @return OS parser
    static std::unique_ptr<ISysOsParser> create(const std::string& platform)
    {
        if (platform == "ubuntu")
        {
            return std::make_unique<UbuntuOsParser>();
        }

        if (platform == "centos")
        {
            return std::make_unique<CentosOsParser>();
        }

        if (platform == "unix")
        {
            return std::make_unique<UnixOsParser>();
        }

        if (platform == "bsd")
        {
            return std::make_unique<BSDOsParser>();
        }

        if (platform == "fedora")
        {
            return std::make_unique<FedoraOsParser>();
        }

        if (platform == "debian")
        {
            return std::make_unique<DebianOsParser>();
        }

        if (platform == "gentoo")
        {
            return std::make_unique<GentooOsParser>();
        }

        if (platform == "slackware")
        {
            return std::make_unique<SlackwareOsParser>();
        }

        if (platform == "suse")
        {
            return std::make_unique<SuSEOsParser>();
        }

        if (platform == "rhel")
        {
            return std::make_unique<RedHatOsParser>();
        }

        throw std::runtime_error {"Unsupported platform."};
    }
};
