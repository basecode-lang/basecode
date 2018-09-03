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
#include "identifier.h"
#include "initializer.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    identifier::identifier(
            compiler::module* module,
            block* parent_scope,
            compiler::symbol_element* name,
            compiler::initializer* initializer) : element(module, parent_scope, element_type_t::identifier),
                                                  _symbol(name),
                                                  _initializer(initializer) {
    }

    bool identifier::on_infer_type(
            const compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = _type_ref->type();
        result.reference = _type_ref;
        return true;
    }

    bool identifier::on_emit(compiler::session& session) {
        if (_type_ref->type()->element_type() == element_type_t::namespace_type)
            return true;

        auto stack_frame = session.stack_frame();

        vm::stack_frame_entry_t* frame_entry = nullptr;
        if (stack_frame != nullptr)
            frame_entry = stack_frame->find_up(_symbol->name());

        session.emit_context().allocate_variable(
            _symbol->name(),
            _type_ref->type(),
            _usage,
            frame_entry);
        return true;
    }

    bool identifier::inferred_type() const {
        return _inferred_type;
    }

    bool identifier::on_is_constant() const {
        return _symbol->is_constant();
    }

    void identifier::inferred_type(bool value) {
        _inferred_type = value;
    }

    identifier_usage_t identifier::usage() const {
        return _usage;
    }

    compiler::type_reference* identifier::type_ref() {
        return _type_ref;
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

    void identifier::initializer(compiler::initializer* value) {
        _initializer = value;
    }

};