//
// Created by valerij on 8/13/21.
//

#include "Source.h"

#include <utility>


Source::Source(std::vector<std::string> lines, std::function<bool(char)> skip) :
        lines(std::move(lines)), skip(std::move(skip)) {}

Source::Iterator::Iterator(const Source* const source, const std::vector<std::string>::const_iterator& lineIt,
                           const std::basic_string<char, std::char_traits<char>, std::allocator<char>>::const_iterator& charIt)
        : source(source), lineIt(lineIt), charIt(charIt) {}

const char& Source::Iterator::operator*() const {
    return *charIt;
}

const char* Source::Iterator::operator->() const {
    return &(this->operator*());
}

Source::Iterator& Source::Iterator::operator++() {
    do {
        charIt++;
        if (charIt == lineIt->end()) {
            do {
                lineIt++;
            } while (lineIt->empty());
            if (!isEnd()) {
                charIt = lineIt->begin();
            }
        }
    } while (source->skip(this->operator*()));
    return *this;
}

Source::Iterator Source::Iterator::operator++(int) {
    Iterator res = *this;
    ++(*this);
    return res;
}

bool Source::Iterator::isEnd() const {
    return lineIt == source->lines.end();
}

bool operator==(const Source::Iterator& a, const Source::Iterator& b) {
    if (a.isEnd() && b.isEnd()) {
        return true;
    }
    if (!a.isEnd() && !b.isEnd()) {
        return a.charIt == b.charIt;
    }
    return false;
}

bool operator!=(const Source::Iterator& a, const Source::Iterator& b) {
    return !(a == b);
}

Source getSource(const std::vector<std::string>& lines, bool legacyMode) {
    if (legacyMode) {
        std::set<char> legacyCharacters = {'[', ']', '>', '<', '+', '-', '.', ','};
        return Source(lines, [=](const char c) { return legacyCharacters.count(c) == 0; });
    } else {
        return Source(lines, ::isspace);
    }
}

int Source::Iterator::getLine() const {
    if (isEnd()) return source->lines.size() + 1;
    return std::distance(source->lines.begin(), lineIt) + 1;
}

int Source::Iterator::getLinePosition() const {
    if (isEnd()) return source->lines.back().size() + 1;
    return std::distance(lineIt->begin(), charIt) + 1;
}

Source::Iterator Source::begin() const {
    if (this->lines.empty())
        return end();

    auto firstNonEmptyLineIt = std::find_if(this->lines.begin(), this->lines.end(), [](auto s) { return !s.empty(); });
    if (firstNonEmptyLineIt == this->lines.end()) {
        return end();
    }

    Iterator it = {this, firstNonEmptyLineIt, firstNonEmptyLineIt->begin()};
    while (!it.isEnd() && skip(*it)) {
        it++;
    }
    return it;
}

Source::Iterator Source::end() const {
    return {this, this->lines.end(), std::string("").begin()};
}
