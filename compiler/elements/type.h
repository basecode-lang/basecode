// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <string>
#include "element.h"
#include "element_types.h"

namespace basecode::compiler {

    class type : public element {
    public:
        type();

        ~type() override;

        void add_field(
            const std::string& name,
            compiler::type* type,
            compiler::initializer* initializer);

        inline uint64_t min() const {
            return _min;
        }

        inline uint64_t max() const {
            return _max;
        }

        inline std::string name() const {
            return _name;
        }

        inline size_t field_count() const {
            return _fields.size();
        }

        bool remove_field(const std::string& name);

        compiler::field* find_field(const std::string& name);

    private:
        uint64_t _min;
        uint64_t _max;
        std::string _name;
        field_map_t _fields {};
    };

};