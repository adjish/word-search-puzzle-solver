#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <locale>
#include <unordered_set>
#include <vector>

int main(int argc, const char *argv[])
{
    constexpr int DEFAULT_HIGHLIGHT_COLOR_CODE = 31;

    bool ignoreCase{false};
    int highlightColorCode{DEFAULT_HIGHLIGHT_COLOR_CODE};

    std::string crosswordPath, wordsPath;
    std::unordered_set<std::string> words_input;
    std::vector<std::string> inputCrossword, crosswordLowered, args(argv, argv + argc);
    std::ifstream crosswordFile, wordsFile;
    std::locale loc;

    try
    {
        loc = std::locale("");
    }
    catch (const std::runtime_error &)
    {
        loc = std::locale::classic();
    }

    std::ios_base::sync_with_stdio(false);

    if (argc == 3 && !args[1].empty() && args[1].front() != '-' && !args[2].empty() && args[2].front() != '-')
    {
        crosswordPath = std::move(args[1]);
        wordsPath = std::move(args[2]);
    }
    else
    {
        auto args_number = static_cast<size_t>(argc);

        for (size_t i = 1; i < args_number; ++i)
        {
            std::string_view const option{args[i]};

            if (option == "--ignore-case" || option == "-i")
            {
                ignoreCase = true;
                continue;
            }

            if (option == "--crossword-file")
            {
                if (i + 1 < args_number)
                {
                    ++i;
                    crosswordPath = std::move(args[i]);
                    continue;
                }

                std::cerr << "No crossword file specified!\n";
                return EXIT_FAILURE;
            }

            if (option == "--words-file")
            {
                if (i + 1 < args_number)
                {
                    ++i;
                    wordsPath = std::move(args[i]);
                    continue;
                }

                std::cerr << "No words file specified!\n";
                return EXIT_FAILURE;
            }

            if (option == "--highlight-color")
            {
                if (i + 1 < args_number)
                {
                    ++i;

                    try
                    {
                        highlightColorCode = std::stoi(args[i]);
                    }
                    catch (const std::exception &)
                    {
                        std::cerr << "Not a valid integer!\n";
                        return EXIT_FAILURE;
                    }

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

            bool const invalidOption = (option != "--help" && option != "-h" && option != "-?");

            if (invalidOption)
            {
                std::cerr << "Invalid argument or option!\n\n";
            }

            std::cout << "Usage:\t" << args[0]
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

    size_t maxLength{1}, height;
    bool const inputFromFiles = !crosswordPath.empty() && !wordsPath.empty();

    if (crosswordPath.empty() != wordsPath.empty())
    {
        std::cerr << "Both files must be specified.\n";
        return EXIT_FAILURE;
    }

    std::string line;

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

            maxLength = std::max(maxLength, line.length());
        }

        height = inputCrossword.size();

        if (height == 0U)
        {
            std::cerr << " Crossword empty.\n";
            return EXIT_FAILURE;
        }

        while (std::getline(wordsFile, line))
        {
            if (ignoreCase)
            {
                std::transform(line.begin(), line.end(), line.begin(),
                               [&loc](unsigned char c) { return std::use_facet<std::ctype<char>>(loc).tolower(c); });
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

        while (std::getline(std::cin, line) && !line.empty())
        {
            maxLength = std::max(maxLength, line.length());

            inputCrossword.push_back(line);

            std::cout.put(' ');
        }

        if (std::cin.eof())
        {
            std::cerr << "End of input reached: exit.\n\n";
            return EXIT_FAILURE;
        }

        height = inputCrossword.size();

        if (height == 0U)
        {
            std::cerr << " Crossword empty.\n\n";
            return EXIT_FAILURE;
        }

        std::cout << " Enter the words to search in the crossword:\n\n ";

        while (std::getline(std::cin, line) && !line.empty())
        {
            if (ignoreCase)
            {
                std::transform(line.begin(), line.end(), line.begin(),
                               [&loc](unsigned char c) { return std::use_facet<std::ctype<char>>(loc).tolower(c); });
            }

            words_input.insert(line);
            words_input.insert(std::string(line.rbegin(), line.rend()));

            std::cout.put(' ');
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
    std::vector<std::string> words(words_input.begin(), words_input.end());

    line.reserve(std::max(height, maxLength));

    std::vector<std::string> *crossword;

    if (ignoreCase)
    {
        crosswordLowered = inputCrossword;

        for (auto &string : crosswordLowered)
        {
            std::transform(string.begin(), string.end(), string.begin(),
                           [&loc](unsigned char c) { return std::use_facet<std::ctype<char>>(loc).tolower(c); });
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
        line = (*crossword)[i];
        std::string_view const lineView(line);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= maxLength; ++l)
                if (lineView.substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights[i][l + j] = true;
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j{0}; j < height; ++j)
            line.push_back((*crossword)[j][i]);

        std::string_view const lineView(line);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= height; ++l)
                if (lineView.substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights[l + j][i] = true;
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j < maxLength && k < height; ++j, ++k)
            line.push_back((*crossword)[k][j]);

        std::string_view const lineView(line);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (lineView.substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights[l + j][l + j + i] = true;
    }

    for (size_t i{1}; i < height; ++i)
    {
        line.clear();

        for (size_t k = i, j{0}; j < maxLength && k < height; ++j, ++k)
            line.push_back((*crossword)[k][j]);

        std::string_view const lineView(line);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (lineView.substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights[i + l + j][l + j] = true;
    }

    for (size_t i{1}; i <= height; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j && (k < maxLength); ++k)
            line.push_back((*crossword)[--j][k]);

        std::string_view const lineView(line);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (lineView.substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights[i - l - j - 1][l + j] = true;
    }

    for (size_t i{1}; i <= maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{height}; k && (j < maxLength); ++j)
            line.push_back((*crossword)[--k][j]);

        std::string_view const lineView(line);

        for (const auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (lineView.substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights[height - l - j - 1][l + j + i] = true;
    }

    for (size_t i{0}; i < height; ++i)
    {
        for (size_t j{0}; j < inputCrossword[i].size(); ++j)
            std::cout << " \x1b[" << (highlights[i][j] ? highlightColorCode : 0) << "m" << inputCrossword[i][j];

        std::cout.put('\n');
    }

    if (!inputFromFiles)
    {
        std::cout.put('\n');
    }

    std::cout << "\x1b[0m";
}
