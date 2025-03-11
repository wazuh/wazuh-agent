#pragma once

#include <map>
#include <regex>
#include <string>
#include <vector>

namespace Utils
{
    /// @brief Converts ISO-8859-1 to UTF-8
    /// @param data string to be converted
    void ISO8859ToUTF8(std::string& data);

    /// @brief Replaces all occurrences of toSearch with toReplace
    /// @param data string to be modified
    /// @param toSearch pattern to search
    /// @param toReplace patter to replace
    /// @return True if replaced
    bool replaceAll(std::string& data, const std::string& toSearch, const std::string& toReplace);

    /// @brief Replaces the first occurrence of toSearch with toReplace
    /// @param data string to be modified
    /// @param toSearch pattern to search
    /// @param toReplace patter to replace
    /// @return True if replaced
    bool replaceFirst(std::string& data, const std::string& toSearch, const std::string& toReplace);

    /// @brief Removes leading characters
    /// @param str string to be trimmed
    /// @param args characters to be removed
    /// @return Trimmed string
    std::string LeftTrim(const std::string& str, const std::string& args = " ");

    /// @brief Removes trailing characters
    /// @param str string to be trimmed
    /// @param args characters to be removed
    /// @return Trimmed string
    std::string RightTrim(const std::string& str, const std::string& args = " ");

    /// @brief Removes leading and trailing characters
    /// @param str string to be trimmed
    /// @param args characters to be removed
    /// @return Trimmed string
    std::string Trim(const std::string& str, const std::string& args = " ");

    /// @brief Splits a string into a vector of strings
    /// @param str string to be split
    /// @param delimiter character to use as delimiter
    /// @return Vector of strings
    std::vector<std::string> split(const std::string& str, const char delimiter);

    /// @brief Splits a string at a specific index
    /// @param str string to be split
    /// @param delimiter character to use as delimiter
    /// @param index index of the value to get
    /// @return string
    std::string splitIndex(const std::string& str, const char delimiter, const size_t index);

    /// @brief Splits a null terminated string into a vector of strings
    /// @param buffer string to be split
    /// @return Vector of strings
    std::vector<std::string> splitNullTerminatedStrings(const char* buffer);

    /// @brief Splits a string into a map of key value pairs
    /// @param str string to be split
    /// @param delimiter character to use as delimiter
    /// @param mapResult map to be filled
    void splitMapKeyValue(const std::string& str, const char delimiter, std::map<std::string, std::string>& mapResult);

    /// @brief Converts a vector of unsigned char to a hex string
    /// @param asciiData vector of unsigned char to be converted
    /// @return Hex string
    std::string asciiToHex(const std::vector<unsigned char>& asciiData);

    /// @brief Converts a string to upper case
    /// @param str string to be converted
    /// @return Upper case string
    std::string toUpperCase(const std::string& str);

    /// @brief Converts a string to lower case
    /// @param str string to be converted
    /// @return Lower case string
    std::string toLowerCase(const std::string& str);

    /// @brief Checks if the string contains upper case characters
    /// @param str string to be checked
    /// @return True if the string contains upper case characters
    bool haveUpperCaseCharacters(const std::string& str);

    /// @brief Converts a string to sentence case
    /// @param str string to be converted
    /// @return Sentence case string
    std::string toSentenceCase(const std::string& str);

    /// @brief Checks if the string starts with the given string
    /// @param str string to be checked
    /// @param start string to check if the string starts with
    /// @return True if the string starts with the given string
    bool startsWith(const std::string& str, const std::string& start);

    /// @brief Checks if the string ends with the given string
    /// @param str string to be checked
    /// @param ending string to check if the string ends with
    /// @return True if the string ends with the given string
    bool endsWith(const std::string& str, const std::string& ending);

    /// @brief Checks if the string contains the given string
    /// @param str string to be checked
    /// @param toSearch string to check if the string contains
    /// @return True if the string contains the given string
    bool contains(const std::string& str, const std::string& toSearch);

    /// @brief Splits a string on the first occurrence of the given string
    /// @param str string to be split
    /// @param args string to use as delimiter
    /// @return Split string
    std::string substrOnFirstOccurrence(const std::string& str, const std::string& args = " ");

    /// @brief Splits a string into a key value pair
    /// @param str string to be split
    /// @param delimiter character to use as delimiter
    /// @param escapeChar character to use as escape
    /// @return Pair of key value
    std::pair<std::string, std::string>
    splitKeyValueNonEscapedDelimiter(const std::string& str, const char delimiter, const char escapeChar);

    /// @brief Finds a regex in a string
    /// @param in string to be searched
    /// @param match string to be filled with the match
    /// @param pattern regex to be searched
    /// @param matchIndex index of the match to be returned
    /// @param start string to start the search
    /// @return True if the regex is found
    bool FindRegexInString(const std::string& in,
                           std::string& match,
                           const std::regex& pattern,
                           const size_t matchIndex = 0,
                           const std::string& start = "");

    /// @brief Checks that the string is alphanumeric and contains all of the special characters passed as argument
    /// @param str Original string
    /// @param specialCharacters Special characters string
    /// @return true if the string is alphanumeric and contains all of the special characters passed as argument
    bool isAlphaNumericWithSpecialCharacters(const std::string& str, const std::string& specialCharacters);

    /// @brief Checks that the string is alphanumeric
    /// @param str Original string
    /// @return true if the string is alphanumeric
    bool isNumber(const std::string& str);

    /// @brief Parses a string to a boolean
    /// @param str String to be parsed
    /// @return Parsed boolean
    bool parseStrToBool(const std::string& str);

    /// @brief Parses a string to a time
    /// @param str String to be parsed
    /// @return Parsed time
    long parseStrToTime(const std::string& str);

    /// @brief Add size padding to a string
    /// @param str Original string
    /// @param padCharacter Character to use for padding
    /// @param minSize Minimum size of the string
    /// @return Padded string
    std::string padString(const std::string& str, const char padCharacter, const int64_t minSize);
} // namespace Utils
