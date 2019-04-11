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

#pragma once

#include <string>
#include <vector>

namespace basecode::compiler {

    class type_name_builder {
    public:
        type_name_builder() = default;

        void clear();

        [[nodiscard]] std::string format() const;

        type_name_builder& add_part(uint32_t value);

        type_name_builder& add_part(const std::string& value);

    private:
        std::vector<std::string> _parts {};
    };

}

