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

#include "graphviz_formatter.h"

namespace basecode::common {

    std::string graphviz_formatter::escape_chars(const std::string_view& value) {
        std::string buffer;
        for (const auto& c : value) {
            if (c == '\"') {
                buffer += "\\\"";
            } else if (c == '{') {
                buffer += "\\{";
            } else if (c == '}') {
                buffer += "\\}";
            } else if (c == '.') {
                buffer += "\\.";
            } else if (c == '|') {
                buffer += "\\|";
            } else if (c == '<') {
                buffer += "\\<";
            } else if (c == '>') {
                buffer += "\\>";
            } else if (c == '=') {
                buffer += "\\=";
            } else {
                buffer += c;
            }
        }
        return buffer;
    }

}