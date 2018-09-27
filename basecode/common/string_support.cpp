// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <vector>
#include <sstream>
#include <fmt/format.h>
#include "string_support.h"

namespace basecode::common {

    std::string word_wrap(
            std::string text,
            size_t width,
            size_t right_pad,
            const char& fill) {
        size_t line_begin = 0;

        while (line_begin < text.size()) {
            const auto ideal_end = line_begin + width;
            size_t line_end = ideal_end <= text.size() ?
                              ideal_end :
                              text.size() - 1;

            if (line_end == text.size() - 1)
                ++line_end;
            else if (std::isspace(text[line_end])) {
                text[line_end++] = '\n';
                for (size_t i = 0; i < right_pad; i++)
                    text.insert(line_end++, 1, fill);
            } else {
                auto end = line_end;
                while (end > line_begin && !std::isspace(text[end]))
                    --end;

                if (end != line_begin) {
                    line_end = end;
                    text[line_end++] = '\n';
                    for (size_t i = 0; i < right_pad; i++)
                        text.insert(line_end++, 1, fill);
                } else {
                    text.insert(line_end++, 1, '\n');
                    for (size_t i = 0; i < right_pad; i++)
                        text.insert(line_end++, 1, fill);
                }
            }

            line_begin = line_end;
        }

        return text;
    }

    std::string escaped_string(const std::string& value) {
        std::stringstream stream;
        bool escaped = false;
        for (auto& ch : value) {
            if (ch == '\\') {
                escaped = true;
            } else {
                if (escaped) {
                    if (ch == 'n')
                        stream << "\n";
                    else if (ch == 'r')
                        stream << "\r";
                    else if (ch == 't')
                        stream << "\t";
                    else if (ch == '\\')
                        stream << "\\";
                    else if (ch == '0')
                        stream << '\0';

                    escaped = false;
                } else {
                    stream << ch;
                }
            }
        }
        return stream.str();
    }

    std::pair<std::string, std::string> size_to_units(size_t size) {
        auto i = 0;
        const char* units[] = {"bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
        while (size > 1024) {
            size /= 1024;
            i++;
        }
        return std::make_pair(
            i > 1 ?
            fmt::format("{}.{}", i, size) :
            fmt::format("{}", size),
            units[i]);
    }

    std::string list_to_string(const std::vector<std::string>& list, const char& sep) {
        std::stringstream stream;

        for (size_t i = 0; i < list.size(); i++) {
            if (i > 0)
                stream << sep;
            stream << list[i];
        }

        return stream.str();
    }

    std::vector<std::string> string_to_list(const std::string& value, const char& sep) {
        std::vector<std::string> list;

        std::istringstream f(value);
        std::string s;
        while (std::getline(f, s, sep)) {
            list.push_back(s);
        }

        return list;
    }

}