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
#include "type.h"
#include "identifier.h"
#include "initializer.h"
#include "pointer_type.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "composite_type.h"

namespace basecode::compiler {

    identifier::identifier(
            compiler::module* module,
            block* parent_scope,
            compiler::symbol_element* name,
            compiler::initializer* initializer) : element(module, parent_scope, element_type_t::identifier),
                                                  _symbol(name),
                                                  _initializer(initializer) {
    }

    bool identifier::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        if (!is_constant() || _initializer == nullptr)
            return false;
        return _initializer->fold(session, result);
    }

    bool identifier::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = _type_ref->type();
        result.reference = _type_ref;
        return true;
    }

    compiler::field* identifier::field() {
        return _field;
    }

    bool identifier::inferred_type() const {
        return _inferred_type;
    }

    bool identifier::on_is_constant() const {
        return _symbol->is_constant();
    }

    bool identifier::is_initialized() const {
        auto type = _type_ref->type();
        if (type->is_array_type()
        ||  type->is_pointer_type()) {
            return _initializer != nullptr;
        }
        if (type->is_composite_type()) {
            auto composite_type = dynamic_cast<compiler::composite_type*>(type);
            if (composite_type != nullptr)
                return composite_type->has_at_least_one_initializer();
        }
        return _initializer != nullptr;
    }

    std::string identifier::label_name() const {
        return fmt::format("{}_{}", _symbol->name(), id());
    }

    void identifier::inferred_type(bool value) {
        _inferred_type = value;
    }

    identifier_usage_t identifier::usage() const {
        return _usage;
    }

    void identifier::field(compiler::field* value) {
        _field = value;
    }

    bool identifier::on_as_bool(bool& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_bool(value);
    }

    void identifier::usage(identifier_usage_t value) {
        _usage = value;
    }

    compiler::initializer* identifier::initializer() {
        return _initializer;
    }

    compiler::type_reference* identifier::type_ref() {
        return _type_ref;
    }

    bool identifier::on_as_float(double& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_float(value);
    }

    void identifier::type_ref(compiler::type_reference* t) {
        _type_ref = t;
    }

    compiler::symbol_element* identifier::symbol() const {
        return _symbol;
    }

    bool identifier::on_as_integer(uint64_t& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_integer(value);
    }

    bool identifier::on_as_string(std::string& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_string(value);
    }

    void identifier::on_owned_elements(element_list_t& list) {
        if (_initializer != nullptr)
            list.emplace_back(_initializer);
        if( _symbol != nullptr)
            list.emplace_back(_symbol);
    }

    bool identifier::on_as_rune(common::rune_t& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_rune(value);
    }

    void identifier::initializer(compiler::initializer* value) {
        _initializer = value;
    }

    bool identifier::on_as_identifier(compiler::identifier*& value) const {
        value = const_cast<compiler::identifier*>(this);
        return true;
    }

}