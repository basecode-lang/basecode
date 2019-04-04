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
#include <compiler/scope_manager.h>
#include <compiler/element_builder.h>
#include "identifier.h"
#include "initializer.h"
#include "symbol_element.h"
#include "namespace_element.h"

namespace basecode::compiler {

    namespace_element::namespace_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expr) : element(module, parent_scope, element_type_t::namespace_e),
                                       _expression(expr) {
    }

    bool namespace_element::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.types.emplace_back(
            session
                .scope_manager()
                .find_type(qualified_symbol_t("namespace")));
        return true;
    }

    std::string namespace_element::name() {
        std::string name("unknown");
        switch (parent_element()->element_type()) {
            case element_type_t::initializer: {
                auto parent_init = dynamic_cast<compiler::initializer*>(parent_element());
                auto parent_identifier = dynamic_cast<compiler::identifier*>(parent_init->parent_element());
                name = parent_identifier->symbol()->fully_qualified_name();
                break;
            }
            case element_type_t::identifier: {
                auto parent_identifier = dynamic_cast<compiler::identifier*>(parent_element());
                name = parent_identifier->symbol()->fully_qualified_name();
                break;
            }
            default:
                break;
        }
        return name;
    }

    bool namespace_element::on_is_constant() const {
        return true;
    }

    compiler::element* namespace_element::expression() {
        return _expression;
    }

    void namespace_element::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

}