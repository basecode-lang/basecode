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

#include "identifier.h"
#include "symbol_element.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    identifier_reference::identifier_reference(
        block* parent_scope,
        const qualified_symbol_t& symbol,
        compiler::identifier* identifier) : element(parent_scope, element_type_t::identifier_reference),
                                            _symbol(symbol),
                                            _identifier(identifier) {
    }

    bool identifier_reference::on_emit(
            common::result& r,
            emit_context_t& context) {
        if (_identifier == nullptr)
            return false;
        return _identifier->emit(r, context);
    }

    bool identifier_reference::resolved() const {
        return _identifier != nullptr;
    }

    bool identifier_reference::on_is_constant() const {
        if (_identifier == nullptr)
            return false;
        return _identifier->is_constant();
    }

    bool identifier_reference::on_as_bool(bool& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_bool(value);
    }

    compiler::identifier* identifier_reference::identifier() {
        return _identifier;
    }

    bool identifier_reference::on_as_float(double& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_float(value);
    }

    const qualified_symbol_t& identifier_reference::symbol() const {
        return _symbol;
    }

    bool identifier_reference::on_as_integer(uint64_t& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_integer(value);
    }

    bool identifier_reference::on_as_string(std::string& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_string(value);
    }

    void identifier_reference::identifier(compiler::identifier* value) {
        _identifier = value;
    }

    compiler::type* identifier_reference::on_infer_type(const compiler::program* program) {
        if (_identifier != nullptr)
            return _identifier->infer_type(program);
        return nullptr;
    }

};