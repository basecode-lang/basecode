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
#include "block.h"
#include "intrinsic.h"
#include "for_element.h"
#include "declaration.h"
#include "argument_list.h"
#include "type_reference.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "range_intrinsic.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    for_element::for_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::declaration* induction_decl,
            compiler::element* expression,
            compiler::block* body) : element(module, parent_scope, element_type_t::for_e),
                                     _body(body),
                                     _expression(expression),
                                     _induction_decl(induction_decl) {
    }

    compiler::block* for_element::body() {
        return _body;
    }

    compiler::element* for_element::expression() {
        return _expression;
    }

    compiler::declaration* for_element::induction_decl() {
        return _induction_decl;
    }

    bool for_element::on_emit(compiler::session& session) {
        auto& builder = session.builder();
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto begin_label_name = fmt::format("{}_begin", label_name());
        auto body_label_name = fmt::format("{}_body", label_name());
        auto exit_label_name = fmt::format("{}_exit", label_name());

        switch (_expression->element_type()) {
            case element_type_t::intrinsic: {
                auto intrinsic = dynamic_cast<compiler::intrinsic*>(_expression);
                if (intrinsic->name() == "range") {
                    auto begin_label_ref = assembler.make_label_ref(begin_label_name);
                    auto exit_label_ref = assembler.make_label_ref(exit_label_name);

                    assembler.push_control_flow(vm::control_flow_t {
                        .exit_label = exit_label_ref,
                        .continue_label = begin_label_ref
                    });

                    vm::register_t target_reg {
                        .size = vm::op_sizes::byte,
                        .type = vm::register_type_t::integer
                    };
                    assembler.allocate_reg(target_reg);
                    defer({
                        assembler.free_reg(target_reg);
                        assembler.pop_control_flow();
                    });

                    auto range = dynamic_cast<compiler::range_intrinsic*>(intrinsic);

                    block->sub_reg_by_immediate(
                        vm::register_t::sp(),
                        vm::register_t::sp(),
                        8);

                    auto start_arg = range->arguments()->param_by_name("start");
                    auto induction_init = builder.make_binary_operator(
                        parent_scope(),
                        operator_type_t::assignment,
                        _induction_decl->identifier(),
                        start_arg);
                    induction_init->make_non_owning();
                    defer(session.elements().remove(induction_init->id()));
                    induction_init->emit(session);

                    assembler.push_target_register(target_reg);

                    auto dir_arg = range->arguments()->param_by_name("dir");
                    uint64_t dir_value;
                    if (!dir_arg->as_integer(dir_value))
                        return false;

                    auto kind_arg = range->arguments()->param_by_name("kind");
                    uint64_t kind_value;
                    if (!kind_arg->as_integer(kind_value))
                        return false;

                    auto step_op_type = dir_value == 0 ?
                        operator_type_t::add :
                        operator_type_t::subtract;
                    auto cmp_op_type = operator_type_t::less_than;
                    switch (kind_value) {
                        case 0: {
                            switch (dir_value) {
                                case 0:
                                    cmp_op_type = operator_type_t::less_than_or_equal;
                                    break;
                                case 1:
                                    cmp_op_type = operator_type_t::greater_than_or_equal;
                                    break;
                            }
                            break;
                        }
                        case 1: {
                            switch (dir_value) {
                                case 0:
                                    cmp_op_type = operator_type_t::less_than;
                                    break;
                                case 1:
                                    cmp_op_type = operator_type_t::greater_than;
                                    break;
                            }
                            break;
                        }
                    }

                    auto stop_arg = range->arguments()->param_by_name("stop");
                    block->label(assembler.make_label(begin_label_name));
                    auto comparison_op = builder.make_binary_operator(
                        parent_scope(),
                        cmp_op_type,
                        _induction_decl->identifier(),
                        stop_arg);
                    comparison_op->make_non_owning();
                    defer(session.elements().remove(comparison_op->id()));
                    comparison_op->emit(session);
                    block->bz(target_reg, exit_label_ref);

                    assembler.pop_target_register();

                    block->label(assembler.make_label(body_label_name));
                    _body->emit(session);

                    auto step_param = range->arguments()->param_by_name("step");
                    auto induction_step = builder.make_binary_operator(
                        parent_scope(),
                        step_op_type,
                        _induction_decl->identifier(),
                        step_param);
                    auto induction_assign = builder.make_binary_operator(
                        parent_scope(),
                        operator_type_t::assignment,
                        _induction_decl->identifier(),
                        induction_step);
                    induction_step->make_non_owning();
                    induction_assign->make_non_owning();
                    defer({
                        session.elements().remove(induction_assign->id());
                        session.elements().remove(induction_step->id());
                    });
                    induction_assign->emit(session);

                    block->jump_direct(begin_label_ref);

                    block->label(assembler.make_label(exit_label_name));
                    block->add_reg_by_immediate(
                        vm::register_t::sp(),
                        vm::register_t::sp(),
                        8);
                }
                break;
            }
            default: {
                block->comment("XXX: unsupported scenario", 4);
                break;
            }
        }

        return true;
    }

    void for_element::on_owned_elements(element_list_t& list) {
        if (_body != nullptr)
            list.emplace_back(_body);

        if (_expression != nullptr)
            list.emplace_back(_expression);

        if (_induction_decl != nullptr)
            list.emplace_back(_induction_decl);
    }

};