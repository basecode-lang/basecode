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

#include <sstream>
#include <common/string_support.h>
#include "type_name_builder.h"

namespace basecode::compiler {

    void type_name_builder::clear() {
        _parts.clear();
    }

    std::string type_name_builder::format() const {
        std::stringstream stream {};

        stream << "__";

        for (size_t i = 0; i < _parts.size(); i++) {
            const auto& part = _parts[i];
            if (part.empty())
                continue;

            if (i > 0) {
                if (part[0] != '_')
                    stream << "_";
            }

            stream << part;
        }

        stream << "__";

        return stream.str();
    }

    type_name_builder& type_name_builder::add_part(uint32_t value) {
        _parts.emplace_back(std::to_string(value));
        return *this;
    }

    type_name_builder& type_name_builder::add_part(const std::string& value) {
        auto trimmed = value;

        trimmed.erase(
            trimmed.begin(),
            std::find_if(
                trimmed.begin(),
                trimmed.end(),
                [](unsigned char c) { return c != '_'; }));

        trimmed.erase(
            std::find_if(
                trimmed.rbegin(),
                trimmed.rend(),
                [](unsigned char c) { return c != '_'; }).base(),
            trimmed.end());

        _parts.emplace_back(trimmed);

        return *this;
    }

}