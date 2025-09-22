#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

void error(bool opening, bool firstArgument, const char *argument1, const char *argument2)
{
    printf("\n Unable to %s \"%s\"!\n\n", opening ? "open" : "close", firstArgument ? argument1 : argument2);
    exit(EXIT_FAILURE);
}

void checkEOF()
{
    if (std::cin.eof())
    {
        fputs("Exit.\n\n", stdout);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char *argv[])
{
    unsigned long long max_length{0};
    size_t position;
    std::string line;
    std::vector<std::string> words, crossword;
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

            if (max_length < line.length())
                max_length = line.length();
        }

        if (!crossword.size())
        {
            fputs(" Crossword empty.\n", stdout);
            exit(EXIT_FAILURE);
        }

        while (std::getline(file2, line))
        {
            words.push_back(line);
            std::reverse(line.begin(), line.end());
            words.push_back(line);
        }

        if (!words.size())
        {
            fputs(" Word list empty.\n", stdout);
            exit(EXIT_FAILURE);
        }

        file1.close();
        file2.close();
    }
    else
    {
        fputs("\n Enter your crossword:\n\n ", stdout);

        while (std::getline(std::cin, line))
        {
            if (max_length < line.length())
                max_length = line.length();

            crossword.push_back(line);

            if (line.empty())
                break;

            putchar(' ');
        }

        checkEOF();

        crossword.pop_back();

        if (!crossword.size())
        {
            fputs(" Crossword empty.\n\n", stdout);
            exit(EXIT_FAILURE);
        }

        fputs(" Enter the words to search in the crossword:\n\n ", stdout);

        while (std::getline(std::cin, line))
        {
            words.push_back(line);
            std::reverse(line.begin(), line.end());
            words.push_back(line);

            if (line.empty())
                break;

            putchar(' ');
        }

        checkEOF();

        words.pop_back();
        words.pop_back();

        if (!words.size())
        {
            fputs(" Word list empty.\n\n", stdout);
            exit(EXIT_FAILURE);
        }
    }

    putchar(' ');
    highlights.resize(crossword.size());

    for (size_t i{crossword.size()}; i--;)
    {
        crossword.at(i).resize(max_length);
        highlights.at(i).resize(max_length);
    }

    for (size_t i{crossword.size()}; i--;)
        for (auto word : words)
            if ((position = crossword.at(i).find(word)) != std::string::npos)
                std::fill(highlights.at(i).begin() + position, highlights.at(i).begin() + position + word.length(),
                          true);

    for (size_t i{0}; i < max_length; ++i)
    {
        line.clear();

        for (size_t j{0}; j < crossword.size(); ++j)
            line.push_back(crossword.at(j).at(i));

        for (size_t k{0}; k < words.size(); ++k)
            for (size_t l{0}; (l + words.at(k).length() <= line.size()) && !words.at(k).empty(); ++l)
                if (line.substr(l, words.at(k).length()) == words.at(k))
                    for (size_t m{0}; m < words.at(k).size(); ++m)
                        highlights.at(l + m).at(i) = true;
    }

    for (size_t i{0}; i < max_length; ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j < max_length && k < crossword.size() && crossword.at(k).at(j); ++j, ++k)
            line += crossword.at(k).at(j);

        for (auto word : words)
            if ((position = line.find(word)) != std::string::npos)
                for (size_t j{0}; j < word.length(); ++j)
                    highlights.at(position + j).at(position + j + i) = true;
    }

    for (size_t i{1}; i < crossword.size(); ++i)
    {
        line.clear();

        for (size_t k = i, j{0}; j < max_length && k < crossword.size(); ++j, ++k)
            line += crossword.at(k).at(j);

        for (auto word : words)
            if ((position = line.find(word)) != std::string::npos)
                for (size_t j{0}; j < word.length(); ++j)
                    highlights.at(i + position + j).at(position + j) = true;
    }

    for (size_t i{1}; i <= crossword.size(); ++i)
    {
        line.clear();

        for (size_t j = i, k{0}; j && (k < max_length); --j, ++k)
            line += crossword.at(j - 1).at(k);

        for (auto word : words)
            if ((position = line.find(word)) != std::string::npos)
                for (size_t j{0}; j < word.length(); ++j)
                    highlights.at(i - position - j - 1).at(position + j) = true;
    }

    for (size_t i{1}; i <= max_length; ++i)
    {
        line.clear();

        for (size_t j = i, k{crossword.size() - 1}; k && (j < max_length); --k, ++j)
            line += crossword.at(k).at(j - 1);

        for (auto word : words)
            if ((position = line.find(word)) != std::string::npos)
                for (size_t j{0}; j < word.length(); ++j)
                    highlights.at(position - j).at(position + j + i - 1) = true;
    }

    for (size_t i{0}; i < crossword.size(); ++i)
    {
        for (size_t j{0}; j < crossword.at(i).size(); ++j)
            std::cout << "\33[" << highlights.at(i).at(j) * 31 << "m" << crossword.at(i).at(j) << "\33[0m";

        fputs("\n ", stdout);
    }

    putchar('\n');
}
