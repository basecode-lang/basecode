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

#include "cast.h"

namespace basecode::compiler {

    cast::cast(
            element* parent,
            compiler::type* type,
            element* expr) : element(parent, element_type_t::cast),
                             _expression(expr),
                             _type(type) {
    }

    bool cast::on_emit(
            common::result& r,
            vm::assembler& assembler,
            const emit_context_t& context) {
        if (_expression == nullptr)
            return true;
        return _expression->emit(r, assembler, context);
    }

    element* cast::expression() {
        return _expression;
    }

    compiler::type* cast::type() {
        return _type;
    }

    compiler::type* cast::on_infer_type(const compiler::program* program) {
        return _type;
    }

};