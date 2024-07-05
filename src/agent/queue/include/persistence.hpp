#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


using json = nlohmann::json;


/**
 * @brief
 *
 */
class Persistence
{
private:
    std::string filename;

public:
    Persistence(const std::string& fname)
        : filename(fname)
    {
    }

    bool createOrCheckFile()
    {
        std::cout << "createOrCheckFile " << std::endl;
        std::ifstream file(filename);
        if (file.good()) {
            file.close();
            return true;  // File exists
        } else {
            std::ofstream newFile(filename);
            if (newFile.is_open()) {
                newFile.close();
                return true;  // File created successfully
            }
            return false;  // Failed to create file
        }
    }

    bool writeLine(const json data)
    {
        std::cout << "writeLine: " << data << std::endl;
        std::ofstream file(filename, std::ios::app);
        if (file.is_open()) {
            file << to_string(data) << std::endl;
            file.close();
            return true;
        }
        return false;
    }

    /**
     * @brief Reads and return N lines from the queue
     *
     * @param linesQuantity
     * @return std::string
     */
    json readNLines(int linesQuantity)
    {
        std::cout << "readNLines number " << linesQuantity << std::endl;
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
     * @brief Pops N lines from the queue
     *
     * @param linesQuantity
     * @return int number of lines actually popped
     */
    int removeNLines(int linesQuantity)
    {
        std::cout << "removeNLines: " << linesQuantity << std::endl;
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

    int getItems() const
    {
        std::cout << "getItems " << std::endl;
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