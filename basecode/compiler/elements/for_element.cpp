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

#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "type.h"
#include "block.h"
#include "intrinsic.h"
#include "for_element.h"
#include "declaration.h"
#include "argument_list.h"
#include "type_reference.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "range_intrinsic.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    for_element::for_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::declaration* induction_decl,
            compiler::element* expression,
            compiler::block* body) : element(module, parent_scope, element_type_t::for_e),
                                     _body(body),
                                     _expression(expression),
                                     _induction_decl(induction_decl) {
    }

    compiler::block* for_element::body() {
        return _body;
    }

    compiler::element* for_element::expression() {
        return _expression;
    }

    compiler::declaration* for_element::induction_decl() {
        return _induction_decl;
    }

    void for_element::on_owned_elements(element_list_t& list) {
        if (_body != nullptr)
            list.emplace_back(_body);

        if (_expression != nullptr)
            list.emplace_back(_expression);

        if (_induction_decl != nullptr)
            list.emplace_back(_induction_decl);
    }

};