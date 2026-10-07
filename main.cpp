#include <algorithm>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string_view>
#include <unordered_set>
#include <vector>

int main(int argc, char *argv[])
{
    constexpr int DEFAULT_HIGHLIGHT_COLOR_CODE = 31;

    bool ignoreCase{false};
    int highlightColorCode{DEFAULT_HIGHLIGHT_COLOR_CODE};

    std::string crosswordPath, wordsPath;
    std::unordered_set<std::string> wordsInput;
    std::vector<std::string> inputCrossword, crosswordLowered;
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

    auto print_usage = [argv]() {
        std::cout << "Usage:\t" << argv[0]
                  << " [crossword-file word-list-file]\n"
                     "\t-i, --ignore-case\tcase insensitive search\n"
                     "\t--crossword-file FILE\tpath of crossword file\n"
                     "\t--words-file FILE\tpath of words file\n"
                     "\t--highlight-color CODE\tANSI code of highlight color (30-37 or 90-97)\n"
                     "\t--help\t\t\tshow help\n";
    };

    const auto args_number = static_cast<size_t>(argc);

    for (size_t i = 1; i < args_number; ++i)
    {
        std::string_view const option{argv[i]};

        if (option == "--ignore-case" || option == "-i")
        {
            ignoreCase = true;
        }
        else if (option == "--crossword-file")
        {
            if (++i < args_number)
            {
                crosswordPath = argv[i];
            }
            else
            {
                std::cerr << "No crossword file specified!\n";
                return EXIT_FAILURE;
            }
        }
        else if (option == "--words-file")
        {
            if (++i < args_number)
            {
                wordsPath = argv[i];
            }
            else
            {
                std::cerr << "No words file specified!\n";
                return EXIT_FAILURE;
            }
        }
        else if (option == "--highlight-color")
        {
            if (++i < args_number)
            {
                std::string_view const value{argv[i]};
                auto const [end, ec] = std::from_chars(value.data(), value.data() + value.size(), highlightColorCode);

                if (ec != std::errc{} || end != value.data() + value.size())
                {
                    std::cerr << "Not a valid integer!\n";
                    return EXIT_FAILURE;
                }

                bool const validColor = (highlightColorCode >= 30 && highlightColorCode <= 37) ||
                                        (highlightColorCode >= 90 && highlightColorCode <= 97);

                if (!validColor)
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
        else if (!option.empty() && option.front() != '-')
        {
            if (crosswordPath.empty())
            {
                crosswordPath = option;
            }
            else if (wordsPath.empty())
            {
                wordsPath = option;
            }
            else
            {
                std::cerr << "Too many positional arguments specified!\n\n";
                print_usage();
                return EXIT_FAILURE;
            }
        }
        else
        {
            std::cerr << "Invalid argument or option!\n\n";
            print_usage();
            return EXIT_FAILURE;
        }
    }

    if (crosswordPath.empty() != wordsPath.empty())
    {
        std::cerr << "Both files must be specified.\n";
        return EXIT_FAILURE;
    }

    size_t maxLength{1};
    bool const inputFromFiles = !crosswordPath.empty() && !wordsPath.empty();

    std::string line;

    auto read_line = [](std::istream &is, std::string &str) -> std::istream & {
        auto &ret = std::getline(is, str);

        if (!str.empty() && str.back() == '\r')
        {
            str.pop_back();
        }

        return ret;
    };

    auto to_lower = [&ctype](std::string &str) {
        std::transform(str.begin(), str.end(), str.begin(), [&ctype](char c) { return ctype.tolower(c); });
    };

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

        while (read_line(crosswordFile, line))
        {
            inputCrossword.push_back(line);

            maxLength = std::max(maxLength, line.length());
        }

        if (inputCrossword.empty())
        {
            std::cerr << " Crossword empty.\n";
            return EXIT_FAILURE;
        }

        while (read_line(wordsFile, line))
        {
            if (line.empty())
            {
                continue;
            }

            if (ignoreCase)
            {
                to_lower(line);
            }

            wordsInput.insert(line);
            wordsInput.insert(std::string(line.rbegin(), line.rend()));
        }

        if (wordsInput.empty())
        {
            std::cerr << " Word list empty.\n";
            return EXIT_FAILURE;
        }
    }
    else
    {
        std::cout << "\n Enter your crossword:\n\n ";

        while (read_line(std::cin, line))
        {
            if (line.empty())
            {
                break;
            }

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

        while (read_line(std::cin, line))
        {
            if (line.empty())
            {
                break;
            }

            if (ignoreCase)
            {
                to_lower(line);
            }

            wordsInput.insert(line);
            wordsInput.insert(std::string(line.rbegin(), line.rend()));

            std::cout.put(' ');
        }

        if (wordsInput.empty())
        {
            std::cerr << (std::cin.eof() ? "End of input reached: exit.\n\n" : " Word list empty.\n\n");

            return EXIT_FAILURE;
        }
    }

    const size_t height = inputCrossword.size();

    std::vector<size_t> rowLength(height);

    for (size_t i{0}; i < height; ++i)
    {
        rowLength[i] = inputCrossword[i].size();
    }

    std::vector<std::vector<std::uint8_t>> highlights(height, std::vector<std::uint8_t>(maxLength, 0));
    std::vector<std::string> words(wordsInput.begin(), wordsInput.end());

    line.reserve(std::max(height, maxLength));

    if (ignoreCase)
    {
        crosswordLowered = inputCrossword;

        for (auto &row : crosswordLowered)
        {
            to_lower(row);
        }
    }

    std::vector<std::string> &crossword = ignoreCase ? crosswordLowered : inputCrossword;

    for (auto &row : crossword)
    {
        row.resize(maxLength);
    }

    auto search_and_highlight = [&](std::string_view lineView, auto mark_highlight) {
        for (const auto &word : words)
        {
            size_t position = 0;

            while ((position = lineView.find(word, position)) != std::string_view::npos)
            {
                for (size_t j{0}; j < word.length(); ++j)
                {
                    mark_highlight(position + j);
                }

                position++;
            }
        }
    };

    for (size_t i{height}; i--;)
    {
        std::string_view const lineView(crossword[i]);

        search_and_highlight(lineView, [&](size_t idx) { highlights[i][idx] = true; });
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j{0}; j < height; ++j)
        {
            line.push_back(crossword[j][i]);
        }

        std::string_view const lineView(line);

        search_and_highlight(lineView, [&](size_t idx) { highlights[idx][i] = true; });
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j < maxLength && k < height; ++j, ++k)
        {
            line.push_back(crossword[k][j]);
        }

        std::string_view const lineView(line);

        search_and_highlight(lineView, [&](size_t idx) { highlights[idx][idx + i] = true; });
    }

    for (size_t i{1}; i < height; ++i)
    {
        line.clear();

        for (size_t k = i, j{0}; j < maxLength && k < height; ++j, ++k)
        {
            line.push_back(crossword[k][j]);
        }

        std::string_view const lineView(line);

        search_and_highlight(lineView, [&](size_t idx) { highlights[i + idx][idx] = true; });
    }

    for (size_t i{1}; i <= height; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j && (k < maxLength); ++k)
        {
            line.push_back(crossword[--j][k]);
        }

        std::string_view const lineView(line);

        search_and_highlight(lineView, [&](size_t idx) { highlights[i - idx - 1][idx] = true; });
    }

    for (size_t i{1}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{height}; k && (j < maxLength); ++j)
        {
            line.push_back(crossword[--k][j]);
        }

        std::string_view const lineView(line);

        search_and_highlight(lineView, [&](size_t idx) { highlights[height - idx - 1][idx + i] = true; });
    }

    for (size_t i{0}; i < height; ++i)
    {
        for (size_t j{0}; j < rowLength[i]; ++j)
        {
            if (highlights[i][j])
            {
                std::cout << " \x1b[1;" << highlightColorCode << "m" << inputCrossword[i][j];
            }
            else
            {
                std::cout << " \x1b[0m" << inputCrossword[i][j];
            }
        }

        std::cout.put('\n');
    }

    if (!inputFromFiles)
    {
        std::cout.put('\n');
    }

    std::cout << "\x1b[0m";
}
