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

#include "comment.h"

namespace basecode::compiler {

    comment::comment(
            block* parent_scope,
            comment_type_t type,
            const std::string& value) : element(parent_scope, element_type_t::comment),
                                        _value(value),
                                        _type(type) {
    }

    std::string comment::value() const {
        return _value;
    }

    comment_type_t comment::type() const {
        return _type;
    }

};