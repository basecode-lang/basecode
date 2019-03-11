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

#include <vm/ffi.h>
#include <vm/terp.h>
#include <fmt/format.h>
#include <vm/assembler.h>
#include <vm/basic_block.h>
#include <compiler/session.h>
#include "elements.h"
#include "element_map.h"
#include "scope_manager.h"
#include "element_builder.h"
#include "string_intern_map.h"
#include "byte_code_emitter.h"

namespace basecode::compiler {

    // +-------------+
    // | ...         |
    // | locals      | -offsets
    // | fp          | 0
    // | return addr | +offsets
    // | return slot |
    // | param 1     |
    // | param 2     |
    // +-------------+
    //

    ///////////////////////////////////////////////////////////////////////////

    void temp_count_result_t::update() {
        if (_ints > ints)
            ints = _ints + 1;

        if (_floats > floats)
            floats = _floats + 1;

        _ints = _floats = 0;
    }

    void temp_count_result_t::count(compiler::type* type) {
        switch (type->number_class()) {
            case number_class_t::integer: {
                ++_ints;
                break;
            }
            case number_class_t::floating_point: {
                ++_floats;
                break;
            }
            default: {
                break;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    enum class cast_mode_t : uint8_t {
        noop,
        integer_truncate,
        integer_sign_extend,
        integer_zero_extend,
        float_extend,
        float_truncate,
        float_to_integer,
        integer_to_float,
    };

    ///////////////////////////////////////////////////////////////////////////

    byte_code_emitter::byte_code_emitter(compiler::session& session) : _session(session) {
    }

    ///////////////////////////////////////////////////////////////////////////

    void byte_code_emitter::pop_flow_control() {
        if (_control_flow_stack.empty())
            return;
    }

    flow_control_t* byte_code_emitter::current_flow_control() {
        if (_control_flow_stack.empty())
            return nullptr;
        return &_control_flow_stack.top();
    }

    void byte_code_emitter::push_flow_control(const flow_control_t& control_flow) {
        _control_flow_stack.push(control_flow);
    }

    ///////////////////////////////////////////////////////////////////////////

    void byte_code_emitter::read(
            vm::basic_block* block,
            emit_result_t& result,
            uint8_t number) {
        if (result.operands.empty())
            return;

        const auto& local_ref = result.operands.front();
        if (local_ref.type() != vm::instruction_operand_type_t::named_ref)
            return;

        auto number_class = result.type_result.inferred_type->number_class();
        auto temp_name = temp_local_name(number_class, number);

        auto named_ref = *local_ref.data<vm::assembler_named_ref_t*>();
        if (named_ref->type == vm::assembler_named_ref_type_t::label)
            return;

        if (temp_name == named_ref->name)
            return;

        auto is_pointer_type = result.type_result.inferred_type->is_pointer_type();
        auto is_composite = result.type_result.inferred_type->is_composite_type();
        auto size = is_composite ?
            vm::op_sizes::qword :
            vm::op_size_for_byte_size(result.type_result.inferred_type->size_in_bytes());

        const auto has_offset = result.operands.size() == 2;
        const auto& offset = has_offset ?
                             result.operands[1] :
                             vm::instruction_operand_t();

        if (!is_temp_local(local_ref) || has_offset) {
            vm::instruction_operand_t temp(_session.assembler().make_named_ref(
                vm::assembler_named_ref_type_t::local,
                temp_name,
                size));

            if (is_pointer_type) {
                block->load(temp, local_ref, offset);
                result.operands.push_back(temp);
            } else if (is_composite) {
                if (has_offset) {
                    block->move(temp, local_ref, offset);
                    result.operands.push_back(temp);
                }
            } else {
                block->load(temp, local_ref, offset);
                result.operands.push_back(temp);
            }
        }
    }

    bool byte_code_emitter::emit() {
        identifier_by_section_t vars {};
        vars.sections.insert(std::make_pair(vm::section_t::bss,     element_list_t()));
        vars.sections.insert(std::make_pair(vm::section_t::ro_data, element_list_t()));
        vars.sections.insert(std::make_pair(vm::section_t::data,    element_list_t()));
        vars.sections.insert(std::make_pair(vm::section_t::text,    element_list_t()));

        intern_string_literals();

        auto bootstrap_block = emit_bootstrap_block();
        if (bootstrap_block == nullptr)
            return false;

        if (!emit_type_table())
            return false;

        if (!emit_interned_string_table())
            return false;

        if (!emit_section_tables(vars))
            return false;

        if (!emit_procedure_types(vars))
            return false;

        auto start_block = emit_start_block({bootstrap_block});
        if (start_block == nullptr)
            return false;

        auto initializer_block = emit_initializers({start_block}, vars);
        if (initializer_block == nullptr)
            return false;

        auto last_implicit_block = emit_implicit_blocks({initializer_block}, vars);
        if (last_implicit_block == nullptr)
            return false;

        auto finalizer_block = emit_finalizers({last_implicit_block}, vars);
        if (finalizer_block == nullptr)
            return false;

        auto result = emit_end_block({finalizer_block});

        return result;
    }

    bool byte_code_emitter::emit_block(
            vm::basic_block* basic_block,
            compiler::block* block,
            identifier_list_t& locals,
            temp_local_list_t& temp_locals) {
        const auto excluded_parent_types = element_type_set_t{
            element_type_t::directive,
        };

        if (!block->is_parent_type_one_of(excluded_parent_types)) {
            if (block->has_stack_frame()) {
                if (!begin_stack_frame(basic_block, block, locals, temp_locals))
                    return false;
            } else {
                for (const auto& temp : temp_locals)
                    basic_block->local(temp.type, temp.name, temp.offset);

                for (auto var : locals) {
                    basic_block->move(
                        vm::instruction_operand_t(_session.assembler().make_named_ref(
                            vm::assembler_named_ref_type_t::local,
                            var->label_name(),
                            vm::op_size_for_byte_size(var->type_ref()->type()->size_in_bytes()))),
                        vm::instruction_operand_t(_session.assembler().make_named_ref(
                            vm::assembler_named_ref_type_t::label,
                            var->label_name())));
                }
            }
        }

        const auto& statements = block->statements();
        for (size_t index = 0; index < statements.size(); ++index) {
            auto stmt = statements[index];

            for (auto label : stmt->labels()) {
                emit_result_t label_result {};
                if (!emit_element(&basic_block, label, label_result))
                    return false;
            }

            auto expr = stmt->expression();
            if (expr != nullptr
            &&  expr->element_type() == element_type_t::defer)
                continue;

            auto flow_control = current_flow_control();
            if (flow_control != nullptr) {
                compiler::element* prev = nullptr;
                compiler::element* next = nullptr;

                if (index > 0)
                    prev = statements[index - 1];
                if (index < statements.size() - 1)
                    next = statements[index + 1];

                auto& values_map = flow_control->values;
                values_map[next_element] = next;
                values_map[previous_element] = prev;
            }

            emit_result_t stmt_result;
            if (!emit_element(&basic_block, stmt, stmt_result))
                return false;
        }

        auto working_stack = block->defer_stack();
        while (!working_stack.empty()) {
            auto deferred = working_stack.top();

            emit_result_t defer_result {};
            if (!emit_element(&basic_block, deferred, defer_result))
                return false;
            working_stack.pop();
        }

        if (block->has_stack_frame()) {
            end_stack_frame(basic_block, block, locals);
        }

        reset_temp();

        return true;
    }

    bool byte_code_emitter::emit_element(
            vm::basic_block** basic_block,
            compiler::element* e,
            emit_result_t& result) {
        auto& builder = _session.builder();
        auto& assembler = _session.assembler();

        auto current_block = *basic_block;

        e->infer_type(_session, result.type_result);

        switch (e->element_type()) {
            case element_type_t::cast: {
                // numeric casts
                // ------------------------------------------------------------------------
                // casting between two integers of the same size (s32 -> u32)
                // is a no-op
                //
                // casting from a larger integer to a smaller integer
                // (u32 -> u8) will truncate via move
                //
                // casting from smaller integer to larger integer (u8 -> u32) will:
                //  - zero-extend if the source is unsigned
                //  - sign-extend if the source is signed
                //
                // casting from float to an integer will round the float towards zero
                //
                // casting from an integer to a float will produce the
                // floating point representation of the integer, rounded if necessary
                //
                // casting from f32 to f64 is lossless
                //
                // casting from f64 to f32 will produce the closest possible value, rounded if necessary
                //
                // casting bool to and integer type will yield 1 or 0
                //
                // casting any integer type whose LSB is set will yield true; otherwise, false
                //
                // pointer casts
                // ------------------------------------------------------------------------
                // integer to pointer type:
                //
                auto cast = dynamic_cast<compiler::cast*>(e);
                auto expr = cast->expression();

                auto cast_temp = allocate_temp();
                emit_result_t expr_result {};
                if (!emit_element(basic_block, expr, expr_result))
                    return false;
                read(current_block, expr_result, cast_temp);

                cast_mode_t mode;
                auto type_ref = cast->type();
                auto source_number_class = expr_result.type_result.inferred_type->number_class();
                auto source_size = expr_result.type_result.inferred_type->size_in_bytes();
                auto target_number_class = type_ref->type()->number_class();
                auto target_size = type_ref->type()->size_in_bytes();

                if (source_number_class == number_class_t::none) {
                    _session.error(
                        expr->module(),
                        "C073",
                        fmt::format(
                            "cannot cast from type: {}",
                            expr_result.type_result.type_name()),
                        expr->location());
                    return false;
                } else if (target_number_class == number_class_t::none) {
                    _session.error(
                        expr->module(),
                        "C073",
                        fmt::format(
                            "cannot cast to type: {}",
                            type_ref->symbol().name),
                        cast->type_location());
                    return false;
                }

                if (source_number_class == number_class_t::integer
                &&  target_number_class == number_class_t::integer) {
                    if (source_size == target_size) {
                        mode = cast_mode_t::integer_truncate;
                    } else if (source_size > target_size) {
                        mode = cast_mode_t::integer_truncate;
                    } else {
                        auto source_numeric_type = dynamic_cast<compiler::numeric_type*>(expr_result.type_result.inferred_type);
                        if (source_numeric_type->is_signed()) {
                            mode = cast_mode_t::integer_sign_extend;
                        } else {
                            mode = cast_mode_t::integer_zero_extend;
                        }
                    }
                } else if (source_number_class == number_class_t::floating_point
                       &&  target_number_class == number_class_t::floating_point) {
                    if (source_size == target_size) {
                        mode = cast_mode_t::float_truncate;
                    } else if (source_size > target_size) {
                        mode = cast_mode_t::float_truncate;
                    } else {
                        mode = cast_mode_t::float_extend;
                    }
                } else {
                    if (source_number_class == number_class_t::integer) {
                        mode = cast_mode_t::integer_to_float;
                    } else {
                        mode = cast_mode_t::float_to_integer;
                    }
                }

                current_block->comment(
                    fmt::format(
                        "cast<{}> from type {}",
                        type_ref->name(),
                        expr_result.type_result.type_name()),
                    vm::comment_location_t::after_instruction);

                vm::instruction_operand_t target_operand(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    temp_local_name(target_number_class, cast_temp),
                    vm::op_size_for_byte_size(target_size)));
                result.operands.emplace_back(target_operand);

                switch (mode) {
                    case cast_mode_t::noop: {
                        break;
                    }
                    case cast_mode_t::integer_truncate: {
                        current_block->move(
                            target_operand,
                            expr_result.operands.back());
                        break;
                    }
                    case cast_mode_t::integer_sign_extend: {
                        current_block->moves(
                            target_operand,
                            expr_result.operands.back());
                        break;
                    }
                    case cast_mode_t::integer_zero_extend: {
                        current_block->movez(
                            target_operand,
                            expr_result.operands.back());
                        break;
                    }
                    case cast_mode_t::float_extend:
                    case cast_mode_t::float_truncate:
                    case cast_mode_t::integer_to_float:
                    case cast_mode_t::float_to_integer: {
                        current_block->convert(
                            target_operand,
                            expr_result.operands.back());
                        break;
                    }
                }

                free_temp();
                break;
            }
            case element_type_t::if_e: {
                auto if_e = dynamic_cast<compiler::if_element*>(e);
                auto begin_label_name = fmt::format("{}_entry", if_e->label_name());
                auto true_label_name = fmt::format("{}_true", if_e->label_name());
                auto false_label_name = fmt::format("{}_false", if_e->label_name());
                auto end_label_name = fmt::format("{}_exit", if_e->label_name());

                auto predicate_temp = allocate_temp();
                vm::instruction_operand_t result_operand(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    temp_local_name(number_class_t::integer, predicate_temp)));
                result.operands.emplace_back(result_operand);

                auto predicate_block = _blocks.make();
                assembler.blocks().emplace_back(predicate_block);
                predicate_block->predecessors().emplace_back(current_block);
                current_block->successors().emplace_back(predicate_block);

                predicate_block->label(assembler.make_label(begin_label_name));

                emit_result_t predicate_result {};
                if (!emit_element(&predicate_block, if_e->predicate(), predicate_result))
                    return false;
                read(predicate_block, predicate_result, predicate_temp);

                predicate_block->bz(
                    predicate_result.operands.back(),
                    vm::instruction_operand_t(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        false_label_name)));

                auto true_block = _blocks.make();
                assembler.blocks().emplace_back(true_block);
                true_block->predecessors().emplace_back(predicate_block);

                true_block->label(assembler.make_label(true_label_name));

                emit_result_t true_result {};
                if (!emit_element(&true_block, if_e->true_branch(), true_result))
                    return false;

                if (!true_block->is_current_instruction(vm::op_codes::jmp)
                &&  !true_block->is_current_instruction(vm::op_codes::rts)) {
                    true_block->jump_direct(vm::instruction_operand_t(
                        assembler.make_named_ref(
                            vm::assembler_named_ref_type_t::label,
                            end_label_name)));
                }

                auto false_block = _blocks.make();
                assembler.blocks().emplace_back(false_block);
                false_block->predecessors().emplace_back(predicate_block);

                false_block->label(assembler.make_label(false_label_name));
                auto false_branch = if_e->false_branch();
                if (false_branch != nullptr) {
                    emit_result_t false_result {};
                    if (!emit_element(&false_block, false_branch, false_result))
                        return false;
                } else {
                    false_block->nop();
                }

                predicate_block->add_successors({true_block, false_block});

                auto exit_block = _blocks.make();
                assembler.blocks().emplace_back(exit_block);
                exit_block->add_predecessors({true_block, false_block});
                exit_block->label(assembler.make_label(end_label_name));

                *basic_block = exit_block;

                free_temp();
                break;
            }
            case element_type_t::with: {
                auto with = dynamic_cast<compiler::with*>(e);
                auto body = with->body();
                if (body != nullptr) {
                    emit_result_t body_result {};
                    if (!emit_element(basic_block, body, body_result))
                        return false;
                }
                break;
            }
            case element_type_t::for_e: {
                auto for_e = dynamic_cast<compiler::for_element*>(e);
                auto begin_label_name = fmt::format("{}_begin", for_e->label_name());
                auto body_label_name = fmt::format("{}_body", for_e->label_name());
                auto exit_label_name = fmt::format("{}_exit", for_e->label_name());

                auto for_expr = for_e->expression();
                switch (for_expr->element_type()) {
                    case element_type_t::intrinsic: {
                        auto intrinsic = dynamic_cast<compiler::intrinsic*>(for_expr);
                        if (intrinsic->name() == "range") {
                            auto begin_label_ref = assembler.make_named_ref(
                                vm::assembler_named_ref_type_t::label,
                                begin_label_name);
                            auto exit_label_ref = assembler.make_named_ref(
                                vm::assembler_named_ref_type_t::label,
                                exit_label_name);

                            flow_control_t flow_control {
                                .exit_label = exit_label_ref,
                                .continue_label = begin_label_ref
                            };
                            push_flow_control(flow_control);
                            defer(pop_flow_control());

                            auto range = dynamic_cast<compiler::range_intrinsic*>(intrinsic);

                            auto start_arg = range->arguments()->param_by_name("start");
                            auto induction_init = builder.make_binary_operator(
                                for_e->parent_scope(),
                                operator_type_t::assignment,
                                for_e->induction_decl()->identifier(),
                                start_arg);
                            induction_init->make_non_owning();
                            defer(_session.elements().remove(induction_init->id()));
                            if (!emit_element(basic_block, induction_init, result))
                                return false;

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
                            current_block->label(assembler.make_label(begin_label_name));
                            auto comparison_op = builder.make_binary_operator(
                                for_e->parent_scope(),
                                cmp_op_type,
                                for_e->induction_decl()->identifier(),
                                stop_arg);
                            comparison_op->make_non_owning();
                            defer(_session.elements().remove(comparison_op->id()));

                            emit_result_t cmp_result;
                            if (!emit_element(basic_block, comparison_op, cmp_result))
                                return false;
                            current_block->bz(
                                cmp_result.operands.back(),
                                vm::instruction_operand_t(exit_label_ref));

                            current_block->label(assembler.make_label(body_label_name));
                            if (!emit_element(basic_block, for_e->body(), result))
                                return false;

                            auto step_param = range->arguments()->param_by_name("step");
                            auto induction_step = builder.make_binary_operator(
                                for_e->parent_scope(),
                                step_op_type,
                                for_e->induction_decl()->identifier(),
                                step_param);
                            auto induction_assign = builder.make_binary_operator(
                                for_e->parent_scope(),
                                operator_type_t::assignment,
                                for_e->induction_decl()->identifier(),
                                induction_step);
                            induction_step->make_non_owning();
                            induction_assign->make_non_owning();
                            defer({
                                _session.elements().remove(induction_assign->id());
                                _session.elements().remove(induction_step->id());
                            });
                            if (!emit_element(basic_block, induction_assign, result))
                                return false;

                            current_block->jump_direct(vm::instruction_operand_t(begin_label_ref));

                            current_block->label(assembler.make_label(exit_label_name));
                        }
                        break;
                    }
                    default: {
                        current_block->comment("XXX: unsupported scenario", 4);
                        break;
                    }
                }
                break;
            }
            case element_type_t::label: {
                current_block->blank_line();
                current_block->label(assembler.make_label(e->label_name()));
                break;
            }
            case element_type_t::block: {
                identifier_list_t locals {};

                auto scope_block = dynamic_cast<compiler::block*>(e);

                temp_local_list_t temp_locals {};
                if (!make_temp_locals(scope_block, temp_locals))
                    return false;

                if (!emit_block(current_block, scope_block, locals, temp_locals))
                    return false;
                break;
            }
            case element_type_t::field: {
                auto field = dynamic_cast<compiler::field*>(e);
                auto decl = field->declaration();
                if (decl != nullptr) {
                    emit_result_t decl_result {};
                    if (!emit_element(basic_block, decl, decl_result))
                        return false;
                }
                break;
            }
            case element_type_t::defer: {
                auto defer = dynamic_cast<compiler::defer_element*>(e);
                auto expr = defer->expression();
                if (expr != nullptr) {
                    emit_result_t expr_result {};
                    if (!emit_element(basic_block, expr, expr_result))
                        return false;
                }
                break;
            }
            case element_type_t::module: {
                auto module = dynamic_cast<compiler::module*>(e);
                auto scope = module->scope();
                if (scope != nullptr) {
                    emit_result_t scope_result {};
                    if (!emit_element(basic_block, scope, scope_result))
                        return false;
                }
                break;
            }
            case element_type_t::case_e: {
                auto case_e = dynamic_cast<compiler::case_element*>(e);
                auto true_label_name = fmt::format("{}_true", case_e->label_name());
                auto false_label_name = fmt::format("{}_false", case_e->label_name());

                auto flow_control = current_flow_control();
                if (flow_control == nullptr) {
                    // XXX: error
                    return false;
                }

                vm::basic_block* predicate_block = nullptr;
                flow_control->fallthrough = false;

                auto is_default_case = case_e->expression() == nullptr;

                vm::assembler_named_ref_t* fallthrough_label = nullptr;
                if (!is_default_case) {
                    auto next = boost::any_cast<compiler::element*>(flow_control->values[next_element]);
                    if (next != nullptr
                    &&  next->element_type() == element_type_t::statement) {
                        auto stmt = dynamic_cast<compiler::statement*>(next);
                        if (stmt != nullptr
                        &&  stmt->expression()->element_type() == element_type_t::case_e) {
                            auto next_case = dynamic_cast<compiler::case_element*>(stmt->expression());
                            auto next_true_label_name = fmt::format("{}_true", next_case->label_name());
                            fallthrough_label = assembler.make_named_ref(
                                vm::assembler_named_ref_type_t::label,
                                next_true_label_name);
                        }
                    }

                    auto switch_expr = boost::any_cast<compiler::element*>(flow_control->values[switch_expression]);
                    auto equals_op = builder.make_binary_operator(
                        case_e->parent_scope(),
                        operator_type_t::equals,
                        switch_expr,
                        case_e->expression());
                    equals_op->make_non_owning();
                    defer(_session.elements().remove(equals_op->id()));

                    predicate_block = _blocks.make();
                    assembler.blocks().emplace_back(predicate_block);

                    if (flow_control->predecessor != nullptr) {
                        flow_control->predecessor->successors().emplace_back(predicate_block);
                        predicate_block->predecessors().emplace_back(flow_control->predecessor);
                    } else {
                        predicate_block->predecessors().emplace_back(current_block);
                    }

                    flow_control->predecessor = predicate_block;

                    emit_result_t equals_result {};
                    if (!emit_element(&predicate_block, equals_op, equals_result))
                        return false;

                    predicate_block->bz(
                        equals_result.operands.back(),
                        vm::instruction_operand_t(assembler.make_named_ref(
                            vm::assembler_named_ref_type_t::label,
                            false_label_name)));
                }

                auto true_block = _blocks.make();
                assembler.blocks().emplace_back(true_block);

                true_block->label(assembler.make_label(true_label_name));
                if (!emit_element(&true_block, case_e->scope(), result))
                    return false;

                if (!is_default_case) {
                    if (flow_control->fallthrough) {
                        true_block->jump_direct(vm::instruction_operand_t(fallthrough_label));
                    } else {
                        true_block->jump_direct(vm::instruction_operand_t(flow_control->exit_label));
                    }
                }

                auto exit_block = _blocks.make();
                assembler.blocks().emplace_back(exit_block);
                exit_block->label(assembler.make_label(false_label_name));

                if (predicate_block != nullptr) {
                    true_block->add_predecessors({predicate_block});
                    exit_block->add_predecessors({predicate_block});
                    predicate_block->add_successors({true_block, exit_block});
                } else {
                    true_block->add_successors({exit_block});
                    true_block->add_predecessors({current_block});
                }

                *basic_block = exit_block;
                break;
            }
            case element_type_t::break_e: {
                auto break_e = dynamic_cast<compiler::break_element*>(e);
                vm::assembler_named_ref_t* label_ref = nullptr;

                std::string label_name;
                if (break_e->label() != nullptr) {
                    label_name = break_e->label()->label_name();
                    label_ref = assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        label_name);
                } else {
                    auto flow_control = current_flow_control();
                    if (flow_control == nullptr
                    ||  flow_control->exit_label == nullptr) {
                        _session.error(
                            break_e->module(),
                            "P081",
                            "no valid exit label on stack.",
                            break_e->location());
                        return false;
                    }
                    label_ref = flow_control->exit_label;
                    label_name = label_ref->name;
                }

                current_block->comment(
                    fmt::format("break: {}", label_name),
                    vm::comment_location_t::after_instruction);
                current_block->jump_direct(vm::instruction_operand_t(label_ref));
                break;
            }
            case element_type_t::while_e: {
                auto while_e = dynamic_cast<compiler::while_element*>(e);
                auto begin_label_name = fmt::format("{}_begin", while_e->label_name());
                auto body_label_name = fmt::format("{}_body", while_e->label_name());
                auto exit_label_name = fmt::format("{}_exit", while_e->label_name());
                auto end_label_name = fmt::format("{}_end", while_e->label_name());

                auto begin_label_ref = assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    begin_label_name);
                auto exit_label_ref = assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    exit_label_name);

