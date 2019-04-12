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

#include "element.h"

namespace basecode::compiler {

    class raw_block : public element {
    public:
        raw_block(
            compiler::module* module,
            block* parent_scope,
            const std::string_view& value);

        std::string_view value() const;

    private:
        std::string_view _value;
    };

}

