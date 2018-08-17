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
#include "transmute.h"
#include "symbol_element.h"

namespace basecode::compiler {

    transmute::transmute(
            compiler::module* module,
            block* parent_scope,
            compiler::type* type,
            element* expr) : element(module, parent_scope, element_type_t::cast),
                             _expression(expr),
                             _type(type) {
    }

    element* transmute::expression() {
        return _expression;
    }

    compiler::type* transmute::type() {
        return _type;
    }

    bool transmute::on_emit(compiler::session& session) {
        if (_expression == nullptr)
            return true;

        auto source_type = _expression->infer_type(session);
        if (source_type->number_class() == type_number_class_t::none) {
            session.error(
                this,
                "C073",
                fmt::format("cannot transmute from type: {}", source_type->symbol()->name()),
                _expression->location());
            return false;
        } else if (_type->number_class() == type_number_class_t::none) {
            session.error(
                this,
                "C073",
                fmt::format("cannot transmute to type: {}", _type->symbol()->name()),
                _type_location);
            return false;
        }

        auto& assembler = session.assembler();
        auto target_reg = assembler.current_target_register();
        auto instruction_block = assembler.current_block();

        auto temp_reg = register_for(session, _expression);
        if (!temp_reg.valid)
            return false;

        assembler.push_target_register(temp_reg.reg);
        _expression->emit(session);
        assembler.pop_target_register();

        instruction_block->move_reg_to_reg(*target_reg, temp_reg.reg);
        instruction_block->current_entry()->comment(
            fmt::format("transmute<{}>", _type->symbol()->name()),
            session.emit_context().indent);

        return true;
    }

    void transmute::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    void transmute::type_location(const common::source_location& loc) {
        _type_location = loc;
    }

    compiler::type* transmute::on_infer_type(const compiler::session& session) {
        return _type;
    }

};