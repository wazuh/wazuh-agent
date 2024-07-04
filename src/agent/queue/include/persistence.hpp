#include <iostream>
#include <string>
#include <vector>

/**
 * @brief
 *
 */
class Persistence {
private:
    std::string filename;

public:
    Persistence(const std::string& fname) : filename(fname) {}

    bool createOrCheckFile() {
        std::cout << "createOrCheckFile " << std::endl;
        return false;  // Failed to create file
    }

    bool writeLine(const std::string& line) {
        std::cout << "writeLine: " << line << std::endl;

        return true;
    }

    std::string readLine(int lineNumber) {
        std::cout << "readLine number " << lineNumber << std::endl;
        return "";  // Return empty string if line not found
    }

    std::string readLineStartingWith(int number) {
        std::cout << "readLineStartingWith: " << number << std::endl;
        return "";
    }

    bool removeLine(const std::string& lineToRemove) {

        std::cout << "removeLine: " << lineToRemove << std::endl;

        return true;
    }
};