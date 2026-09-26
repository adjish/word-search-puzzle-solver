#include <algorithm>
#include <fstream>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <unordered_set>
#include <vector>

int main(int argc, char *argv[])
{
    constexpr int DEFAULT_HIGHLIGHT_COLOR_CODE = 31;

    bool ignoreCase{false};
    int highlightColorCode{DEFAULT_HIGHLIGHT_COLOR_CODE};

    std::string crosswordPath, wordsPath;
    std::unordered_set<std::string> words_input;
    std::vector<std::string> inputCrossword, crosswordLowered, args{argv, argv + argc};
    std::locale loc;

    try
    {
        loc = std::locale("");
    }
    catch (const std::runtime_error &)
    {
        loc = std::locale::classic();
    }

    auto const &ctype = std::use_facet<std::ctype<char>>(loc);

    std::ios_base::sync_with_stdio(false);

    auto print_usage = [&args]() {
        std::cout << "Usage:\t" << args[0]
                  << " [crossword-file word-list-file]\n"
                     "\t-i, --ignore-case\tcase insensitive search\n"
                     "\t--crossword-file FILE\tpath of crossword file\n"
                     "\t--words-file FILE\tpath of words file\n"
                     "\t--highlight-color CODE\tANSI code of highlight color (30-37 or 90-97)\n"
                     "\t--help\t\t\tshow help\n";
    };

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
            }
            else if (option == "--crossword-file")
            {
                if (i + 1 < args_number)
                {
                    crosswordPath = std::move(args[++i]);
                }
                else
                {
                    std::cerr << "No crossword file specified!\n";
                    return EXIT_FAILURE;
                }
            }
            else if (option == "--words-file")
            {
                if (i + 1 < args_number)
                {
                    wordsPath = std::move(args[++i]);
                }
                else
                {
                    std::cerr << "No words file specified!\n";
                    return EXIT_FAILURE;
                }
            }
            else if (option == "--highlight-color")
            {
                if (i + 1 < args_number)
                {
                    try
                    {
                        highlightColorCode = std::stoi(args[++i]);

                        bool const validColor = (highlightColorCode >= 30 && highlightColorCode <= 37) ||
                                                (highlightColorCode >= 90 && highlightColorCode <= 97);

                        if (!validColor)
                        {
                            throw std::invalid_argument("Color code out of range");
                        }
                    }
                    catch (const std::exception &)
                    {
                        std::cerr << "Invalid ANSI color code!\n";
                        return EXIT_FAILURE;
                    }
                }
                else
                {
                    std::cerr << "No color code specified!\n";
                    return EXIT_FAILURE;
                }
            }
            else if (option == "--help" || option == "-h" || option == "-?")
            {
                print_usage();
                return EXIT_SUCCESS;
            }
            else
            {
                std::cerr << "Invalid argument or option!\n\n";
                print_usage();
                return EXIT_FAILURE;
            }
        }
    }

    size_t maxLength{1};
    bool const inputFromFiles = !crosswordPath.empty() && !wordsPath.empty();

    if (crosswordPath.empty() != wordsPath.empty())
    {
        std::cerr << "Both files must be specified.\n";
        return EXIT_FAILURE;
    }

    std::string line;

    if (inputFromFiles)
    {
        std::ifstream crosswordFile(crosswordPath), wordsFile(wordsPath);

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

        if (inputCrossword.empty())
        {
            std::cerr << " Crossword empty.\n";
            return EXIT_FAILURE;
        }

        while (std::getline(wordsFile, line))
        {
            if (ignoreCase)
            {
                std::transform(line.begin(), line.end(), line.begin(), [&ctype](char c) { return ctype.tolower(c); });
            }

            words_input.insert(line);
            words_input.insert(std::string(line.rbegin(), line.rend()));
        }

        if (words_input.empty())
        {
            std::cerr << " Word list empty.\n";
            return EXIT_FAILURE;
        }
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

        if (inputCrossword.empty())
        {
            std::cerr << (std::cin.eof() ? "End of input reached: exit.\n\n" : " Crossword empty.\n\n");
            return EXIT_FAILURE;
        }

        if (std::cin.eof())
        {
            std::cerr << "Unexpected end of input: crossword must be followed by an empty line and word list.\n\n";
            return EXIT_FAILURE;
        }

        std::cout << " Enter the words to search in the crossword:\n\n ";

        while (std::getline(std::cin, line) && !line.empty())
        {
            if (ignoreCase)
            {
                std::transform(line.begin(), line.end(), line.begin(), [&ctype](char c) { return ctype.tolower(c); });
            }

            words_input.insert(line);
            words_input.insert(std::string(line.rbegin(), line.rend()));

            std::cout.put(' ');
        }

        if (words_input.empty())
        {
            std::cerr << (std::cin.eof() ? "End of input reached: exit.\n\n" : " Word list empty.\n\n");

            return EXIT_FAILURE;
        }
    }

    const size_t height = inputCrossword.size();

    std::vector<std::vector<bool>> highlights(height, std::vector<bool>(maxLength));
    std::vector<std::string> words(words_input.begin(), words_input.end());

    line.reserve(std::max(height, maxLength));

    std::vector<std::string> *crossword;

    if (ignoreCase)
    {
        crosswordLowered = inputCrossword;

        for (auto &row : crosswordLowered)
        {
            std::transform(row.begin(), row.end(), row.begin(), [&ctype](char c) { return ctype.tolower(c); });
        }

        crossword = &crosswordLowered;
    }
    else
    {
        crossword = &inputCrossword;
    }

    for (auto &row : (*crossword))
    {
        row.resize(maxLength);
    }

    for (size_t i{height}; i--;)
    {
        std::string_view const lineView((*crossword)[i]);

        for (const auto &word : words)
        {
            size_t position = 0;

            while ((position = lineView.find(word, position)) != std::string_view::npos)
            {
                for (size_t j{0}; j < word.length(); ++j)
                    highlights[i][position + j] = true;

                position++;
            }
        }
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j{0}; j < height; ++j)
            line.push_back((*crossword)[j][i]);

        std::string_view const lineView(line);

        for (const auto &word : words)
        {
            size_t position = 0;

            while ((position = lineView.find(word, position)) != std::string_view::npos)
            {
                for (size_t j{0}; j < word.length(); ++j)
                    highlights[position + j][i] = true;

                position++;
            }
        }
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j < maxLength && k < height; ++j, ++k)
            line.push_back((*crossword)[k][j]);

        std::string_view const lineView(line);

        for (const auto &word : words)
        {
            size_t position = 0;

            while ((position = lineView.find(word, position)) != std::string_view::npos)
            {
                for (size_t j{0}; j < word.length(); ++j)
                    highlights[position + j][position + j + i] = true;

                position++;
            }
        }
    }

    for (size_t i{1}; i < height; ++i)
    {
        line.clear();

        for (size_t k = i, j{0}; j < maxLength && k < height; ++j, ++k)
            line.push_back((*crossword)[k][j]);

        std::string_view const lineView(line);

        for (const auto &word : words)
        {
            size_t position = 0;

            while ((position = lineView.find(word, position)) != std::string_view::npos)
            {
                for (size_t j{0}; j < word.length(); ++j)
                    highlights[i + position + j][position + j] = true;

                position++;
            }
        }
    }

    for (size_t i{1}; i <= height; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j && (k < maxLength); ++k)
            line.push_back((*crossword)[--j][k]);

        std::string_view const lineView(line);

        for (const auto &word : words)
        {
            size_t position = 0;

            while ((position = lineView.find(word, position)) != std::string_view::npos)
            {
                for (size_t j{0}; j < word.length(); ++j)
                    highlights[i - position - j - 1][position + j] = true;

                position++;
            }
        }
    }

    for (size_t i{1}; i <= maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{height}; k && (j < maxLength); ++j)
            line.push_back((*crossword)[--k][j]);

        std::string_view const lineView(line);

        for (const auto &word : words)
        {
            size_t position = 0;

            while ((position = lineView.find(word, position)) != std::string_view::npos)
            {
                for (size_t j{0}; j < word.length(); ++j)
                    highlights[height - position - j - 1][position + j + i] = true;

                position++;
            }
        }
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
