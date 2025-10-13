#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

inline void error(const char *argument)
{
    std::cerr << " Unable to open \"" << argument << "\"!\n";
    exit(EXIT_FAILURE);
}

inline void checkEOF()
{
    if (std::cin.eof())
    {
        std::cerr << "Exit.\n\n";
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char *argv[])
{
    size_t maxLength{1}, height;
    std::string line;
    std::unordered_set<std::string> words;
    std::vector<std::string> crossword;

    if (argc == 3)
    {
        const char *crosswordPath = argv[1], *wordsPath = argv[2];
        std::ifstream crosswordFile, wordsFile;
        crosswordFile.open(crosswordPath);

        if (!crosswordFile.is_open())
            error(crosswordPath);

        wordsFile.open(wordsPath);

        if (!wordsFile.is_open())
            error(wordsPath);

        while (std::getline(crosswordFile, line))
        {
            crossword.push_back(line);

            if (maxLength < line.length())
                maxLength = line.length();
        }

        height = crossword.size();

        if (!height)
        {
            std::cerr << " Crossword empty.\n";
            return EXIT_FAILURE;
        }

        while (std::getline(wordsFile, line))
        {
            words.insert(line);
            words.insert(std::string(line.rbegin(), line.rend()));
        }

        if (words.empty())
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

            crossword.push_back(line);

            std::cout << ' ';
        }

        checkEOF();

        height = crossword.size();

        if (!height)
        {
            std::cerr << " Crossword empty.\n";
            return EXIT_FAILURE;
        }

        std::cout << " Enter the words to search in the crossword:\n\n ";

        while (std::getline(std::cin, line) && line.length())
        {
            words.insert(line);
            words.insert(std::string(line.rbegin(), line.rend()));

            std::cout << ' ';
        }

        checkEOF();

        if (words.empty())
        {
            std::cerr << " Word list empty.\n";
            return EXIT_FAILURE;
        }
    }

    std::vector<std::vector<bool>> highlights(height, std::vector<bool>(maxLength, 0));

    for (size_t i{height}; i--;)
    {
        crossword.at(i).resize(maxLength);
    }

    for (size_t i{height}; i--;)
    {
        line = crossword.at(i);

        for (auto &word : words)
            for (size_t l{0}; l + word.length() <= maxLength; ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t m{0}; m < word.length(); ++m)
                        highlights.at(i).at(l + m) = true;
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j{0}; j < height; ++j)
            line.push_back(crossword.at(j).at(i));

        for (auto &word : words)
            for (size_t l{0}; l + word.length() <= height; ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t m{0}; m < word.length(); ++m)
                        highlights.at(l + m).at(i) = true;
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j < maxLength && k < height && crossword.at(k).at(j); ++j, ++k)
            line += crossword.at(k).at(j);

        for (auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights.at(l + j).at(l + j + i) = true;
    }

    for (size_t i{1}; i < height; ++i)
    {
        line.clear();

        for (size_t k = i, j{0}; j < maxLength && k < height; ++j, ++k)
            line += crossword.at(k).at(j);

        for (auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights.at(i + l + j).at(l + j) = true;
    }

    for (size_t i{1}; i <= height; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j && (k < maxLength); --j, ++k)
            line += crossword.at(j - 1).at(k);

        for (auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights.at(i - l - j - 1).at(l + j) = true;
    }

    for (size_t i{1}; i <= maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{height - 1}; k && (j < maxLength); --k, ++j)
            line += crossword.at(k).at(j);

        for (auto &word : words)
            for (size_t l{0}; l + word.length() <= line.length(); ++l)
                if (std::string_view(line).substr(l, word.length()) == word)
                    for (size_t j{0}; j < word.length(); ++j)
                        highlights.at(height - 1 - l - j).at(l + j + i) = true;
    }

    for (size_t i{0}; i < height; ++i)
    {
        std::cout << ' ';

        for (size_t j{0}; j < crossword.at(i).size(); ++j)
            std::cout << "\33[" << highlights.at(i).at(j) * 31 << "m" << crossword.at(i).at(j) << " ";

        std::cout << '\n';
    }

    if (argc != 3)
    {
        std::cout << '\n';
    }

    std::cout << "\33[0m";
}
