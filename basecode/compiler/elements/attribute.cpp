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
#include <compiler/element_builder.h>
#include "attribute.h"

namespace basecode::compiler {

    attribute::attribute(
            compiler::module* module,
            compiler::block* parent_scope,
            const std::string_view& name,
            compiler::element* expr) : element(module, parent_scope, element_type_t::attribute),
                                       _name(name),
                                       _expr(expr) {
    }

    compiler::element* attribute::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_attribute(
            new_scope,
            _name,
            _expr != nullptr ? _expr->clone<compiler::element>(session, new_scope) : nullptr);
    }

    std::string_view attribute::name() const {
        return _name;
    }

    compiler::element* attribute::expression() {
        return _expr;
    }

    bool attribute::on_as_bool(bool& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_bool(value);
    }

    bool attribute::on_as_float(double& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_float(value);
    }

    bool attribute::on_as_string(std::string& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_string(value);
    }

    void attribute::on_owned_elements(element_list_t& list) {
        if (_expr != nullptr)
            list.emplace_back(_expr);
    }

    bool attribute::on_as_integer(integer_result_t& result) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_integer(result);
    }

}