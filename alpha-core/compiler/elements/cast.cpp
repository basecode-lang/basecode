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
#include "cast.h"
#include "numeric_type.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

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

    cast::cast(
            compiler::module* module,
            block* parent_scope,
            compiler::type_reference* type,
            element* expr) : element(module, parent_scope, element_type_t::cast),
                             _expression(expr),
                             _type_ref(type) {
    }

    bool cast::on_infer_type(
            const compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = _type_ref->type();
        result.reference = _type_ref;
        return true;
    }

    element* cast::expression() {
        return _expression;
    }

    compiler::type_reference* cast::type() {
        return _type_ref;
    }

    //
    // numeric casts
    // ------------------------------------------------------------------------
    // casting between two integers of the same size (s32 -> u32) is a no-op
    // casting from a larger integer to a smaller integer (u32 -> u8) will truncate via move
    // casting from smaller integer to larger integer (u8 -> u32) will:
    //  - zero-extend if the source is unsigned
    //  - sign-extend if the source is signed
    // casting from float to an integer will round the float towards zero
    // casting from an integer to a float will produce the floating point representation of the
    //    integer, rounded if necessary
    // casting from f32 to f64 is lossless
    // casting from f64 to f32 will produce the closest possible value, rounded if necessary
    // casting bool to and integer type will yield 1 or 0
    // casting any integer type whose LSB is set will yield true; otherwise, false
    //
    // pointer casts
    // ------------------------------------------------------------------------
    // integer to pointer type:
    //
    bool cast::on_emit(compiler::session& session) {
        if (_expression == nullptr)
            return true;

        infer_type_result_t infer_type_result {};
        if (!_expression->infer_type(session, infer_type_result)) {
            // XXX: error
            return false;
        }

        cast_mode_t mode;
        auto source_number_class = infer_type_result.inferred_type->number_class();
        auto source_size = infer_type_result.inferred_type->size_in_bytes();
        auto target_number_class = _type_ref->type()->number_class();
        auto target_size = _type_ref->type()->size_in_bytes();

        if (source_number_class == type_number_class_t::none) {
            session.error(
                this,
                "C073",
                fmt::format("cannot cast from type: {}", infer_type_result.type_name()),
                _expression->location());
            return false;
        } else if (target_number_class == type_number_class_t::none) {
            session.error(
                this,
                "C073",
                fmt::format("cannot cast to type: {}", _type_ref->symbol().name),
                _type_location);
            return false;
        }

        auto& assembler = session.assembler();
        auto target_reg = assembler.current_target_register();
        auto instruction_block = assembler.current_block();

        if (source_number_class == type_number_class_t::integer
        &&  target_number_class == type_number_class_t::integer) {
            if (source_size == target_size) {
                mode = cast_mode_t::integer_truncate;
            } else if (source_size > target_size) {
                mode = cast_mode_t::integer_truncate;
            } else {
                auto source_numeric_type = dynamic_cast<compiler::numeric_type*>(infer_type_result.inferred_type);
                if (source_numeric_type->is_signed()) {
                    mode = cast_mode_t::integer_sign_extend;
                } else {
                    mode = cast_mode_t::integer_zero_extend;
                }
            }
        } else if (source_number_class == type_number_class_t::floating_point
               &&  target_number_class == type_number_class_t::floating_point) {
            if (source_size == target_size) {
                mode = cast_mode_t::float_truncate;
            } else if (source_size > target_size) {
                mode = cast_mode_t::float_truncate;
            } else {
                mode = cast_mode_t::float_extend;
            }
        } else {
            if (source_number_class == type_number_class_t::integer) {
                mode = cast_mode_t::integer_to_float;
            } else {
                mode = cast_mode_t::float_to_integer;
            }
        }

        auto temp_reg = register_for(session, _expression);
        if (!temp_reg.valid)
            return false;
        temp_reg.clean_up = true;

        assembler.push_target_register(temp_reg.reg);
        _expression->emit(session);
        assembler.pop_target_register();

        switch (mode) {
            case cast_mode_t::integer_truncate: {
                instruction_block->move_reg_to_reg(*target_reg, temp_reg.reg);
                break;
            }
            case cast_mode_t::integer_sign_extend: {
                instruction_block->moves_reg_to_reg(*target_reg, temp_reg.reg);
                break;
            }
            case cast_mode_t::integer_zero_extend: {
                instruction_block->movez_reg_to_reg(*target_reg, temp_reg.reg);
                break;
            }
            case cast_mode_t::float_extend:
            case cast_mode_t::float_truncate:
            case cast_mode_t::integer_to_float:
            case cast_mode_t::float_to_integer: {
                instruction_block->convert(*target_reg, temp_reg.reg);
                break;
            }
            default: {
                break;
            }
        }

        instruction_block->comment(
            fmt::format(
                "cast<{}> from type {}",
                _type_ref->symbol().name,
                infer_type_result.type_name()),
            4);

        return true;
    }

    void cast::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    void cast::type_location(const common::source_location& loc) {
        _type_location = loc;
    }

};