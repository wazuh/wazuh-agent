#include <agent_runner.hpp>
#include <logger.hpp>

#include <openssl/ssl.h>

#include <stdexcept>

int main(int argc, char* argv[])
{
    const Logger logger;

    try
    {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();

        const AgentRunner agentRunner(argc, argv);
        return agentRunner.Run();
    }
    catch (const std::exception& e)
    {
        LogCritical("An error occurred: {}.", e.what());
        return 1;
    }
}
