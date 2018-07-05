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

#include "program.h"
#include "string_literal.h"

namespace basecode::compiler {

    string_literal::string_literal(
            element* parent,
            const std::string& value) : element(parent, element_type_t::string_literal),
                                        _value(value) {
    }

    bool string_literal::on_as_string(std::string& value) const {
        value = _value;
        return true;
    }

    compiler::type* string_literal::on_infer_type(const compiler::program* program) {
        return program->find_type_up("string");
    }

}