                push_flow_control(flow_control_t{
                    .exit_label = exit_label_ref,
                    .continue_label = begin_label_ref
                });
                defer(pop_flow_control());

                current_block->label(assembler.make_label(begin_label_name));

                emit_result_t predicate_result {};
                if (!emit_element(basic_block, while_e->predicate(), predicate_result))
                    return false;

                current_block->bz(
                    predicate_result.operands.back(),
                    vm::instruction_operand_t(exit_label_ref));

                current_block->label(assembler.make_label(body_label_name));
                if (!emit_element(basic_block, while_e->body(), result))
                    return false;

                current_block->jump_direct(vm::instruction_operand_t(begin_label_ref));

                current_block->label(assembler.make_label(exit_label_name));
                current_block->nop();
                current_block->label(assembler.make_label(end_label_name));
                break;
            }
            case element_type_t::return_e: {
                auto return_e = dynamic_cast<compiler::return_element*>(e);
                auto return_type_field = return_e->field();
                if (return_type_field != nullptr) {
                    auto temp = allocate_temp();
                    emit_result_t expr_result {};
                    if (!emit_element(basic_block, return_e->expressions().front(), expr_result))
                        return false;
                    read(current_block, expr_result, temp);

                    current_block->store(
                        vm::instruction_operand_t(assembler.make_named_ref(
                            vm::assembler_named_ref_type_t::local,
                            return_type_field->declaration()->identifier()->label_name())),
                        expr_result.operands.back());
                    free_temp();
                }

                current_block->move(
                    vm::instruction_operand_t::sp(),
                    vm::instruction_operand_t::fp());
                current_block->pop(vm::instruction_operand_t::fp());
                current_block->rts();
                break;
            }
            case element_type_t::switch_e: {
                auto switch_e = dynamic_cast<compiler::switch_element*>(e);
                auto begin_label_name = fmt::format("{}_entry", switch_e->label_name());
                auto exit_label_name = fmt::format("{}_exit", switch_e->label_name());

                auto exit_label_ref = assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    exit_label_name);

                flow_control_t flow_control {
                    .exit_label = exit_label_ref,
                };
                flow_control.values.insert(std::make_pair(
                    switch_expression,
                    switch_e->expression()));
                push_flow_control(flow_control);
                defer(pop_flow_control());

                auto entry_block = _blocks.make();
                assembler.blocks().emplace_back(entry_block);
                current_block->successors().emplace_back(entry_block);
                entry_block->predecessors().emplace_back(current_block);

                entry_block->label(assembler.make_label(begin_label_name));
                if (!emit_element(&entry_block, switch_e->scope(), result))
                    return false;

                auto exit_block = _blocks.make();
                assembler.blocks().emplace_back(exit_block);
                exit_block->predecessors().emplace_back(entry_block);

                exit_block->label(assembler.make_label(exit_label_name));
                exit_block->nop();

                *basic_block = exit_block;

                break;
            }
            case element_type_t::intrinsic: {
                auto intrinsic = dynamic_cast<compiler::intrinsic*>(e);
                const auto& name = intrinsic->name();

                auto args = intrinsic->arguments()->elements();
                if (name == "address_of") {
                    auto arg = args[0];

                    emit_result_t arg_result {};
                    if (!emit_element(basic_block, arg, arg_result))
                        return false;

                    const auto has_offset = arg_result.operands.size() == 2;
                    const auto& temp = arg_result.operands.front();
                    const auto& offset = has_offset ?
                                         arg_result.operands[1] :
                                         vm::instruction_operand_t();
                    if (!offset.is_empty())
                        current_block->move(temp, temp, offset);
                    result.skip_read = true;
                    result.operands = {temp};
                } else if (name == "alloc") {
                    auto arg = args[0];

                    auto arg_temp = allocate_temp();
                    emit_result_t arg_result {};
                    if (!emit_element(basic_block, arg, arg_result))
                        return false;
                    read(current_block, arg_result, arg_temp);

                    auto result_temp = allocate_temp();
                    vm::instruction_operand_t result_operand(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::local,
                        temp_local_name(result.type_result.inferred_type->number_class(), result_temp)));
                    result.operands.emplace_back(result_operand);

                    current_block->alloc(
                        vm::op_sizes::byte,
                        result_operand,
                        arg_result.operands.back());

                    free_temp();
                    free_temp();
                } else if (name == "free") {
                    auto arg = args[0];

                    auto arg_temp = allocate_temp();
                    emit_result_t arg_result {};
                    if (!emit_element(basic_block, arg, arg_result))
                        return false;
                    read(current_block, arg_result, arg_temp);

                    current_block->free(arg_result.operands.back());

                    free_temp();
                } else if (name == "fill") {
                    auto dest_arg = args[0];
                    auto value_arg = args[1];
                    auto length_arg = args[2];

                    auto dest_temp = allocate_temp();
                    emit_result_t dest_arg_result {};
                    if (!emit_element(basic_block, dest_arg, dest_arg_result))
                        return false;
                    read(current_block, dest_arg_result, dest_temp);

                    auto value_temp = allocate_temp();
                    emit_result_t value_arg_result {};
                    if (!emit_element(basic_block, value_arg, value_arg_result))
                        return false;
                    read(current_block, value_arg_result, value_temp);

                    auto len_temp = allocate_temp();
                    emit_result_t length_arg_result {};
                    if (!emit_element(basic_block, length_arg, length_arg_result))
                        return false;
                    read(current_block, length_arg_result, len_temp);

                    current_block->fill(
                        vm::op_sizes::byte,
                        dest_arg_result.operands.back(),
                        value_arg_result.operands.back(),
                        length_arg_result.operands.back());

                    free_temp();
                    free_temp();
                    free_temp();
                } else if (name == "copy") {
                    auto dest_arg = args[0];
                    auto src_arg = args[1];
                    auto size_arg = args[2];

                    auto dest_temp = allocate_temp();
                    emit_result_t dest_arg_result {};
                    if (!emit_element(basic_block, dest_arg, dest_arg_result))
                        return false;
                    read(current_block, dest_arg_result, dest_temp);

                    auto src_temp = allocate_temp();
                    emit_result_t src_arg_result {};
                    if (!emit_element(basic_block, src_arg, src_arg_result))
                        return false;
                    read(current_block, src_arg_result, src_temp);

                    auto size_temp = allocate_temp();
                    emit_result_t size_arg_result {};
                    if (!emit_element(basic_block, size_arg, size_arg_result))
                        return false;
                    read(current_block, size_arg_result, size_temp);

                    current_block->copy(
                        vm::op_sizes::byte,
                        dest_arg_result.operands.back(),
                        src_arg_result.operands.back(),
                        size_arg_result.operands.back());

                    free_temp();
                    free_temp();
                    free_temp();
                }
                break;
            }
            case element_type_t::directive: {
                auto directive = dynamic_cast<compiler::directive*>(e);
                const std::string& name = directive->name();
                if (name == "assembly") {
                    auto assembly_directive = dynamic_cast<compiler::assembly_directive*>(directive);
                    auto expr = assembly_directive->expression();
                    auto raw_block = dynamic_cast<compiler::raw_block*>(expr);

                    common::source_file source_file;
                    if (!source_file.load(_session.result(), raw_block->value() + "\n"))
                        return false;

                    auto success = assembler.assemble_from_source(
                        _session.result(),
                        source_file,
                        current_block,
                        expr->parent_scope());
                    if (!success)
                        return false;
                } else if (name == "if") {
                    auto if_directive = dynamic_cast<compiler::if_directive*>(directive);
                    auto true_expr = if_directive->true_body();
                    if (true_expr != nullptr) {
                        current_block->comment(
                            "directive: if/elif/else",
                            vm::comment_location_t::after_instruction);
                        emit_result_t if_result {};
                        if (!emit_element(basic_block, true_expr, if_result))
                            return false;
                    }
                } else if (name == "run") {
                    auto run_directive = dynamic_cast<compiler::run_directive*>(directive);
                    current_block->comment(
                        "directive: run",
                        vm::comment_location_t::after_instruction);
                    current_block->meta_begin();
                    emit_result_t run_result {};
                    if (!emit_element(basic_block, run_directive->expression(), run_result))
                        return false;
                    current_block->meta_end();
                }
                break;
            }
            case element_type_t::statement: {
                auto statement = dynamic_cast<compiler::statement*>(e);
                auto expr = statement->expression();
                if (expr != nullptr) {
                    emit_result_t expr_result {};
                    if (!emit_element(basic_block, expr, expr_result))
                        return false;
                }
                break;
            }
            case element_type_t::proc_call: {
                auto proc_call = dynamic_cast<compiler::procedure_call*>(e);
                auto procedure_type = proc_call->procedure_type();
                auto label = proc_call->identifier()->label_name();
                auto is_foreign = procedure_type->is_foreign();

                size_t target_size = 8;
                std::string return_temp_name {};
                compiler::type* return_type = nullptr;

                auto return_type_field = procedure_type->return_type();
                if (return_type_field != nullptr) {
                    return_type = return_type_field->identifier()->type_ref()->type();
                    if (return_type != nullptr) {
                        target_size = return_type->size_in_bytes();
                        return_temp_name = temp_local_name(
                            return_type->number_class(),
                            allocate_temp());
                    }
                }

                if (!is_foreign)
                    current_block->push_locals(assembler, return_temp_name);

                auto arg_list = proc_call->arguments();
                if (arg_list != nullptr) {
                    emit_result_t arg_list_result {};
                    if (!emit_element(basic_block, arg_list, arg_list_result))
                        return false;
                }

                if (is_foreign) {
                    auto& ffi = _session.ffi();

                    auto func = ffi.find_function(procedure_type->foreign_address());
                    if (func == nullptr) {
                        _session.error(
                            proc_call->module(),
                            "X000",
                            fmt::format(
                                "unable to find foreign function by address: {}",
                                procedure_type->foreign_address()),
                            proc_call->location());
                        return false;
                    }

                    current_block->comment(
                        fmt::format("call: {}", label),
                        vm::comment_location_t::after_instruction);

                    vm::instruction_operand_t address_operand(procedure_type->foreign_address());

                    if (func->is_variadic()) {
                        vm::function_value_list_t args {};
                        if (!arg_list->as_ffi_arguments(_session, args))
                            return false;

                        auto signature_id = common::id_pool::instance()->allocate();
                        func->call_site_arguments.insert(std::make_pair(signature_id, args));

                        current_block->call_foreign(
                            address_operand,
                            vm::instruction_operand_t(
                                static_cast<uint64_t>(signature_id),
                                vm::op_sizes::dword));
                    } else {
                        current_block->call_foreign(address_operand);
                    }
                } else {
                    if (return_type != nullptr) {
                        current_block->comment(
                            "return slot",
                            vm::comment_location_t::after_instruction);
                        current_block->sub(
                            vm::instruction_operand_t::sp(),
                            vm::instruction_operand_t::sp(),
                            vm::instruction_operand_t(static_cast<uint64_t>(8), vm::op_sizes::byte));
                    }

                    current_block->comment(
                        fmt::format("call: {}", label),
                        vm::comment_location_t::after_instruction);
                    current_block->call(vm::instruction_operand_t(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        label)));
                }

                if (!return_temp_name.empty()) {
                    vm::instruction_operand_t result_operand(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::local,
                        return_temp_name,
                        vm::op_size_for_byte_size(target_size)));
                    result.operands.emplace_back(result_operand);
                    current_block->pop(result_operand);
