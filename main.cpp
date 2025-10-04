#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

inline void error(bool opening, bool firstArgument, const char *argument1, const char *argument2)
{
    std::cerr << "\n Unable to " << (opening ? "open" : "close") << " \"" << (firstArgument ? argument1 : argument2)
              << "\"!\n\n";
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
    using Diff = std::vector<int>::difference_type;

    unsigned long long maxLength{1}, height;
    size_t position;
    std::string line;
    std::unordered_set<std::string> words;
    std::vector<std::string> crossword;
    std::vector<std::vector<bool>> highlights;

    if (argc == 3)
    {
        const char *argument1 = argv[1], *argument2 = argv[2];
        std::ifstream file1, file2;
        file1.open(argument1);

        if (!file1.is_open())
            error(true, true, argument1, argument2);

        file2.open(argument2);

        if (!file2.is_open())
            error(true, false, argument1, argument2);

        while (std::getline(file1, line))
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

        while (std::getline(file2, line))
        {
            words.insert(line);
            std::reverse(line.begin(), line.end());
            words.insert(line);
        }

        if (!words.size())
        {
            std::cerr << " Word list empty.\n";
            return EXIT_FAILURE;
        }

        file1.close();
        file2.close();
    }
    else
    {
        std::cout << "\n Enter your crossword:\n\n ";

        while (std::getline(std::cin, line))
        {
            if (maxLength < line.length())
                maxLength = line.length();

            crossword.push_back(line);

            if (line.empty())
                break;

            std::cout << ' ';
        }

        checkEOF();

        crossword.pop_back();

        height = crossword.size();

        if (!height)
        {
            std::cerr << " Crossword empty.\n";
            return EXIT_FAILURE;
        }

        std::cout << " Enter the words to search in the crossword:\n\n ";

        while (std::getline(std::cin, line))
        {
            words.insert(line);
            std::reverse(line.begin(), line.end());
            words.insert(line);

            if (line.empty())
                break;

            std::cout << ' ';
        }

        checkEOF();

        words.erase("");

        if (!words.size())
        {
            std::cerr << " Word list empty.\n";
            return EXIT_FAILURE;
        }
    }

    std::cout << ' ';
    highlights.resize(height);

    for (size_t i{height}; i--;)
    {
        crossword.at(i).resize(maxLength);
        highlights.at(i).resize(maxLength);
    }

    for (size_t i{height}; i--;)
        for (auto &word : words)
            if ((position = crossword.at(i).find(word)) != std::string::npos)
                std::fill(highlights.at(i).begin() + static_cast<Diff>(position),
                          highlights.at(i).begin() + static_cast<Diff>(position + word.length()), true);

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j{0}; j < height; ++j)
            line.push_back(crossword.at(j).at(i));

        for (auto &word : words)
            for (size_t l{0}; (l + word.length() <= line.size()) && !word.empty(); ++l)
                if (line.substr(l, word.length()) == word)
                    for (size_t m{0}; m < word.size(); ++m)
                        highlights.at(l + m).at(i) = true;
    }

    for (size_t i{0}; i < maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j < maxLength && k < height && crossword.at(k).at(j); ++j, ++k)
            line += crossword.at(k).at(j);

        for (auto &word : words)
            if ((position = line.find(word)) != std::string::npos)
                for (size_t j{0}; j < word.length(); ++j)
                    highlights.at(position + j).at(position + j + i) = true;
    }

    for (size_t i{1}; i < height; ++i)
    {
        line.clear();

        for (size_t k = i, j{0}; j < maxLength && k < height; ++j, ++k)
            line += crossword.at(k).at(j);

        for (auto &word : words)
            if ((position = line.find(word)) != std::string::npos)
                for (size_t j{0}; j < word.length(); ++j)
                    highlights.at(i + position + j).at(position + j) = true;
    }

    for (size_t i{1}; i <= height; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j && (k < maxLength); --j, ++k)
            line += crossword.at(j - 1).at(k);

        for (auto &word : words)
            if ((position = line.find(word)) != std::string::npos)
                for (size_t j{0}; j < word.length(); ++j)
                    highlights.at(i - position - j - 1).at(position + j) = true;
    }

    for (size_t i{1}; i <= maxLength; ++i)
    {
        line.clear();

        for (size_t j = i, k{height - 1}; k && (j < maxLength); --k, ++j)
            line += crossword.at(k).at(j);

        for (auto &word : words)
            if ((position = line.find(word)) != std::string::npos)
                for (size_t j{0}; j < word.length(); ++j)
                    highlights.at(height - 1 - position - j).at(position + j + i) = true;
    }

    for (size_t i{0}; i < height; ++i)
    {
        for (size_t j{0}; j < crossword.at(i).size(); ++j)
            std::cout << "\33[" << highlights.at(i).at(j) * 31 << "m" << crossword.at(i).at(j) << " ";

        std::cout << "\n ";
    }

    std::cout << '\n';
}
