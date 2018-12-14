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

#include "import.h"

namespace basecode::compiler {

    import::import(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expr,
            compiler::element* from_expr,
            compiler::module_reference* imported_module) : element(module, parent_scope, element_type_t::import_e),
                                                           _expression(expr),
                                                           _from_expression(from_expr),
                                                           _imported_module(imported_module) {
    }

    compiler::element* import::expression() {
        return _expression;
    }

    compiler::element* import::from_expression() {
        return _from_expression;
    }

    void import::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
        if (_from_expression != nullptr)
            list.emplace_back(_from_expression);
    }

    compiler::module_reference* import::imported_module() {
        return _imported_module;
    }

};