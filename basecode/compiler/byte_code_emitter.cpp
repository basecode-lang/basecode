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

    struct proc_call_edge_t {
        compiler::element* site = nullptr;
        compiler::procedure_call* call = nullptr;
    };

    using proc_call_edge_list_t = std::vector<proc_call_edge_t>;

    static void walk_call_graph_edges(
            const proc_call_edge_list_t& edges,
            procedure_call_set_t& proc_call_set,
            compiler::element* site) {
        for (const auto& edge : edges) {
            if (edge.site != site)
                continue;
            proc_call_set.insert(edge.call);
            walk_call_graph_edges(
                edges,
                proc_call_set,
                edge.call->procedure_type());
        }
    }

    static compiler::element* find_call_site(compiler::procedure_call* proc_call) {
        auto current_scope = proc_call->parent_scope();
        while (current_scope != nullptr) {
            auto parent_element = current_scope->parent_element();
            switch (parent_element->element_type()) {
                case element_type_t::module:
                case element_type_t::proc_type: {
                    return parent_element;
                }
                default: {
                    break;
                }
            }
            current_scope = current_scope->parent_scope();
        }
        return nullptr;
    }

    static proc_call_edge_list_t find_root_edges(const proc_call_edge_list_t& edges) {
        proc_call_edge_list_t root_edges {};
        for (const auto& edge : edges) {
            if (edge.site->element_type() == element_type_t::module)
                root_edges.emplace_back(edge);
        }
        return root_edges;
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

    byte_code_emitter::byte_code_emitter(
            compiler::session& session) : _variables(session),
                                          _session(session) {
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

    bool byte_code_emitter::emit() {
        _variables.initialize();

        intern_string_literals();

        auto start_block = emit_start_block();
        if (start_block == nullptr)
            return false;

        auto last_implicit_block = emit_implicit_blocks({start_block});

        vm::basic_block_list_t end_predecessors {};
        if (last_implicit_block != nullptr)
            end_predecessors.emplace_back(last_implicit_block);
        else
            end_predecessors.emplace_back(start_block);

        if (!emit_end_block(end_predecessors))
            return false;

        if (!emit_procedure_types())
            return false;

        if (!emit_type_table())
            return false;

        if (!emit_interned_string_table())
            return false;

        if (!emit_section_tables())
            return false;

        return true;
    }

    bool byte_code_emitter::emit_block(
            vm::basic_block** basic_block,
            compiler::block* block) {
        auto& assembler = _session.assembler();
        const auto excluded_parent_types = element_type_set_t{
            element_type_t::directive,
        };

        uint64_t locals_size = 0;
        variable_list_t to_init {};
        bool reset_temp_block = false;
        bool clear_stack_frame = false;
        auto frame_block = *basic_block;
        vm::basic_block* code_block = nullptr;

        if (!block->is_parent_type_one_of(excluded_parent_types)
        && !_in_stack_frame) {
            auto locals = _variables.variables();
            for (auto local : locals) {
                if (local->type == variable_type_t::temporary)
                    continue;

                if (local->type == variable_type_t::local
                ||  local->type == variable_type_t::module
                ||  local->type == variable_type_t::parameter
                ||  local->type == variable_type_t::return_parameter) {
                    to_init.emplace_back(local);
                }

                int64_t offset = 0;
                if (local->field_offset.base_ref == nullptr) {
                    offset = local->frame_offset;
                    if (local->type == variable_type_t::local)
                        locals_size += local->size_in_bytes();
                } else {
                    offset = local->field_offset.from_start;
                }

                if (!local->flag(variable_t::flags_t::in_block)) {
                    // XXX: fix this method to accept a string_view instead of string
                    frame_block->local(
                        number_class_to_local_type(local->number_class),
                        local->label,
                        offset,
                        std::string(variable_type_to_group(local->type)));
                    local->flag(variable_t::flags_t::in_block, true);
                }
            }
            locals_size = common::align(locals_size, 8);

            if (_temps_block == nullptr) {
                _temps_block = _blocks.make();
                assembler.blocks().emplace_back(_temps_block);
                frame_block->successors().emplace_back(_temps_block);
                _temps_block->predecessors().emplace_back(frame_block);
                reset_temp_block = true;
            }

            code_block = _blocks.make();
            assembler.blocks().emplace_back(code_block);
            code_block->predecessors().emplace_back(_temps_block);
            _temps_block->successors().emplace_back(code_block);

            *basic_block = code_block;
        }

        if (block->has_stack_frame()
        && !_in_stack_frame) {
            _in_stack_frame = true;
            clear_stack_frame = true;

            code_block->push(vm::instruction_operand_t::fp());
            code_block->move(
                vm::instruction_operand_t::fp(),
                vm::instruction_operand_t::sp());
            if (locals_size > 0) {
                code_block->sub(
                    vm::instruction_operand_t::sp(),
                    vm::instruction_operand_t::sp(),
                    vm::instruction_operand_t(
                        static_cast<uint64_t>(locals_size),
                        vm::op_sizes::dword));
            }
        }

        for (auto local : to_init) {
            auto named_ref = assembler.make_named_ref(
                vm::assembler_named_ref_type_t::local,
                local->label,
                vm::op_size_for_byte_size(local->size_in_bytes()));
            _variables.activate(code_block, named_ref);
        }

        defer({
            if (reset_temp_block)
                _temps_block = nullptr;

            if (clear_stack_frame) {
                _in_stack_frame = false;

                auto current_block = *basic_block;

                auto exit_block = _blocks.make();
                assembler.blocks().emplace_back(exit_block);
                exit_block->predecessors().emplace_back(current_block);

                if (!current_block->is_current_instruction(vm::op_codes::rts)
                && !_return_emitted) {
                    exit_block->move(
                        vm::instruction_operand_t::sp(),
                        vm::instruction_operand_t::fp());
                    exit_block->pop(vm::instruction_operand_t::fp());
                }

                _return_emitted = false;
            }
        });

        const auto& statements = block->statements();
        for (size_t index = 0; index < statements.size(); ++index) {
            auto stmt = statements[index];

            for (auto label : stmt->labels()) {
                emit_result_t label_result {};
                if (!emit_element(basic_block, label, label_result))
                    return false;
            }

            auto expr = stmt->expression();
            if (expr != nullptr
            &&  expr->element_type() == element_type_t::defer) {
                continue;
            }

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
            if (!emit_element(basic_block, stmt, stmt_result))
                return false;
        }

        auto working_stack = block->defer_stack();
        while (!working_stack.empty()) {
            auto deferred = working_stack.top();

            emit_result_t defer_result {};
            if (!emit_element(basic_block, deferred, defer_result))
                return false;
            working_stack.pop();
        }

        if (_temps_block != nullptr) {
            auto temp_locals = _variables.temps();
            for (auto temp : temp_locals) {
                if (!temp->flag(variable_t::flags_t::in_block)) {
                    _temps_block->local(
                        number_class_to_local_type(temp->number_class),
                        temp->label);
                    temp->flag(variable_t::flags_t::in_block, true);
                }
            }
        }

        return true;
    }

    bool byte_code_emitter::emit_element(
            vm::basic_block** basic_block,
            compiler::element* e,
            emit_result_t& result) {
        auto& labels = _session.labels();
        auto& builder = _session.builder();
        auto& assembler = _session.assembler();

        auto current_block = *basic_block;

        vm::op_sizes op_size {};
        inferred_type_t* inferred = nullptr;

        e->infer_type(_session, result.type_result);

        if (!result.type_result.types.empty()) {
            inferred = &result.type_result.types.back();
            if (!inferred->type->is_composite_type()) {
                op_size = vm::op_size_for_byte_size(inferred->type->size_in_bytes());
            } else if (inferred->type->is_pointer_type()) {
                op_size = vm::op_sizes::qword;
            }
        }

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

                emit_result_t expr_result {};
                if (!emit_element(basic_block, expr, expr_result))
                    return false;

                const auto& expr_inferred = expr_result.type_result.types.back();

                cast_mode_t mode;
                auto type_ref = cast->type();
                auto source_number_class = expr_inferred.type->number_class();
                auto source_size = expr_inferred.type->size_in_bytes();
                auto target_number_class = type_ref->type()->number_class();
                auto target_size = type_ref->type()->size_in_bytes();

                if (source_number_class == number_class_t::none) {
                    _session.error(
                        expr->module(),
                        "C073",
                        fmt::format(
                            "cannot cast from type: {}",
                            expr_inferred.type_name()),
                        expr->location());
                    return false;
                } else if (target_number_class == number_class_t::none) {
                    _session.error(
                        expr->module(),
                        "C073",
                        fmt::format(
                            "cannot cast to type: {}",
                            type_ref->symbol_override().name),
                        cast->type()->location());
                    return false;
                }

                if (source_number_class == number_class_t::integer
                &&  target_number_class == number_class_t::integer) {
                    if (source_size == target_size) {
                        mode = cast_mode_t::integer_truncate;
                    } else if (source_size > target_size) {
                        mode = cast_mode_t::integer_truncate;
                    } else {
                        auto source_numeric_type = dynamic_cast<compiler::numeric_type*>(expr_inferred.type);
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
                        "cast: {} -> {}",
                        expr_inferred.type_name(),
                        type_ref->symbol()->name()),
                    vm::comment_location_t::after_instruction);

                auto result_operand = target_operand(
                    result,
                    target_number_class,
                    vm::op_size_for_byte_size(target_size));

                switch (mode) {
                    case cast_mode_t::noop: {
                        break;
                    }
                    case cast_mode_t::integer_truncate: {
                        current_block->move(
                            *result_operand,
                            expr_result.operands.back());
                        break;
                    }
                    case cast_mode_t::integer_sign_extend: {
                        current_block->moves(
                            *result_operand,
                            expr_result.operands.back());
                        break;
                    }
                    case cast_mode_t::integer_zero_extend: {
                        current_block->movez(
                            *result_operand,
                            expr_result.operands.back());
                        break;
                    }
                    case cast_mode_t::float_extend:
                    case cast_mode_t::float_truncate:
                    case cast_mode_t::integer_to_float:
                    case cast_mode_t::float_to_integer: {
                        current_block->convert(
                            *result_operand,
                            expr_result.operands.back());
                        break;
                    }
                }

                release_temps(expr_result.temps);
                break;
            }
            case element_type_t::if_e: {
                auto if_e = dynamic_cast<compiler::if_element*>(e);
                auto begin_label_name = fmt::format("{}_entry", if_e->label_name());
                auto true_label_name = fmt::format("{}_true", if_e->label_name());
                auto false_label_name = fmt::format("{}_false", if_e->label_name());
                auto end_label_name = fmt::format("{}_exit", if_e->label_name());

                // XXX: revisit
                target_operand(result);

                auto predicate_block = _blocks.make();
                assembler.blocks().emplace_back(predicate_block);
                predicate_block->predecessors().emplace_back(current_block);
                current_block->successors().emplace_back(predicate_block);
                *basic_block = predicate_block;

                predicate_block->label(labels.make(begin_label_name, predicate_block));
                emit_result_t predicate_result {};
                if (!emit_element(basic_block, if_e->predicate(), predicate_result))
                    return false;
                predicate_block = *basic_block;
                predicate_block->bz(
                    predicate_result.operands.back(),
                    vm::instruction_operand_t(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        false_label_name)));
                release_temps(predicate_result.temps);

                auto true_block = _blocks.make();
                assembler.blocks().emplace_back(true_block);
                true_block->predecessors().emplace_back(predicate_block);
                *basic_block = true_block;

                true_block->label(labels.make(true_label_name, true_block));
                emit_result_t true_result {};
                if (!emit_element(basic_block, if_e->true_branch(), true_result))
                    return false;
                true_block = *basic_block;

                if (!true_block->is_current_instruction(vm::op_codes::jmp)
                &&  !true_block->is_current_instruction(vm::op_codes::rts)) {
                    true_block->jump_direct(vm::instruction_operand_t(
                        assembler.make_named_ref(
                            vm::assembler_named_ref_type_t::label,
                            end_label_name)));
                }

                release_temps(true_result.temps);

                auto false_block = _blocks.make();
                assembler.blocks().emplace_back(false_block);
                false_block->predecessors().emplace_back(predicate_block);
                *basic_block = false_block;

                false_block->label(labels.make(false_label_name, false_block));
                auto false_branch = if_e->false_branch();
                if (false_branch != nullptr) {
                    emit_result_t false_result {};
                    if (!emit_element(basic_block, false_branch, false_result))
                        return false;
                    false_block = *basic_block;
                    release_temps(false_result.temps);
                } else {
                    false_block->nop();
                }

                predicate_block->add_successors({true_block, false_block});

                auto exit_block = _blocks.make();
                assembler.blocks().emplace_back(exit_block);
                exit_block->add_predecessors({true_block, false_block});
                exit_block->label(labels.make(end_label_name, exit_block));

                true_block->successors().emplace_back(exit_block);
                false_block->successors().emplace_back(exit_block);

                *basic_block = exit_block;

                break;
            }
            case element_type_t::with: {
                auto with = dynamic_cast<compiler::with*>(e);
                auto body = with->body();
                if (body != nullptr) {
                    emit_result_t body_result {};
                    if (!emit_element(basic_block, body, body_result))
                        return false;
                    release_temps(body_result.temps);
                }
                break;
            }
            case element_type_t::for_e: {
                auto for_e = dynamic_cast<compiler::for_element*>(e);
                auto entry_label_name = fmt::format("{}_entry", for_e->label_name());
                auto body_label_name = fmt::format("{}_body", for_e->label_name());
                auto step_label_name = fmt::format("{}_step", for_e->label_name());
                auto exit_label_name = fmt::format("{}_exit", for_e->label_name());

                auto for_expr = for_e->expression();
                switch (for_expr->element_type()) {
                    case element_type_t::intrinsic: {
                        auto intrinsic = dynamic_cast<compiler::intrinsic*>(for_expr);
                        switch (intrinsic->type()) {
                            case intrinsic_type_t::range: {
                                auto begin_label_ref = assembler.make_named_ref(
                                    vm::assembler_named_ref_type_t::label,
                                    entry_label_name);
                                auto step_label_ref = assembler.make_named_ref(
                                    vm::assembler_named_ref_type_t::label,
                                    step_label_name);
                                auto exit_label_ref = assembler.make_named_ref(
                                    vm::assembler_named_ref_type_t::label,
                                    exit_label_name);

                                flow_control_t flow_control {
                                    .exit_label = exit_label_ref,
                                    .continue_label = step_label_ref
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

                                auto init_block = _blocks.make();
                                assembler.blocks().emplace_back(init_block);
                                init_block->predecessors().emplace_back(current_block);
                                current_block->successors().emplace_back(init_block);

                                *basic_block = init_block;
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
                                            default:
                                                // XXX: error
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
                                            default:
                                                // XXX: error
                                                break;
                                        }
                                        break;
                                    }
                                    default: {
                                        // XXX: error
                                        break;
                                    }
                                }

                                auto stop_arg = range->arguments()->param_by_name("stop");

                                auto predicate_block = _blocks.make();
                                assembler.blocks().emplace_back(predicate_block);
                                predicate_block->predecessors().emplace_back(init_block);
                                init_block->successors().emplace_back(predicate_block);

                                predicate_block->label(labels.make(entry_label_name, predicate_block));
                                auto comparison_op = builder.make_binary_operator(
                                    for_e->parent_scope(),
                                    cmp_op_type,
                                    for_e->induction_decl()->identifier(),
                                    stop_arg);
                                comparison_op->make_non_owning();
                                defer(_session.elements().remove(comparison_op->id()));

                                *basic_block = predicate_block;

                                emit_result_t cmp_result;
                                if (!emit_element(basic_block, comparison_op, cmp_result))
                                    return false;
                                predicate_block->bz(
                                    cmp_result.operands.back(),
                                    vm::instruction_operand_t(exit_label_ref));
                                release_temps(cmp_result.temps);
                                auto original_predicate_block = predicate_block;
                                predicate_block = *basic_block;

                                auto body_block = _blocks.make();
                                assembler.blocks().emplace_back(body_block);
                                body_block->predecessors().emplace_back(predicate_block);
                                *basic_block = body_block;

                                body_block->label(labels.make(body_label_name, body_block));
                                emit_result_t body_result;
                                if (!emit_element(basic_block, for_e->body(), body_result))
                                    return false;
                                body_block = *basic_block;
                                release_temps(body_result.temps);

                                auto step_block = _blocks.make();
                                assembler.blocks().emplace_back(step_block);
                                step_block->predecessors().emplace_back(body_block);
                                step_block->successors().emplace_back(predicate_block);
                                body_block->successors().emplace_back(step_block);
                                *basic_block = step_block;

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

                                step_block->label(labels.make(step_label_name, step_block));
                                emit_result_t step_result;
                                if (!emit_element(basic_block, induction_assign, step_result))
                                    return false;
                                step_block->jump_direct(vm::instruction_operand_t(begin_label_ref));
                                release_temps(step_result.temps);

                                auto exit_block = _blocks.make();
                                assembler.blocks().emplace_back(exit_block);
                                exit_block->predecessors().emplace_back(original_predicate_block);
                                exit_block->label(labels.make(exit_label_name, exit_block));
                                *basic_block = exit_block;

                                original_predicate_block->add_successors({body_block, exit_block});
                                break;
                            }
                            default: {
                                break;
                            }
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
                current_block->label(labels.make(e->label_name(), current_block));
                break;
            }
            case element_type_t::block: {
                auto scope_block = dynamic_cast<compiler::block*>(e);
                if (!emit_block(basic_block, scope_block))
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
                    release_temps(decl_result.temps);
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
                    release_temps(expr_result.temps);
                }
                break;
            }
            case element_type_t::yield: {
                // XXX: not complete; place holder
                auto yield = dynamic_cast<compiler::yield*>(e);
                auto expr = yield->expression();
                if (expr != nullptr) {
                    emit_result_t expr_result {};
                    if (!emit_element(basic_block, expr, expr_result))
                        return false;
                    release_temps(expr_result.temps);
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
                    release_temps(scope_result.temps);
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

                    *basic_block = predicate_block;

                    emit_result_t equals_result {};
                    if (!emit_element(basic_block, equals_op, equals_result))
                        return false;

                    predicate_block = *basic_block;
                    predicate_block->bz(
                        equals_result.operands.back(),
                        vm::instruction_operand_t(assembler.make_named_ref(
                            vm::assembler_named_ref_type_t::label,
                            false_label_name)));

                    release_temps(equals_result.temps);
                }

                auto true_block = _blocks.make();
                assembler.blocks().emplace_back(true_block);
                true_block->add_predecessors({*basic_block});
                *basic_block = true_block;

                true_block->label(labels.make(true_label_name, true_block));
                if (!emit_element(basic_block, case_e->scope(), result))
                    return false;
                true_block = *basic_block;

                if (!is_default_case) {
                    if (flow_control->fallthrough) {
                        true_block->jump_direct(vm::instruction_operand_t(fallthrough_label));
                        labels.add_cfg_edge(true_block, fallthrough_label->name);
                    } else {
                        true_block->jump_direct(vm::instruction_operand_t(flow_control->exit_label));
                    }
                }

                auto exit_block = _blocks.make();
                assembler.blocks().emplace_back(exit_block);
                exit_block->label(labels.make(false_label_name, exit_block));

                true_block->add_predecessors({predicate_block});
                true_block->add_successors({exit_block});

                exit_block->add_predecessors({predicate_block});

                if (predicate_block != nullptr) {
                    predicate_block->add_successors({true_block, exit_block});
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

                auto exit_block = _blocks.make();
                assembler.blocks().emplace_back(exit_block);
                exit_block->predecessors().emplace_back(current_block);

                exit_block->comment(
                    fmt::format("break: {}", label_name),
                    vm::comment_location_t::after_instruction);
                exit_block->jump_direct(vm::instruction_operand_t(label_ref));
                labels.add_cfg_edge(exit_block, label_name);

                *basic_block = exit_block;
                break;
            }
            case element_type_t::while_e: {
                auto while_e = dynamic_cast<compiler::while_element*>(e);
                auto entry_label_name = fmt::format("{}_entry", while_e->label_name());
                auto body_label_name = fmt::format("{}_body", while_e->label_name());
                auto exit_label_name = fmt::format("{}_exit", while_e->label_name());

                auto entry_label_ref = assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    entry_label_name);
                auto exit_label_ref = assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    exit_label_name);

                push_flow_control(flow_control_t{
                    .exit_label = exit_label_ref,
                    .continue_label = entry_label_ref
                });
                defer(pop_flow_control());

                auto predicate_block = _blocks.make();
                assembler.blocks().emplace_back(predicate_block);
                predicate_block->predecessors().emplace_back(current_block);
                *basic_block = predicate_block;

                if (!fill_referenced_identifiers(predicate_block, while_e->predicate()))
                    return false;

                predicate_block->label(labels.make(entry_label_name, predicate_block));

                emit_result_t predicate_result {};
                if (!emit_element(basic_block, while_e->predicate(), predicate_result))
                    return false;
                predicate_block->bz(
                    predicate_result.operands.back(),
                    vm::instruction_operand_t(exit_label_ref));
                auto original_predicate_block = predicate_block;
                release_temps(predicate_result.temps);
                predicate_block = *basic_block;

                auto body_block = _blocks.make();
                assembler.blocks().emplace_back(body_block);
                body_block->predecessors().emplace_back(original_predicate_block);
                body_block->successors().emplace_back(original_predicate_block);
                *basic_block = body_block;

                body_block->label(labels.make(body_label_name, body_block));
                if (!emit_element(basic_block, while_e->body(), result))
                    return false;
                body_block = *basic_block;
                body_block->jump_direct(vm::instruction_operand_t(entry_label_ref));

                auto exit_block = _blocks.make();
                assembler.blocks().emplace_back(exit_block);
                exit_block->predecessors().emplace_back(original_predicate_block);

                exit_block->label(labels.make(exit_label_name, exit_block));
                exit_block->nop();

                original_predicate_block->add_successors({body_block, exit_block});

                *basic_block = exit_block;
                break;
            }
            case element_type_t::return_e: {
                auto return_e = dynamic_cast<compiler::return_element*>(e);
                auto proc_type = return_e->find_parent_of_type<compiler::procedure_type>(element_type_t::proc_type);

                auto return_block = _blocks.make();
                assembler.blocks().emplace_back(return_block);
                return_block->predecessors().emplace_back(current_block);
                current_block->successors().emplace_back(return_block);

                *basic_block = return_block;

                auto return_parameters = proc_type->return_parameters();
                if (!return_parameters.empty()) {
                    size_t index = 0;
                    for (auto expr : return_e->expressions()) {
                        const auto field_name = fmt::format("_{}", index++);
                        auto fld = return_parameters.find_by_name(field_name);
                        if (fld == nullptr) {
                            // XXX: error
                            return false;
                        }

                        emit_result_t expr_result{};
                        if (!emit_element(basic_block, expr, expr_result))
                            return false;
                        return_block = *basic_block;

                        auto named_ref = assembler.make_named_ref(
                            vm::assembler_named_ref_type_t::local,
                            fld->declaration()->identifier()->label_name());

                        if (!_variables.activate(return_block, named_ref))
                            return false;

                        emit_result_t lhs{};
                        lhs.operands.push_back(vm::instruction_operand_t(named_ref));
                        if (!_variables.assign(return_block, lhs, expr_result))
                            return false;

                        release_temps(expr_result.temps);
                    }
                }

                return_block->move(
                    vm::instruction_operand_t::sp(),
                    vm::instruction_operand_t::fp());
                return_block->pop(vm::instruction_operand_t::fp());
                return_block->rts();

                _return_emitted = true;
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
                *basic_block = entry_block;

                entry_block->label(labels.make(begin_label_name, entry_block));
                if (!emit_element(basic_block, switch_e->scope(), result))
                    return false;
                entry_block = *basic_block;

                auto exit_block = _blocks.make();
                assembler.blocks().emplace_back(exit_block);
                exit_block->predecessors().emplace_back(entry_block);

                exit_block->label(labels.make(exit_label_name, exit_block));
                exit_block->nop();

                *basic_block = exit_block;

                break;
            }
            case element_type_t::intrinsic: {
                auto intrinsic = dynamic_cast<compiler::intrinsic*>(e);
                const auto& args = intrinsic->arguments()->elements();
                switch (intrinsic->type()) {
                    case intrinsic_type_t::free: {
                        auto arg = args[0];

                        emit_result_t arg_result {};
                        if (!emit_element(basic_block, arg, arg_result))
                            return false;

                        current_block->free(arg_result.operands.back());

                        release_temps(arg_result.temps);
                        break;
                    }
                    case intrinsic_type_t::copy: {
                        auto dest_arg = args[0];
                        auto src_arg = args[1];
                        auto size_arg = args[2];

                        emit_result_t dest_arg_result {};
                        if (!emit_element(basic_block, dest_arg, dest_arg_result))
                            return false;

                        emit_result_t src_arg_result {};
                        if (!emit_element(basic_block, src_arg, src_arg_result))
                            return false;

                        emit_result_t size_arg_result {};
                        if (!emit_element(basic_block, size_arg, size_arg_result))
                            return false;

                        current_block->copy(
                            vm::op_sizes::byte,
                            dest_arg_result.operands.back(),
                            src_arg_result.operands.back(),
                            size_arg_result.operands.back());

                        release_temps(dest_arg_result.temps);
                        release_temps(src_arg_result.temps);
                        release_temps(size_arg_result.temps);
                        break;
                    }
                    case intrinsic_type_t::fill: {
                        auto dest_arg = args[0];
                        auto value_arg = args[1];
                        auto length_arg = args[2];

                        emit_result_t dest_arg_result {};
                        if (!emit_element(basic_block, dest_arg, dest_arg_result))
                            return false;

                        emit_result_t value_arg_result {};
                        if (!emit_element(basic_block, value_arg, value_arg_result))
                            return false;

                        emit_result_t length_arg_result {};
                        if (!emit_element(basic_block, length_arg, length_arg_result))
                            return false;

                        current_block->fill(
                            vm::op_sizes::byte,
                            dest_arg_result.operands.back(),
                            value_arg_result.operands.back(),
                            length_arg_result.operands.back());

                        release_temps(dest_arg_result.temps);
                        release_temps(value_arg_result.temps);
                        release_temps(length_arg_result.temps);
                        break;
                    }
                    case intrinsic_type_t::alloc: {
                        auto arg = args[0];

                        emit_result_t arg_result {};
                        if (!emit_element(basic_block, arg, arg_result))
                            return false;

                        auto result_operand = target_operand(result);
                        current_block->alloc(
                            vm::op_sizes::byte,
                            *result_operand,
                            arg_result.operands.back());

                        release_temps(arg_result.temps);
                        break;
                    }
                    case intrinsic_type_t::range: {
                        break;
                    }
                    case intrinsic_type_t::size_of: {
                        break;
                    }
                    case intrinsic_type_t::type_of: {
                        break;
                    }
                    case intrinsic_type_t::align_of: {
                        break;
                    }
                    case intrinsic_type_t::length_of: {
                        break;
                    }
                    case intrinsic_type_t::address_of: {
                        auto arg = args[0];

                        emit_result_t arg_result {};
                        if (!emit_element(basic_block, arg, arg_result))
                            return false;

                        auto result_operand = target_operand(result);
                        if (!_variables.address_of(current_block, arg_result, *result_operand))
                            return false;
                        break;
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case element_type_t::directive: {
                auto directive = dynamic_cast<compiler::directive*>(e);
                switch (directive->type()) {
                    case directive_type_t::run: {
                        auto run_directive = dynamic_cast<compiler::run_directive*>(directive);
                        current_block->comment(
                            "directive: run begin",
                            vm::comment_location_t::after_instruction);
                        current_block->meta_begin();

                        emit_result_t run_result {};
                        if (!emit_element(basic_block, run_directive->expression(), run_result))
                            return false;
                        release_temps(run_result.temps);

                        current_block = *basic_block;
                        current_block->comment(
                            "directive: run end",
                            vm::comment_location_t::after_instruction);
                        current_block->meta_end();
                        break;
                    }
                    case directive_type_t::if_e: {
                        auto if_directive = dynamic_cast<compiler::if_directive*>(directive);
                        auto true_expr = if_directive->true_body();
                        if (true_expr != nullptr) {
                            current_block->comment(
                                "directive: if/elif/else",
                                vm::comment_location_t::after_instruction);
                            emit_result_t if_result {};
                            if (!emit_element(basic_block, true_expr, if_result))
                                return false;
                            release_temps(if_result.temps);
                        }
                        break;
                    }
                    case directive_type_t::eval: {
                        break;
                    }
                    case directive_type_t::type: {
                        break;
                    }
                    case directive_type_t::assert: {
                        break;
                    }
                    case directive_type_t::foreign: {
                        break;
                    }
                    case directive_type_t::assembly: {
                        auto assembly_directive = dynamic_cast<compiler::assembly_directive*>(directive);
                        auto expr = assembly_directive->expression();
                        auto raw_block = dynamic_cast<compiler::raw_block*>(expr);

                        common::source_file source_file;
                        if (!source_file.load(_session.result(), std::string(raw_block->value()) + "\n"))
                            return false;

                        auto success = assembler.assemble_from_source(
                            _session.result(),
                            _session.labels(),
                            _session.options().term_builder,
                            source_file,
                            current_block,
                            expr->parent_scope());
                        if (!success)
                            return false;
                        break;
                    }
                    case directive_type_t::core_type: {
                        break;
                    }
                    case directive_type_t::intrinsic_e: {
                        break;
                    }
                    default: {
                        break;
                    }
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
                    release_temps(expr_result.temps);
                }
                break;
            }
            case element_type_t::proc_call: {
                auto proc_call = dynamic_cast<compiler::procedure_call*>(e);
                auto procedure_type = proc_call->procedure_type();
                auto label = proc_call->identifier()->label_name();
                auto is_foreign = procedure_type->is_foreign();

                struct return_parameter_result_t {
                    compiler::field* field = nullptr;
                    vm::instruction_operand_t* operand = nullptr;
                };

                variable_set_t excluded_vars {};
                std::vector<return_parameter_result_t> return_results {};

                const auto& return_parameters = procedure_type->return_parameters();
                const auto return_size = common::align(
                    return_parameters.size_in_bytes(),
                    8);
                if (!return_parameters.empty()) {
                    auto start_result_operands_size = result.operands.size();

                    return_results.resize(return_parameters.size());
                    result.operands.resize(return_parameters.size());

                    size_t index = 0;
                    const auto& fields = return_parameters.as_list();
                    for (auto fld : fields) {
                        auto field_type = fld->identifier()->type_ref()->type();

                        return_parameter_result_t return_result {};
                        return_result.field = fld;

                        if (index < start_result_operands_size) {
                            return_result.operand = &result.operands[index];
                            auto named_ref = return_result.operand->data<vm::named_ref_with_offset_t>();
                            if (named_ref != nullptr)
                                excluded_vars.insert(_variables.find(named_ref->ref->name));
                        } else {
                            vm::op_sizes size = vm::op_sizes::qword;
                            if (!field_type->is_composite_type())
                                size = vm::op_size_for_byte_size(field_type->size_in_bytes());

                            auto temp = _variables.retain_temp(field_type->number_class());
                            result.temps.emplace_back(temp);
                            excluded_vars.insert(temp->variable);

                            result.operands[index] = vm::instruction_operand_t(assembler.make_named_ref(
                                vm::assembler_named_ref_type_t::local,
                                temp->name(),
                                size));

                            return_result.operand = &result.operands[index];
                        }

                        return_results[index] = return_result;

                        index++;
                    }
                }

                auto grouped_variables = _variables.group_variables(excluded_vars);

                auto prologue_block = _blocks.make();
                assembler.blocks().emplace_back(prologue_block);
                prologue_block->predecessors().emplace_back(current_block);
                current_block->successors().emplace_back(prologue_block);
                *basic_block = prologue_block;

                prologue_block->label(
                    labels.make(fmt::format("{}_prologue",proc_call->label_name()),
                    prologue_block));
                if (!is_foreign)
                    _variables.save_locals_to_stack(prologue_block, grouped_variables);

                auto arg_list = proc_call->arguments();
                if (arg_list != nullptr) {
                    emit_result_t arg_list_result {};
                    if (!emit_element(basic_block, arg_list, arg_list_result))
                        return false;
                    prologue_block = *basic_block;
                    release_temps(arg_list_result.temps);
                }

                if (!is_foreign && !return_parameters.empty()) {
                    prologue_block->comment(
                        "return slot",
                        vm::comment_location_t::after_instruction);
                    prologue_block->sub(
                        vm::instruction_operand_t::sp(),
                        vm::instruction_operand_t::sp(),
                        vm::instruction_operand_t(
                            static_cast<uint64_t>(return_size),
                            vm::op_sizes::byte));
                }

                auto call_block = _blocks.make();
                assembler.blocks().emplace_back(call_block);
                call_block->predecessors().emplace_back(prologue_block);
                prologue_block->successors().emplace_back(call_block);
                *basic_block = call_block;

                call_block->label(
                    labels.make(fmt::format("{}_invoke",proc_call->label_name()),
                    call_block));

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

                    call_block->comment(
                        fmt::format("call: {}", label),
                        vm::comment_location_t::after_instruction);

                    vm::instruction_operand_t address_operand(procedure_type->foreign_address());

                    if (func->is_variadic()) {
                        vm::function_value_list_t args {};
                        if (!arg_list->as_ffi_arguments(_session, args))
                            return false;

                        auto signature_id = common::id_pool::instance()->allocate();
                        func->call_site_arguments.insert(std::make_pair(signature_id, args));

                        call_block->call_foreign(
                            address_operand,
                            vm::instruction_operand_t(
                                static_cast<uint64_t>(signature_id),
                                vm::op_sizes::dword));
                    } else {
                        call_block->call_foreign(address_operand);
                    }
                } else {
                    call_block->comment(
                        fmt::format("call: {}", label),
                        vm::comment_location_t::after_instruction);
                    call_block->call(vm::instruction_operand_t(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        label)));
                    labels.add_cfg_edge(call_block, label);
                }

                auto epilogue_block = _blocks.make();
                assembler.blocks().emplace_back(epilogue_block);
                epilogue_block->predecessors().emplace_back(call_block);
                call_block->successors().emplace_back(epilogue_block);
                *basic_block = epilogue_block;

                epilogue_block->label(
                    labels.make(fmt::format("{}_epilogue",proc_call->label_name()),
                    epilogue_block));

                if (!return_parameters.empty()) {
                    uint64_t offset = 0;

                    for (const auto& return_result : return_results) {
                        auto field_type = return_result.field->identifier()->type_ref()->type();
                        if (field_type->is_composite_type()) {
                            epilogue_block->move(
                                *return_result.operand,
                                vm::instruction_operand_t::sp(),
                                vm::instruction_operand_t(offset, vm::op_sizes::word));
                        } else {
                            epilogue_block->load(
                                *return_result.operand,
                                vm::instruction_operand_t::sp(),
                                vm::instruction_operand_t(offset, vm::op_sizes::word));
                        }
                        offset += field_type->size_in_bytes();
                    }
                }

                if (arg_list->allocated_size() > 0) {
                    epilogue_block->comment(
                        "free stack space",
                        vm::comment_location_t::after_instruction);
                    epilogue_block->add(
                        vm::instruction_operand_t::sp(),
                        vm::instruction_operand_t::sp(),
                        vm::instruction_operand_t(
                            arg_list->allocated_size() + return_size,
                            vm::op_sizes::word));
                }

                if (!is_foreign)
                    _variables.restore_locals_from_stack(epilogue_block, grouped_variables);

                auto next_block = _blocks.make();
                assembler.blocks().emplace_back(next_block);
                next_block->predecessors().emplace_back(epilogue_block);
                epilogue_block->successors().emplace_back(next_block);

                *basic_block = next_block;
                break;
            }
            case element_type_t::transmute: {
                auto transmute = dynamic_cast<compiler::transmute*>(e);
                auto expr = transmute->expression();
                auto type_ref = transmute->type();

                emit_result_t expr_result {};
                if (!emit_element(basic_block, expr, expr_result))
                    return false;

                const auto& expr_inferred = expr_result.type_result.types.back();

                if (expr_inferred.type->number_class() == number_class_t::none) {
                    _session.error(
                        expr->module(),
                        "C073",
                        fmt::format(
                            "cannot transmute from type: {}",
                            expr_inferred.type_name()),
                        expr->location());
                    return false;
                } else if (type_ref->type()->number_class() == number_class_t::none) {
                    _session.error(
                        transmute->module(),
                        "C073",
                        fmt::format(
                            "cannot transmute to type: {}",
                            type_ref->symbol_override().name),
                        transmute->type()->location());
                    return false;
                }

                auto target_number_class = type_ref->type()->number_class();
                auto target_size = type_ref->type()->size_in_bytes();

                current_block->comment(
                    fmt::format("transmute: ", type_ref->symbol_override().name),
                    vm::comment_location_t::after_instruction);

                auto result_operand = target_operand(
                    result,
                    target_number_class,
                    vm::op_size_for_byte_size(target_size));

                current_block->move(
                    *result_operand,
                    expr_result.operands.back(),
                    vm::instruction_operand_t::empty());

                release_temps(expr_result.temps);
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

                auto exit_block = _blocks.make();
                assembler.blocks().emplace_back(exit_block);
                exit_block->predecessors().emplace_back(current_block);

                exit_block->comment(
                    fmt::format("continue: {}", label_name),
                    vm::comment_location_t::after_instruction);
                exit_block->jump_direct(vm::instruction_operand_t(label_ref));
                labels.add_cfg_edge(exit_block, label_name);

                *basic_block = exit_block;
                break;
            }
            case element_type_t::identifier: {
                auto var = dynamic_cast<compiler::identifier*>(e);
                auto named_ref = assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    var->label_name(),
                    op_size);
                result.operands.emplace_back(named_ref);
                if (!_variables.activate(current_block, named_ref, result.is_assign_target)) {
                    _session.error(
                        var->module(),
                        "X000",
                        fmt::format("variable map missing identifier: {}", var->symbol()->name()),
                        var->location());
                    return false;
                }
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
                    release_temps(expr_result.temps);
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
                    release_temps(assignment_result.temps);
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
                    release_temps(expr_result.temps);
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
                auto is_float = numeric_type::narrow_to_value(value) == "f32"sv;
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
                auto size = inferred->type->size_in_bytes();
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
                if (!emit_arguments(basic_block, arg_list, arg_list->elements()))
                    return false;
                break;
            }
            case element_type_t::assembly_label: {
                auto label = dynamic_cast<compiler::assembly_label*>(e);
                auto name = label->reference()->identifier()->label_name();
                if (assembler.has_local(name)) {
                    result.operands.emplace_back(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::local,
                        name));
                } else {
                    result.operands.emplace_back(assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        name));
                }
                break;
            }
            case element_type_t::unary_operator: {
                auto unary_op = dynamic_cast<compiler::unary_operator*>(e);
                auto op_type = unary_op->operator_type();

                emit_result_t rhs_emit_result {};
                if (!emit_element(basic_block, unary_op->rhs(), rhs_emit_result))
                    return false;

                auto result_operand = target_operand(
                    result,
                    inferred->type->number_class(),
                    op_size);

                switch (op_type) {
                    case operator_type_t::negate: {
                        current_block->comment("unary_op: negate", vm::comment_location_t::after_instruction);
                        current_block->neg(
                            *result_operand,
                            rhs_emit_result.operands.back());
                        break;
                    }
                    case operator_type_t::binary_not: {
                        current_block->comment("unary_op: binary not", vm::comment_location_t::after_instruction);
                        current_block->not_op(
                            *result_operand,
                            rhs_emit_result.operands.back());
                        break;
                    }
                    case operator_type_t::logical_not: {
                        current_block->comment("unary_op: logical not", vm::comment_location_t::after_instruction);
                        current_block->cmp(
                            result_operand->size(),
                            rhs_emit_result.operands.back(),
                            vm::instruction_operand_t(static_cast<uint64_t>(1), vm::op_sizes::byte));
                        current_block->setnz(*result_operand);
                        break;
                    }
                    case operator_type_t::pointer_dereference: {
                        if (!_variables.deref(current_block, rhs_emit_result, result))
                            return false;
                        break;
                    }
                    default:
                        break;
                }

                release_temps(rhs_emit_result.temps);
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
                        if (!emit_arithmetic_operator(basic_block, binary_op, result))
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
                        if (is_logical_conjunction_operator(binary_op->operator_type())) {
                            if (!emit_short_circuited_relational_operator(
                                    basic_block,
                                    binary_op,
                                    result)) {
                                return false;
                            }
                        } else {
                            if (!emit_simple_relational_operator(
                                    basic_block,
                                    binary_op,
                                    result)) {
                                return false;
                            }
                        }
                        break;
                    }
                    case operator_type_t::subscript: {
                        emit_result_t lhs_result {};
                        if (!emit_element(basic_block, binary_op->lhs(), lhs_result))
                            return false;

                        const auto& lhs_inferred = lhs_result.type_result.types.back();

                        size_t size_in_bytes = 0;
                        auto ptr_type = dynamic_cast<compiler::pointer_type*>(lhs_inferred.type);
                        if (ptr_type != nullptr) {
                            size_in_bytes = ptr_type->base_type_ref()->type()->size_in_bytes();
                        }

                        // XXX: the idea here is that we look up the "place holder" integer
                        //      literals and change the value to match the correct subscript size.
                        //const auto& ids = binary_op->rhs()->ids();
                        //for (auto id : ids) {
                        //}

                        emit_result_t rhs_result {};
                        if (!emit_element(basic_block, binary_op->rhs(), rhs_result))
                            return false;

                        current_block = *basic_block;

                        auto temp = _variables.retain_temp();
                        vm::instruction_operand_t target_operand(assembler.make_named_ref(
                            vm::assembler_named_ref_type_t::local,
                            temp->name(),
                            vm::op_sizes::qword));
                        result.temps.emplace_back(temp);
                        result.operands.emplace_back(target_operand);

                        current_block->madd(
                            target_operand,
                            rhs_result.operands.back(),
                            vm::instruction_operand_t(
                                static_cast<uint64_t>(size_in_bytes),
                                vm::op_sizes::byte),
                            lhs_result.operands.back());

                        if (!result.is_assign_target) {
                            auto value_temp = _variables.retain_temp();
                            vm::instruction_operand_t value_operand(assembler.make_named_ref(
                                vm::assembler_named_ref_type_t::local,
                                value_temp->name(),
                                vm::op_size_for_byte_size(size_in_bytes)));
                            result.temps.emplace_back(value_temp);
                            result.operands.emplace_back(value_operand);
                            current_block->load(value_operand, target_operand);
                        }

                        release_temps(lhs_result.temps);
                        release_temps(rhs_result.temps);
                        break;
                    }
                    case operator_type_t::member_access: {
                        if (!emit_element(basic_block, binary_op->rhs(), result))
                            return false;
                        break;
                    }
                    case operator_type_t::assignment: {
                        auto is_array_subscript = false;
                        auto lhs_bin_op = dynamic_cast<compiler::binary_operator*>(binary_op->lhs());
                        if (lhs_bin_op != nullptr) {
                            is_array_subscript = lhs_bin_op->operator_type() == operator_type_t::subscript;
                        }

                        emit_result_t lhs_result {};
                        lhs_result.is_assign_target = true;
                        if (!emit_element(basic_block, binary_op->lhs(), lhs_result))
                            return false;

                        emit_result_t rhs_result {};
                        if (!lhs_result.operands.empty() && !is_array_subscript) {
                            rhs_result.operands.emplace_back(lhs_result.operands.front());
                        }

                        if (!emit_element(basic_block, binary_op->rhs(), rhs_result))
                            return false;

                        current_block = *basic_block;

                        const auto& lhs_inferred = lhs_result.type_result.types.back();
                        const auto& rhs_inferred = rhs_result.type_result.types.back();

                        auto copy_required = false;
                        auto lhs_is_composite = lhs_inferred.type->is_composite_type();
                        auto rhs_is_composite = rhs_inferred.type->is_composite_type();

                        if (!lhs_inferred.type->is_pointer_type()) {
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

                        if (!_variables.assign(
                                current_block,
                                lhs_result,
                                rhs_result,
                                copy_required,
                                is_array_subscript)) {
                            return false;
                        }

                        release_temps(lhs_result.temps);
                        release_temps(rhs_result.temps);
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
            case element_type_t::language_type:
            case element_type_t::namespace_type:
            case element_type_t::composite_type:
            case element_type_t::type_reference:
            case element_type_t::spread_operator:
            case element_type_t::label_reference:
            case element_type_t::module_reference:
            case element_type_t::unknown_identifier:
            case element_type_t::value_sink_literal:
            case element_type_t::uninitialized_literal: {
                break;
            }
            case element_type_t::identifier_reference: {
                auto var_ref = dynamic_cast<compiler::identifier_reference*>(e);
                auto offset = var_ref->field_offset();
                auto label = offset.label_name();
                if (!label.empty()) {
                    auto named_ref = assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::local,
                        label,
                        op_size);
                    result.operands.emplace_back(named_ref);

                    if (!_variables.activate(current_block, named_ref, result.is_assign_target))
                        return false;
                } else {
                    auto identifier = var_ref->identifier();
                    if (identifier != nullptr) {
                        if (!emit_element(basic_block, identifier, result))
                            return false;
                    }
                }
                break;
            }
            case element_type_t::assembly_literal_label: {
                /// XXX: fix!  make_named_ref should accept a string_view
                auto label = dynamic_cast<compiler::assembly_literal_label*>(e);
                result.operands.emplace_back(vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    std::string(label->name()))));
                break;
            }
        }
        return true;
    }

    bool byte_code_emitter::emit_type_info(
            vm::basic_block* block,
            compiler::type* type) {
        auto& assembler = _session.assembler();

        auto type_name = type->symbol()->name();
        auto type_name_len = static_cast<uint32_t>(type_name.length());
        auto label_name = type::make_info_label_name(type);

        block->comment(fmt::format("type: {}", type_name), 0);
        block->label(_session.labels().make(label_name, block));

        block->dwords({type_name_len});
        block->dwords({type_name_len});
        block->qwords({assembler.make_named_ref(
            vm::assembler_named_ref_type_t::label,
            type::make_literal_data_label_name(type))});

        return true;
    }

    bool byte_code_emitter::emit_arguments(
            vm::basic_block** basic_block,
            compiler::argument_list* arg_list,
            const compiler::element_list_t& elements) {
        for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
            compiler::type* type = nullptr;

            element* arg = *it;
            switch (arg->element_type()) {
                case element_type_t::argument_list: {
                    auto list = dynamic_cast<compiler::argument_list*>(arg);
                    if (!emit_arguments(basic_block, list, list->elements()))
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
                    emit_result_t arg_result {};
                    if (!emit_element(basic_block, arg, arg_result))
                        return false;

                    auto current_block = *basic_block;
                    const auto& arg_inferred = arg_result.type_result.types.back();

                    if (!arg_list->is_foreign_call()) {
                        type = arg_inferred.type;
                        switch (type->element_type()) {
                            case element_type_t::array_type:
                            case element_type_t::tuple_type:
                            case element_type_t::composite_type: {
                                auto size = static_cast<uint64_t>(common::align(
                                    type->size_in_bytes(),
                                    8));
                                current_block->sub(
                                    vm::instruction_operand_t::sp(),
                                    vm::instruction_operand_t::sp(),
                                    vm::instruction_operand_t(size, vm::op_sizes::word));
                                current_block->copy(
                                    vm::op_sizes::byte,
                                    vm::instruction_operand_t::sp(),
                                    arg_result.operands.back(),
                                    vm::instruction_operand_t(size, vm::op_sizes::word));
                                break;
                            }
                            default: {
                                current_block->push(arg_result.operands.back());
                                break;
                            }
                        }
                    } else {
                        current_block->push(arg_result.operands.back());
                    }

                    release_temps(arg_result.temps);
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

    bool byte_code_emitter::emit_type_table() {
        auto& labels = _session.labels();
        auto& assembler = _session.assembler();

        auto type_info_block = _blocks.make();
        assembler.blocks().emplace_back(type_info_block);

        type_info_block->pre_blank_lines(1);
        type_info_block->section(vm::section_t::ro_data);
        type_info_block->align(16);

        auto used_types = _session.used_types();
        for (auto type : used_types) {
            type_info_block->blank_line();
            type_info_block->align(4);
            type_info_block->string(
                labels.make(compiler::type::make_literal_label_name(type), type_info_block),
                labels.make(compiler::type::make_literal_data_label_name(type), type_info_block),
                type->symbol()->name());
        }

        type_info_block->align(8);
        type_info_block->label(labels.make("_ti_array", type_info_block));
        type_info_block->qwords({static_cast<uint64_t>(used_types.size())});
        for (auto type : used_types) {
            if (type == nullptr)
                continue;

            if (type->element_type() == element_type_t::generic_type
            ||  type->element_type() == element_type_t::unknown_type) {
                continue;
            }

            type_info_block->blank_line();
            emit_type_info(type_info_block, type);
        }

        return true;
    }

    bool byte_code_emitter::emit_section_tables() {
        auto& vars = _variables.module_variables();

        auto& assembler = _session.assembler();
        auto block = _blocks.make();
        block->pre_blank_lines(1);

        for (const auto& section : vars.sections) {
            if (section.second.empty())
                continue;

            block->blank_line();
            block->section(section.first);

            if (!emit_section_type_literals(block, section.second))
                return false;

            for (auto e : section.second) {
                if (!emit_section_variable(block, e))
                    return false;
            }
        }

        if (!block->entries().empty())
            assembler.blocks().emplace_back(block);

        return true;
    }

    bool byte_code_emitter::emit_procedure_types() {
        proc_call_edge_list_t edges {};

        auto proc_calls = _session
            .elements()
            .find_by_type<compiler::procedure_call>(element_type_t::proc_call);
        for (auto proc_call : proc_calls) {
            if (proc_call->is_foreign())
                continue;

            auto call_site = find_call_site(proc_call);
            if (call_site->id() == proc_call->procedure_type()->id())
                continue;

            edges.push_back(proc_call_edge_t {call_site, proc_call});
        }

        procedure_call_set_t proc_call_set {};
        procedure_type_set_t proc_type_set {};

        auto root_edges = find_root_edges(edges);
        for (const auto& edge : root_edges) {
            walk_call_graph_edges(edges, proc_call_set, edge.site);
        }

        for (auto proc_call : proc_call_set) {
            auto proc_type = proc_call->procedure_type();
            if (proc_type != nullptr) {
                proc_type_set.insert(proc_type);
            } else {
                _session.error(
                    proc_call->module(),
                    "X000",
                    "no valid procedure instance for call site.",
                    proc_call->location());
            }
        }

        if (_session.result().is_failed())
            return false;

        auto& assembler = _session.assembler();
        for (auto instance : proc_type_set) {
            auto basic_block = _blocks.make();
            assembler.blocks().emplace_back(basic_block);
            basic_block->pre_blank_lines(1);
            if (!emit_procedure_instance(&basic_block, instance))
                return false;
        }

        auto end_block = _blocks.make();
        assembler.blocks().emplace_back(end_block);
        end_block->pre_blank_lines(1);
        end_block->program_end();

        return true;
    }

    bool byte_code_emitter::emit_section_type_literals(
            vm::basic_block* basic_block,
            const element_list_t& elements) {
        auto& labels = _session.labels();

        identifier_list_t arrays {};
        for (auto e : elements) {
            auto var = dynamic_cast<compiler::identifier*>(e);
            if (var->type_ref()->is_array_type())
                arrays.emplace_back(var);
        }

        for (auto var : arrays) {
            basic_block->blank_line();
            basic_block->label(labels.make(
                fmt::format("{}_data", var->label_name()),
                basic_block));

            auto init = var->initializer();
            if (init == nullptr) {
                auto array_type = dynamic_cast<compiler::array_type*>(var->type_ref()->type());
                basic_block->reserve_byte(array_type->data_size());
                continue;
            }

            auto literal = dynamic_cast<compiler::type_literal*>(init->expression());
            auto type_param = literal->type_params().front()->type();
            if (type_param->is_composite_type()) {
                // XXX: recursively emit arrays
            } else {
                const auto symbol_type = vm::integer_symbol_type_for_size(
                    type_param->size_in_bytes());
                vm::data_value_variant_list_t values{};

                for (auto arg : literal->args()->elements()) {
                    infer_type_result_t type_result{};
                    if (!arg->infer_type(_session, type_result))
                        return false;

                    uint64_t value;
                    if (arg->as_integer(value)) {
                        switch (symbol_type) {
                            case vm::symbol_type_t::unknown:
                                break;
                            case vm::symbol_type_t::u8:
                            case vm::symbol_type_t::bytes: {
                                values.emplace_back(static_cast<uint8_t>(value));
                                break;
                            }
                            case vm::symbol_type_t::u16: {
                                values.emplace_back(static_cast<uint16_t>(value));
                                break;
                            }
                            case vm::symbol_type_t::f32:
                            case vm::symbol_type_t::u32: {
                                values.emplace_back(static_cast<uint32_t>(value));
                                break;
                            }
                            case vm::symbol_type_t::f64:
                            case vm::symbol_type_t::u64: {
                                values.emplace_back(value);
                                break;
                            }
                        }
                    }
                }

                basic_block->values(values);
            }
        }

        return true;
    }

    bool byte_code_emitter::emit_section_variable_data(
            vm::basic_block* basic_block,
            compiler::identifier* var) {
        auto var_type = var->type_ref()->type();
        auto is_initialized = var->is_initialized();
        auto composite_type = dynamic_cast<compiler::composite_type*>(var_type);
        if (composite_type == nullptr) {
            basic_block->comment(
                var->label_name(),
                vm::comment_location_t::after_instruction);
        }

        switch (var_type->element_type()) {
            case element_type_t::bool_type: {
                bool value = false;
                var->as_bool(value);

                if (!is_initialized)
                    basic_block->reserve_byte(1);
                else
                    basic_block->bytes({static_cast<uint8_t>(value ? 1 : 0)});
                break;
            }
            case element_type_t::rune_type: {
                common::rune_t value = common::rune_invalid;
                var->as_rune(value);

                if (!is_initialized)
                    basic_block->reserve_byte(4);
                else
                    basic_block->dwords({static_cast<uint32_t>(value)});
                break;
            }
            case element_type_t::pointer_type: {
                if (!is_initialized) {
                    basic_block->reserve_qword(1);
                } else {
                    auto init = var->initializer();
                    if (init != nullptr && init->is_constant()) {
                        emit_result_t result {};
                        if (!emit_element(&basic_block, init, result))
                            return false;
                        const auto& operand = result.operands.back();
                        switch (operand.type()) {
                            case vm::instruction_operand_type_t::named_ref: {
                                auto named_ref = operand.data<vm::named_ref_with_offset_t>();
                                if (named_ref != nullptr)
                                    basic_block->qwords({named_ref->ref});
                                else
                                    basic_block->qwords({static_cast<uint64_t>(0)});
                                break;
                            }
                            default: {
                                basic_block->qwords({static_cast<uint64_t>(0)});
                                break;
                            }
                        }
                    } else {
                        basic_block->qwords({static_cast<uint64_t>(0)});
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
                            basic_block->reserve_byte(1);
                        else
                            basic_block->bytes({static_cast<uint8_t>(value)});
                        break;
                    case vm::symbol_type_t::u16:
                        if (!is_initialized)
                            basic_block->reserve_word(1);
                        else
                            basic_block->words({static_cast<uint16_t>(value)});
                        break;
                    case vm::symbol_type_t::f32:
                    case vm::symbol_type_t::u32:
                        if (!is_initialized)
                            basic_block->reserve_dword(1);
                        else
                            basic_block->dwords({static_cast<uint32_t>(value)});
                        break;
                    case vm::symbol_type_t::f64:
                    case vm::symbol_type_t::u64:
                        if (!is_initialized)
                            basic_block->reserve_qword(1);
                        else
                            basic_block->qwords({value});
                        break;
                    case vm::symbol_type_t::bytes:
                        break;
                    default:
                        break;
                }
                break;
            }
            case element_type_t::array_type: {
                auto& assembler = _session.assembler();

                auto array_type = dynamic_cast<compiler::array_type*>(var_type);
                auto number_of_elements = array_type->number_of_elements();

                basic_block->dwords({static_cast<uint32_t>(number_of_elements)});
                basic_block->reserve_byte(4);
                basic_block->qwords({assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    fmt::format("{}_data", var->label_name()))});

                break;
            }
            case element_type_t::tuple_type:
            case element_type_t::composite_type: {
                if (!is_initialized) {
                    basic_block->reserve_byte(var_type->size_in_bytes());
                } else {
                    if (!emit_section_variable_composite_type(basic_block, composite_type))
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

    bool byte_code_emitter::emit_section_variable(
            vm::basic_block* basic_block,
            compiler::element* e) {
        auto& labels = _session.labels();

        auto var = dynamic_cast<compiler::identifier*>(e);
        if (var != nullptr) {
            auto var_type = var->type_ref()->type();
            auto composite_type = dynamic_cast<compiler::composite_type*>(var_type);
            if (composite_type != nullptr)
                composite_type->calculate_size();

            basic_block->blank_line();

            auto type_alignment = static_cast<uint8_t>(var_type->alignment());
            if (type_alignment > 1)
                basic_block->align(type_alignment);

            basic_block->comment(fmt::format(
                "identifier type: {}",
                var->type_ref()->symbol()->name()));
            auto var_label = labels.make(var->label_name(), basic_block);
            basic_block->label(var_label);

            if (!emit_section_variable_data(basic_block, var))
                return false;
        }

        return true;
    }

    bool byte_code_emitter::emit_procedure_epilogue(
            vm::basic_block** basic_block,
            compiler::procedure_type* proc_type) {
        if (proc_type->is_foreign())
            return true;

        if (!proc_type->has_return()) {
            auto& assembler = _session.assembler();

            auto return_block = _blocks.make();
            assembler.blocks().emplace_back(return_block);
            return_block->predecessors().emplace_back(*basic_block);

            return_block->rts();

            *basic_block = return_block;
        }

        return true;
    }

    bool byte_code_emitter::emit_procedure_instance(
            vm::basic_block** basic_block,
            compiler::procedure_type* proc_type) {
        if (proc_type->is_foreign())
            return true;

        auto scope_block = proc_type->body_scope();

        _variables.build(scope_block, proc_type);

        if (!emit_procedure_prologue(basic_block, proc_type))
            return false;

        if (!emit_block(basic_block, scope_block))
            return false;

        return emit_procedure_epilogue(basic_block, proc_type);
    }

    bool byte_code_emitter::emit_procedure_prologue(
            vm::basic_block** basic_block,
            compiler::procedure_type* proc_type) {
        if (proc_type->is_foreign())
            return true;

        auto current_block = *basic_block;
        auto& labels = _session.labels();

        auto procedure_label = proc_type->label_name();

        current_block->align(vm::instruction_t::alignment);
        current_block->label(labels.make(procedure_label, current_block));
        current_block->reset("local");
        current_block->reset("frame");

        const auto& return_parameters = proc_type->return_parameters();
        if (!return_parameters.empty()) {
            current_block->frame_offset("return", 16);
            current_block->frame_offset(
                "parameter",
                common::align(16 + return_parameters.size_in_bytes(), 8));
        } else {
            current_block->frame_offset("parameter", 16);
        }

        return true;
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

    bool byte_code_emitter::emit_arithmetic_operator(
            vm::basic_block** basic_block,
            compiler::binary_operator* binary_op,
            emit_result_t& result) {
        emit_result_t lhs_result {};
        if (!result.operands.empty())
            lhs_result.target_size = result.operands.front().size();
        if (!emit_element(basic_block, binary_op->lhs(), lhs_result))
            return false;

        emit_result_t rhs_result {};
        if (!result.operands.empty())
            rhs_result.target_size = result.operands.front().size();
        if (!emit_element(basic_block, binary_op->rhs(), rhs_result))
            return false;

        auto current_block = *basic_block;
        const auto& inferred = result.type_result.types.back();

        const auto target_size = result.target_size != vm::op_sizes::none ?
            result.target_size :
            vm::op_size_for_byte_size(inferred.type->size_in_bytes());
        auto result_operand = target_operand(
            result,
            inferred.type->number_class(),
            target_size);

        switch (binary_op->operator_type()) {
            case operator_type_t::add: {
                current_block->add(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::divide: {
                current_block->div(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::modulo: {
                current_block->mod(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::multiply: {
                current_block->mul(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::exponent: {
                current_block->pow(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::subtract: {
                current_block->sub(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::binary_or: {
                current_block->or_op(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::shift_left: {
                current_block->shl(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::binary_and: {
                current_block->and_op(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::binary_xor: {
                current_block->xor_op(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::rotate_left: {
                current_block->rol(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::shift_right: {
                current_block->shr(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            case operator_type_t::rotate_right: {
                current_block->ror(
                    *result_operand,
                    lhs_result.operands.back(),
                    rhs_result.operands.back());
                break;
            }
            default:
                break;
        }

        release_temps(lhs_result.temps);
        release_temps(rhs_result.temps);

        return true;
    }

    bool byte_code_emitter::fill_referenced_identifiers(
            vm::basic_block* basic_block,
            compiler::element* e) {
        auto& assembler = _session.assembler();
        switch (e->element_type()) {
            case element_type_t::binary_operator: {
                auto bin_op = dynamic_cast<compiler::binary_operator*>(e);
                if (bin_op->lhs()->element_type() == element_type_t::identifier_reference) {
                    auto var = dynamic_cast<compiler::identifier_reference*>(bin_op->lhs());
                    auto named_ref = assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::local,
                        var->identifier()->label_name());
                    if (!_variables.fill(basic_block, named_ref))
                        return false;
                }
                if (bin_op->rhs()->element_type() == element_type_t::identifier_reference) {
                    auto var = dynamic_cast<compiler::identifier_reference*>(bin_op->rhs());
                    auto named_ref = assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::local,
                        var->identifier()->label_name());
                    if (!_variables.fill(basic_block, named_ref))
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

    bool byte_code_emitter::emit_interned_string_table() {
        auto& labels = _session.labels();
        auto& assembler = _session.assembler();

        auto block = _blocks.make();
        block->pre_blank_lines(1);
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
                labels.make(fmt::format("_intern_str_lit_{}", kvp.second), block),
                labels.make(fmt::format("_intern_str_lit_{}_data", kvp.second), block),
                escaped);
        }

        assembler.blocks().emplace_back(block);

        return true;
    }

    vm::basic_block* byte_code_emitter::emit_start_block() {
        auto& labels = _session.labels();
        auto& assembler = _session.assembler();

        auto start_block = _blocks.make();

        start_block->section(vm::section_t::text);
        start_block->align(vm::instruction_t::alignment);
        start_block->label(labels.make("_start", start_block));

        start_block->move(
            vm::instruction_operand_t::fp(),
            vm::instruction_operand_t::sp());

        assembler.blocks().emplace_back(start_block);

        return start_block;
    }

    bool byte_code_emitter::emit_simple_relational_operator(
            vm::basic_block** basic_block,
            compiler::binary_operator* binary_op,
            emit_result_t& result) {
        auto& labels = _session.labels();
        auto& assembler = _session.assembler();

        auto result_operand = target_operand(
            result,
            number_class_t::integer,
            vm::op_sizes::byte);

        emit_result_t lhs_result {};
        if (!emit_element(basic_block, binary_op->lhs(), lhs_result))
            return false;

        emit_result_t rhs_result {};
        if (!emit_element(basic_block, binary_op->rhs(), rhs_result))
            return false;

        auto current_block = *basic_block;
        const auto& lhs_inferred = lhs_result.type_result.types.back();
        auto is_signed = lhs_inferred.type->is_signed();

        current_block->cmp(
            lhs_result.operands.back(),
            rhs_result.operands.back());

        switch (binary_op->operator_type()) {
            case operator_type_t::equals: {
                current_block->setz(*result_operand);
                break;
            }
            case operator_type_t::less_than: {
                if (is_signed)
                    current_block->setl(*result_operand);
                else
                    current_block->setb(*result_operand);
                break;
            }
            case operator_type_t::not_equals: {
                current_block->setnz(*result_operand);
                break;
            }
            case operator_type_t::greater_than: {
                if (is_signed)
                    current_block->setg(*result_operand);
                else
                    current_block->seta(*result_operand);
                break;
            }
            case operator_type_t::less_than_or_equal: {
                if (is_signed)
                    current_block->setle(*result_operand);
                else
                    current_block->setbe(*result_operand);
                break;
            }
            case operator_type_t::greater_than_or_equal: {
                if (is_signed)
                    current_block->setge(*result_operand);
                else
                    current_block->setae(*result_operand);
                break;
            }
            default: {
                break;
            }
        }

        release_temps(lhs_result.temps);
        release_temps(rhs_result.temps);

        auto exit_block = _blocks.make();
        assembler.blocks().emplace_back(exit_block);
        exit_block->predecessors().emplace_back(current_block);
        current_block->successors().emplace_back(exit_block);

        auto exit_label_name = fmt::format("{}_exit", binary_op->label_name());
        exit_block->label(labels.make(exit_label_name, current_block));

        *basic_block = exit_block;

        return true;
    }

    bool byte_code_emitter::emit_section_variable_composite_type(
            vm::basic_block* basic_block,
            compiler::composite_type* composite_type) {
        size_t offset = 0;
        const auto& fields = composite_type->fields().as_list();
        for (auto fld : fields) {
            auto field_var = fld->identifier();

            if (offset < fld->start_offset()) {
                basic_block->comment(
                    "padding",
                    vm::comment_location_t::after_instruction);
                basic_block->reserve_byte(fld->start_offset() - offset);
            }

            if (!emit_section_variable_data(basic_block, field_var))
                return false;

            offset = fld->end_offset();
        }

        return true;
    }

    vm::instruction_operand_t* byte_code_emitter::target_operand(
            emit_result_t& result,
            number_class_t number_class,
            vm::op_sizes size) {
        if (result.operands.empty()) {
            auto& assembler = _session.assembler();
            auto temp = _variables.retain_temp(number_class);
            vm::instruction_operand_t target_operand(assembler.make_named_ref(
                vm::assembler_named_ref_type_t::local,
                temp->name(),
                size));
            result.temps.emplace_back(temp);
            result.operands.emplace_back(target_operand);
        }
        return &result.operands.front();
    }

    bool byte_code_emitter::emit_short_circuited_relational_operator(
            vm::basic_block** basic_block,
            compiler::binary_operator* binary_op,
            emit_result_t& result) {
        auto& labels = _session.labels();
        auto& assembler = _session.assembler();

        auto result_operand = target_operand(
            result,
            number_class_t::integer,
            vm::op_sizes::byte);

        auto lhs_result = result;
        if (!emit_element(basic_block, binary_op->lhs(), lhs_result))
            return false;

        auto current_block = *basic_block;
        auto exit_label_name = fmt::format("{}_exit", binary_op->label_name());
        auto exit_label_ref = assembler.make_named_ref(
            vm::assembler_named_ref_type_t::label,
            exit_label_name);

        switch (binary_op->operator_type()) {
            case operator_type_t::logical_or: {
                current_block->bnz(
                    *result_operand,
                    vm::instruction_operand_t(exit_label_ref));
                break;
            }
            case operator_type_t::logical_and: {
                current_block->bz(
                    *result_operand,
                    vm::instruction_operand_t(exit_label_ref));
                break;
            }
            default: {
                break;
            }
        }

        auto rhs_block = _blocks.make();
        assembler.blocks().emplace_back(rhs_block);
        rhs_block->predecessors().emplace_back(current_block);
        current_block->successors().emplace_back(rhs_block);

        auto rhs_label_name = fmt::format("{}_rhs", binary_op->label_name());
        rhs_block->label(labels.make(rhs_label_name, current_block));

        *basic_block = rhs_block;

        auto rhs_result = result;
        if (!emit_element(basic_block, binary_op->rhs(), rhs_result))
            return false;

        current_block = *basic_block;

        release_temps(lhs_result.temps);
        release_temps(rhs_result.temps);

        auto exit_block = _blocks.make();
        assembler.blocks().emplace_back(exit_block);
        exit_block->predecessors().emplace_back(current_block);
        current_block->successors().emplace_back(exit_block);

        exit_block->label(labels.make(exit_label_name, current_block));

        *basic_block = exit_block;

        return true;
    }

    std::string byte_code_emitter::interned_string_data_label(common::id_t id) {
        common::id_t intern_id;
        _session.interned_strings().element_id_to_intern_id(id, intern_id);
        return fmt::format("_intern_str_lit_{}_data", intern_id);
    }

    void byte_code_emitter::release_temps(std::vector<temp_pool_entry_t*> temps) {
        for (auto t : temps)
            _variables.release_temp(t);
    }

    bool byte_code_emitter::emit_end_block(const vm::basic_block_list_t& predecessors) {
        auto& labels = _session.labels();
        auto& assembler = _session.assembler();

        auto end_block = _blocks.make();
        for (auto p : predecessors)
            p->add_successors({end_block});
        end_block->add_predecessors(predecessors);

        end_block->pre_blank_lines(1);
        end_block->align(vm::instruction_t::alignment);
        end_block->label(labels.make("_end", end_block));
        end_block->exit();

        assembler.blocks().emplace_back(end_block);

        return true;
    }

    vm::basic_block* byte_code_emitter::emit_implicit_blocks(const vm::basic_block_list_t& predecessors) {
        auto& labels = _session.labels();
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
            if (!block->has_statements())
                continue;

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

            if (!_variables.build(block))
                return nullptr;

            implicit_block->label(labels.make(block->label_name(), implicit_block));
            implicit_block->reset("local");
            implicit_block->reset("frame");

            if (!emit_block(&implicit_block, block))
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

}
