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

#include <vm/instruction_block.h>
#include "type.h"
#include "identifier.h"
#include "initializer.h"

namespace basecode::compiler {

    identifier::identifier(
            element* parent,
            const std::string& name,
            compiler::initializer* initializer) : element(parent, element_type_t::identifier),
                                                  _name(name),
                                                  _initializer(initializer) {
    }

    bool identifier::on_emit(
            common::result& r,
            vm::assembler& assembler,
            const emit_context_t& context) {
        if (_type->element_type() == element_type_t::namespace_type)
            return true;

        auto instruction_block = assembler.current_block();
        auto target_reg = instruction_block->current_target_register();
        if (target_reg == nullptr)
            return true;

        instruction_block->move_label_to_ireg(target_reg->reg.i, _name);
        return true;
    }

    bool identifier::constant() const {
        return _constant;
    }

    compiler::type* identifier::type() {
        return _type;
    }

    std::string identifier::name() const {
        return _name;
    }

    void identifier::constant(bool value) {
        _constant = value;
    }

    bool identifier::inferred_type() const {
        return _inferred_type;
    }

    void identifier::type(compiler::type* t) {
        _type = t;
    }

    void identifier::inferred_type(bool value) {
        _inferred_type = value;
    }

    compiler::initializer* identifier::initializer() {
        return _initializer;
    }

    bool identifier::on_as_bool(bool& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_bool(value);
    }

    bool identifier::on_as_float(double& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_float(value);
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

};