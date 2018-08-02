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

#include <vm/instruction_block.h>
#include "type.h"
#include "identifier.h"
#include "initializer.h"
#include "symbol_element.h"

namespace basecode::compiler {

    identifier::identifier(
            block* parent_scope,
            compiler::symbol_element* name,
            compiler::initializer* initializer) : element(parent_scope, element_type_t::identifier),
                                                  _symbol(name),
                                                  _initializer(initializer) {
    }

    bool identifier::on_emit(
            common::result& r,
            emit_context_t& context) {
        if (_type->element_type() == element_type_t::namespace_type)
            return true;

        auto instruction_block = context.assembler->current_block();

        vm::stack_frame_entry_t* frame_entry = nullptr;
        auto stack_frame = instruction_block->stack_frame();
        if (stack_frame != nullptr)
            frame_entry = stack_frame->find_up(_symbol->name());

        auto var = context.allocate_variable(
            r,
            _symbol->name(),
            _type,
            _usage,
            frame_entry);
        var->read(instruction_block);
        return true;
    }

    compiler::type* identifier::type() {
        return _type;
    }

    bool identifier::inferred_type() const {
        return _inferred_type;
    }

    bool identifier::on_is_constant() const {
        return _symbol->is_constant();
    }

    void identifier::type(compiler::type* t) {
        _type = t;
    }

    void identifier::inferred_type(bool value) {
        _inferred_type = value;
    }

    identifier_usage_t identifier::usage() const {
        return _usage;
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

    compiler::type* identifier::on_infer_type(const compiler::program* program) {
        return _type;
    }

};