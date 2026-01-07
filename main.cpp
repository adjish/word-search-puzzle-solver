#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

#define DEFAULT_HIGHLIGHT_COLOR_CODE 31

int main(int argc, const char *argv[])
{
    size_t maxLength{1}, height;
    bool inputFromFiles, ignoreCase = false;
    const char *crosswordPath = nullptr, *wordsPath = nullptr;
    int highlightColorCode = DEFAULT_HIGHLIGHT_COLOR_CODE;
    std::string line;
    std::unordered_set<std::string> words_input;
    std::vector<std::string> *crossword;
    std::vector<std::string> inputCrossword, crosswordLowered;
    std::ifstream crosswordFile, wordsFile;

    std::ios_base::sync_with_stdio(false);

    if (argc == 3)
    {
        crosswordPath = argv[1];
        wordsPath = argv[2];
    }
    else
    {
        bool invalidOption;

        for (int i = 1; i < argc; ++i)
        {
            const char *option = argv[i];

            if (!strcmp(option, "--ignore-case") || !strcmp(option, "-i"))
            {
                ignoreCase = true;
                continue;
            }

            if (!strcmp(option, "--crossword-file"))
            {
                if (i + 1 < argc)
                {
                    ++i;
                    crosswordPath = argv[i];
                    continue;
                }

                std::cerr << "No crossword file specified!\n";
                return EXIT_FAILURE;
            }

            if (!strcmp(option, "--words-file"))
            {
                if (i + 1 < argc)
                {
                    ++i;
                    wordsPath = argv[i];
                    continue;
                }

                std::cerr << "No words file specified!\n";
                return EXIT_FAILURE;
            }

            if (!strcmp(option, "--highlight-color"))
            {
                if (i + 1 < argc)
                {
                    ++i;
                    highlightColorCode = atoi(argv[i]);

                    if (highlightColorCode < 30 || highlightColorCode > 37)
                    {
                        std::cerr << "Invalid ANSI color code!\n";
                        return EXIT_FAILURE;
                    }

                    continue;
                }

                std::cerr << "No color code specified!\n";
                return EXIT_FAILURE;
            }

            invalidOption = (strcmp(option, "--help") && strcmp(option, "-h") && strcmp(option, "-?"));

            if (invalidOption)
            {
                std::cerr << "Invalid argument or option!\n\n";
            }

            std::cout << "Usage:\t" << argv[0]
                      << " [crossword-file word-list-file]\n"
                         "\t-i, --ignore-case\tcase insensitive search\n"
                         "\t--crossword-file FILE\tpath of crossword file\n"
                         "\t--words-file FILE\tpath of words file\n"
                         "\t--highlight-color CODE\tANSI code of highlight color\n"
                         "\t--help\t\t\tshow help\n";

            if (invalidOption)
            {
                return EXIT_FAILURE;
            }

            return EXIT_SUCCESS;
        }
    }

    inputFromFiles = crosswordPath != nullptr && wordsPath != nullptr;

    if (inputFromFiles)
    {
        crosswordFile.open(crosswordPath);
        wordsFile.open(wordsPath);

        if (!crosswordFile)
        {
            std::cerr << " Unable to open \"" << crosswordPath << "\"!\n";
            return EXIT_FAILURE;
        }

        if (!wordsFile)
        {
            std::cerr << " Unable to open \"" << wordsPath << "\"!\n";
            return EXIT_FAILURE;
        }

        while (std::getline(crosswordFile, line))
        {
            inputCrossword.push_back(line);

            if (maxLength < line.length())
                maxLength = line.length();
        }

        height = inputCrossword.size();

        if (!height)
        {
            std::cerr << " Crossword empty.\n";
            return EXIT_FAILURE;
        }

        while (std::getline(wordsFile, line))
        {
            if (ignoreCase)
            {
                std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c) { return std::tolower(c); });
            }

            words_input.insert(line);
            words_input.insert(std::string(line.rbegin(), line.rend()));
        }

        if (words_input.empty())
        {
            std::cerr << " Word list empty.\n";
            return EXIT_FAILURE;
        }

        crosswordFile.close();
        wordsFile.close();
    }
    else
    {
        std::cout << "\n Enter your crossword:\n\n ";

        while (std::getline(std::cin, line) && line.length())
        {
            if (maxLength < line.length())
                maxLength = line.length();

            inputCrossword.push_back(line);

            std::cout << ' ';
        }

        if (std::cin.eof())
        {
            std::cerr << "End of input reached: exit.\n\n";
            return EXIT_FAILURE;
        }

        height = inputCrossword.size();

        if (!height)
        {
            std::cerr << " Crossword empty.\n\n";
            return EXIT_FAILURE;
        }

        std::cout << " Enter the words to search in the crossword:\n\n ";

        while (std::getline(std::cin, line) && line.length())
        {
            if (ignoreCase)
            {
                std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c) { return std::tolower(c); });
            }

            words_input.insert(line);
            words_input.insert(std::string(line.rbegin(), line.rend()));

            std::cout << ' ';
        }

        if (std::cin.eof())
        {
            std::cerr << "End of input reached: exit.\n\n";
            return EXIT_FAILURE;
        }

        if (words_input.empty())
        {
            std::cerr << " Word list empty.\n\n";
            return EXIT_FAILURE;
        }
    }

    std::vector<std::vector<bool>> highlights(height, std::vector<bool>(maxLength, false));
    std::vector<std::string_view> words;

    words.reserve(words_input.size());

    std::copy(words_input.begin(), words_input.end(), std::back_inserter(words));

    if (ignoreCase)
    {
        for (auto string : inputCrossword)
        {
            std::transform(string.begin(), string.end(), string.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            crosswordLowered.push_back(string);
        }

        crossword = &crosswordLowered;
    }
    else
    {
        crossword = &inputCrossword;
    }

    for (auto &string : (*crossword))
    {
        string.resize(maxLength);
    }

    for (size_t i{height}; i--;)
    {
        line = (*crossword).at(i);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= maxLength; ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights.at(i).at(l + j) = true;
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j{0}; j < height; ++j)
            line.push_back((*crossword).at(j).at(i));

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= height; ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights.at(l + j).at(i) = true;
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j < maxLength && k < height && (*crossword).at(k).at(j); ++j, ++k)
            line += (*crossword).at(k).at(j);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights.at(l + j).at(l + j + i) = true;
    }

    for (size_t i{1}; i < height; ++i)
    {
        line.clear();

        for (size_t k = i, j{0}; j < maxLength && k < height; ++j, ++k)
            line += (*crossword).at(k).at(j);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights.at(i + l + j).at(l + j) = true;
    }

    for (size_t i{1}; i <= height; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j-- && (k < maxLength); ++k)
            line += (*crossword).at(j).at(k);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights.at(i - l - j - 1).at(l + j) = true;
    }

    for (size_t i{1}; i <= maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{height}; --k && (j < maxLength); ++j)
            line += (*crossword).at(k).at(j);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights.at(height - 1 - l - j).at(l + j + i) = true;
    }

    for (size_t i{0}; i < height; ++i)
    {
        for (size_t j{0}; j < inputCrossword.at(i).size(); ++j)
            std::cout << " \33[" << highlights.at(i).at(j) * highlightColorCode << "m" << inputCrossword.at(i).at(j);

        std::cout << '\n';
    }

    if (!inputFromFiles)
    {
        std::cout << '\n';
    }

    std::cout << "\33[0m";
}
