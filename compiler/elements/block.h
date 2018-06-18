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

#include "type.h"
#include "element.h"
#include "identifier.h"

namespace basecode::compiler {

    class block : public element {
    public:
        explicit block(block* parent);

        ~block() override;

        type_map_t& types() {
            return _types;
        }

        element_list_t& children();

        identifier_map_t& identifiers() {
            return _identifiers;
        }

    private:
        type_map_t _types {};
        element_list_t _children {};
        identifier_map_t _identifiers {};
    };

};