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
#include "type_reference.h"

namespace basecode::compiler {

    transmute::transmute(
            compiler::module* module,
            block* parent_scope,
            compiler::type_reference* type,
            element* expr) : element(module, parent_scope, element_type_t::cast),
                             _expression(expr),
                             _type_ref(type) {
    }

    element* transmute::expression() {
        return _expression;
    }

    bool transmute::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = _type_ref->type();
        result.reference = _type_ref;
        return true;
    }

    compiler::type_reference* transmute::type() {
        return _type_ref;
    }

    bool transmute::on_emit(compiler::session& session) {
        if (_expression == nullptr)
            return true;

        infer_type_result_t infer_type_result {};
        if (!_expression->infer_type(session, infer_type_result)) {
            // XXX: error
            return false;
        }

        if (infer_type_result.inferred_type->number_class() == type_number_class_t::none) {
            session.error(
                this,
                "C073",
                fmt::format("cannot transmute from type: {}", infer_type_result.type_name()),
                _expression->location());
            return false;
        } else if (_type_ref->type()->number_class() == type_number_class_t::none) {
            session.error(
                this,
                "C073",
                fmt::format("cannot transmute to type: {}", _type_ref->symbol().name),
                _type_location);
            return false;
        }

        auto& assembler = session.assembler();
        auto target_reg = assembler.current_target_register();
        auto block = assembler.current_block();

        variable_handle_t temp_var;
        if (!session.variable(_expression, temp_var))
            return false;
        temp_var->read();

        block->comment(
            fmt::format("transmute<{}>", _type_ref->symbol().name),
            4);
        block->move_reg_to_reg(*target_reg, temp_var->value_reg());

        return true;
    }

    void transmute::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    void transmute::type_location(const common::source_location& loc) {
        _type_location = loc;
    }

};