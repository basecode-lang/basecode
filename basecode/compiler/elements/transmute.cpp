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
            compiler::block* parent_scope,
            compiler::type_reference* type,
            compiler::element* expr) : element(module, parent_scope, element_type_t::transmute),
                                       _expression(expr),
                                       _type_ref(type) {
    }

//    bool transmute::on_emit(
//            compiler::session& session,
//            compiler::emit_context_t& context,
//            compiler::emit_result_t& result) {
//        if (_expression == nullptr)
//            return true;
//
//        infer_type_result_t infer_type_result {};
//        if (!_expression->infer_type(session, infer_type_result)) {
//            // XXX: error
//            return false;
//        }
//
//        if (infer_type_result.inferred_type->number_class() == type_number_class_t::none) {
//            session.error(
//                module(),
//                "C073",
//                fmt::format("cannot transmute from type: {}", infer_type_result.type_name()),
//                _expression->location());
//            return false;
//        } else if (_type_ref->type()->number_class() == type_number_class_t::none) {
//            session.error(
//                module(),
//                "C073",
//                fmt::format("cannot transmute to type: {}", _type_ref->symbol().name),
//                _type_location);
//            return false;
//        }
//
//        auto target_number_class = _type_ref->type()->number_class();
//        auto target_size = _type_ref->type()->size_in_bytes();
//
//        auto& assembler = session.assembler();
//        auto block = assembler.current_block();
//
//        variable_handle_t temp_var;
//        if (!session.variable(_expression, temp_var))
//            return false;
//        temp_var->read();
//
//        block->comment(
//            fmt::format("transmute<{}>", _type_ref->symbol().name),
//            vm::comment_location_t::after_instruction);
//
//        vm::instruction_operand_t target_operand;
//        auto target_type = target_number_class == type_number_class_t::integer ?
//                           vm::register_type_t::integer :
//                           vm::register_type_t::floating_point;
//        if (!vm::instruction_operand_t::allocate(
//                assembler,
//                target_operand,
//                vm::op_size_for_byte_size(target_size),
//                target_type)) {
//            return false;
//        }
//
//        result.operands.emplace_back(target_operand);
//
//        block->move(
//            target_operand,
//            temp_var->emit_result().operands.back(),
//            vm::instruction_operand_t::empty());
//
//        return true;
//    }

    bool transmute::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = _type_ref->type();
        result.reference = _type_ref;
        return true;
    }

    bool transmute::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _expression = fold_result.element;
        return true;
    }

    compiler::element* transmute::expression() {
        return _expression;
    }

    compiler::type_reference* transmute::type() {
        return _type_ref;
    }

    void transmute::expression(compiler::element* value) {
        _expression = value;
    }

    void transmute::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    void transmute::type_location(const common::source_location& loc) {
        _type_location = loc;
    }

};