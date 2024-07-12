#include <iostream>
#include <fstream>

#include "persistence.h"

using json = nlohmann::json;


/**
 * @brief Implementation of persistence for storing in plain text file
 *
 */
class FileStorage : public Persistence
{
private:
    std::string filename;

public:
    /**
     * @brief Construct a new File Storage object
     *
     * @param fname full path of the file to be used as persistance
     */
    FileStorage(const std::string& fname)
        : filename(fname)
    {
        //TODO:
        // createOrCheckFile();
    }

    // Delete copy constructor
    FileStorage(const FileStorage&) = delete;

    // Delete copy assignment operator
    FileStorage& operator=(const FileStorage&) = delete;

    // Delete move constructor
    FileStorage(FileStorage&&) = delete;

    // Delete move assignment operator
    FileStorage& operator=(FileStorage&&) = delete;

    ~FileStorage() {};

    bool createOrCheckFile()
    {
        std::ifstream file(filename);
        if (file.good()) {
            file.close();
            // File exists
            return true;
        } else {
            std::ofstream newFile(filename);
            if (newFile.is_open()) {
                newFile.close();
                return true;
            }
            // Failed to create file
            return false;
        }
    }

    void Store(const json& data) override
    {
        std::ofstream file(filename, std::ios::app);
        if (file.is_open()) {
            file << to_string(data) << std::endl;
            file.close();
        }
    }

    /**
     * @brief Reads and return N lines from the queue
     *
     * @param linesQuantity
     * @return std::string
     */
    json RetrieveMultiple(int linesQuantity) override
    {
        std::ifstream file(filename);
        std::string line;
        int currentLine = 0;
        json outputArray = {};

        while (std::getline(file, line)) {
            currentLine++;
            outputArray.emplace_back(line);
            std::cout << "output: " << outputArray << std::endl;
            if (currentLine == linesQuantity) {
                file.close();
                return outputArray;
            }
        }

        // less messages than asked for
        file.close();
        return outputArray;
    }

    /**
     * @brief 
     * 
     * @param LineID 
     * @return json 
     */
    json Retrieve(int LineID) override
    {
        return RetrieveMultiple(1);
    }

    /**
     * @brief Pops N lines from the queue
     *
     * @param linesQuantity
     * @return int number of lines actually popped
     */
    int RemoveMultiple(int linesQuantity) override
    {
        std::vector<std::string> all_lines;
        std::ifstream ifs(filename);
        std::string line;

        // Read all lines from the file
        while (std::getline(ifs, line)) {
            all_lines.push_back(line);
        }
        ifs.close();

        // Check if n is greater than the number of lines in the file
        if (linesQuantity > all_lines.size()) {
            linesQuantity = all_lines.size();
        }

        // Create a vector for the lines to return and remove
        std::vector<std::string> last_n_lines(all_lines.begin(), all_lines.begin() + linesQuantity);
        all_lines.erase(all_lines.begin(), all_lines.begin() + linesQuantity);

        // Write the remaining lines back to the file
        std::ofstream ofs(filename, std::ios::out | std::ios::trunc);
        for (const auto& remaining_line : all_lines) {
            ofs << remaining_line << std::endl;
        }
        ofs.close();

        return linesQuantity;
    }

    /**
     * @brief 
     * 
     * @param LinesID 
     * @return int 
     */
    int Remove(int LinesID) override
    {
        return RemoveMultiple(1);
    }

    /**
     * @brief Get the Element Count object
     * 
     * @return int 
     */
    int GetElementCount() override
    {
        std::vector<std::string> all_lines;
        std::ifstream ifs(filename);
        std::string line;

        // Read all lines from the file
        while (std::getline(ifs, line)) {
            all_lines.push_back(line);
        }
        ifs.close();

        return all_lines.size();
    }
};