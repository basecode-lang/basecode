#pragma once

#include <map>
#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include <variant>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include "ast.h"

namespace basecode {

    class result;

    struct scanner_pos_t {
        uint32_t line;
        size_t index;
        uint32_t column;
    };

    struct parser_input_t {
        static void split_to_lines(
                const std::string& source,
                std::vector<std::string>& source_lines) {
            source_lines.clear();
            std::stringstream stream;
            stream << source << "\n";
            std::string line;
            while (std::getline(stream, line)) {
                source_lines.push_back(line);
            }
        }

        parser_input_t() : source("") {
        }

        explicit parser_input_t(const std::string& source) {
            this->source = source;
            split_to_lines(this->source, this->source_lines);
        }

        inline bool empty() const {
            return source.empty();
        }

        inline size_t length() const {
            return source.length();
        }

        inline std::string& operator[](size_t index) {
            assert(index >= 0 && index < source_lines.size());
            return source_lines[index];
        }

        inline const std::string& operator[](size_t index) const {
            assert(index >= 0 && index < source_lines.size());
            return source_lines[index];
        }

        std::string source;
        std::vector<std::string> source_lines {};
    };

};