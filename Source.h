//
// Created by valerij on 8/13/21.
//

#ifndef YABFPP_SOURCE_H
#define YABFPP_SOURCE_H

#include <vector>
#include <string>
#include <functional>


class Source {
private:
    const std::vector<std::string> lines;
    const std::function<bool(char)> skip;
public:
    Source(std::vector<std::string> lines, std::function<bool(char)> skip):
        lines(std::move(lines)), skip(std::move(skip)) {}

    class Iterator {
    private:
        const Source* const source;
        std::vector<std::string>::const_iterator lineIt;
        std::string::const_iterator charIt;
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = char;
        using pointer = char*;
        using reference = char&;

        Iterator(const Source* const source,
                 const std::vector<std::string>::const_iterator& lineIt,
                 const std::string::const_iterator& charIt);

        const char& operator*() const;

        const char* operator->() const;

        Iterator& operator++();

        Iterator operator++(int);

        [[nodiscard]] bool isEnd() const;

        [[nodiscard]] int getLine() const;

        int getLinePosition() const;

        friend bool operator==(const Iterator& a, const Iterator& b);

        friend bool operator!=(const Iterator& a, const Iterator& b);
    };

    [[nodiscard]] Iterator begin() const;

    [[nodiscard]] Iterator end() const;
};

Source getSource(const std::vector<std::string>& lines, bool legacyMode);

#endif //YABFPP_SOURCE_H
