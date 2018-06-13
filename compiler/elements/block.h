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

#include "element.h"
#include "element_types.h"

namespace basecode::compiler {

    class block : public element {
    public:
        explicit block(block* parent);

        ~block() override;

        block* parent() const;

        element_list_t& children();

        inline size_t type_count() const {
            return _types.size();
        }

        inline size_t identifier_count() const {
            return _identifiers.size();
        }

        bool remove_type(const std::string& name);

        bool remove_identifier(const std::string& name);

        compiler::type* find_type(const std::string& name);

        compiler::identifier* find_identifier(const std::string& name);

    private:
        type_map_t _types {};
        block* _parent = nullptr;
        element_list_t _children {};
        identifier_map_t _identifiers {};
    };

};