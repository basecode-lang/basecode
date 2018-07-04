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
#include "expression.h"

namespace basecode::compiler {

    expression::expression(
            element* parent,
            element* root) : element(parent, element_type_t::expression),
                             _root(root) {
    }

    element* expression::root() {
        return _root;
    }

    bool expression::on_is_constant() const {
        if (_root == nullptr)
            return false;
        return _root->is_constant();
    }

    compiler::type* expression::on_infer_type(const compiler::program* program) {
        if (_root == nullptr)
            return nullptr;
        return _root->infer_type(program);
    }

};