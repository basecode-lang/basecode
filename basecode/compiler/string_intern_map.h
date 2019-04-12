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

#include "compiler_types.h"

namespace basecode::compiler {

    using intern_string_map_t = std::unordered_map<std::string_view, common::id_t>;
    using element_to_intern_id_map_t = std::unordered_map<common::id_t, common::id_t>;
    using intern_id_to_view_map_t = std::unordered_map<common::id_t, std::string_view>;

    class string_intern_map {
    public:
        string_intern_map() = default;

        bool id(
            compiler::string_literal* literal,
            common::id_t& intern_id);

        void reset();

        bool string(
            common::id_t id,
            std::string_view& data) const;

        bool element_id_to_intern_id(
            common::id_t element_id,
            common::id_t& intern_id) const;

        intern_string_map_t::const_iterator end() const;

        intern_string_map_t::const_iterator begin() const;

        common::id_t intern(compiler::string_literal* literal);

    private:
        intern_string_map_t _strings {};
        intern_id_to_view_map_t _id_to_view {};
        element_to_intern_id_map_t _element_to_intern_ids {};
    };

}