//                    free_temp();
                }

                if (arg_list->allocated_size() > 0) {
                    current_block->comment(
                        "free stack space",
                        vm::comment_location_t::after_instruction);
                    current_block->add(
                        vm::instruction_operand_t::sp(),
                        vm::instruction_operand_t::sp(),
                        vm::instruction_operand_t(arg_list->allocated_size(), vm::op_sizes::word));
                }

                if (!is_foreign)
                    current_block->pop_locals(assembler, return_temp_name);

                break;
            }
            case element_type_t::transmute: {
                auto transmute = dynamic_cast<compiler::transmute*>(e);
                auto expr = transmute->expression();
                auto type_ref = transmute->type();

                auto temp = allocate_temp();
                emit_result_t expr_result {};
                if (!emit_element(basic_block, expr, expr_result))
                    return false;
                read(current_block, expr_result, temp);

                if (expr_result.type_result.inferred_type->number_class() == number_class_t::none) {
                    _session.error(
                        expr->module(),
                        "C073",
                        fmt::format(
                            "cannot transmute from type: {}",
                            expr_result.type_result.type_name()),
                        expr->location());
                    return false;
                } else if (type_ref->type()->number_class() == number_class_t::none) {
                    _session.error(
                        transmute->module(),
                        "C073",
                        fmt::format(
                            "cannot transmute to type: {}",
                            type_ref->symbol().name),
                        transmute->type_location());
                    return false;
                }

                auto target_number_class = type_ref->type()->number_class();
                auto target_size = type_ref->type()->size_in_bytes();

                current_block->comment(
                    fmt::format("transmute<{}>", type_ref->symbol().name),
                    vm::comment_location_t::after_instruction);

                vm::instruction_operand_t target_operand(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    temp_local_name(target_number_class, temp),
                    vm::op_size_for_byte_size(target_size)));
                result.operands.emplace_back(target_operand);

                current_block->move(
                    target_operand,
                    expr_result.operands.back(),
                    vm::instruction_operand_t::empty());

                free_temp();
                break;
            }
            case element_type_t::continue_e: {
                auto continue_e = dynamic_cast<compiler::continue_element*>(e);
                vm::assembler_named_ref_t* label_ref = nullptr;

                std::string label_name;
                if (continue_e->label() != nullptr) {
                    label_name = continue_e->label()->label_name();
                    label_ref = assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        label_name);
                } else {
                    auto flow_control = current_flow_control();
                    if (flow_control == nullptr
                    ||  flow_control->continue_label == nullptr) {
                        _session.error(
                            continue_e->module(),
                            "P081",
                            "no valid continue label on stack.",
                            continue_e->location());
                        return false;
                    }
                    label_ref = flow_control->continue_label;
                    label_name = label_ref->name;
                }

                current_block->comment(
                    fmt::format("continue: {}", label_name),
                    vm::comment_location_t::after_instruction);
                current_block->jump_direct(vm::instruction_operand_t(label_ref));
                break;
            }
            case element_type_t::identifier: {
                auto var = dynamic_cast<compiler::identifier*>(e);
                result.operands.emplace_back(_session.assembler().make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    var->label_name(),
                    vm::op_size_for_byte_size(result.type_result.inferred_type->size_in_bytes())));
                break;
            }
            case element_type_t::expression: {
                auto expr = dynamic_cast<compiler::expression*>(e);
                auto root = expr->root();
                if (root != nullptr)
                    return emit_element(basic_block, root, result);
                break;
            }
            case element_type_t::assignment: {
                auto assignment = dynamic_cast<compiler::assignment*>(e);
                for (auto expr : assignment->expressions()) {
                    emit_result_t expr_result {};
                    if (!emit_element(basic_block, expr, expr_result))
                        return false;
                }
                break;
            }
            case element_type_t::declaration: {
                auto decl = dynamic_cast<compiler::declaration*>(e);
                auto assignment = decl->assignment();
                if (assignment != nullptr) {
                    emit_result_t assignment_result {};
                    if (!emit_element(basic_block, assignment, assignment_result))
                        return false;
                }
                break;
            }
            case element_type_t::namespace_e: {
                auto ns = dynamic_cast<compiler::namespace_element*>(e);
                auto expr = ns->expression();
                if (expr != nullptr) {
                    emit_result_t expr_result {};
                    if (!emit_element(basic_block, expr, expr_result))
                        return false;
                }
                break;
            }
            case element_type_t::initializer: {
                auto init = dynamic_cast<compiler::initializer*>(e);
                auto expr = init->expression();
                if (expr != nullptr)
                    return emit_element(basic_block, expr, result);
                break;
            }
            case element_type_t::fallthrough: {
                auto flow_control = current_flow_control();
                if (flow_control != nullptr)
                    flow_control->fallthrough = true;
                else {
                    _session.error(
                        e->module(),
                        "X000",
                        "fallthrough is only valid within a case.",
                        e->location());
                    return false;
                }
                break;
            }
            case element_type_t::nil_literal: {
                result.operands.emplace_back(vm::instruction_operand_t(
                    static_cast<uint64_t>(0),
                    vm::op_sizes::qword));
                break;
            }
            case element_type_t::type_literal: {
                break;
            }
            case element_type_t::float_literal: {
                auto float_literal = dynamic_cast<compiler::float_literal*>(e);
                auto value = float_literal->value();
                auto is_float = numeric_type::narrow_to_value(value) == "f32";
                if (is_float) {
                    auto temp_value = static_cast<float>(value);
                    result.operands.emplace_back(vm::instruction_operand_t(temp_value));
                } else {
                    result.operands.emplace_back(vm::instruction_operand_t(value));
                }
                break;
            }
            case element_type_t::string_literal: {
                result.operands.emplace_back(vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    interned_string_data_label(e->id()))));
                break;
            }
            case element_type_t::boolean_literal: {
                auto bool_literal = dynamic_cast<compiler::boolean_literal*>(e);
                result.operands.emplace_back(vm::instruction_operand_t(
                    static_cast<uint64_t>(bool_literal->value() ? 1 : 0),
                    vm::op_sizes::byte));
                break;
            }
            case element_type_t::integer_literal: {
                auto integer_literal = dynamic_cast<compiler::integer_literal*>(e);
                auto size = result.type_result.inferred_type->size_in_bytes();
                result.operands.emplace_back(vm::instruction_operand_t(
                    integer_literal->value(),
                    vm::op_size_for_byte_size(size)));
                break;
            }
            case element_type_t::character_literal: {
                auto char_literal = dynamic_cast<compiler::character_literal*>(e);
                result.operands.emplace_back(vm::instruction_operand_t(
                    static_cast<int64_t>(char_literal->rune()),
                    vm::op_sizes::dword));
                break;
            }
            case element_type_t::argument_list: {
                auto arg_list = dynamic_cast<compiler::argument_list*>(e);
                if (!emit_arguments(current_block, arg_list, arg_list->elements()))
                    return false;
                break;
            }
            case element_type_t::assembly_label: {
                auto label = dynamic_cast<compiler::assembly_label*>(e);
                auto name = label->reference()->identifier()->label_name();
                if (assembler.has_local(name)) {
                    result.operands.emplace_back(_session.assembler().make_named_ref(
                        vm::assembler_named_ref_type_t::local,
                        name));
                } else {
                    result.operands.emplace_back(_session.assembler().make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        name));
                }
                break;
            }
            case element_type_t::unary_operator: {
                auto unary_op = dynamic_cast<compiler::unary_operator*>(e);
                auto op_type = unary_op->operator_type();

                auto rhs_temp = allocate_temp();
                emit_result_t rhs_emit_result {};
                if (!emit_element(basic_block, unary_op->rhs(), rhs_emit_result))
                    return false;
                read(current_block, rhs_emit_result, rhs_temp);

                auto is_composite_type = rhs_emit_result.type_result.inferred_type->is_composite_type();
                auto size = vm::op_size_for_byte_size(result.type_result.inferred_type->size_in_bytes());
                if (op_type == operator_type_t::pointer_dereference
                &&  !is_composite_type) {
                    auto pointer_type = dynamic_cast<compiler::pointer_type*>(result.type_result.inferred_type);
                    size = vm::op_size_for_byte_size(pointer_type->base_type_ref()->type()->size_in_bytes());
                }

                auto result_temp = allocate_temp();
                vm::instruction_operand_t result_operand(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    temp_local_name(result.type_result.inferred_type->number_class(), result_temp),
                    size));

                switch (op_type) {
                    case operator_type_t::negate: {
                        current_block->comment("unary_op: negate", vm::comment_location_t::after_instruction);
                        current_block->neg(
                            result_operand,
                            rhs_emit_result.operands.back());
                        result.operands.emplace_back(result_operand);
                        break;
                    }
                    case operator_type_t::binary_not: {
                        current_block->comment("unary_op: binary not", vm::comment_location_t::after_instruction);
                        current_block->not_op(
                            result_operand,
                            rhs_emit_result.operands.back());
                        result.operands.emplace_back(result_operand);
                        break;
                    }
                    case operator_type_t::logical_not: {
                        current_block->comment("unary_op: logical not", vm::comment_location_t::after_instruction);
                        current_block->cmp(
                            result_operand.size(),
                            rhs_emit_result.operands.back(),
                            vm::instruction_operand_t(static_cast<uint64_t>(1), vm::op_sizes::byte));
                        current_block->setnz(result_operand);
                        result.operands.emplace_back(result_operand);
                        break;
                    }
                    case operator_type_t::pointer_dereference: {
                        if (!is_composite_type) {
                            current_block->load(
                                result_operand,
                                rhs_emit_result.operands.back());
                            result.operands.push_back(result_operand);
                        } else {
                            result.operands.push_back(rhs_emit_result.operands.back());
                        }
                        break;
                    }
                    default:
                        break;
                }

                free_temp();
                free_temp();
                break;
            }
            case element_type_t::binary_operator: {
                auto binary_op = dynamic_cast<compiler::binary_operator*>(e);

                switch (binary_op->operator_type()) {
                    case operator_type_t::add:
                    case operator_type_t::modulo:
                    case operator_type_t::divide:
                    case operator_type_t::subtract:
                    case operator_type_t::multiply:
                    case operator_type_t::exponent:
                    case operator_type_t::binary_or:
                    case operator_type_t::shift_left:
                    case operator_type_t::binary_and:
                    case operator_type_t::binary_xor:
                    case operator_type_t::shift_right:
                    case operator_type_t::rotate_left:
                    case operator_type_t::rotate_right: {
                        if (!emit_arithmetic_operator(current_block, binary_op, result))
                            return false;
                        break;
                    }
                    case operator_type_t::equals:
                    case operator_type_t::less_than:
                    case operator_type_t::not_equals:
                    case operator_type_t::logical_or:
                    case operator_type_t::logical_and:
                    case operator_type_t::greater_than:
                    case operator_type_t::less_than_or_equal:
                    case operator_type_t::greater_than_or_equal: {
                        if (!emit_relational_operator(current_block, binary_op, result))
                            return false;
                        break;
                    }
                    case operator_type_t::subscript: {
                        current_block->comment("XXX: implement subscript operator", 4);
                        current_block->nop();
                        break;
                    }
                    case operator_type_t::member_access: {
                        if (result.operands.size() < 2) {
                            result.operands.resize(2);
                        }

                        emit_result_t lhs_result {};
                        if (!emit_element(basic_block, binary_op->lhs(), lhs_result))
                            return false;

                        result.operands[0] = lhs_result.operands.front();

                        int64_t offset = 0;
                        if (lhs_result.operands.size() == 2) {
                            auto& offset_operand = lhs_result.operands.back();
                            if (!offset_operand.is_empty())
                                offset = *offset_operand.data<int64_t>();
                        }

                        auto type = lhs_result.type_result.inferred_type;
                        if (lhs_result.type_result.inferred_type->is_pointer_type()) {
                            auto pointer_type = dynamic_cast<compiler::pointer_type*>(lhs_result.type_result.inferred_type);
                            type = pointer_type->base_type_ref()->type();
                        }

                        auto composite_type = dynamic_cast<compiler::composite_type*>(type);
                        if (composite_type != nullptr) {
                            auto rhs_ref = dynamic_cast<compiler::identifier_reference*>(binary_op->rhs());
                            auto field = composite_type->fields().find_by_name(rhs_ref->symbol().name);
                            if (field != nullptr) {
                                offset += field->start_offset();
                            }
                        }

                        result.operands[1] = vm::instruction_operand_t::offset(offset, vm::op_sizes::word);
                        break;
                    }
                    case operator_type_t::assignment: {
                        auto rhs_temp = allocate_temp();
                        emit_result_t rhs_result {};
                        if (!emit_element(basic_block, binary_op->rhs(), rhs_result))
                            return false;
                        read(current_block, rhs_result, rhs_temp);

                        emit_result_t lhs_result {};
                        if (!emit_element(basic_block, binary_op->lhs(), lhs_result))
                            return false;

                        auto copy_required = false;
                        auto lhs_is_composite = lhs_result.type_result.inferred_type->is_composite_type();
                        auto rhs_is_composite = rhs_result.type_result.inferred_type->is_composite_type();

                        if (!lhs_result.type_result.inferred_type->is_pointer_type()) {
                            if (lhs_is_composite && !rhs_is_composite) {
                                _session.error(
                                    binary_op->module(),
                                    "X000",
                                    "cannot assign scalar to composite type.",
                                    binary_op->rhs()->location());
                                return false;
                            }

                            if (!lhs_is_composite && rhs_is_composite) {
                                _session.error(
                                    binary_op->module(),
                                    "X000",
                                    "cannot assign composite type to scalar.",
                                    binary_op->rhs()->location());
                                return false;
                            }

                            copy_required = lhs_is_composite && rhs_is_composite;
                        }

                        const auto has_offset = lhs_result.operands.size() == 2;
                        if (copy_required) {
                            const auto size = static_cast<uint64_t>(rhs_result.type_result.inferred_type->size_in_bytes());
                            if (has_offset) {
                                auto copy_temp = allocate_temp();
                                vm::instruction_operand_t temp_target(assembler.make_named_ref(
                                    vm::assembler_named_ref_type_t::local,
                                    temp_local_name(number_class_t::integer, copy_temp),
                                    vm::op_sizes::qword));
                                current_block->move(
                                    temp_target,
                                    lhs_result.operands.front(),
                                    lhs_result.operands.back());
                                current_block->copy(
                                    vm::op_sizes::byte,
                                    temp_target,
                                    rhs_result.operands.back(),
                                    vm::instruction_operand_t(size));
                                free_temp();
                            } else {
                                current_block->copy(
                                    vm::op_sizes::byte,
                                    lhs_result.operands.back(),
                                    rhs_result.operands.back(),
                                    vm::instruction_operand_t(size));
                            }
                        } else {
                            if (has_offset) {
                                current_block->store(
                                    lhs_result.operands[0],
                                    rhs_result.operands.back(),
                                    lhs_result.operands[1]);
                            } else {
                                current_block->store(
                                    lhs_result.operands.back(),
                                    rhs_result.operands.back());
                            }
                        }

                        free_temp();
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case element_type_t::symbol:
            case element_type_t::element:
            case element_type_t::comment:
            case element_type_t::program:
            case element_type_t::import_e:
            case element_type_t::rune_type:
            case element_type_t::proc_type:
            case element_type_t::bool_type:
            case element_type_t::attribute:
            case element_type_t::raw_block:
            case element_type_t::tuple_type:
            case element_type_t::array_type:
            case element_type_t::module_type:
            case element_type_t::family_type:
            case element_type_t::unknown_type:
            case element_type_t::numeric_type:
            case element_type_t::pointer_type:
            case element_type_t::generic_type:
            case element_type_t::argument_pair:
            case element_type_t::proc_instance:
            case element_type_t::namespace_type:
            case element_type_t::composite_type:
            case element_type_t::type_reference:
            case element_type_t::spread_operator:
            case element_type_t::label_reference:
            case element_type_t::module_reference:
            case element_type_t::unknown_identifier:
            case element_type_t::uninitialized_literal: {
                break;
            }
            case element_type_t::identifier_reference: {
                auto var_ref = dynamic_cast<compiler::identifier_reference*>(e);
                auto identifier = var_ref->identifier();
                if (identifier != nullptr) {
                    if (!emit_element(basic_block, identifier, result))
                        return false;
                }
                break;
            }
            case element_type_t::assembly_literal_label: {
                auto label = dynamic_cast<compiler::assembly_literal_label*>(e);
                result.operands.emplace_back(vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    label->name())));
                break;
            }
        }
        return true;
    }

    bool byte_code_emitter::emit_type_info(
            vm::basic_block* block,
            compiler::type* type) {
        if (type == nullptr)
            return false;

        if (type->element_type() == element_type_t::generic_type
        ||  type->element_type() == element_type_t::unknown_type) {
            return true;
        }

        auto& assembler = _session.assembler();

        auto type_name = type->name();
        auto type_name_len = static_cast<uint32_t>(type_name.length());
        auto label_name = type::make_info_label_name(type);

        block->blank_line();
        block->comment(fmt::format("type: {}", type_name), 0);
        block->label(assembler.make_label(label_name));

        block->dwords({type_name_len});
        block->dwords({type_name_len});
        block->qwords({assembler.make_named_ref(
            vm::assembler_named_ref_type_t::label,
            type::make_literal_data_label_name(type))});

        return true;
    }

    bool byte_code_emitter::count_temps(
            compiler::element* e,
            temp_count_result_t& result) {
        switch (e->element_type()) {
            case element_type_t::if_e: {
                auto if_e = dynamic_cast<compiler::if_element*>(e);
                if (!count_temps(if_e->predicate(), result))
                    return false;
                auto false_branch = if_e->false_branch();
                if (false_branch != nullptr
                &&  false_branch->element_type() == element_type_t::if_e) {
                    if (!count_temps(false_branch, result))
                        return false;
                }
                break;
            }
            case element_type_t::cast: {
                auto cast = dynamic_cast<compiler::cast*>(e);
                if (!count_temps(cast->expression(), result))
                    return false;
                break;
            }
            case element_type_t::field: {
                auto field = dynamic_cast<compiler::field*>(e);
                if (!count_temps(field->identifier(), result))
                    return false;
                break;
            }
            case element_type_t::switch_e: {
                auto switch_e = dynamic_cast<compiler::switch_element*>(e);
                if (!count_temps(switch_e->expression(), result))
                    return false;
                break;
            }
            case element_type_t::proc_type: {
                auto proc_type = dynamic_cast<compiler::procedure_type*>(e);
                auto return_type = proc_type->return_type();
                if (return_type != nullptr) {
                    if (!count_temps(return_type, result))
                        return false;
                }
                break;
            }
            case element_type_t::directive: {
                auto directive = dynamic_cast<compiler::directive*>(e);
                if (directive->name() == "run") {
                    auto run_directive = dynamic_cast<compiler::run_directive*>(directive);
                    if (!count_temps(run_directive->expression(), result))
                        return false;
                }
                break;
            }
            case element_type_t::return_e: {
                auto return_e = dynamic_cast<compiler::return_element*>(e);
                for (auto expr : return_e->expressions()) {
                    if (!count_temps(expr, result))
                        return false;
                }
                break;
            }
            case element_type_t::intrinsic: {
                auto intrinsic = dynamic_cast<compiler::intrinsic*>(e);
                if (!count_temps(intrinsic->arguments(), result))
                    return false;
                if (!count_temps(intrinsic->procedure_type(), result))
                    return false;
                break;
            }
            case element_type_t::transmute: {
                auto transmute = dynamic_cast<compiler::transmute*>(e);
                if (!count_temps(transmute->expression(), result))
                    return false;
                break;
            }
            case element_type_t::proc_call: {
                auto proc_call = dynamic_cast<compiler::procedure_call*>(e);
                if (!count_temps(proc_call->arguments(), result))
                    return false;
                if (!count_temps(proc_call->procedure_type(), result))
                    return false;
                break;
            }
            case element_type_t::expression: {
                auto expr = dynamic_cast<compiler::expression*>(e)->root();
                if (expr != nullptr) {
                    if (!count_temps(expr, result))
                        return false;
                }
                break;
            }
            case element_type_t::assignment: {
                auto assignment = dynamic_cast<compiler::assignment*>(e);
                for (auto expr : assignment->expressions()) {
                    if (!count_temps(expr, result))
                        return false;
                }
                break;
            }
            case element_type_t::identifier: {
                infer_type_result_t type_result {};
                if (!e->infer_type(_session, type_result))
                    return false;
                result.count(type_result.inferred_type);
                break;
            }
            case element_type_t::declaration: {
                auto decl = dynamic_cast<compiler::declaration*>(e);
                auto assignment = decl->assignment();
                if (assignment != nullptr) {
                    if (!count_temps(assignment, result))
                        return false;
                }
                break;
            }
            case element_type_t::argument_list: {
                auto arg_list = dynamic_cast<compiler::argument_list*>(e);
                for (auto arg : arg_list->elements()) {
                    if (!count_temps(arg, result))
                        return false;
                }
                break;
            }
            case element_type_t::unary_operator: {
                auto unary_op = dynamic_cast<compiler::unary_operator*>(e);
                if (!count_temps(unary_op->rhs(), result))
                    return false;
                infer_type_result_t type_result {};
                if (!e->infer_type(_session, type_result))
                    return false;
                result.count(type_result.inferred_type);
                break;
            }
            case element_type_t::binary_operator: {
                auto bin_op = dynamic_cast<compiler::binary_operator*>(e);
                if (!count_temps(bin_op->lhs(), result))
                    return false;
                if (!count_temps(bin_op->rhs(), result))
                    return false;
                infer_type_result_t type_result {};
                if (!e->infer_type(_session, type_result))
                    return false;
                result.count(type_result.inferred_type);
                break;
            }
            case element_type_t::identifier_reference: {
                auto ref = dynamic_cast<compiler::identifier_reference*>(e);
                if (ref->identifier() != nullptr) {
                    if (!count_temps(ref->identifier(), result))
                        return false;
                }
                break;
            }
            default: {
                break;
            }
        }

        return true;
    }

    bool byte_code_emitter::make_temp_locals(
            compiler::block* block,
            temp_local_list_t& locals) {
        temp_count_result_t result {};

        auto success = _session.scope_manager().visit_child_blocks(
            _session.result(),
            [&](compiler::block* scope) {
                for (auto stmt : scope->statements()) {
                    auto expr = stmt->expression();
                    if (expr == nullptr)
                        continue;
                    if (!count_temps(expr, result))
                        return false;
                    result.update();
                }
                return true;
            },
            block);
        if (!success)
            return false;

        for (size_t i = 1; i <= result.ints; i++) {
            locals.push_back(temp_local_t{
                fmt::format("itemp{}", i),
                0,
                vm::op_sizes::qword,
                vm::local_type_t::integer});
        }

        for (size_t i = 1; i <= result.floats; i++) {
            locals.push_back(temp_local_t{
                fmt::format("ftemp{}", i),
                0,
                vm::op_sizes::qword,
                vm::local_type_t::floating_point});
        }

        return true;
    }

    bool byte_code_emitter::emit_type_table() {
        auto& assembler = _session.assembler();

        auto type_info_block = _blocks.make();
        assembler.blocks().emplace_back(type_info_block);

        type_info_block->section(vm::section_t::ro_data);

        auto used_types = _session.used_types();
        for (auto type : used_types) {
            type_info_block->blank_line();
            type_info_block->align(4);
            type_info_block->string(
                assembler.make_label(compiler::type::make_literal_label_name(type)),
                assembler.make_label(compiler::type::make_literal_data_label_name(type)),
                type->name());
        }

        type_info_block->blank_line();
        type_info_block->align(8);
        type_info_block->label(assembler.make_label("_ti_array"));
        type_info_block->qwords({used_types.size()});
        for (auto type : used_types) {
            emit_type_info(type_info_block, type);
        }

        return true;
    }

    vm::basic_block* byte_code_emitter::emit_bootstrap_block() {
        auto& assembler = _session.assembler();

        auto block = _blocks.make();
        block->jump_direct(vm::instruction_operand_t(assembler.make_named_ref(
            vm::assembler_named_ref_type_t::label,
            "_start")));

        assembler.blocks().emplace_back(block);

        return block;
    }

    void byte_code_emitter::intern_string_literals() {
        auto literals = _session
            .elements()
            .find_by_type<compiler::string_literal>(element_type_t::string_literal);
        for (auto literal : literals) {
            if (literal->is_parent_type_one_of({
                    element_type_t::attribute,
                    element_type_t::directive,
                    element_type_t::module_reference})) {
                continue;
            }

            _session.intern_string(literal);
        }
    }

    bool byte_code_emitter::emit_interned_string_table() {
        auto& assembler = _session.assembler();

        auto block = _blocks.make();
        block->comment("interned string literals", 0);
        block->section(vm::section_t::ro_data);

        auto interned_strings = _session.interned_strings();
        for (const auto& kvp : interned_strings) {
            block->blank_line();
            block->align(4);
            block->comment(
                fmt::format("\"{}\"", kvp.first),
                0);

            std::string escaped {};
            if (!compiler::string_literal::escape(kvp.first, escaped)) {
                _session.error(
                    nullptr,
                    "X000",
                    fmt::format("invalid escape sequence: {}", kvp.first),
                    {});
                return false;
            }

            block->string(
                assembler.make_label(fmt::format("_intern_str_lit_{}", kvp.second)),
                assembler.make_label(fmt::format("_intern_str_lit_{}_data", kvp.second)),
                escaped);
        }

        assembler.blocks().emplace_back(block);

        return true;
    }

    bool byte_code_emitter::emit_section_tables(identifier_by_section_t& vars) {
        if (!group_identifiers(vars))
            return false;

        auto& assembler = _session.assembler();
        auto block = _blocks.make();

        for (const auto& section : vars.sections) {
            block->blank_line();
            block->section(section.first);

            for (auto e : section.second)
                emit_section_variable(block, section.first, e);
        }

        assembler.blocks().emplace_back(block);

        return true;
    }

    bool byte_code_emitter::emit_end_block(const vm::basic_block_list_t& predecessors) {
        auto& assembler = _session.assembler();

        auto end_block = _blocks.make();
        for (auto p : predecessors)
            p->add_successors({end_block});
        end_block->add_predecessors(predecessors);

        end_block->pre_blank_lines(1);
        end_block->align(vm::instruction_t::alignment);
        end_block->label(assembler.make_label("_end"));
        end_block->exit();

        assembler.blocks().emplace_back(end_block);

        return true;
    }

    vm::basic_block* byte_code_emitter::emit_start_block(
            const vm::basic_block_list_t& predecessors) {
        auto& assembler = _session.assembler();

        auto start_block = _blocks.make();
        start_block->add_predecessors(predecessors);
        for (auto p : predecessors)
            p->add_successors({start_block});

        start_block->pre_blank_lines(1);
        start_block->align(vm::instruction_t::alignment);
        start_block->label(assembler.make_label("_start"));

        start_block->move(
            vm::instruction_operand_t::fp(),
            vm::instruction_operand_t::sp());

        assembler.blocks().emplace_back(start_block);

        return start_block;
    }

    bool byte_code_emitter::referenced_module_variables(
            vm::basic_block* basic_block,
            compiler::block* block,
            const identifier_by_section_t& vars,
            identifier_list_t& locals) {
        auto& scope_manager = _session.scope_manager();

        element_id_set_t processed {};
        return scope_manager.visit_child_blocks(
            _session.result(),
            [&](compiler::block* scope) {
                for (auto ref_id : scope->references().as_list()) {
                    auto ref = dynamic_cast<compiler::identifier_reference*>(_session.elements().find(ref_id));
                    if (ref == nullptr)
                        continue;

                    auto var = ref->identifier();
                    if (var->field() != nullptr)
                        continue;

                    auto type = var->type_ref()->type();
                    if (type->is_type_one_of({
                            element_type_t::module_type,
                            element_type_t::namespace_type,
                            element_type_t::proc_type})) {
                        continue;
                    }

                    //if (vars.identifiers.count(var->id()) == 0) {
                    //    auto init = var->initializer();
                    //    if (init != nullptr && init->is_constant())
                    //        continue;
                    //}

                    if (processed.count(var->id()) > 0)
                        continue;

                    processed.insert(var->id());

                    if (var->usage() == identifier_usage_t::stack
                    ||  var->parent_scope()->has_stack_frame()) {
                        continue;
                    }

                    basic_block->local(
                        vm::local_type_t::integer,
                        var->label_name());
                    locals.emplace_back(var);
                }

                return true;
            },
            block);
    }

    vm::basic_block* byte_code_emitter::emit_implicit_blocks(
            const vm::basic_block_list_t& predecessors,
            const identifier_by_section_t& vars) {
        auto& assembler = _session.assembler();

        block_list_t implicit_blocks {};
        auto module_refs = _session
            .elements()
            .find_by_type<compiler::module_reference>(element_type_t::module_reference);
        for (auto mod_ref : module_refs) {
            auto block = mod_ref->reference()->scope();
            if (block->statements().empty())
                continue;
            implicit_blocks.emplace_back(block);
        }
        implicit_blocks.emplace_back(_session.program().module()->scope());

        vm::basic_block_list_t basic_blocks {};

        for (auto block : implicit_blocks) {
            auto implicit_block = _blocks.make();
            basic_blocks.emplace_back(implicit_block);
            assembler.blocks().emplace_back(implicit_block);

            implicit_block->pre_blank_lines(1);

            auto parent_element = block->parent_element();
            switch (parent_element->element_type()) {
                case element_type_t::namespace_e: {
                    auto parent_ns = dynamic_cast<compiler::namespace_element*>(parent_element);
                    implicit_block->comment(fmt::format(
                        "namespace: {}",
                        parent_ns->name()));
                    break;
                }
                case element_type_t::module: {
                    auto parent_module = dynamic_cast<compiler::module*>(parent_element);
                    implicit_block->comment(fmt::format(
                        "module: {}",
                        parent_module->source_file()->path().string()));
                    break;
                }
                default:
                    break;
            }

            implicit_block->label(assembler.make_label(block->label_name()));
            implicit_block->reset("local");
            implicit_block->reset("frame");
            implicit_block->frame_offset("locals", -8);

            identifier_list_t locals {};
            if (!referenced_module_variables(implicit_block, block, vars, locals))
                return nullptr;

            temp_local_list_t temp_locals {};
            if (!make_temp_locals(block, temp_locals))
                return nullptr;

            if (!emit_block(implicit_block, block, locals, temp_locals))
                return nullptr;
        }

        for (size_t i = 0; i < basic_blocks.size(); i++) {
            if (i == 0) {
                basic_blocks[0]->add_predecessors(predecessors);
                for (auto p : predecessors)
                    p->add_successors({basic_blocks[0]});
                continue;
            }
            basic_blocks[i]->add_predecessors({basic_blocks[i - 1]});
            basic_blocks[i - 1]->add_successors({basic_blocks[i]});
        }

        return basic_blocks.empty() ? nullptr : basic_blocks.back();
    }

    bool byte_code_emitter::emit_procedure_types(const identifier_by_section_t& vars) {
        procedure_instance_set_t proc_instance_set {};

        auto proc_calls = _session
            .elements()
            .find_by_type<compiler::procedure_call>(element_type_t::proc_call);
        for (auto proc_call : proc_calls) {
            if (proc_call->is_foreign())
                continue;

            auto proc_type = proc_call->procedure_type();
            if (proc_type == nullptr)
                return false;

            auto instance = proc_type->instance_for(_session, proc_call);
            if (instance != nullptr)
                proc_instance_set.insert(instance);
        }

        if (_session.result().is_failed())
            return false;

        auto& assembler = _session.assembler();
        for (auto instance : proc_instance_set) {
            auto basic_block = _blocks.make();
            assembler.blocks().emplace_back(basic_block);
            basic_block->pre_blank_lines(1);
            if (!emit_procedure_instance(basic_block, instance, vars))
                return false;
        }

        return true;
    }

    vm::basic_block* byte_code_emitter::emit_finalizers(
            const vm::basic_block_list_t& predecessors,
            const identifier_by_section_t& vars) {
        auto& assembler = _session.assembler();

        auto block = _blocks.make();
        block->add_predecessors(predecessors);
        for (auto p : predecessors)
            p->add_successors({block});
        assembler.blocks().emplace_back(block);
        block->pre_blank_lines(1);
        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label("_finalizer"));
        block->reset("local");
        block->reset("frame");

        std::vector<compiler::identifier*> to_finalize {};
        for (const auto& section : vars.sections) {
            for (compiler::element* e : section.second) {
                auto var = dynamic_cast<compiler::identifier*>(e);
                if (var == nullptr)
                    continue;

                if (!var->type_ref()->is_composite_type())
                    continue;

                auto local_type = number_class_to_local_type(var->type_ref()->type()->number_class());
                block->local(local_type, var->label_name());
                to_finalize.emplace_back(var);
            }
        }

        if (!to_finalize.empty())
            block->blank_line();

        for (auto var : to_finalize) {
            block->move(
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    var->label_name())),
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    var->label_name())));
        }

        for (auto var : to_finalize) {
            emit_finalizer(block, var);
        }

        return block;
    }

    bool byte_code_emitter::group_identifiers(identifier_by_section_t& vars) {
        auto& scope_manager = _session.scope_manager();

        auto bss_list = vars.variable_section(vm::section_t::bss);
        auto data_list = vars.variable_section(vm::section_t::data);
        auto ro_list = vars.variable_section(vm::section_t::ro_data);

        std::set<common::id_t> processed_identifiers {};

        const element_type_set_t parent_types {element_type_t::field};
        const element_type_set_t ignored_types {
            element_type_t::generic_type,
            element_type_t::namespace_type,
            element_type_t::module_reference,
        };

        auto identifier_refs = _session
            .elements()
            .find_by_type<compiler::identifier_reference>(element_type_t::identifier_reference);
        for (auto ref : identifier_refs) {
            auto var = ref->identifier();
            if (processed_identifiers.count(var->id()) > 0)
                continue;

            processed_identifiers.insert(var->id());

            if (scope_manager.within_local_scope(var->parent_scope()))
                continue;

            auto var_parent = var->parent_element();
            if (var_parent != nullptr
            &&  var_parent->is_parent_type_one_of(parent_types)) {
                continue;
            }

            auto var_type = var->type_ref()->type();
            if (var_type == nullptr) {
                // XXX: this is an error!
                return false;
            }

            if (var_type->is_type_one_of(ignored_types)) {
                continue;
            }

            auto init = var->initializer();
            if (init != nullptr) {
                switch (init->expression()->element_type()) {
                    case element_type_t::directive: {
                        auto directive = dynamic_cast<compiler::directive*>(init->expression());
                        if (directive->name() == "type")
                            continue;
                        break;
                    }
                    case element_type_t::proc_type:
                    case element_type_t::composite_type:
                    case element_type_t::type_reference:
                    case element_type_t::module_reference:
                        continue;
                    default:
                        break;
                }
            }

            if (var->is_constant()) {
                ro_list->emplace_back(var);
            } else {
                vars.identifiers.insert(var->id());

                if (init == nullptr) {
                    bss_list->emplace_back(var);
                } else {
                    data_list->emplace_back(var);
                }
            }
        }

        return true;
    }

    vm::basic_block* byte_code_emitter::emit_initializers(
            const vm::basic_block_list_t& predecessors,
            const identifier_by_section_t& vars) {
        auto& assembler = _session.assembler();

        auto block = _blocks.make();
        block->add_predecessors(predecessors);
        for (auto p : predecessors)
            p->add_successors({block});
        assembler.blocks().emplace_back(block);
        block->pre_blank_lines(1);
        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label("_initializer"));
        block->reset("local");
        block->reset("frame");

        identifier_list_t to_init {};
        for (const auto& section : vars.sections) {
            for (compiler::element* e : section.second) {
                auto var = dynamic_cast<compiler::identifier*>(e);
                if (var == nullptr)
                    continue;

                if (!var->type_ref()->is_composite_type())
                    continue;

                auto init = var->initializer();
                if (init != nullptr) {
                    if (init->expression()->element_type() == element_type_t::uninitialized_literal)
                        continue;
                }

                auto local_type = number_class_to_local_type(var->type_ref()->type()->number_class());
                block->local(local_type, var->label_name());
                to_init.emplace_back(var);
            }
        }

        if (!to_init.empty())
            block->blank_line();

        for (auto var : to_init) {
            block->move(
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    var->label_name())),
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    var->label_name())));
        }

        for (auto var : to_init) {
            emit_initializer(block, var);
        }

        return block;
    }

    bool byte_code_emitter::emit_section_variable(
            vm::basic_block* block,
            vm::section_t section,
            compiler::element* e) {
        auto& assembler = _session.assembler();

        switch (e->element_type()) {
            case element_type_t::type_literal: {
                auto type_literal = dynamic_cast<compiler::type_literal*>(e);
                block->blank_line();
                block->align(4);
                auto var_label = assembler.make_label(type_literal->label_name());
                block->label(var_label);
                // XXX: emit data
                break;
            }
            case element_type_t::identifier: {
                auto var = dynamic_cast<compiler::identifier*>(e);

                auto var_type = var->type_ref()->type();
                auto composite_type = dynamic_cast<compiler::composite_type*>(var_type);
                if (composite_type != nullptr)
                    composite_type->calculate_size();

                auto init = var->initializer();
                auto is_initialized = init != nullptr || section == vm::section_t::bss;

                block->blank_line();

                auto type_alignment = static_cast<uint8_t>(var_type->alignment());
                if (type_alignment > 1)
                    block->align(type_alignment);

                block->comment(fmt::format(
                    "identifier type: {}",
                    var->type_ref()->name()));
                auto var_label = assembler.make_label(var->label_name());
                block->label(var_label);

                switch (var_type->element_type()) {
                    case element_type_t::bool_type: {
                        bool value = false;
                        var->as_bool(value);

                        if (!is_initialized)
                            block->reserve_byte(1);
                        else
                            block->bytes({static_cast<uint8_t>(value ? 1 : 0)});
                        break;
                    }
                    case element_type_t::rune_type: {
                        common::rune_t value = common::rune_invalid;
                        var->as_rune(value);

                        if (!is_initialized)
                            block->reserve_byte(4);
                        else
                            block->dwords({static_cast<uint32_t>(value)});
                        break;
                    }
                    case element_type_t::pointer_type: {
                        if (!is_initialized) {
                            block->reserve_qword(1);
                        } else {
                            if (init != nullptr && init->is_constant()) {
                                emit_result_t result {};
                                if (!emit_element(&block, init, result))
                                    return false;
                                const auto& operand = result.operands.back();
                                switch (operand.type()) {
                                    case vm::instruction_operand_type_t::named_ref: {
                                        auto named_ref = operand.data<vm::assembler_named_ref_t*>();
                                        if (named_ref != nullptr)
                                            block->qwords({*named_ref});
                                        else
                                            block->qwords({0});
                                        break;
                                    }
                                    default: {
                                        block->qwords({0});
                                        break;
                                    }
                                }
                            } else {
                                block->qwords({0});
                            }
                        }
                        break;
                    }
                    case element_type_t::numeric_type: {
                        uint64_t value = 0;
                        auto symbol_type = vm::integer_symbol_type_for_size(var_type->size_in_bytes());

                        if (var_type->number_class() == number_class_t::integer) {
                            var->as_integer(value);
                        } else {
                            double temp = 0;
                            if (var->as_float(temp)) {
                                vm::register_value_alias_t alias {};
                                if (symbol_type == vm::symbol_type_t::u32)
                                    alias.dwf = static_cast<float>(temp);
                                else
                                    alias.qwf = temp;
                                value = alias.qw;
                            }
                        }

                        switch (symbol_type) {
                            case vm::symbol_type_t::u8:
                                if (!is_initialized)
                                    block->reserve_byte(1);
                                else
                                    block->bytes({static_cast<uint8_t>(value)});
                                break;
                            case vm::symbol_type_t::u16:
                                if (!is_initialized)
                                    block->reserve_word(1);
                                else
                                    block->words({static_cast<uint16_t>(value)});
                                break;
                            case vm::symbol_type_t::f32:
                            case vm::symbol_type_t::u32:
                                if (!is_initialized)
                                    block->reserve_dword(1);
                                else
                                    block->dwords({static_cast<uint32_t>(value)});
                                break;
                            case vm::symbol_type_t::f64:
                            case vm::symbol_type_t::u64:
                                if (!is_initialized)
                                    block->reserve_qword(1);
                                else
                                    block->qwords({value});
                                break;
                            case vm::symbol_type_t::bytes:
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case element_type_t::array_type:
                    case element_type_t::tuple_type:
                    case element_type_t::composite_type: {
                        block->reserve_byte(var_type->size_in_bytes());
                        break;
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            default:
                break;
        }

        return true;
    }

    bool byte_code_emitter::emit_primitive_initializer(
            vm::basic_block* block,
            const vm::instruction_operand_t& base_local,
            compiler::identifier* var,
            int64_t offset) {
        auto var_type = var->type_ref()->type();
        auto init = var->initializer();

        uint64_t default_value = var_type->element_type() == element_type_t::rune_type ?
                                 common::rune_invalid :
                                 0;
        vm::instruction_operand_t value(
            default_value,
            vm::op_size_for_byte_size(var_type->size_in_bytes()));
        vm::instruction_operand_t* value_ptr = &value;

        emit_result_t result {};
        if (init != nullptr) {
            auto expr = init->expression();
            if (expr != nullptr
            &&  expr->element_type() == element_type_t::uninitialized_literal) {
                return true;
            }

            if (!emit_element(&block, init, result))
                return false;

            value_ptr = &result.operands.back();
        }

        block->comment(
            fmt::format("initializer: {}: {}", var->label_name(), var_type->name()),
            vm::comment_location_t::after_instruction);
        block->store(
            base_local,
            *value_ptr,
            vm::instruction_operand_t::offset(offset));
        return true;
    }

    bool byte_code_emitter::emit_finalizer(
            vm::basic_block* block,
            compiler::identifier* var) {
        auto var_type = var->type_ref()->type();

        block->comment(
            fmt::format("finalizer: {}: {}", var->label_name(), var_type->name()),
            4);

        return true;
    }

    bool byte_code_emitter::emit_initializer(
            vm::basic_block* block,
            compiler::identifier* var) {
        vm::instruction_operand_t base_local(_session.assembler().make_named_ref(
            vm::assembler_named_ref_type_t::local,
            var->label_name()));

        std::vector<compiler::identifier*> list {};
        list.emplace_back(var);

        uint64_t offset = 0;

        while (!list.empty()) {
            auto next_var = list.front();
            list.erase(std::begin(list));

            auto var_type = next_var->type_ref()->type();

            switch (var_type->element_type()) {
                case element_type_t::rune_type:
                case element_type_t::bool_type:
                case element_type_t::numeric_type:
                case element_type_t::pointer_type: {
                    offset = common::align(offset, var_type->alignment());
                    if (!emit_primitive_initializer(block, base_local, next_var, offset))
                        return false;
                    offset += var_type->size_in_bytes();
                    break;
                }
                case element_type_t::tuple_type:
                case element_type_t::composite_type: {
                    auto composite_type = dynamic_cast<compiler::composite_type*>(var_type);
                    switch (composite_type->type()) {
                        case composite_types_t::enum_type: {
                            if (!emit_primitive_initializer(block, base_local, next_var, offset))
                                return false;
                            offset += var_type->size_in_bytes();
                            break;
                        }
                        case composite_types_t::union_type: {
                            // XXX: intentional no-op
                            break;
                        }
                        case composite_types_t::struct_type: {
                            auto field_list = composite_type->fields().as_list();
                            size_t index = 0;
                            for (auto fld: field_list) {
                                list.emplace(std::begin(list) + index, fld->identifier());
                                index++;
                            }
                            offset = common::align(offset, composite_type->alignment());
                            break;
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        return true;
    }

    bool byte_code_emitter::end_stack_frame(
            vm::basic_block* basic_block,
            compiler::block* block,
            const identifier_list_t& locals) {
        identifier_list_t to_finalize {};
        for (compiler::element* e : locals) {
            auto var = dynamic_cast<compiler::identifier*>(e);
            if (var == nullptr)
                continue;

            if (!var->type_ref()->is_composite_type()) {
                continue;
            }

            to_finalize.emplace_back(var);
        }

        if (!basic_block->is_current_instruction(vm::op_codes::rts)) {
            basic_block->move(
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t::fp());
            basic_block->pop(vm::instruction_operand_t::fp());
        }

        return true;
    }

    bool byte_code_emitter::begin_stack_frame(
            vm::basic_block* basic_block,
            compiler::block* block,
            identifier_list_t& locals,
            temp_local_list_t& temp_locals) {
        auto& assembler = _session.assembler();
        auto& scope_manager = _session.scope_manager();

        identifier_list_t to_init {};
        int64_t offset = 0;

        for (const auto& temp : temp_locals)
            basic_block->local(temp.type, temp.name);

        scope_manager.visit_child_blocks(
            _session.result(),
            [&](compiler::block* scope) {
                if (scope->is_parent_type_one_of({element_type_t::proc_type}))
                    return true;

                for (auto var : scope->identifiers().as_list()) {
                    if (var->is_constant())
                        continue;

                    auto type = var->type_ref()->type();
                    if (type->is_proc_type())
                        continue;

                    offset += -type->size_in_bytes();

                    var->usage(identifier_usage_t::stack);
                    basic_block->local(
                        vm::local_type_t::integer,
                        var->label_name(),
                        offset,
                        "locals");
                    to_init.emplace_back(var);
                    locals.emplace_back(var);
                }

                return true;
            },
            block);

        basic_block->push(vm::instruction_operand_t::fp());
        basic_block->move(
            vm::instruction_operand_t::fp(),
            vm::instruction_operand_t::sp());

        auto locals_size = common::align(static_cast<uint64_t>(-offset), 8);
        if (locals_size > 0) {
            locals_size += 8;
            basic_block->sub(
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t(static_cast<uint64_t>(locals_size), vm::op_sizes::dword));
        }

        for (auto var : locals) {
            const auto& name = var->label_name();
            const auto& local = basic_block->local(name);
            basic_block->comment(
                fmt::format("address: {}", name),
                vm::comment_location_t::after_instruction);
            if (!local->frame_offset.empty()) {
                basic_block->move(
                    vm::instruction_operand_t(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::local,
                        name)),
                    vm::instruction_operand_t::fp(),
                    vm::instruction_operand_t(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::offset,
                        name,
                        vm::op_sizes::word)));
            } else {
                basic_block->move(
                    vm::instruction_operand_t(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::local,
                        name)),
                    vm::instruction_operand_t(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        name)));
            }
        }

        for (auto var : to_init) {
            if (!emit_initializer(basic_block, var))
                return false;
        }

        return true;
    }

    std::string byte_code_emitter::temp_local_name(
            number_class_t type,
            uint8_t number) {
        switch (type) {
            case number_class_t::none: {
                return fmt::format("temp{}", number);
            }
            case number_class_t::integer: {
                return fmt::format("itemp{}", number);
            }
            case number_class_t::floating_point: {
                return fmt::format("ftemp{}", number);
            }
        }
    }

    bool byte_code_emitter::emit_procedure_epilogue(
            vm::basic_block* block,
            compiler::procedure_type* proc_type) {
        if (proc_type->is_foreign())
            return true;

        if (!proc_type->has_return()) {
            block->rts();
        }

        return true;
    }

    bool byte_code_emitter::emit_procedure_instance(
            vm::basic_block* block,
            compiler::procedure_instance* proc_instance,
            const identifier_by_section_t& vars) {
        auto procedure_type = proc_instance->procedure_type();
        if (procedure_type->is_foreign())
            return true;

        auto scope_block = proc_instance->scope();

        identifier_list_t locals {};
        if (!emit_procedure_prologue(block, procedure_type, locals))
            return false;

        if (!referenced_module_variables(block, scope_block, vars, locals))
            return false;

        temp_local_list_t temp_locals {};
        if (!make_temp_locals(scope_block, temp_locals))
            return false;

        block->blank_line();
        if (!emit_block(block, scope_block, locals, temp_locals))
            return false;

        return emit_procedure_epilogue(block, procedure_type);
    }

    bool byte_code_emitter::emit_procedure_prologue(
            vm::basic_block* block,
            compiler::procedure_type* proc_type,
            identifier_list_t& parameters) {
        if (proc_type->is_foreign())
            return true;

        auto& assembler = _session.assembler();

        auto procedure_label = proc_type->symbol()->name();
        auto parent_init = proc_type->parent_element_as<compiler::initializer>();
        if (parent_init != nullptr) {
            auto parent_var = parent_init->parent_element_as<compiler::identifier>();
            if (parent_var != nullptr) {
                procedure_label = parent_var->label_name();
            }
        }

        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label(procedure_label));
        block->reset("local");
        block->reset("frame");
        block->frame_offset("locals", -8);

        auto return_type = proc_type->return_type();
        if (return_type != nullptr) {
            block->frame_offset("returns", 16);
            block->frame_offset("parameters", 24);

            auto retval_var = return_type->declaration()->identifier();
            parameters.emplace_back(retval_var);
            block->local(
                vm::local_type_t::integer,
                retval_var->label_name(),
                0,
                "returns");
        } else {
            block->frame_offset("parameters", 16);
        }

        int64_t offset = 0;
        auto fields = proc_type->parameters().as_list();
        for (auto fld : fields) {
            auto var = fld->identifier();
            block->local(
                vm::local_type_t::integer,
                var->label_name(),
                offset,
                "parameters");
            offset += 8;
            parameters.emplace_back(var);
        }

        return true;
    }

    bool byte_code_emitter::emit_arguments(
            vm::basic_block* block,
            compiler::argument_list* arg_list,
            const compiler::element_list_t& elements) {
        for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
            compiler::type* type = nullptr;

            element* arg = *it;
            switch (arg->element_type()) {
                case element_type_t::argument_list: {
                    auto list = dynamic_cast<compiler::argument_list*>(arg);
                    if (!emit_arguments(block, list, list->elements()))
                        return false;
                    break;
                }
                case element_type_t::cast:
                case element_type_t::transmute:
                case element_type_t::proc_call:
                case element_type_t::intrinsic:
                case element_type_t::expression:
                case element_type_t::nil_literal:
                case element_type_t::float_literal:
                case element_type_t::string_literal:
                case element_type_t::unary_operator:
                case element_type_t::assembly_label:
                case element_type_t::binary_operator:
                case element_type_t::boolean_literal:
                case element_type_t::integer_literal:
                case element_type_t::character_literal:
                case element_type_t::identifier_reference: {
                    auto temp = allocate_temp();
                    emit_result_t arg_result {};
                    if (!emit_element(&block, arg, arg_result))
                        return false;
                    if (!arg_result.skip_read)
                        read(block, arg_result, temp);

                    if (!arg_list->is_foreign_call()) {
                        type = arg_result.type_result.inferred_type;
                        switch (type->element_type()) {
                            case element_type_t::array_type:
                            case element_type_t::tuple_type:
                            case element_type_t::composite_type: {
                                auto size = static_cast<uint64_t>(common::align(
                                    type->size_in_bytes(),
                                    8));
                                block->sub(
                                    vm::instruction_operand_t::sp(),
                                    vm::instruction_operand_t::sp(),
                                    vm::instruction_operand_t(size, vm::op_sizes::word));
                                block->copy(
                                    vm::op_sizes::byte,
                                    vm::instruction_operand_t::sp(),
                                    arg_result.operands.back(),
                                    vm::instruction_operand_t(size, vm::op_sizes::word));
                                break;
                            }
                            default: {
                                block->push(arg_result.operands.back());
                                break;
                            }
                        }
                    } else {
                        block->push(arg_result.operands.back());
                    }

                    free_temp();
                    break;
                }
                default:
                    break;
            }

            if (type != nullptr) {
                auto size = static_cast<uint64_t>(common::align(
                    type->size_in_bytes(),
                    8));
                arg_list->allocated_size(arg_list->allocated_size() + size);
            }
        }

        return true;
    }

    bool byte_code_emitter::emit_relational_operator(
            vm::basic_block* block,
            compiler::binary_operator* binary_op,
            emit_result_t& result) {
        auto& assembler = _session.assembler();

        auto end_label_name = fmt::format("{}_end", binary_op->label_name());
        auto end_label_ref = assembler.make_named_ref(
            vm::assembler_named_ref_type_t::label,
            end_label_name);

        auto lhs_temp = allocate_temp();

        vm::instruction_operand_t result_operand(assembler.make_named_ref(
            vm::assembler_named_ref_type_t::local,
            temp_local_name(result.type_result.inferred_type->number_class(), lhs_temp),
            vm::op_sizes::byte));
        result.operands.emplace_back(result_operand);

        emit_result_t lhs_result {};
        if (!emit_element(&block, binary_op->lhs(), lhs_result))
            return false;
        read(block, lhs_result, lhs_temp);

        auto rhs_temp = allocate_temp();
        emit_result_t rhs_result {};
        if (!emit_element(&block, binary_op->rhs(), rhs_result))
            return false;
        read(block, rhs_result, rhs_temp);

        auto is_signed = lhs_result.type_result.inferred_type->is_signed();

        if (is_logical_conjunction_operator(binary_op->operator_type())) {
            block->move(
                result_operand,
                lhs_result.operands.back());

            switch (binary_op->operator_type()) {
                case operator_type_t::logical_or: {
                    block->bnz(
                        result_operand,
                        vm::instruction_operand_t(end_label_ref));
                    break;
                }
                case operator_type_t::logical_and: {
                    block->bz(
                        result_operand,
                        vm::instruction_operand_t(end_label_ref));
                    break;
                }
                default: {
                    break;
                }
            }

            block->move(
                result_operand,
                rhs_result.operands.back());
        } else {
            block->cmp(
                lhs_result.operands.back(),
                rhs_result.operands.back());

            switch (binary_op->operator_type()) {
                case operator_type_t::equals: {
                    block->setz(result_operand);
                    break;
                }
                case operator_type_t::less_than: {
                    if (is_signed)
                        block->setl(result_operand);
                    else
                        block->setb(result_operand);
                    break;
                }
                case operator_type_t::not_equals: {
                    block->setnz(result_operand);
                    break;
                }
                case operator_type_t::greater_than: {
                    if (is_signed)
                        block->setg(result_operand);
                    else
                        block->seta(result_operand);
                    break;
                }
                case operator_type_t::less_than_or_equal: {
                    if (is_signed)
                        block->setle(result_operand);
                    else
                        block->setbe(result_operand);
                    break;
                }
                case operator_type_t::greater_than_or_equal: {
                    if (is_signed)
                        block->setge(result_operand);
                    else
                        block->setae(result_operand);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        block->label(assembler.make_label(end_label_name));

        free_temp();
        free_temp();
        return true;
    }

    bool byte_code_emitter::emit_arithmetic_operator(
            vm::basic_block* block,
            compiler::binary_operator* binary_op,
            emit_result_t& result) {
        auto& assembler = _session.assembler();

        auto lhs_temp = allocate_temp();
        auto rhs_temp = allocate_temp();

        emit_result_t lhs_result {};
        if (!emit_element(&block, binary_op->lhs(), lhs_result))
            return false;
        read(block, lhs_result, lhs_temp);

        emit_result_t rhs_result {};
        if (!emit_element(&block, binary_op->rhs(), rhs_result))
            return false;
        read(block, rhs_result, rhs_temp);

        auto size = vm::op_size_for_byte_size(result.type_result.inferred_type->size_in_bytes());
        vm::instruction_operand_t result_operand(assembler.make_named_ref(
            vm::assembler_named_ref_type_t::local,
            temp_local_name(result.type_result.inferred_type->number_class(), lhs_temp),
            size));
        result.operands.emplace_back(result_operand);

        switch (binary_op->operator_type()) {
            case operator_type_t::add: {
                block->add(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::divide: {
                block->div(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::modulo: {
                block->mod(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::multiply: {
                block->mul(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::exponent: {
                block->pow(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::subtract: {
                block->sub(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::binary_or: {
                block->or_op(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::shift_left: {
                block->shl(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::binary_and: {
                block->and_op(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::binary_xor: {
                block->xor_op(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::rotate_left: {
                block->rol(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::shift_right: {
                block->shr(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::rotate_right: {
                block->ror(
                    result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            default:
                break;
        }

        free_temp();
        free_temp();

        return true;
    }

    std::string byte_code_emitter::interned_string_data_label(common::id_t id) {
        common::id_t intern_id;
        _session.interned_strings().element_id_to_intern_id(id, intern_id);
        return fmt::format("_intern_str_lit_{}_data", intern_id);
    }

    bool byte_code_emitter::is_temp_local(const vm::instruction_operand_t& operand) {
        if (operand.type() == vm::instruction_operand_type_t::named_ref) {
            auto named_ref = *operand.data<vm::assembler_named_ref_t*>();
            return named_ref->name.substr(1, 4) == "temp";
        }
        return false;
    }

}
