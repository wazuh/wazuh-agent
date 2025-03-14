#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace Utils
{
    /// @brief Pipeline Writer interface
    /// @details Base class for Write nodes in the pipeline.
    /// @tparam T Message type to be sent to readers in the pipeline.
    /// @tparam Reader Type of the readers that will be chained with the writer.
    template<typename T, typename Reader>
    class IPipelineWriter
    {
    protected:
        /// @brief Sends a message to all the chained readers in the pipeline.
        /// @param data Message data to be sent to readers.
        void send(const T& data)
        {
            for (const auto& reader : m_readers)
            {
                reader->receive(data);
            }
        }

    public:
        /// @brief Destructor
        virtual ~IPipelineWriter() = default;

        /// @brief Adds a reader to the chain.
        /// @param reader shared_ptr to the reader to be added.
        void addReader(std::shared_ptr<Reader>& reader)
        {
            m_readers.push_back(reader);
        }

    private:
        std::vector<std::shared_ptr<Reader>> m_readers;
    };

    /// @brief Helper function to connect nodes in the pipeline.
    /// @details Write class should accept Reader class. Compile time checked.
    /// @tparam Writer Writer class. Should expose an addReader method.
    /// @tparam Reader Reader class.
    /// @param writer Writer instance.
    /// @param reader Reader instance.
    template<typename Writer, typename Reader>
    void connect(std::shared_ptr<Writer> writer, std::shared_ptr<Reader> reader)
    {
        if (writer && reader)
        {
            writer->addReader(reader);
        }
    }
} // namespace Utils
